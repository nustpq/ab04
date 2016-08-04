/*
*********************************************************************************************************
*                               UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        iM501 Communication Related
*
*                                          Atmel AT91SAM3U4C
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : im501_comm.c
* Version       : V1.1.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <ucos_ii.h>

#include "im501_comm.h"
#include "defined.h"
#include "noah_cmd.h"
#include "uif_dsp.h"
#include "uif_twi.h"

/*----------------------------------------------------------------------------*/
//this is a global array,define here and need export
SET_VEC_CFG  Global_VEC_Cfg; 
VOICE_BUF  voice_buf_data;
uint8_t   Reg_RW_Data[ 4200 ];
/*----------------------------------------------------------------------------*/

static uint32_t im501_irq_counter;
static uint32_t im501_irq_gpio;
static uint32_t im501_key_words_detect;
static uint32_t im501_service_int_enabled;

DataSource *pSource = &source_spi0;             //i don't known wich spi is used. so declare a global point here
                                                //if AB04 isn't use this port,i can switch it convenient
extern OS_EVENT  *Load_Vec_Sem_lock ;

/*
*********************************************************************************************************
*                                           im501_change_if_speed()
*
* Description :  change current iM501 interface speed 
*
* Argument(s) :   if_type     is for indicating which interface will be actived :
*                               I2C : if_type = 1
*                               SPI : if_type = 2
*                speed_type   is for indicating which speed level(low/high)will be actived :
*                               low speed  : type = 0
*                               high speed : type = 1
*
* Return(s)   :  none.           
*
* Note(s)     :  none.

*********************************************************************************************************
*/
void im501_change_if_speed( uint8_t if_type, uint8_t speed_type )
{
    uint32_t twi_hz;
    if( speed_type == 0) 
    { //change to low speed
        if( if_type == 1 ) 
        { //I2C interface 
#if OLD_CODE          
            TWI_Init( 20 * 1000 );
#else
            twi_hz = 20 * 1000;
            if( NULL != source_twi0.init_source )
                source_twi0.init_source( &source_twi0,&twi_hz );
#endif            
        } 
        else if( if_type == 2 ) 
        { //SPI 
#if OLD_CODE
            SPI_Init( 1500000,  Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].attribute ); 
#else
            VOICE_BUF_CFG Spi_Cfg;
            Spi_Cfg.slave = 1;
            Spi_Cfg.spi_speed = 1500000;
            Spi_Cfg.spi_mode  = Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].attribute;
            if( NULL != source_spi1.set_peripheral )
                source_spi1.set_peripheral( &source_spi1,&Spi_Cfg );
              
#endif
        } 
        
    } 
    else 
    { //change to normal speed
        if( if_type == 1 ) 
        { //I2C interface 
#if OLD_CODE
            TWI_Init( Global_UIF_Setting[ UIF_TYPE_I2C - 1 ].speed * 1000 ); 
#else
            twi_hz = Global_UIF_Setting[ UIF_TYPE_I2C - 1 ].speed * 1000;
            if( NULL != source_twi0.init_source )
                source_twi0.init_source( &source_twi0,&twi_hz );
#endif
        } 
        else if( if_type == 2 ) 
        { //SPI 
#if OLD_CODE
            SPI_Init(Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].speed * 1000,  Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].attribute );
#else
            VOICE_BUF_CFG Spi_Cfg;
            Spi_Cfg.slave = 1;
            Spi_Cfg.spi_speed = Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].speed * 1000;
            Spi_Cfg.spi_mode  = Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].attribute;
            if( NULL != source_spi1.set_peripheral )
                source_spi1.set_peripheral( &source_spi1,&Spi_Cfg );
#endif
        }         
    }  
    
    
}


