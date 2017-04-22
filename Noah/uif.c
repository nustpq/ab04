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
*                                        FM DSP W/R RELATED OPERATIONS
*
*                                          Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : uif.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#include <includes.h>
#include "fm1388_spi.h"
#include "fm1388d.h"
#include "ruler.h"

extern Fm1388 fm1388;
INTERFACE_CFG   Global_UIF_Setting[ UIF_TYPE_CMD_NUM ];     //ruler type = 3
unsigned char   Reg_RW_Data[ EMB_BUF_SIZE ];

////////////////////////////////////////////////////////////////////////////////

//disabled endian reverse, as PC scripts will do it!
void Reverse_Endian( unsigned char *pdata, unsigned char size ) 
{
    
    unsigned char i;
    unsigned char temp;
    
    for( i = 0 ; i< size>>1 ; i++ ) {
        
      temp = *( pdata+i );
      *( pdata+i ) = *( pdata + size -1 - i );
      *( pdata + size -1 - i ) = temp;
      
    }     
    
}

/*
*********************************************************************************************************
*                                           Dump_Data()
*
* Description : print data package on debug uart for debug .
* Argument(s) : *pdata : pointer to data address
*                 size : N bytes
*               
* Return(s)   : None.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Dump_Data ( unsigned char *pdata, unsigned int size )
{

#if( false )
    unsigned int i ;
    
    if( size == 0 ) {  
        return; 
    }
    
    APP_TRACE_INFO(("\r\n---------------------- Dump Data(Hex) -------------------------"));
    
    for( i = 0; i < size ; i++ ) {         
        if( i%16 == 0 ) {
            APP_TRACE_INFO(("\r\n"));
        }
        APP_TRACE_INFO((" %02X ",*pdata++));
    }  
    
    APP_TRACE_INFO(("\r\n---------------------------------------------------------------\r\n"));
#endif
}

/*
*********************************************************************************************************
*                                           Setup_Interface()
*
* Description : Send command to .
* Argument(s) : INTERFACE_CFG : 
*               
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Setup_Interface( INTERFACE_CFG *pInterface_Cfg )
{       
    unsigned char err; 
    unsigned int  temp, temp2;
    unsigned char scl_no, sda_no;
    
    APP_TRACE_INFO(("\r\nSetup_Interface: if_type=%d, speed=%dkHz, attribute=0x%X ",\
                         pInterface_Cfg->if_type,pInterface_Cfg->speed, pInterface_Cfg->attribute));
    err   = NULL;
    temp  = pInterface_Cfg->speed ;
    temp2 = pInterface_Cfg->attribute ;
      
//    if(  (Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].speed   == pInterface_Cfg->speed) &&
//         (Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].if_type == pInterface_Cfg->if_type) )  {
//       
//        if( Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].attribute == pInterface_Cfg->attribute ) {
//            APP_TRACE_INFO(("\r\nNo need to set same interface\r\n"));
//        } else {
//            Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].attribute = pInterface_Cfg->attribute;
//            APP_TRACE_INFO(("\r\nChanged the interface attribute!\r\n"));
//        }    
//        return err;
//    }
    if(  (Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].speed   == pInterface_Cfg->speed)  &&
         (Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].if_type == pInterface_Cfg->if_type)  &&
         (Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].attribute == pInterface_Cfg->attribute ) ) {        
            APP_TRACE_INFO(("\r\nNo need to set same interface\r\n"));       
            return err;
    }

    switch( pInterface_Cfg->if_type )  {
        
        case UIF_TYPE_I2C :
            if( Global_UIF_Setting[ UIF_TYPE_I2C - 1 ].speed  == temp ) {
                break;
            }
            if( temp <= 1000 && temp >= 10) { //10k ~ 1M
                APP_TRACE_INFO(("\r\nI2C port is set to %d kHz\r\n",temp)); 
                //TWI_Init( temp * 1000 );
                temp = temp * 1000 ;
                source_twi0.init_source( &source_twi0,&temp );
                       
            }  else {
                APP_TRACE_INFO(("\r\nERROR: I2C speed not support %d kHz\r\n",temp));
                err = SET_I2C_ERR ;
            }   
        break ;  
        
        case UIF_TYPE_I2C_GPIO :
            scl_no = GET_I2C_GPIO_SCL(pInterface_Cfg->attribute);
            sda_no = GET_I2C_GPIO_SDA(pInterface_Cfg->attribute);
            //if( temp <= 400 && temp >= 10) { 
                I2C_GPIO_Init( temp * 1000, scl_no, sda_no  );     
                APP_TRACE_INFO(("\r\nSet GPIO emluated I2C port :%d kHz\r\n",temp));        
            //}  else {
            //    APP_TRACE_INFO(("\r\nERROR: I2C speed not support %d kHz\r\n",temp));
            //    err = SET_I2C_ERR ;
            //}   
        break ;
        
        case UIF_TYPE_SPI :
            if( Global_SPI_Rec_Start == 1 ) { //in case last SPI record not ended normally 
                err = SET_SPI_ERR ;               
                break;//in case audio not stopped normally                         
            }
            if( Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].speed  == temp &&\
                Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].attribute == temp2 ) {
                break;
            }            
            if( temp <= 48000 && temp >= 400) { 
          
                spi0_cfg.spi_speed   = temp * 1000;
                spi0_cfg.spi_format  = temp2;                        
//                source_spi0.init_source( &source_spi0, &spi0_cfg ) ;
                init_spi0( NULL , NULL );
                APP_TRACE_INFO(("\r\nSet SPI interface\r\n"));  
                
            }  else {
                APP_TRACE_INFO(("\r\nERROR: SPI speed not support %d kHz\r\n",temp));
                err= SET_SPI_ERR ;
            }              
        break ;
        
        case UIF_TYPE_FM36_PATH :  
            I2C_Switcher( I2C_SWITCH_FM36 );           
            if( (pInterface_Cfg->attribute == ATTRI_FM36_PATH_PWD_BP ) ){//&& ( Global_UIF_Setting[ UIF_TYPE_FM36_PATH - 1 ].attribute != ATTRI_FM36_PATH_PWD_BP ) ) {                
                err = FM36_PWD_Bypass();                
            } 
            if( (pInterface_Cfg->attribute == ATTRI_FM36_PATH_NORMAL ) && ( Global_UIF_Setting[ UIF_TYPE_FM36_PATH - 1 ].attribute != ATTRI_FM36_PATH_NORMAL ) ) {                  
                err = Init_FM36_AB03_Preset();                
            }
            break ;
        
        case UIF_TYPE_FM36_PDMCLK :
            I2C_Switcher( I2C_SWITCH_FM36 ); 
            //err = FM36_PDM_CLK_Set( GET_BYTE_HIGH_4BIT(pInterface_Cfg->attribute), GET_BYTE_LOW_4BIT(pInterface_Cfg->attribute), 1 ); //pdm_dac_clk, pdm_adc_clk, type=ontheflychange
            Global_UIF_Setting[ UIF_TYPE_FM36_PDMCLK - 1 ].attribute = pInterface_Cfg->attribute; //save clock data in attribute to global for  Init_FM36_AB03_Preset() use
            err = Init_FM36_AB03_Preset(); //be careful if the I2C switch status changed during OSTimeDly() in side this routine                    
        break ;
        
        case UIF_TYPE_GPIO :       
            err = GPIOPIN_Set( GET_BYTE_HIGH_4BIT(pInterface_Cfg->attribute), GET_BYTE_LOW_4BIT(pInterface_Cfg->attribute));
        break ; 
        
        case UIF_TYPE_I2C_Mixer :       
            err = I2C_Switcher( pInterface_Cfg->attribute );
        break ;
        
        case UIF_TYPE_GPIO_CLK :       
            //CS_GPIO_Init( pInterface_Cfg->attribute );
        break ;   
    
        case UIF_TYPE_DUT_ID:
  
        break;
        default:
            err = UIF_TYPE_NOT_SUPPORT_ERR;
        break;
        
    }
    
    
    if ( err == NULL ) {
        Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].attribute = pInterface_Cfg->attribute;
        Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].if_type   = pInterface_Cfg->if_type;
        Global_UIF_Setting[ pInterface_Cfg->if_type - 1 ].speed     = pInterface_Cfg->speed;
    }
    
    return err ; 
    
    
}


/*
*********************************************************************************************************
*                                           Dump_Data()
*
* Description : print data package on debug uart for debug .
* Argument(s) : *pdata : pointer to data address
*                 size : N bytes
*               
* Return(s)   : None.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Raw_Write( RAW_WRITE *p_raw_write )
{  
    
    unsigned char  state, err;
    unsigned char  buf[8] ; 
    unsigned int   mem_addr;   
    unsigned char *pChar;
    unsigned int   i, size;
    
//    APP_TRACE_INFO(("\r\nRaw_Write: if_type=%d, dev_addr=0x%02X, data_len=%d ",\
//                         p_raw_write->if_type,p_raw_write->dev_addr,p_raw_write->data_len));    
//    Dump_Data( p_raw_write->pdata,  p_raw_write->data_len );    
//    
    err    = NO_ERR;
    pChar  = p_raw_write->pdata ;
        
    switch( p_raw_write->if_type ) {
        
        ////////////////////////////////////////////////////////////////////////   
        case UIF_TYPE_I2C: 
            switch( Global_UIF_Setting[UIF_TYPE_I2C - 1 ].attribute ) {
                
                case  ATTRI_I2C_IM401_LOAD_CODE :  //iM401            
                    //I2C_Mixer( I2C_MIX_UIF_S );
                    OSTimeDly(1);                 
                    buf[0] = 0xF0;
                    buf[1] = *pChar++;
                    buf[2] = *pChar++;
                    state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                 
                    if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;                 
                    }
                    buf[0] = 0xF1;
                    buf[1] = *pChar++;
                    buf[2] = *pChar++;
                    state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                  
                    if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;  
                    }                    
                    buf[0] = 0xF8; 
                    size   = (p_raw_write->data_len-4)>>2 ;
                    for( unsigned int i =0 ; i <size ; i++ ) {  
                        buf[1] = *pChar++;
                        buf[2] = *pChar++;
                        buf[3] = *pChar++;
                        buf[4] = *pChar++;
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 5,  NULL );               
                        if (state != SUCCESS) {
                            return I2C_BUS_ERR;                   
                        }                      
                        //OSTimeDly(1); 
                    }
                 break;   
             
                case  ATTRI_I2C_FM1388_LOAD_EEPROM : //FM1388 EEPROM  
                    //I2C_Mixer( I2C_MIX_UIF_M );
                    size = p_raw_write->data_len / EEPROM_ALLOWED_DATA_PACK_SIZE ;       
                    for( i = 0 ; i < size ; i++ ) { 
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, p_raw_write->pdata, EEPROM_ALLOWED_DATA_PACK_SIZE, NULL );       
                        if (state != SUCCESS) {
                            return I2C_BUS_ERR;                  
                        } 
                        p_raw_write->pdata += EEPROM_ALLOWED_DATA_PACK_SIZE;                    
                        OSTimeDly(5);  //EEPROM page write wait time = 5ms                   
                    }
                    size = p_raw_write->data_len % EEPROM_ALLOWED_DATA_PACK_SIZE ; 
                    if( size ) {
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, p_raw_write->pdata, size, NULL );       
                        if (state != SUCCESS) {
                            return I2C_BUS_ERR;
                        }
                     }
                break;
            
                case ATTRI_I2C_FM1388_LOAD_CODE :                      
                    //I2C_Mixer( I2C_MIX_UIF_M );
                    size = p_raw_write->data_len % FM1388_I2C_DATA_PACK_SIZE ; 
                    if( size ) {                       
                        return I2C_BUS_ERR;                      
                    }
                    size = p_raw_write->data_len / FM1388_I2C_DATA_PACK_SIZE - 1;
                    mem_addr = *(unsigned int*)pChar ;
                    pChar+= 4;
                    for( i = 0 ; i < size ; i++ ) {                         
                        buf[0] = 1;
                        buf[1] =  (unsigned char)(mem_addr>>8);
                        buf[2] =  (unsigned char)(mem_addr>>0);
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                 
                        if ( state != SUCCESS ) {
                            return I2C_BUS_ERR;                  
                        }
                        buf[0] = 2;
                        buf[1] =  (unsigned char)(mem_addr>>24);
                        buf[2] =  (unsigned char)(mem_addr>>16);
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                 
                        if ( state != SUCCESS ) {
                            return I2C_BUS_ERR;                  
                        } 
                        buf[0] = 3;
                        buf[2] = *pChar++;
                        buf[1] = *pChar++;
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                 
                        if ( state != SUCCESS ) {
                            return I2C_BUS_ERR;                  
                        } 
                        buf[0] = 4;
                        buf[2] = *pChar++;
                        buf[1] = *pChar++;
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                 
                        if ( state != SUCCESS ) {
                            return I2C_BUS_ERR;                  
                        } 
                        buf[0] = 0;
                        buf[1] = 0;
                        buf[2] = 3; //32bit write
                        state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 3,  NULL );                 
                        if ( state != SUCCESS ) {
                            return I2C_BUS_ERR;                  
                        } 
                        mem_addr += 4 ;
                    }   
                break;
                case ATTRI_I2C_IM205 :  //iM205                         
                    state =  I2C_GPIO_Write_iM205 ( p_raw_write->dev_addr>>1, *pChar, *(pChar+1) );                                
                    if ( state != SUCCESS ) {
                        return I2C_BUS_ERR; 
                    }  
                break;
             
                case ATTRI_I2C_IM501_LOAD_CODE_IRAM : //iM501 IRAM  
                     //I2C_Mixer( I2C_MIX_UIF_S );
                     //OSTimeDly(1);            
                     buf[0] = 0x4A;  //Command byte, write I2C host register with one address byte and two data bytes
                     buf[1] = 0x08;  //address, byte counter                 
                     buf[2] = (p_raw_write->data_len - 3 - 4) & 0xFF;
                     buf[3] = ((p_raw_write->data_len - 3 - 4) >> 8) & 0xFF; 
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 4,  NULL );                 
                     if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;                  
                     }
                     buf[0] = 0x0D;  
                     
                     buf[1] = *pChar++; //Addrss LSB
                     buf[2] = *pChar++; 
                     buf[3] = *pChar++; //Addrss MSB  
                     
                     buf[4] = *pChar++; //data LSB
                     buf[5] = *pChar++;
                     buf[6] = *pChar++;
                     buf[7] = *pChar++; //data MSB
                     
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 8,  NULL );                  
                     if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;  
                     }                    
                     buf[0] = 0x88; //data only        
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 1,  NULL );                 
                     if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;  
                     }
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, pChar, p_raw_write->data_len - 3 - 4,  NULL );               
                     if (state != SUCCESS) {
                        return I2C_BUS_ERR;                  
                     }                      
                     //OSTimeDly(1);   
                 break;
                
                 case ATTRI_I2C_IM501_LOAD_CODE_DRAM : //iM501 DRAM 
                     //I2C_Mixer( I2C_MIX_UIF_S );
                     //OSTimeDly(1);            
                     buf[0] = 0x4A;  //Command byte, write I2C host register with one address byte and two data bytes
                     buf[1] = 0x08;  //address, byte counter                 
                     buf[2] = (p_raw_write->data_len - 3 - 2 ) & 0xFF;
                     buf[3] = ((p_raw_write->data_len - 3 - 2 ) >> 8) & 0xFF;
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 4,  NULL );                 
                     if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;  
                     }
                     buf[0] = 0x2B;
                     
                     buf[1] = *pChar++; //Addrss LSB
                     buf[2] = *pChar++;  
                     buf[3] = *pChar++; //Addrss MSB 
                     
                     buf[4] = *pChar++; //data LSB
                     buf[5] = *pChar++; //data MSB 
                     
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 6,  NULL );                  
                     if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;   
                     }                    
                     buf[0] = 0xA8; //data only        
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, buf, 1,  NULL );                 
                     if ( state != SUCCESS ) {
                        return I2C_BUS_ERR;  
                     }
                     state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, pChar, p_raw_write->data_len - 3 - 2,  NULL );               
                     if (state != SUCCESS) {
                        return I2C_BUS_ERR;                   
                     }                      
                     //OSTimeDly(1);  
                 break;
                 
                 default:// Normal case  
                    //I2C_Mixer( I2C_MIX_UIF_S );
                    state =  TWID_WriteBuffer_API( p_raw_write->dev_addr>>1, 0, 0, p_raw_write->pdata, p_raw_write->data_len, NULL );       
                    if (state != SUCCESS) {
                        return I2C_BUS_ERR;                  
                    }
                    //delay_us(40);
                 break; 
            }
        break;
        
        
        case UIF_TYPE_I2C_GPIO :
               state =  I2C_GPIO_Write (p_raw_write->dev_addr>>1, p_raw_write->pdata, p_raw_write->data_len) ;
               if (state != SUCCESS) {
                   APP_TRACE_INFO(("\r\nI2C_GPIO_WRITE write err = %d\r\n",state));
                   return I2C_BUS_ERR;                  
               }
        break;
        //////////////////////////////////////////////////////////
        
        case UIF_TYPE_SPI:
              //FM1388 
              if( Global_SPI_Rec_Start == 1 ) {
                  err = SPI_BUS_ERR;
                  return err;
              }
              if( Global_UIF_Setting[ UIF_TYPE_SPI - 1 ].attribute == ATTRI_SPI_FM1388_LOAD_CODE ) {
                  size = p_raw_write->data_len / FM1388_ALLOWED_DATA_PACK_SIZE ; 
                  APP_TRACE_INFO(("\r\nUIF_TYPE_SPI 1388 Load code:[ %d ] ",i));
                  for( i = 0 ; i < size ; i++ ) {
                      APP_TRACE_INFO((">"));
//                      state =  SPI_WriteBuffer_API( p_raw_write->pdata, FM1388_ALLOWED_DATA_PACK_SIZE ); 
                      state = SPI_WriteBuffer_API( &fm1388,p_raw_write->pdata, FM1388_ALLOWED_DATA_PACK_SIZE );
                      if (state != SUCCESS) {
                          APP_TRACE_INFO(("\r\nUIF_TYPE_SPI 1388 error: %d",state));
                          err = SPI_BUS_ERR;
                          return err;
                      }
                      p_raw_write->pdata += FM1388_ALLOWED_DATA_PACK_SIZE;                      
                      //OSTimeDly(5); 
                  }
                  size = p_raw_write->data_len  % FM1388_ALLOWED_DATA_PACK_SIZE ;
                  if( size ) {
                      //state = SPI_WriteBuffer_API( p_raw_write->pdata, size); 
                      state = SPI_WriteBuffer_API( &fm1388,p_raw_write->pdata, size );
                      if (state != SUCCESS) {
                          err = SPI_BUS_ERR;
//                          assert( 0 );
                      }
                  }
                  
              } else {
                  //state = SPI_WriteBuffer_API( p_raw_write->pdata, p_raw_write->data_len ); 
                  state = SPI_WriteBuffer_API( &fm1388,p_raw_write->pdata, p_raw_write->data_len ); 
                  if (state != SUCCESS) {
                      err = SPI_BUS_ERR;
                  }
              } 
        break;
        
        case UIF_TYPE_GPIO:
             err = GPIOPIN_Set( p_raw_write->dev_addr, *(p_raw_write->pdata) );

        break; 
        
        case UIF_TYPE_GPIO_CLK:
             err = CS_GPIO_Write( *(p_raw_write->pdata), *(p_raw_write->pdata +1) );

        break;
        
        default:
             err = UIF_TYPE_NOT_SUPPORT_ERR ;             
        break;
    }
        
    return err;   
    
    
}

/*
*********************************************************************************************************
*                                           Raw_Read()
*
* Description : read data, and some pre data can be write just before read .
* Argument(s) :  p_raw_read : pointer to RAW_READ type address
*                
*               
* Return(s)   : None.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Raw_Read( RAW_READ *p_raw_read )
{  
    
    unsigned char err, state;
    unsigned char *pbuf;
    
    err  = NO_ERR;
    pbuf = (unsigned char *)Reg_RW_Data; //global usage
    
    memset( ( void * )Reg_RW_Data , 0xff , sizeof( Reg_RW_Data ) );
    
//    APP_TRACE_INFO(("\r\nRaw_Read:  if_type=%d, dev_addr=0x%02X, data_len_read=%d, data_len_write=%d ",\
//                         p_raw_read->if_type,p_raw_read->dev_addr,p_raw_read->data_len_read,p_raw_read->data_len_write ));
//    
//    Dump_Data( p_raw_read->pdata_write,  p_raw_read->data_len_write );
     
    switch( p_raw_read->if_type ) {
        
        case UIF_TYPE_I2C:        
              //I2C_Mixer( I2C_MIX_UIF_S );
              if( Global_UIF_Setting[p_raw_read->if_type - 1 ].attribute == ATTRI_I2C_IM205 ) {                  
                  state = I2C_GPIO_Read_iM205(p_raw_read->dev_addr>>1, *(p_raw_read->pdata_write), pbuf); 
                  if (state != SUCCESS) {
                      err = I2C_BUS_ERR;
                      break;
                  }
                  
              } else {
                  state =  TWID_WriteBuffer_API( p_raw_read->dev_addr>>1,
                                       0, 
                                       0, 
                                       p_raw_read->pdata_write, 
                                       p_raw_read->data_len_write, 
                                       NULL );     
                  if (state != SUCCESS) {
                      err = I2C_BUS_ERR;
                      break;
                  } 
              
                  state =  TWID_ReadBuffer_API( p_raw_read->dev_addr>>1,
                                      0, 
                                      0, 
                                      pbuf, 
                                      p_raw_read->data_len_read, 
                                      NULL );     
                  if (state != SUCCESS) {
                      err = I2C_BUS_ERR;
                      break;
                  } 
                 
              }
        break;
        
        case UIF_TYPE_I2C_GPIO :
               state =  I2C_GPIO_Write (p_raw_read->dev_addr>>1, p_raw_read->pdata_write, p_raw_read->data_len_write) ;
               if (state != SUCCESS) {
                   APP_TRACE_INFO(("\r\nI2C_GPIO_READ write err = %d\r\n",state));
                   return I2C_BUS_ERR;                  
               }
               state =  I2C_GPIO_Read (p_raw_read->dev_addr>>1, pbuf, p_raw_read->data_len_read) ;
               if (state != SUCCESS) {
                   APP_TRACE_INFO(("\r\nI2C_GPIO_READ read err = %d\r\n",state));
                   return I2C_BUS_ERR;                  
               }
        break;
        
        case UIF_TYPE_SPI:                   
            if( Global_SPI_Rec_Start == 1 ) {
                err = SPI_BUS_ERR;
                return err;
            }

            state =  SPI_WriteReadBuffer_API(  &fm1388,
                                               pbuf, 
                                               p_raw_read->pdata_write, 
                                               p_raw_read->data_len_read , 
                                               p_raw_read->data_len_write);// +1 fix SPI bug
             
              if (state != SUCCESS) {
                  err = SPI_BUS_ERR;
                  APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d",state));
                  break;
              }    
#if 0   
              pbuf = pbuf + 1; //fix bug  --no this bug in AB04
#endif
            
//              for(unsigned int i=0; i<p_raw_read->data_len_read;i++){
//                  *(pbuf+i)= i/64;
//              }
    
        break;
               
        case UIF_TYPE_GPIO:
              err = GPIOPIN_Get( p_raw_read->dev_addr, pbuf );
        break;
        
        default:
             err = UIF_TYPE_NOT_SUPPORT_ERR ;             
        break;
        
    }       
    
    if( err != NO_ERR ) {
        APP_TRACE_INFO(("\r\nRaw_Read() failed: %d", err));
        
    } else {
        p_raw_read->pdata_read = pbuf ; //save data pointer
        Dump_Data( pbuf,  p_raw_read->data_len_read ); 
        
    }
    
    return err ;      
    
}


unsigned char GPIO_Session( GPIO_SESSION *p_gpio_session )
{
    
   unsigned char err;
   unsigned char i;
   
   err = 0;
   
//   for( i=0; i<p_gpio_session->gpio_num; i++ )   {
//      GPIOPIN_Set( GET_BYTE_HIGH_4BIT(p_gpio_session->gpio_value[i]), GET_BYTE_LOW_4BIT(p_gpio_session->gpio_value[i]) );
//      if( p_gpio_session->delay_us[i] < 2000 ) {
//          delay_us( p_gpio_session->delay_us[i] );
//      }else{
//          OSTimeDly( p_gpio_session->delay_us[i] );
//      }
//   } 
   
   return err;
   
}




/*
*********************************************************************************************************
*                                       AB_POST()
*
* Description : Audio bridge Power-On-Self-Test use.
*
* Argument(s) : None.
* Return(s)   : error number.
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char AB_POST( void )
{
    unsigned char  err;
    
    APP_TRACE_INFO(("\r\nStart Audio Bridge POST :\r\n"));

    APP_TRACE_INFO(("\r\n1. FPGA... \r\n"));
    err = FPGA_POST_Setup();
    if( err != NO_ERR ) {
        Global_Bridge_POST = FPGA_CFG_ERR;
        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
        return Global_Bridge_POST;
    } else {
        APP_TRACE_INFO(("\r\n---OK\r\n"));
    }
    
    APP_TRACE_INFO(("\r\n2. CODEC... \r\n"));       
    err = aic3204_init_default();  
    if( err != NO_ERR ) {
        Global_Bridge_POST = POST_ERR_CODEC;
        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
        return Global_Bridge_POST;
    } else {
        APP_TRACE_INFO(("\r\n---OK\r\n"));
    }

    APP_TRACE_INFO(("\r\n3. FM36 DSP... \r\n"));
    I2C_Switcher( I2C_SWITCH_FM36 ); 
    err = Init_FM36_AB03( SAMPLE_RATE_DEFAULT, 0, 1, 0, SAMPLE_LENGTH_DEFAULT, 1, 1  ); //force reset FM36, Lin from SP1.Slot0 
    if( err != NO_ERR ) {
        Global_Bridge_POST = POST_ERR_FM36;
        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
        return Global_Bridge_POST;
    } else {
        APP_TRACE_INFO(("\r\n---OK\r\n"));
    }

}
/*
*********************************************************************************************************
*                                       Set_Volume()
*
* Description : Set DMIC PGA gain, LOUT and SPKOUT attenuation gain at the same time
*
* Argument(s) : pdata : pointer to SET_VOLUME structure data
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Set_Volume(  SET_VOLUME *pdata )
{
    unsigned char  err = 0 ;
    /*
    APP_TRACE_INFO(( "Set Volume :: " ));
    if( pdata->mic == SET_VOLUME_MUTE ) {
        APP_TRACE_INFO(( "Mute MIC :  " ));
    } else {
        APP_TRACE_INFO(( "Mic_Gain = %d dB :  ", pdata->mic ));
    }

    if( pdata->lout == SET_VOLUME_MUTE ) {
        APP_TRACE_INFO(( "Mute LOUT :  " ));
    } else {
        APP_TRACE_INFO(( "LOUT_Gain = -%d.%d dB :  ", pdata->lout/10, pdata->lout%10 ));
    }

    if( pdata->spk == SET_VOLUME_MUTE ) {
        APP_TRACE_INFO(( "Mute SPK :  " ));
    } else {
        APP_TRACE_INFO(( "SPK_Gain = -%d.%d dB :  ", pdata->spk/10, pdata->spk%10 ));
    }

    //APP_TRACE_INFO(("Set Volume : Mic_Gain[%d]dB, LOUT_Gain[-%d.%d]dB, SPKOUT_Gain[-%d.%d]dB : ",
    //                     pdata->mic, pdata->lout/10, pdata->lout%10, pdata->spk/10, pdata->spk%10 ));
    //APP_TRACE_INFO(("\r\n%6.6f, %6.6f\r\n",2.31,0.005));

    I2C_Mixer(I2C_MIX_FM36_CODEC);

    err = DMIC_PGA_Control( pdata->mic );
    //APP_TRACE_INFO((" %s [0x%X]\r\n", err == OS_ERR_NONE ? "OK" : "FAIL" , err ));
    if( OS_ERR_NONE != err ) {
        APP_TRACE_INFO(( "FAIL [0x%X]\r\n", err ));
        I2C_Mixer(I2C_MIX_UIF_S);
        return err;
    }
    err = CODEC_Set_Volume( pdata->spk, pdata->lout, pdata->lin );
    if( OS_ERR_NONE != err ) {
        APP_TRACE_INFO(( "FAIL [0x%X]\r\n", err ));
        I2C_Mixer(I2C_MIX_UIF_S);
        return err;
    }
    APP_TRACE_INFO(( "OK\r\n" ));

    I2C_Mixer(I2C_MIX_UIF_S);
    */
    return err;
}