/*
*********************************************************************************************************
*                                           im501_switch_i2c_spi()
*
* Description :  change iM501 INTERNAL actived interface type 
*
* Argument(s) :  if_type     is for indicating which interface will be actived :
*                            I2C : if_type = 1
*                            SPI : if_type = 2
*
*                spi_mode    is for indicating SPI format( 0~3 ), used only when if_type=2
*
* Return(s)   :  error number.           
*
* Note(s)     : 

*********************************************************************************************************
*/
uint8_t im501_switch_i2c_spi( uint8_t if_type, uint8_t spi_mode )
{
    uint8_t err;
    uint8_t data;
    
    err = im501_read_reg_i2c( iM501_I2C_SPI_REG, &data );
    if( err != NO_ERR ) {
        return err;
    }
    if(( data & 0x04 ) && (if_type == 2) ) { // I2C mode, need switch to SPI
        err = im501_write_reg_spi(iM501_I2C_SPI_REG, 0x80+(spi_mode&0x03));
        if( err != NO_ERR ) {
            return err;
        }
        err = im501_write_reg_spi(iM501_I2C_SPI_REG, spi_mode&0x03);
        if( err != NO_ERR ) {
            return err;
        }
    } else if( (!(data & 0x04 )) && (if_type == 1) ) { // SPI mode, need switch to I2C
        err = im501_write_reg_i2c(iM501_I2C_SPI_REG, 0x04);
        if( err != NO_ERR ) {
            return err;
        }
    }
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           im501_check_pll_ready()
*
* Description :  Check if iM501 PLL is locked 
*
* Argument(s) :  None.
*
* Return(s)   :  error number.           
*
* Note(s)     :  none.

*********************************************************************************************************
*/
uint8_t im501_check_pll_ready( void )
{
    uint32_t data;
    uint8_t err;
     
    if( Global_VEC_Cfg.if_type == 1 ) { //I2C interface         
        err =  im501_read_dram_i2c( CHECK_PLL_READY_ADDR, (uint8_t*)&data );         
    } else if(Global_VEC_Cfg.if_type == 2 ) { //SPI         
        err =  im501_read_dram_spi( CHECK_PLL_READY_ADDR, (uint8_t*)&data );
    } else {
        err = 1;
    }
     
    if( err != 0 ) {
        return 1;   
    }
    
    if( data & 0x20 ) { //check bit5 == 1 ?        
        return 1; //ready
    }else {
        return 0; //not ready
    }   
    
}


/*
*********************************************************************************************************
*                                           MCU_Load_Vec()
*
* Description :  Load parameters stored in flash to iM401/iM501 and turn on / turn off PDM clock accordingly.
*
* Argument(s) :  firsttime     is used to identify this is a full power up - power down cycle(firsttime=0),
*                              or just a power down cycle for first stage(firsttime=1)                          
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t MCU_Load_Vec( uint8_t firsttime )
{     
    uint8_t err;
    uint8_t i,j;
    uint8_t index, *pChar, buf[6]; 
    uint8_t dev_addr, data_num;
    FLASH_INFO    flash_info;   
    
    err = NO_ERR;    
    
    if( Global_VEC_Cfg.type == 41 ) {
        APP_TRACE_INFO(("\r\nLoad iM401 vec stored in MCU flash...\r\n"));
        dev_addr = iM401_I2C_ADDR;        
    } else if(Global_VEC_Cfg.type == 51 ) {
        APP_TRACE_INFO(("\r\nLoad iM501 vec stored in MCU flash...\r\n"));
        dev_addr = iM501_I2C_ADDR;       
    } else {
        APP_TRACE_INFO(("\r\nvec conf data error!\r\n")); 
        return FW_VEC_SET_CFG_ERR;         
    }
    
    if( Global_VEC_Cfg.flag != 0x55 ) {
        APP_TRACE_INFO(("\r\nvec conf data error!\r\n")); 
        return FW_VEC_SET_CFG_ERR;
    }
    
    OSSemPend( Load_Vec_Sem_lock, 0, &err );
    
    if( firsttime == 0 ) {   //not first time pwd
        
        index = Global_VEC_Cfg.vec_index_a ; //Power up
        if(  Global_VEC_Cfg.pdm_clk_off ) 
        {
            I2C_Mixer( I2C_MIX_FM36_CODEC );
            FM36_PDMADC_CLK_OnOff( 1 , 0 ); //enable PDM clock    
            I2C_Mixer( I2C_MIX_UIF_S );             
        }
        if( index != 0 ) {        
            Read_Flash_State(&flash_info, FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * index  );
            if( flash_info.f_w_state != FW_DOWNLAD_STATE_FINISHED ) {    
              APP_TRACE_INFO(("\r\nvec data state error...\r\n"));
              OSSemPost( Load_Vec_Sem_lock );
              return FW_VEC_SAVE_STATE_ERR;
            }   
            APP_TRACE_INFO(("Load vec[%d] (%d Bytes) ...\r\n",index,flash_info.bin_size));
            pChar = (uint8_t *)FLASH_ADDR_FW_VEC + index * FLASH_ADDR_FW_VEC_SIZE;
            
            if(  Global_VEC_Cfg.if_type == 1 ) { //I2C interface
                for( i = 0; i < flash_info.bin_size ;  ) {
                    data_num = *(pChar+i) ;
                    err =  TWID_Write(  dev_addr>>1, 0, 0, pChar + i + 1 , data_num, NULL); 
                    i += ( data_num + 1 );               
                    if ( err != SUCCESS )  {
                        OSSemPost( Load_Vec_Sem_lock );
                        return (I2C_BUS_ERR) ;
                    }
                }
            }else if( Global_VEC_Cfg.if_type == 2 ) { //SPI interface
                for( i = 0; i < flash_info.bin_size ;  ) {
                    data_num = *(pChar+i) ;
                    if(data_num > sizeof(buf)) {
                        OSSemPost( Load_Vec_Sem_lock );
                        return (SPI_BUS_ERR);
                    }
                    for(j = 0; j < data_num ; j++) {
                        buf[j] = *(pChar + i + 1 + j);
                    }                
                    err = SPI_WriteBuffer_API( &buf, data_num );                  
                    i += ( data_num + 1 );
                    if (err != SUCCESS) {
                        OSSemPost( Load_Vec_Sem_lock );
                        return (SPI_BUS_ERR);
                    }
                }
            }            
        }  
        
        OSTimeDly( Global_VEC_Cfg.delay );  //Delay mSecond
        if( Global_VEC_Cfg.trigger_en == 0 ) {
            OSSemPost( Load_Vec_Sem_lock ); 
            return err; 
        }
    }
    
    index = Global_VEC_Cfg.vec_index_b ; // Power down
    if( index != 0 ) {        
        Read_Flash_State(&flash_info, FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * index );
        if( flash_info.f_w_state != FW_DOWNLAD_STATE_FINISHED ) {
          err = FW_VEC_SAVE_STATE_ERR;
          APP_TRACE_INFO(("\r\nvec data state error...\r\n"));
          OSSemPost( Load_Vec_Sem_lock );
          return err;
        }       
        APP_TRACE_INFO(("Load vec[%d] (%d Bytes) ...\r\n",index,flash_info.bin_size));
        pChar = (uint8_t *)FLASH_ADDR_FW_VEC + index * FLASH_ADDR_FW_VEC_SIZE;
        if(  Global_VEC_Cfg.if_type == 1 ) { //I2C interface
            for( i = 0; i < flash_info.bin_size ;  ) {
                data_num = *(pChar+i) ;
                err =  TWID_Write(  dev_addr>>1, 0, 0, pChar + i + 1 , data_num, NULL); 
                i += ( data_num + 1 );     
                if ( err != SUCCESS )  {
                    OSSemPost( Load_Vec_Sem_lock );
                    return(I2C_BUS_ERR) ;
                }
            }
        } else if( Global_VEC_Cfg.if_type == 2 ) { //SPI interface
            for( i = 0; i < flash_info.bin_size ;  ) {
                data_num = *(pChar+i) ; 
                if(data_num > sizeof(buf)) {
                    OSSemPost( Load_Vec_Sem_lock );
                    return (SPI_BUS_ERR);
                }
                for(j = 0; j < data_num ; j++) {
                    buf[j] = *(pChar + i + 1 + j);
                }                
                err = SPI_WriteBuffer_API( &buf, data_num ); 
                i += ( data_num + 1 );
                if (err != SUCCESS) {
                    OSSemPost( Load_Vec_Sem_lock );
                    return (SPI_BUS_ERR);
                }
            }           
        }        
    }
    
    if( Global_VEC_Cfg.pdm_clk_off ) {
       //OSTimeDly(30);//delay for iM501 test 
       // wait for iM501 PLL change stable
       uint32_t time_start = OSTimeGet();       
       im501_change_if_speed(Global_VEC_Cfg.if_type, 0);
       while( im501_check_pll_ready() == 0 ) {
           if( (OSTimeGet() - time_start) > 500 ){ //max 500ms timeout
               break;
           }
       };
       im501_change_if_speed(Global_VEC_Cfg.if_type,1);
      
       I2C_Mixer(I2C_MIX_FM36_CODEC);
       FM36_PDMADC_CLK_OnOff(0,0); //disable PDM clock
       I2C_Mixer(I2C_MIX_UIF_S);          
    }     
    
    OSSemPost( Load_Vec_Sem_lock );
    return err;     
    
}
#else

uint8_t MCU_Load_Vec( uint8_t firsttime )
{     
    uint8_t err;
    uint8_t i,j;
    uint8_t index, *pChar, buf[6]; 
    uint8_t dev_addr, data_num;
    FLASH_INFO    flash_info; 
    
    DataSource *pSource = &source_twi0;
    OPTIONPARAMETER *twi_option = ( OPTIONPARAMETER * )pSource->privateData;
    
    twi_option->iaddress = 0;
    twi_option->isize = 0;
    
    err = NO_ERR;    
    
    if( Global_VEC_Cfg.type == 41 ) {
        APP_TRACE_INFO(("\r\nLoad iM401 vec stored in MCU flash...\r\n"));
        dev_addr = iM401_I2C_ADDR;        
    } else if(Global_VEC_Cfg.type == 51 ) {
        APP_TRACE_INFO(("\r\nLoad iM501 vec stored in MCU flash...\r\n"));
        dev_addr = iM501_I2C_ADDR;       
    } else {
        APP_TRACE_INFO(("\r\nvec conf data error!\r\n")); 
        return FW_VEC_SET_CFG_ERR;         
    }
    
    if( Global_VEC_Cfg.flag != 0x55 ) {
        APP_TRACE_INFO(("\r\nvec conf data error!\r\n")); 
        return FW_VEC_SET_CFG_ERR;
    }
    
    OSSemPend( Load_Vec_Sem_lock, 0, &err );
    
    if( firsttime == 0 ) {   //not first time pwd
        
        index = Global_VEC_Cfg.vec_index_a ; //Power up
        if(  Global_VEC_Cfg.pdm_clk_off ) 
        {
//            I2C_Mixer( I2C_MIX_FM36_CODEC );                    //avoid compile error
//            FM36_PDMADC_CLK_OnOff( 1 , 0 );                     //avoid compile error    
//            I2C_Mixer( I2C_MIX_UIF_S );                         //avoid compile error           
        }
        if( index != 0 ) 
        {        
//            Read_Flash_State(&flash_info, FLASH_ADDR_FW_VEC_STATE       \
                                + AT91C_IFLASH_PAGE_SIZE * index  );              //avoid compile error
            
            if( flash_info.f_w_state != FW_DOWNLAD_STATE_FINISHED ) 
            {    
              APP_TRACE_INFO(("\r\nvec data state error...\r\n"));
              OSSemPost( Load_Vec_Sem_lock );
              return FW_VEC_SAVE_STATE_ERR;
            }   
            APP_TRACE_INFO(("Load vec[%d] (%d Bytes) ...\r\n",index,flash_info.bin_size));
//            pChar = ( uint8_t *)FLASH_ADDR_FW_VEC + index * FLASH_ADDR_FW_VEC_SIZE;   //avoid compile error
            
            if(  Global_VEC_Cfg.if_type == 1 ) { //I2C interface
                for( i = 0; i < flash_info.bin_size ;  ) {
                    data_num = *(pChar+i) ;
                    twi_option->address = dev_addr >> 1;
                    err = pSource->buffer_write( pSource, pChar + i + 1, data_num );
//                    err =  TWID_Write(  dev_addr>>1, 0, 0, pChar + i + 1 , data_num, NULL); 
                    i += ( data_num + 1 );               
                    if ( err != SUCCESS )  {
                        OSSemPost( Load_Vec_Sem_lock );
                        return (I2C_BUS_ERR) ;
                    }
                }
            }else if( Global_VEC_Cfg.if_type == 2 ) { //SPI interface
                for( i = 0; i < flash_info.bin_size ;  ) {
                    data_num = *(pChar+i) ;
                    if(data_num > sizeof(buf)) {
                        OSSemPost( Load_Vec_Sem_lock );
                        return (SPI_BUS_ERR);
                    }
                    for(j = 0; j < data_num ; j++) {
                        buf[j] = *(pChar + i + 1 + j);
                    }                
//                    err = SPI_WriteBuffer_API( &buf, data_num );    //avoid compile warning
                    err = pSource->buffer_write( pSource , buf , data_num );
                    i += ( data_num + 1 );
                    if (err != SUCCESS) {
                        OSSemPost( Load_Vec_Sem_lock );
                        return (SPI_BUS_ERR);
                    }
                }
            }            
        }  
        
        OSTimeDly( Global_VEC_Cfg.delay );  //Delay mSecond
        if( Global_VEC_Cfg.trigger_en == 0 ) {
            OSSemPost( Load_Vec_Sem_lock ); 
            return err; 
        }
    }
    
    index = Global_VEC_Cfg.vec_index_b ; // Power down
    if( index != 0 ) {        
//        Read_Flash_State(&flash_info, FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * index );  //avoid compile error
        if( flash_info.f_w_state != FW_DOWNLAD_STATE_FINISHED ) {
          err = FW_VEC_SAVE_STATE_ERR;
          APP_TRACE_INFO(("\r\nvec data state error...\r\n"));
          OSSemPost( Load_Vec_Sem_lock );
          return err;
        }       
        APP_TRACE_INFO(("Load vec[%d] (%d Bytes) ...\r\n",index,flash_info.bin_size));
//        pChar = ( uint8_t * )FLASH_ADDR_FW_VEC + index * FLASH_ADDR_FW_VEC_SIZE;              //avoid compile error
        if(  Global_VEC_Cfg.if_type == 1 ) { //I2C interface
            for( i = 0; i < flash_info.bin_size ;  ) {
                data_num = *(pChar+i) ;
                twi_option->address = dev_addr >> 1;
                err = pSource->buffer_write( pSource , pChar + i + 1 ,data_num );
//                err =  TWID_Write(  dev_addr>>1, 0, 0, pChar + i + 1 , data_num, NULL); 
                i += ( data_num + 1 );     
                if ( err != SUCCESS )  {
                    OSSemPost( Load_Vec_Sem_lock );
                    return(I2C_BUS_ERR) ;
                }
            }
        } else if( Global_VEC_Cfg.if_type == 2 ) { //SPI interface
            for( i = 0; i < flash_info.bin_size ;  ) {
                data_num = *(pChar+i) ; 
                if(data_num > sizeof(buf)) {
                    OSSemPost( Load_Vec_Sem_lock );
                    return (SPI_BUS_ERR);
                }
                for(j = 0; j < data_num ; j++) {
                    buf[j] = *(pChar + i + 1 + j);
                }                
//                err = SPI_WriteBuffer_API( &buf, data_num );   //--aviod compile warning
                err = pSource->buffer_write( pSource , buf , data_num );
                i += ( data_num + 1 );
                if (err != SUCCESS) {
                    OSSemPost( Load_Vec_Sem_lock );
                    return (SPI_BUS_ERR);
                }
            }           
        }        
    }
    
    if( Global_VEC_Cfg.pdm_clk_off ) {
       //OSTimeDly(30);//delay for iM501 test 
       // wait for iM501 PLL change stable
       uint32_t time_start = OSTimeGet();       
       im501_change_if_speed(Global_VEC_Cfg.if_type, 0);
       while( im501_check_pll_ready() == 0 ) {
           if( (OSTimeGet() - time_start) > 500 ){ //max 500ms timeout
               break;
           }
       };
       im501_change_if_speed(Global_VEC_Cfg.if_type,1);
      
//       I2C_Mixer(I2C_MIX_FM36_CODEC);                         //avoid compile error
//       FM36_PDMADC_CLK_OnOff(0,0); //disable PDM clock
//       I2C_Mixer(I2C_MIX_UIF_S);          
    }     
    
    OSSemPost( Load_Vec_Sem_lock );
    return err;     
    
}
#endif


/*
*********************************************************************************************************
*                                           im501_read_reg_i2c()
*
* Description :  read iM501 Reg via I2C, just read one byte.
*
* Argument(s) :  reg_addr      is the iM501 register adress(one byte length)
*
*                *pdata        is point to where read back data will be stored  
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t im501_read_reg_i2c( uint8_t reg_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[2];
    
    err    =  NO_ERR;
    buf[0] =  IM501_I2C_CMD_REG_RD;
    buf[1] =  reg_addr;
    state  =  TWID_Write( iM501_I2C_ADDR>>1,
                         0, 
                         0, 
                         buf, 
                         sizeof(buf), 
                         NULL );     
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;
        return err;
    } 
   
    state =  TWID_Read( iM501_I2C_ADDR>>1,
                        0, 
                        0, 
                        pdata, 
                        1, 
                        NULL );     
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;        
    } 
    
    return err;
    
}
#else

uint8_t im501_read_reg_i2c( uint8_t reg_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[2];
    
    DataSource *pSource = &source_twi0;
    OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource ->privateData;
    
    err    =  NO_ERR;
    buf[0] =  IM501_I2C_CMD_REG_RD;
    buf[1] =  reg_addr;
 
    option->address = iM501_I2C_ADDR>>1;
    option->iaddress = 0;
    option->isize = 0;
    
    state = pSource->buffer_write( pSource,buf,sizeof( buf ) );        
    if (state != SUCCESS) 
    {
        err = I2C_BUS_ERR;
        return err;
    } 
    
    
    state = pSource->buffer_read( pSource,pdata,sizeof( buf ) );    
    if (state != SUCCESS) 
    {
        err = I2C_BUS_ERR;        
    } 
    
    return err;
    
}
#endif


/*
*********************************************************************************************************
*                                           im501_read_reg_spi()
*
* Description :  read iM501 Reg via SPI, just read one byte.
*
* Argument(s) :  reg_addr      is the iM501 register adress(one byte length)
*
*                *pdata        is point to where read back data will be stored
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
uint8_t im501_read_reg_spi( uint8_t reg_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[2];
    uint8_t *pbuf;
    
    err  = NO_ERR;
    pbuf = (uint8_t *)Reg_RW_Data; //global usage
  
    buf[0] =  IM501_SPI_CMD_REG_RD;
    buf[1] =  reg_addr;
    
    state =  SPI_WriteReadBuffer_API(  pbuf, 
                                       buf, 
                                       1 , 
                                       sizeof(buf));
             
    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        //APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d",state));
        return err;
    }              
    pbuf = pbuf + 1; //fix bug
    *pdata = *pbuf;
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           im501_write_reg_i2c()
*
* Description :  write iM501 Reg via I2C, just write one byte.
*
* Argument(s) :  reg_addr      is the iM501 register adress(one byte length)
*
*                data          is the data will be write to im501
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t im501_write_reg_i2c( uint8_t reg_addr, uint8_t data )
{
    uint8_t err, state;
    uint8_t buf[3];
    
    err    =  NO_ERR;
    buf[0] =  IM501_I2C_CMD_REG_WR_1;
    buf[1] =  reg_addr;
    buf[2] =  data;
    state  =  TWID_Write( iM501_I2C_ADDR>>1,
                         0, 
                         0, 
                         buf, 
                         sizeof(buf), 
                         NULL );     
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;
        return err;
    } 
    
    return err;
 
}
#else
uint8_t im501_write_reg_i2c( uint8_t reg_addr, uint8_t data )
{
    uint8_t err, state;
    uint8_t buf[3];
    
    DataSource *pSource = &source_twi0;
    OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource ->privateData;
    
    err    =  NO_ERR;
    buf[0] =  IM501_I2C_CMD_REG_WR_1;
    buf[1] =  reg_addr;
    buf[2] =  data;
    
    option->address = iM501_I2C_ADDR>>1;
    option->iaddress = 0;
    option->isize = 0;

    state  =  pSource->buffer_write( pSource,buf , sizeof( buf ) );
    if (state != SUCCESS) 
    {
        err = I2C_BUS_ERR;
        return err;
    } 
    
    return err;
    
    
}
#endif
/*
*********************************************************************************************************
*                                           im501_write_reg_spi()
*
* Description :  write iM501 Reg via SPI, just write one byte.
*
* Argument(s) :  reg_addr      is the iM501 register adress(one byte length)
*
*                data          is the data will be write to im501
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
uint8_t im501_write_reg_spi( uint8_t reg_addr, uint8_t data )
{
    uint8_t err, state;
    uint8_t buf[3];
    
    err    =  NO_ERR;
    buf[0] =  IM501_SPI_CMD_REG_WR;
    buf[1] =  reg_addr;
    buf[2] =  data;
      
//    state =  SPI_WriteBuffer_API( buf, sizeof(buf) ); 
    state = pSource->buffer_write( pSource , buf , sizeof( buf ) );
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;
        return err;
    } 
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           im501_read_dram_i2c()
*
* Description :  read iM501 DRAM via I2C, just read 4 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 3 bytes length and need 4bytes alignment
*
*                *pdata         is point to where read back data will be stored   
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t im501_read_dram_i2c( uint32_t mem_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[4];
    
    err   =  NO_ERR;
    buf[0] =  IM501_I2C_CMD_DM_RD;
    buf[1] =  mem_addr & 0xFF;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    state =  TWID_Write( iM501_I2C_ADDR>>1,
                         0, 
                         0, 
                         buf, 
                         sizeof(buf),  
                         NULL );     
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;
        return err;
    }   
    err = im501_read_reg_i2c(0x0A, pdata);
    if( err != NO_ERR ) {
        return err;
    }
    err = im501_read_reg_i2c(0x0B, pdata+1);
    if( err != NO_ERR ) {
        return err;
    }  
    err = im501_read_reg_i2c(0x0C, pdata+2);
    if( err != NO_ERR ) {
        return err;
    } 
    err = im501_read_reg_i2c(0x0D, pdata+3);
    if( err != NO_ERR ) {
        return err;
    }
    
    return err;
    
}
#else
uint8_t im501_read_dram_i2c( uint32_t mem_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[4];
    
    err   =  NO_ERR;
    
    DataSource *pSource = &source_twi0;
    OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;
    
    option->address = iM501_I2C_ADDR>>1;
    option->iaddress = 0;
    option->isize = 0;
    
    buf[0] =  IM501_I2C_CMD_DM_RD;
    buf[1] =  mem_addr & 0xFF;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    
    state = pSource->buffer_write( pSource , buf ,sizeof( buf ) );      
    if (state != SUCCESS) 
    {
        err = I2C_BUS_ERR;
        return err;
    }
    
    err = im501_read_reg_i2c(0x0A, pdata);
    if( err != NO_ERR ) 
    {
        return err;
    }
    
    err = im501_read_reg_i2c(0x0B, pdata+1);
    if( err != NO_ERR ) 
    {
        return err;
    }
    
    err = im501_read_reg_i2c(0x0C, pdata+2);
    if( err != NO_ERR ) 
    {
        return err;
    }
    
    err = im501_read_reg_i2c(0x0D, pdata+3);
    if( err != NO_ERR ) 
    {
        return err;
    }
    
    return err;
    
}
#endif


/*
*********************************************************************************************************
*                                           im501_read_dram_spi()
*
* Description :  burst read iM501 DRAM via SPI, just limited to 4 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 3 bytes length and need 4bytes alignment
*
*                *pdata         is point to where read back data will be stored 
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
uint8_t im501_read_dram_spi( uint32_t mem_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[6];
    uint8_t *pbuf;
    
    err   =  NO_ERR;
    pbuf = (uint8_t *)Reg_RW_Data; //global usage
  
    
    buf[0] =  IM501_SPI_CMD_DM_RD;
    buf[1] =  mem_addr & 0xFC;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    buf[4] =  2;
    buf[5] =  0;    

    state  =  SPI_WriteReadBuffer_API(  pbuf, 
                                       buf, 
                                       4 , 
                                       sizeof(buf) );
            
    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        //APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d",state));
        return err;
    }   
        
    pbuf = pbuf + 1; //fix bug
    for (uint8_t i = 0; i<4; i++ ) {
      *(pdata+i) = *(pbuf+i);
    }
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           im501_burst_read_dram_spi()
*
* Description :  burst read iM501 DRAM via SPI, just limited to 4095 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 3 bytes length and need 4bytes alignment
*
*                **pdata        is point to where the pointer pointing to read back data will be stored 
*
*                data_len       is data length to read in bytes, maxium 4095 bytes  
*
* Return(s)   :  error number.           
*
* Note(s)     :  Non-Reentrant function.   Reg_RW_Data is used.
*                Be care full this function use fixed buffer, and return a **pointer to.
*********************************************************************************************************
*/
uint8_t im501_burst_read_dram_spi( uint32_t mem_addr, uint8_t **pdata, uint32_t data_len )
{
    uint8_t  err, state;
    uint8_t  buf[6];
    uint8_t *pbuf;
    
    err   =  NO_ERR;
    pbuf = (uint8_t *)Reg_RW_Data; //global usage    
   
    buf[0] =  IM501_SPI_CMD_DM_RD;
    buf[1] =  mem_addr     & 0xFF;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    buf[4] =  (data_len>>1)   & 0xFF;
    buf[5] =  (data_len>>(1+8))& 0xFF;    

    state =  SPI_WriteReadBuffer_API(  pbuf, 
                                       buf, 
                                       data_len , 
                                       sizeof(buf) );
            
    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        //APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d",state));
        return err;
    } 
          
    *pdata =  pbuf + 1; 
    
    //Demo_Sine_Gen( *pdata, data_len, 16000, 1 );
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           im501_write_dram_i2c()
*
* Description :  write iM501 DRAM via I2C, just write two bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 3 bytes length and need 4bytes alignment
*
*                *pdata         is pointer to where to be written data stores 
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t im501_write_dram_i2c( uint32_t mem_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[6];
    
    err   =  NO_ERR;
    buf[0] =  IM501_I2C_CMD_DM_WR;
    buf[1] =  mem_addr & 0xFF;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    buf[4] =  *pdata;
    buf[5] =  *(pdata+1);
    
    state =  TWID_Write( iM501_I2C_ADDR>>1,
                         0, 
                         0, 
                         buf, 
                         sizeof(buf),  
                         NULL );  
    
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;
        return err;
    } 
       
    return err;
    
}
#else

uint8_t im501_write_dram_i2c( uint32_t mem_addr, uint8_t *pdata )
{
    uint8_t err =  NO_ERR, state;
    uint8_t buf[6];
    
    DataSource *pSource = &source_twi0;
    OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;
    
    option->address = iM501_I2C_ADDR>>1;
    option->iaddress = 0;
    option->isize = 0;
    
    buf[0] =  IM501_I2C_CMD_DM_WR;
    buf[1] =  mem_addr & 0xFF;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    buf[4] =  *pdata;
    buf[5] =  *(pdata+1);
    
    state = pSource->buffer_write( pSource , buf , sizeof( buf ) );   
    if (state != SUCCESS) 
    {
        err = I2C_BUS_ERR;
        return err;
    } 
       
    return err;
    
}
#endif


/*
*********************************************************************************************************
*                                           im501_write_dram_spi()
*
* Description :  write iM501 DRAM via SPI, just write 4 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 3 bytes length and need 4bytes alignment
*
*                *pdata         is pointer to where to be written data stores 
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
uint8_t im501_write_dram_spi( uint32_t mem_addr, uint8_t *pdata )
{
    uint8_t err, state;
    uint8_t buf[10];
    
    err    =  NO_ERR;
    
    buf[0] =  IM501_SPI_CMD_DM_WR;
    buf[1] =  mem_addr & 0xFF;
    buf[2] =  (mem_addr>>8) & 0xFF;
    buf[3] =  (mem_addr>>16) & 0xFF;
    buf[4] =  2; //2 words
    buf[5] =  0;
    buf[6] =  *pdata;
    buf[7] =  *(pdata+1);
    buf[8] =  *(pdata+2);
    buf[9] =  *(pdata+3);
      
//    state =  SPI_WriteBuffer_API( buf, sizeof(buf) ); 
    state = pSource->buffer_write( pSource , buf , sizeof( buf ) );
    if (state != SUCCESS) {
        err = I2C_BUS_ERR;
        return err;
    } 
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           parse_to_host_command()
*
* Description :  parse cmd from iM501 
*
* Argument(s) :  cmd     is To_Host_CMD type data
*
* Return(s)   :  error number.           
*
* Note(s)     :  
*********************************************************************************************************
*/
uint8_t parse_to_host_command( To_Host_CMD cmd )
{
    uint8_t err; 
//    uint32_t address;
       
    switch( cmd.cmd_byte ) {
        
        case TO_HOST_CMD_KEYWORD_DET : //Info host Keywords detected
            im501_key_words_detect = 1 ;
        break;
                               
        default:
            err = 2;           
        break;
        
    }
            
    return err;
    
}


/*
*********************************************************************************************************
*                                           send_to_dsp_command()
*
* Description :  parse cmd from iM501 
*
* Argument(s) :  cmd     is To_501_CMD type data
*
* Return(s)   :  error number.           
*
* Note(s)     :  
*********************************************************************************************************
*/
uint8_t send_to_dsp_command( To_501_CMD cmd )
{
    uint8_t err;
    uint32_t  i;
    
    err = im501_write_dram_spi( TO_DSP_CMD_ADDR, (uint8_t *)&cmd );
    if( err != 0 ){ 
        return err;
    }
    
    err = im501_write_reg_spi( 0x01, cmd.cmd_byte ); //generate interrupt to DSP
    if( err != 0 ){ 
        return err;
    }

    for( i = 0; i< 50; i++ ) {   //wait for (50*100us = 5ms) to check if DSP finished        
        err = im501_read_dram_spi( TO_DSP_CMD_ADDR, (uint8_t *)&cmd );
        if( err != 0 ){ 
            return err;
        }
        if( cmd.status == 0 ) {
            break;
        } else {
            err = TO_501_CMD_ERR;
        } 
        delay_us(100);
    }
    
    return err;
    
}


/*
*********************************************************************************************************
*                                    Start_Keywords_Detection()
*
* Description :  Configure GPIO interruption for iM501 IRQ
*
* Argument(s) :  gpio_irq  - IRQ connected gpio number
* 
* Return(s)   :  None.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
void Start_Keywords_Detection( uint8_t gpio_irq )
{
    im501_irq_counter = 0;
    im501_key_words_detect = 0 ;
    im501_irq_gpio = gpio_irq ; 
    
    //set gpio interruption
    Enable_Interrupt_For_iM501_IRQ();
    
    //PP_TRACE_INFO(("\r\n::Record voice buffer time cost: %d ms\r\n",time_rec));    
    
}


/*
*********************************************************************************************************
*                                    Request_Enter_PSM()
*
* Description :  Send CMD to iM501 to enter PWM, then turn off PDM Clock 
*
* Argument(s) :  None.
* 
* Return(s)   :  Error number.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
uint8_t Request_Enter_PSM( void )
{
    
    uint8_t err;
           
    im501_change_if_speed( 2,0 ); //change SPI speed to low speed   
    err = Write_CMD_To_iM501( TO_DSP_CMD_REQ_ENTER_PSM, 0 );
    
    if( err != NO_ERR )
    { 
        return err;
    }
    
//    I2C_Mixer( I2C_MIX_FM36_CODEC );                          //avoid compile error
//    FM36_PDMADC_CLK_OnOff( 0 , 0 ); //Disable PDM clock
//    I2C_Mixer( I2C_MIX_UIF_S );    
   
    
    return err;
    
}


/*
*********************************************************************************************************
*                                    Write_CMD_To_iM501()
*
* Description :  Send command data to iM501 
*
* Argument(s) :  cmd_index : command index number
*                para      : parameters
* 
* Return(s)   :  Error number.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
uint8_t Write_CMD_To_iM501( uint8_t cmd_index, uint16_t para )
{
    
    uint8_t err;
    To_501_CMD    cmd;
    
    cmd.cmd_byte = cmd_index;//((cmd_index & 0x3F) << 2) | 0x01; //D[1] : "1", interrupt DSP. This bit generates NMI (non-mask-able interrupt), D[0]: "1" generate mask-able interrupt
    cmd.attri    = para & 0xFFFF ;
    cmd.status   = 1;
    err = send_to_dsp_command( cmd );
    
    return err;
    
}


/*
*********************************************************************************************************
*                                    Service_To_iM501_IRQ()
*
* Description :  Service to iM501 IRQ interruption 
*                Should be in Main Loop, inquiring the IRQ signal by checking im501_irq_counter
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Service_To_iM501_IRQ( void )
{  
    
    uint8_t err;    
    To_Host_CMD   cmd;
    VOICE_BUF_CFG   spi_rec_cfg;                        
    
    if ( im501_irq_counter ) {
        
        im501_irq_counter--;
        
        err = im501_read_dram_spi( TO_HOST_CMD_ADDR, (uint8_t *)&cmd );
        if( err != NO_ERR ){ 
            return ;
        }
    
        err = parse_to_host_command( cmd );
        if( err != NO_ERR ){ 
            APP_TRACE_INFO(("\r\nparse_to_host_command : undefined cmd = 0x%x",cmd.cmd_byte));
        } 
        
        cmd.status = 0;
        err = im501_write_dram_spi( TO_HOST_CMD_ADDR, (uint8_t *)&cmd );
        if( err != NO_ERR ){ 
            return ;
        }
    
        if( im501_key_words_detect == 1 ) {
       
            Disable_Interrupt_For_iM501_IRQ();//disable IRQ interrupt
            
//            I2C_Mixer(I2C_MIX_FM36_CODEC);  //avoid compile error         
            FM36_PDMADC_CLK_OnOff(1,0); //Should Enable PDM clock fast switch, but this will cause pop sound 
//            I2C_Mixer(I2C_MIX_UIF_S);       //avoid compile error            
            im501_change_if_speed(2,1); //change SPI speed to high speed  
            
            spi_rec_cfg.gpio_irq  = im501_irq_gpio; 
            spi_rec_cfg.spi_mode  = Global_UIF_Setting[UIF_TYPE_SPI - 1].speed;
            spi_rec_cfg.spi_speed = Global_UIF_Setting[UIF_TYPE_SPI - 1].attribute;               
            spi_rec_cfg.chip_id   = Global_UIF_Setting[UIF_TYPE_DUT_ID - 1].attribute;  //avoid compile error
            err = SPI_Rec_Start( &spi_rec_cfg ); //send start CMD to audio MCU 
             
        }  
        
    }
  
    
}


/*
*********************************************************************************************************
*                                           ISR_iM501_IRQ()
*
* Description :  Interruption service routine for iM501 IRQ
*
* Argument(s) :  None.
*
* Return(s)   :  None.           
*
* Note(s)     :  The ISR register is read in PioInterruptHandler().
*********************************************************************************************************
*/

void ISR_iM501_IRQ( void )
{    
    
    if( Check_GPIO_Intrrupt( im501_irq_gpio ) ) {         
        im501_irq_counter++;      
    }
    //Service_To_iM501_IRQ();
    
}


void Enable_Interrupt_For_iM501_IRQ( void )
{
    
    Config_GPIO_Interrupt( im501_irq_gpio, ISR_iM501_IRQ ) ;
    im501_service_int_enabled = 1;
   
}
  
        
void Disable_Interrupt_For_iM501_IRQ( void )
{
    if( im501_service_int_enabled ) { 
        im501_service_int_enabled = 0;
        Disable_GPIO_Interrupt( im501_irq_gpio ); //disable IRQ interrupt
    }
   
}

