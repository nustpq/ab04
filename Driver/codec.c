/*
*********************************************************************************************************
*                                           UIF BOARD DRIVER PACKAGE
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
*                                          CODEC TLV320AIC3204 Setup
*
*                                          Atmel AT91SAM3U4C
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : codec.c
* Version       : V2.0.0
* Programmer(s) : PQ
* Porting       : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <ucos_ii.h>
#include "bsp.h"

#include "codec.h"
#include "noah_cmd.h"

#include "uif_twi.h"
#include "uif_object.h"

static const DataSource *pSource; 

static void Pin_Reset_Codec( void )
{

//    PIO_Clear(&pinsGpios[12]);
    UIF_Misc_Off ( CODEC1_RST );  
    OSTimeDly(10) ;
//    PIO_Set(&pinsGpios[12]);
    UIF_Misc_On ( CODEC1_RST );
    OSTimeDly(10) ;
     
}

void enable_PA( bool en )
{
  if( en )
     UIF_Misc_On ( PA_SHUTDOWN );
  else
     UIF_Misc_Off ( PA_SHUTDOWN );
  
}

uint8_t Codec_Read(const DataSource *pSource,uint8_t dev_addr,uint8_t reg,uint8_t *pVal)
{
    TWI_CFG *twi_option = ( TWI_CFG * )pSource->privateData;
    
    uint8_t state = 0;
    
    twi_option->address = dev_addr >> 1 ;
    twi_option->iaddress = reg;
    twi_option->isize = 1;
        
//    state =  TWID_Read( dev_addr>>1, reg, 1, pVal++,1, NULL) ;
    state = pSource->buffer_read( ( void * )pSource , pVal++, 1 );
    return state ;
}


uint8_t Codec_Write(const DataSource *pSource,uint8_t dev_addr,uint8_t reg,uint8_t data)
{      
    uint8_t buf[] = {  data };     
    uint8_t state;
    
    TWI_CFG *twi_option = ( TWI_CFG * )pSource->privateData;
    
    twi_option->address = dev_addr >> 1 ;
    twi_option->iaddress = reg;
    twi_option->isize = 1;
      
//    state =  TWID_Write( dev_addr>>1, reg, 1 , buf, sizeof(buf), NULL);     
//    state = twi1_write( ( void * )pSource , buf, sizeof( buf ) );
    state = pSource->buffer_write( ( void * )pSource, buf , sizeof( buf ) );
    return state ;
}


uint8_t Codec_Read_SPI(uint8_t dev_addr,uint8_t reg,uint8_t *pVal)
{
    uint8_t state = 0;   
    //state =  TWID_Read( dev_addr>>1, reg, 1, pVal++,1, NULL) ;
    return(state);
}


uint8_t Codec_Write_SPI(uint8_t dev_addr,uint8_t reg,uint8_t data)
{      
    uint8_t buf[] = { dev_addr<<1, reg, data };     
    uint8_t state;    
    
    state = SPI_WriteBuffer_API( buf, 3);     
  
    
    return state ;
    
}


//i2c_channel = 0 ~ 7
uint8_t Codec_Mixer(const DataSource *pSource,uint8_t i2c_channel )
{    
     uint8_t buf   ;
     uint8_t state ;
     
     state = 0xe0 ; 
     
     TWI_CFG *twi_option = ( TWI_CFG * )pSource->privateData;
     
     twi_option->address = PCA9548A_ADDRESS >> 1 ;
     twi_option->iaddress = 0;
     twi_option->isize = 0;
     
     if( i2c_channel <= 3 )  
     {//PCA9548A,
        buf   = 0x01<<i2c_channel ; //select i2c_channel
//        state = TWID_Write( PCA9548A_ADDRESS>>1, 0, 0, &buf, 1, NULL); 
        state = twi1_write( ( void * )pSource, &buf , 1 );
     }
     
     return state ;
}



uint8_t Set_Codec(const DataSource *pSource,uint8_t codec_control_type, uint8_t size_para, uint8_t *pdata)
{
    uint8_t i, state = 0 ;
    for(i = 0; i< size_para ; i++)
    {
      if(codec_control_type == 0 ) {
        state = Codec_Write(pSource,CODEC_ADDRESS,*(pdata+i*2),*(pdata+i*2+1));
      } else {
        state = Codec_Write_SPI(AD1938_ADDRESS,*(pdata+i*2),*(pdata+i*2+1));
      }
        if( state != 0 ) break;
    }
    return state;
  
}


uint8_t Get_Codec(const DataSource *pSource,uint8_t codec_control_type, uint8_t reg, uint8_t *pdata)
{
    uint8_t state = 0 ;
    
    if(codec_control_type == 0 ) {
        state = Codec_Read( pSource,CODEC_ADDRESS, reg, pdata) ;
    } else {
        state = Codec_Read_SPI( CODEC_ADDRESS, reg, pdata) ;
    }
    
    return state;
  
}


uint8_t I2CWrite_Codec( const DataSource *pSource,uint8_t reg,uint8_t data )
{
    uint8_t err;
    err = Codec_Write( pSource,AD1937_ADDRESS, reg, data);
    return err;
}


uint8_t I2CWrite_Codec_AIC3204( const DataSource *pSource,uint8_t reg,uint8_t data )
{
    uint8_t err;
    err = Codec_Write( pSource,CODEC_ADDRESS, reg, data);
    return err;
}



/******************************      AD1937        ************************************/
// OSC=12.288Mhz
/*
void ALL_POWER_ON(void)
{
    I2CWrite_Codec(DAC_Control0,0x00);  //
    I2CWrite_Codec(ADC_Control0,0x00);  //
}


void ALL_POWER_OFF(void)
{
    I2CWrite_Codec(DAC_Control0,0x01);  //
    I2CWrite_Codec(ADC_Control0,0x01);  //
}


uint8_t Codec_DAC_Attenuation( uint8_t DAC_NAME, uint32_t x10gain )
{
    
    uint8_t err;
    float temp ;
    
    if( x10gain == 1000 ){        
        return 0 ;
    }
    if (x10gain > 945) {//0.375*252=94.5dB         
        err = CODEC_SETVOL_RANGE_ERR;
        APP_TRACE_INFO(("ERR: CODEC Gain Over Range!\r\n"));        
        return err;
    }
    temp = x10gain/(0.375*10); 
    err = I2CWrite_Codec(DAC_NAME,(uint8_t)temp);
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }
    APP_TRACE_DBG(("\r\nSet CODEC REG[%d]=0x%0X",DAC_NAME,(uint8_t)temp)); 
    
    return 0;

}

//uint8_t Codec_DAC_Attenuation( uint8_t DAC_NAME, float gain )
//{
//    
//    uint8_t err;
//    float temp ;
//      
//    if ( gain > 0 ) {         
//        err = 1;
//        return err;
//    }
//    
//    if(gain< -95.25 ) {
//        temp = 255 ;
//        
//    } else {
//        temp = -gain/0.375; 
//        
//    }
//    
//    err = I2CWrite_Codec(DAC_NAME,(uint8_t)temp);
//    
//    return err;
//    
//}

unsigned short SR_Support[] =         {
                                            //8000,
                                            16000,
                                            24000,
                                            32000,
                                            //44100,
                                            48000
                                            //96000
                                      };

uint8_t Check_SR_Support( uint32_t sample_rate )
{    
   uint32_t i;
   for( i = 0; i<(sizeof(SR_Support)/2); i++ ) {
       if( SR_Support[i] == sample_rate ) {
           return OS_ERR_NONE ; //find the SR
       }
   }   
   return CODEC_SR_NOT_SUPPORT_ERR;  //SR not support
    
}


// Main clock source = 12.288 MHz.   
uint8_t Codec_SetFCLK( uint32_t fclk )
{
    
    uint8_t err;
    uint8_t pll_reg;
    
    switch( fclk )   {
      
        case 8000:  // Need  12.288 / 2 MHz Osc 
            pll_reg = 0x9e;      
            return CODEC_SETFCLK_RANGE_ERR;
            break;            
        case 16000:            
            pll_reg = 0x9e;            
            break;
        case 24000:      
            pll_reg = 0x9c;            
            break;
        case 32000:            
            pll_reg = 0x9a;      
            break;
        case 44100: // Need 11.2896 MHz Osc 
            pll_reg = 0x98;  
            return CODEC_SETFCLK_RANGE_ERR;
            break;
        case 48000:             
            pll_reg = 0x98;            
            break;
        case 96000: // Need 12.288 * 2 MHz Osc       
            pll_reg = 0x98;             
            return CODEC_SETFCLK_RANGE_ERR;
            break;
        case 0 : //power down clock
            pll_reg = 0x01; 
            break;
        default:
            return CODEC_SETFCLK_RANGE_ERR;
            break;
            
    }
    
    err = I2CWrite_Codec( PLL_Control0, pll_reg );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }

    return err;
    
}


uint8_t Codec_SetMode( uint8_t mode )
{
    uint8_t err;
    uint8_t dac_reg0,dac_reg1;
    uint8_t adc_reg1,adc_reg2;
    
    switch( mode )  {
    
        case I2S_MODE:
                 dac_reg0 = 0x00;      //I2S
                 dac_reg1 = 0x70;      //2 channels
                 adc_reg1 = 0x03;      //I2S
                 adc_reg2 = 0x01;      //2 channels
                 break;
        case TDM_MODE:
                 dac_reg0 = 0x40;      //TDM
                 dac_reg1 = 0x74;      // 8 channels
                 adc_reg1 = 0x23;      //TDM
                 adc_reg2 = 0x21;      // 8 channels
                 break;
        case TDM16_MODE:
                 dac_reg0 = 0x40;      //TDM
                 dac_reg1 = 0x76;      // 16 channels
                 adc_reg1 = 0x23;      //TDM
                 adc_reg2 = 0x31;      // 16 channels
                 break;
        default:
                return CODEC_SETMODE_RANGE_ERR;
                break;
    } 
    
    err = I2CWrite_Codec( DAC_Control0, dac_reg0 );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }    
    err = I2CWrite_Codec( DAC_Control1, dac_reg1 );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }
    
    err = I2CWrite_Codec( ADC_Control1, adc_reg1 );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }    
    err = I2CWrite_Codec( ADC_Control2, adc_reg2 );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }

    return err;    
  
}


uint8_t Init_CODEC( uint32_t sample_rate )   
{ 
    uint8_t err;
    uint8_t i;
    static uint32_t sr_saved; 
    
    uint8_t reg_para[][2] = {
        
        { PLL_Control0,0x98 },
        { PLL_Control1,0x00 },  //
        { DAC_Control2,0x18 },  // 16bit    
        { DAC_Mute    ,0xF0 },  // DAC3.4 mute.
        { DAC1L_Volume,0x00 },  // no attenuation on SPK
        { DAC1R_Volume,0x00 },  // no attenuation on SPK
        { DAC2L_Volume,0x00 },  // no attenuation on Lout
        { DAC2R_Volume,0x00 },  // no attenuation on Lout
        { ADC_Control0,0x30 }  //Enable ADC
 //    { DAC3L_Volume,0x00 },  // not used on AB03
 //    { DAC3R_Volume,0x00 },  // not used on AB03 
        
    };
    
    if( sample_rate == sr_saved ) {
        return 0;
    } else {
        sr_saved = sample_rate ;
    }

    for( i = 0; i< sizeof(reg_para)>>1; i++ ) {
      err = I2CWrite_Codec(reg_para[i][0], reg_para[i][1]); 
      if( OS_ERR_NONE != err ) {
          err = CODEC_WR_REG_ERR;
          return err ;
      }
      
    }
    
    err = Codec_SetFCLK( sample_rate );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }    

    err = Codec_SetMode( TDM_MODE );   
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }     

    if( !( Get_Switches() & 0x01 ) ) {
        err = CODEC_LOUT_Small_Gain_En( true ); //attenuation enable
    }
    
    return err;
    
}



*/

uint8_t CODEC_LOUT_Small_Gain_En( bool small_gain )
{
     /*
    uint8_t err;
    uint8_t reg;
    if( small_gain ) {        
        reg = 64;    // 64*0.375=24dB attenuation on Lout     
        APP_TRACE_INFO(("Lout Gain 24dB attenuation: Enabled \r\n"));  
        
    } else {        
        reg = 0;    // 0dB attenuation on Lout        
        APP_TRACE_INFO(("Lout Gain 24dB attenuation: Disabled \r\n"));  
        
    }    
    err = I2CWrite_Codec( DAC2L_Volume, reg );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }    
    
    err = I2CWrite_Codec( DAC2R_Volume, reg );
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    }

    return err; 
    */
     return CODEC_FUNC_NOT_SUPPORT;
    
}

uint8_t encode( int8_t value )
{
  int8_t temp;
  if(value >=0){
      return value;
  }
  else{
      temp= ~( abs( value )-1 );
      return temp;
  }
}

uint8_t CODEC_Set_Volume( const DataSource *pSource,float vol_spk, float vol_lout, float vol_lin )
{
    uint8_t err;            
    float temp = 0;
    uint8_t flag=0;
    uint8_t Mic_PGA=0,ADC_GAIN=0;
    
    vol_spk= (vol_spk - (int)vol_spk%5)/10;
    vol_lout=(vol_lout - (int)vol_lout%5)/10;
    vol_lin=(vol_lin - (int)vol_lin%5)/10;
       
    if (vol_lin < -12){
        vol_lin=-12;
    }
    if(vol_lin > 67.5){
        vol_lin=67.5;
    }
    if (vol_spk < -69.5){
        vol_spk=-69.5;  
    }
    if(vol_spk > 53){
         vol_spk=53;
    }
     if (vol_lout < -69.5){
        vol_lout=-69.5;  
    }
    if(vol_lout > 53){
         vol_lout=53;
    }
        
    for( uint8_t i=0 ;i < 95+1 ;i++ ){
      for(int8_t j = -24;j < 40+1;j++){
          temp=i*0.5+j*0.5;
          if(temp==vol_lin){
              Mic_PGA=encode(i);
              ADC_GAIN=encode(j);//now not support negative
              flag=1;
          }
       if(flag==1)break;   
      }
    if(flag==1)break; 
    }
    flag=0;
    err = I2CWrite_Codec_AIC3204(pSource,0,1); //switch to Page1
    if( OS_ERR_NONE != err ) {
        err = CODEC_WR_REG_ERR;
        return err ;
    } 
    I2CWrite_Codec_AIC3204(pSource,59,Mic_PGA); 
    I2CWrite_Codec_AIC3204(pSource,60,Mic_PGA); 
    I2CWrite_Codec_AIC3204(pSource,0,0); //switch to Page0
    I2CWrite_Codec_AIC3204(pSource,83,ADC_GAIN); 
    I2CWrite_Codec_AIC3204(pSource,84,ADC_GAIN); 
   
    int8_t DAC_GAIN=0 ,HPL_GAIN=0 ,LOL_GAIN=0;
    uint8_t flag1=0,flag2=0;
    for(int8_t k=-127;k<48+1;k++){
      for(int8_t m=-6;m<29+1;m++){
          temp=k*0.5+m;
          if(temp==vol_lout && flag1==0){
              DAC_GAIN=encode(k);
              HPL_GAIN=encode(m);
              flag1=1;
          }
          if(temp==vol_spk && flag2==0 ){
              DAC_GAIN=encode(k);
              LOL_GAIN=encode(m);
              flag2=1;
          }
          if(flag1==1 && flag2==1)break;    
      }
      if(flag1==1 && flag2==1)break;    
    }
    flag1=0;
    flag2=0;
    I2CWrite_Codec_AIC3204(pSource,0,0); //switch to Page0
    I2CWrite_Codec_AIC3204(pSource,65,DAC_GAIN); 
    I2CWrite_Codec_AIC3204(pSource,66,DAC_GAIN); 
    I2CWrite_Codec_AIC3204(pSource,0,1); //switch to Page1
    I2CWrite_Codec_AIC3204(pSource,16,HPL_GAIN); 
    I2CWrite_Codec_AIC3204(pSource,17,HPL_GAIN);
    I2CWrite_Codec_AIC3204(pSource,18,LOL_GAIN); 
    I2CWrite_Codec_AIC3204(pSource,19,LOL_GAIN);
    
    return err;
    
}


uint8_t Set_AIC3204_DSP_Offset( uint8_t slot_index ) 
{
    
    uint8_t err;
    
    if( slot_index >6 ) { //slot_index is for line in channels
        return 0x99;
    }      
    err = I2CWrite_Codec_AIC3204( pSource,28, slot_index << 5);
    
    return err;    
    
}

unsigned short CODEC_SUPPORT_SR[] = {
    
    48000, 44100, 32000, 24000, 22050, 16000, 8000
        
};


uint8_t Check_SR_Support( uint32_t sample_rate )
{    
   uint32_t i;
   for( i = 0; i<sizeof(CODEC_SUPPORT_SR)>>1; i++ ) {
       if( CODEC_SUPPORT_SR[i] == sample_rate ) {
           return 0 ; //find the SR
       }
   }   
   return CODEC_SR_NOT_SUPPORT_ERR;  //SR not support
    
}


/*
//CODEC PLL setting based on 24.576MHz MCLK 
unsigned short CODEC_PLL_PARA_TABLE[][14][7] = {
    
    { //mode 0        
          //I2S format
          //BCLK = 16 * 2 * FCLK = 32 * FCLK
	  //parameter for MCLK = 24.576MHz 
          {48000, 44100, 32000, 24000, 22050, 16000, 8000 }, //SR	      
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x83 }, //  REG_NDAC    =
	  {0x82,  0x84,  0x82,  0x84,  0x84,  0x84,  0x81 }, //  REG_MDAC    = --R12
	  {0x80,  0x40,  0x80,  0x80,  0x80,  0x80,  0x00 }, //  REG_DOSR    = --R13-14
	  {0x84,  0x82,  0x84,  0x84,  0x84,  0x84,  0xA0 }, //  REG_BCLK_DIV=  --R30   
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x86 }, //  REG_NADC    = 
	  {0x84,  0x82,  0x84,  0x84,  0x82,  0x84,  0x84 }, //  REG_MADC    = 
	  {0x00,  0x80,  0x40,  0x80,  0x00,  0x80,  0x80 }, //REG_AOSR    =  --R20
	  //{3.072M, 2.8224M, 2.048M, 3.072M, 2.8224M, 2.048M, 1.024M},//     --PDMCLK   :       
	  {0x00,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00 }, //CLK_MUX     =  --Select CODEC_CLKIN
	  {0,     1,     0,     0,     1,    0,     0}, //PLL_EN      = 
	  {0, 	  1,     0,     0,     1,    0,     0}, //PLL_R       = 
	  {0,     2,     0,     0,     2,    0,     0}, //PLL_P       = 
	  {0,     7,     0,     0,     7,    0,     0}, //PLL_J       = 
	  {0,     3500,  0,     0,     3500, 0,     0}  //PLL_D       = 
             
    
    },     

    { //mode 1
          //TDM16 format
          //BCLK = 16 * 8 * FCLK = 128 * FCLK
	  //parameter for MCLK = 24.576MHz 
          {48000, 44100, 32000, 24000, 22050, 16000, 8000 }, //SR	      
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x83 }, //  REG_NDAC    =
	  {0x82,  0x84,  0x82,  0x84,  0x84,  0x84,  0x81 }, //  REG_MDAC    = --R12
	  {0x80,  0x40,  0x80,  0x80,  0x80,  0x80,  0x00 }, //  REG_DOSR    = --R13-14
	  {0x82 , 0x82,  0x82,  0x84,  0x84,  0x84,  0x88 }, //  REG_BCLK_DIV=  --R30   
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x86 }, //  REG_NADC    = 
	  {0x84,  0x82,  0x84 , 0x84,  0x82,  0x84,  0x84 }, //  REG_MADC    = 
	  {0x00,  0x80,  0x40,  0x80,  0x00,  0x80,  0x80 }, //  REG_AOSR    =  --R20
	  //{3.072M, 2.8224M, 2.048M, 3.072M, 2.8224M, 2.048M, 1.024M},//     --PDMCLK   :       
	  {0x00,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00 }, //  CLK_MUX     =  --Select CODEC_CLKIN
	  {0,     1,     0,     0,     1,    0,     0}, //PLL_EN      = 
	  {0, 	  1,     0,     0,     1,    0,     0}, //PLL_R       = 
	  {0,     2,     0,     0,     2,    0,     0}, //PLL_P       = 
	  {0,     7,     0,     0,     7,    0,     0}, //PLL_J       = 
	  {0,     3500,  0,     0,     3500, 0,     0}  //PLL_D       = 
             
    
    },
    
    { //mode 2
          //TDM32 format
          //BCLK = 32 * 8 * FCLK = 256 * FCLK
	  //parameter for MCLK = 24.576MHz 
          {48000, 44100, 32000, 24000, 22050, 16000, 8000 }, //SR	      
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x83 }, //  REG_NDAC    =
	  {0x82,  0x84,  0x82,  0x84,  0x84,  0x84,  0x81 }, //  REG_MDAC    = --R12
	  {0x80,  0x40,  0x80,  0x80,  0x80,  0x80,  0x00 }, //  REG_DOSR    = --R13-14
	  {0x81,  0x81,  0x81,  0x82,  0x82,  0x82,  0x84 }, //  REG_BCLK_DIV=  --R30   
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x86 }, //  REG_NADC    = 
	  {0x84,  0x82,  0x84,  0x84,  0x82,  0x84,  0x84 }, //  REG_MADC    = 
	  {0x00,  0x80,  0x40,  0x80,  0x00,  0x80,  0x80 }, //REG_AOSR    =  --R20
	  //{3.072M, 2.8224M, 2.048M, 3.072M, 2.8224M, 2.048M, 1.024M},//     --PDMCLK   :       
	  {0x00,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00 }, //CLK_MUX     =  --Select CODEC_CLKIN
	  {0,     1,     0,     0,     1,    0,     0}, //PLL_EN      = 
	  {0, 	  1,     0,     0,     1,    0,     0}, //PLL_R       = 
	  {0,     2,     0,     0,     2,    0,     0}, //PLL_P       = 
	  {0,     7,     0,     0,     7,    0,     0}, //PLL_J       = 
	  {0,     3500,  0,     0,     3500, 0,     0}  //PLL_D       = 
    },
    
    { //mode 3
          //I2S32 format
          //BCLK = 32 * 2 * FCLK = 64 * FCLK
	  //parameter for MCLK = 24.576MHz 
          {48000, 44100, 32000, 24000, 22050, 16000, 8000 }, //SR	      
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x83 }, //  REG_NDAC    =
	  {0x82,  0x84,  0x82,  0x84,  0x84,  0x84,  0x81 }, //  REG_MDAC    = --R12
	  {0x80,  0x40,  0x80,  0x80,  0x80,  0x80,  0x00 }, //  REG_DOSR    = --R13-14
	  {0x84,  0x88,  0x88,  0x90,  0x90,  0x90,  0xA0 }, //  REG_BCLK_DIV=  --R30   
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x86 }, //  REG_NADC    = 
	  {0x84,  0x82,  0x84,  0x84,  0x82,  0x84,  0x84 }, //  REG_MADC    = 
	  {0x00,  0x80,  0x40,  0x80,  0x00,  0x80,  0x80 }, //REG_AOSR    =  --R20
	  //{3.072M, 2.8224M, 2.048M, 3.072M, 2.8224M, 2.048M, 1.024M},//     --PDMCLK   :       
	  {0x00,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00 }, //CLK_MUX     =  --Select CODEC_CLKIN R4
	  {0,     1,     0,     0,     1,    0,     0}, //PLL_EN      = 
	  {0, 	  1,     0,     0,     1,    0,     0}, //PLL_R       = 
	  {0,     2,     0,     0,     2,    0,     0}, //PLL_P       = 
	  {0,     7,     0,     0,     7,    0,     0}, //PLL_J       = 
	  {0,     3500,  0,     0,     3500, 0,     0}  //PLL_D       = 
    },
       
    {//mode 4
          //TDM 4slot 32bit
         //BCLK = 32 * 4 * FCLK = 128 * FCLK
	 //parameter for MCLK = 24.576MHz
          {48000, 44100, 32000, 24000, 22050, 16000, 8000 }, //SR	      
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x83 }, //  REG_NDAC    =
	  {0x82,  0x84,  0x82,  0x84,  0x84,  0x84,  0x81 }, //  REG_MDAC    = --R12
	  {0x80,  0x40,  0x80,  0x80,  0x80,  0x80,  0x00 }, //  REG_DOSR    = --R13-14
	  {0x82,  0x82,  0x82,  0x84,  0x84,  0x84,  0x88 }, //  REG_BCLK_DIV=  --R30   
	  {0x82,  0x88,  0x83,  0x82,  0x88,  0x83,  0x86 }, //  REG_NADC    = 
	  {0x84,  0x82,  0x84,  0x84,  0x82,  0x84,  0x84 }, //  REG_MADC    = 
	  {0x00,  0x80,  0x40,  0x80,  0x00,  0x80,  0x80 }, //  REG_AOSR    =  --R20
	  //{3.072M, 2.8224M, 2.048M, 3.072M, 2.8224M, 2.048M, 1.024M},//     --PDMCLK   :       
	  {0x00,  0x03,  0x00,  0x00,  0x03,  0x00,  0x00 }, //CLK_MUX     =  --Select CODEC_CLKIN
	  {0,     1,     0,     0,     1,    0,     0}, //PLL_EN      = 
	  {0, 	  1,     0,     0,     1,    0,     0}, //PLL_R       = 
	  {0,     2,     0,     0,     2,    0,     0}, //PLL_P       = 
	  {0,     7,     0,     0,     7,    0,     0}, //PLL_J       = 
	  {0,     3500,  0,     0,     3500, 0,     0}  //PLL_D       = 
          
    }
    
};


uint8_t audio_interface[] = { 
          0x0c,   //I2S mode,16bit,master
          0x00,   //I2S mode,16bit,Slave
          0x4c,   //DSP mode,16bit,master
          0x40,   //DSP mode,16bit,Slave
          0xcc,   //LJF mode,16bit,master
          0X8c,   //RJF mode,16bit,master
    
};

unsigned short BCLK_SOURCE[] = { //reg29 0:DAC_CLK  1: DAC_MOD_CLK , D3 = 1 for bclk invert
          0x9, //I2S 16bit format
          0x8, //TDM 16bit format
          0x8, //TDM 32bit format
          0x8, //I2S 32bit format
          0x8  //TDM 32bit 4slot format
};
*/          


uint8_t config_aic3204[][2] = {    
       
		      0,0x00, //page0 

		      //Software reset codec
		      1,0X01, 
		      
//		      //SET PLL == MCLK*R*J.D/P   mclk == 12.288MHz;
//		      4,CLK_MUX[SR_Index], 
//		      5,PLL_EN[SR_Index]*128 +PLL_P[SR_Index]*16 + PLL_R[SR_Index],
//		      6,PLL_J[SR_Index],
//		      7,math.floor( PLL_D[SR_Index]/256),
//		      8,PLL_D[SR_Index]%256,		      
//		   
//		      //Set DAC_fs == PLL/NDAC*MDAC*DOSR		  
//		      11,REG_NDAC[SR_Index],  //NDAC=3    
//		      12,REG_MDAC[SR_Index],  //MDAC=4
//		      //DOSR=128
//		      13,math.floor(REG_DOSR[SR_Index]/256), 
//		      14,REG_DOSR[SR_Index]%256,  
//		      
//		      //Set ADC_fs == PLL/NADC*MADC*AOSR   SET PDMCLK=ADC_MOD_CLK = 2.048MHz
//		      18,REG_NADC[SR_Index],  //NADC=3
//		      19,REG_MADC[SR_Index],  //MADC=4
//		      //AOSR=128
//		      20,REG_AOSR[SR_Index],  
//		      //if master mode,reg20,reg30 is needed.
//
//		      //BDIV_CLKIN Multiplexer Control			      	
//		      29,BCLK_SOURCE,           // ADC2DAC_ROUTED is not rounted  ; 
////		      29,0X10+BCLK_SOURCE,    // ADC2DAC_ROUTED	
//		      30,REG_BCLK_DIV[SR_Index], //0X84,  //bclk=bdiv_clkin/4


		      //SET interface mode(I2S,PCM,Left,right)
		      27,0X0c,   //I2S mode,16bit,master
		      //27,0X00,   //I2S mode,16bit,Slave
		      //27,0X4c,   //DSP mode,16bit,master
		      //27,0Xcc,   //LJF mode,16bit,master
		      //27,0X8c,   //RJF mode,16bit,master
//		      27, HS,
//		      //Data offset
		      28,0X00,    //data offset == 0'bclk for I2S Mode, there have a cycle delay in I2S mode itself
//		      //28,0X01,    //data offset == 1'bclk for DSP Mode
		      32,0X00, 
		      33,0X4d, 
		      34,0X20, 
		      53,0X02,  	//Dout is pin5
		      54,0X02,  	//pin4 is i2s data input   	 	
		         
			  //-set DAC channels
		      63,0xE8, //DAC Channel Setup :  0xD4: L2L, R2R; 0xE8: L2R, R2L
		      64,0X00, // 	 
		      65,( 0X100+2*( 0 ) )%0x100, //DAC Volume L set 0 dB  : [-63.5,+24] @ 0.5dB  //20
		      66,( 0X100+2*( 0 ) )%0x100, //DAC Volume R set 0 dB  : [-63.5,+24] @ 0.5dB  //20
		      //-set dmic data pin setting
		      55,0X0e,   // Set MISO as PDM CLK ouput pin
		      56,0X02,   // SCLK pin is enabled		
		      
		      81,0xD0,   // enable ADC and set SCLK as PDM DATA input pin//////-	    
		      //Dmic clock output(=adc_mod_clk), PDM CLK = ADC_MOD_CLK	        
		      82,0X00,   //ADC Fine gain adjust, 0dB, unmute
		      83,2*( 0 ),   //ADC Volume L set 0dB  : [-12,+20] @ 0.5dB   D?¨®¨²0dB¦Ì?¨¦¨¨??2?¡ã¡ä?a??¨¤¡ä¡ê?¡À¨¨???¨¦¡¤3
		      84,2*( 0 ),   //ADC Volume R set 0dB  : [-12,+20] @ 0.5dB   D?¨®¨²0dB¦Ì?¨¦¨¨??2?¡ã¡ä?a??¨¤¡ä¡ê?¡À¨¨???¨¦¡¤3


		      0,0X01,  //page1//////////////////////////
//		          
//		      //-set power
		      1, 0x08,  //disconnect AVDD and DVDD		 
		      2, 0X01,  //enable Master Analog Power Control		
		      3, 0X00,  //Set the DAC L PTM mode to PTM_P3/4
		      4, 0X00,  //Set the DAC R PTM mode to PTM_P3/4		      
		      9, 0XFF,  //All HPOUT,LOUT and Mixer Amplifier are Power up  
		      //9, 0x3C,
		      10,0X00,  //Set the Input Common Mode to 0.9V and Output Common Modefor Headphone to Input Common Mode
	              20,0X00,  //headphone driver startup

		      //-set route settings
		      //CODEC LO to FL124 LIN, single ended
	              12,0X08, //HPL route on
		      13,0X08, //HPR route on
		      14,0X08, //LOL route on
		      15,0X08, //LOR route on
		      
		      //Analog input mixer settings
		      52,0X40, // IN1L to L_MICPGA 
		      54,0X40, // CM1L to L_MICPGA 
		      55,0X40, // IN1R to R_MICPGA 
		      57,0X40, // CM1R to R_MICPGA 	      
		         
		      //-set DAC output gains
		      16,( 0X40+( 0 ) )% 0x50 ,  //HPL 0 db gain :  [-6,+29] @ 1dB
		      17,( 0X40+( 0 ) )% 0x50 ,  //HPR 0 db gain :  [-6,+29] @ 1dB
		      18,( 0X40+( 20 ) )% 0x50 ,  //LOL 0 db gain :  [-6,+29] @ 1dB   //29
		      19,( 0X40+( 20 ) )% 0x50 ,  //LOR 0 db gain :  [-6,+29] @ 1dB	 //29	      		       
		      //-set MIC PGA Gain
		      59,0X00,  //L_MICPGA 0db gain
		      60,0X00,  //R_MICPGA 0db gain	

		     	    
};



uint8_t Generate_FCLK( uint32_t codec_clkin ,uint32_t sr ,uint8_t NDAC ,uint8_t MDAC, uint32_t DOSR )
{
    uint32_t temp;
    
    temp= codec_clkin*1000/NDAC/MDAC/DOSR;
    if(temp== sr){
        return 1 ;
    }
    return 0;
    
}

uint8_t Generate_BCLK( uint32_t codec_clkin ,uint32_t BCLK, uint8_t NDAC, uint8_t BCLK_N_divider )
{
    uint32_t temp;
    
    temp= codec_clkin*1000/NDAC/BCLK_N_divider;
    if(temp== BCLK){
        return 1;
    }
    return 0;
    
}

uint8_t Set_Codec_PLL( const DataSource *pSource,uint32_t sr, uint8_t sample_length, uint8_t slot_num, uint8_t bclk_polarity )
{
    uint8_t err,i;
    uint8_t flag=0;
    uint8_t NDAC ,MDAC, NADC, MADC;
    uint32_t  DOSR ;
    uint8_t DOSR_H,DOSR_L;
    uint8_t BCLK_N_divider;
    uint32_t  BCLK;
    
    uint32_t codec_clkin=73728;
    uint8_t codec_para[6][2];
    
    uint8_t codec_para1[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0xC3 , // PLL power up , P=4, R=1
        0x06 ,0x04 , // J=4
        0x07 ,0x00 , // D=0
        0x08 ,0x00 , // D=0
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };//make sure codec_clkin=73.728M  length=16bit 32bit slot_num=2,4,6,8
    uint8_t codec_para2[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0xC1 , // PLL power up , P=4, R=1
        0x06 ,0x03 , // J=3
        0x07 ,0x00 , // D=0
        0x08 ,0x00 , // D=0
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };//make sure codec_clkin=18.432M  length=24bit slot_num=2,4,6,8
    uint8_t codec_para3[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0xC2 , // PLL power up , P=4, R=2
        0x06 ,0x0D , // J=13
        0x07 ,0x04 , // D=1250
        0x08 ,0xE2 , // D=1250
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };
    //make sure codec_clkin=161.28M  length=16,24,32bit slot_num=1,3,5,7
     uint8_t codec_para4[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0x85 , // PLL power up , P=8, R=5
        0x06 ,0x08 , // J=26
        0x07 ,0x0A , // D=2688
        0x08 ,0x80 , // D=2688
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };//make sure codec_clkin=127.008M  length=24bit slot_num=1,2,4,6,8
     uint8_t codec_para5[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0x81 , // PLL power up , P=8, R=1
        0x06 ,0x30 , // J=48
        0x07 ,0x09 , // D=2344
        0x08 ,0x28 , // D=2344
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };//make sure codec_clkin=148.176M  length=16,32bit slot_num=3,5,7
     uint8_t codec_para6[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0x81 , // PLL power up , P=8, R=1
        0x06 ,0x24 , // J=36
        0x07 ,0x06 , // D=1758
        0x08 ,0xDE , // D=1758
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };//make sure codec_clkin=111.132M  length=24bit slot_num=3,5,7
      uint8_t codec_para7[][2] = {
        0x04 ,0x03 , // PLL is codec_clkin
        0x05 ,0x81 , // PLL power up , P=8, R=1
        0x06 ,0x37 , // J=55
        0x07 ,0x04 , // D=1250
        0x08 ,0xE2 , // D=1250
        0x1D ,0x00 , //BCLK source=DAC_CLK
    };//make sure codec_clkin=169.334M  length=16,32bit slot_num=1,2,4,6,8
    
    if((sample_length==16 || sample_length==32) && (slot_num==1 || slot_num==2 || slot_num==4 || slot_num==6 ||  slot_num==8)){
         for( uint8_t i = 0 ; i< sizeof(codec_para1)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para1[i][0];  
            codec_para[i][1]=codec_para1[i][1];            
         }
          codec_clkin=73728;
    }
    else if((sample_length==16 || sample_length==32) && (slot_num==3 || slot_num==5 || slot_num==7))
    {
         for( uint8_t i = 0 ; i< sizeof(codec_para3)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para3[i][0];  
            codec_para[i][1]=codec_para3[i][1]; 
            codec_clkin=161280;
         }
    }
    if ((sample_length==24) && (slot_num==1 || slot_num==2 || slot_num==4 || slot_num==6 ||  slot_num==8)){
         for( uint8_t i = 0 ; i< sizeof(codec_para2)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para2[i][0];  
            codec_para[i][1]=codec_para2[i][1];     
         }
         codec_clkin=18432;
    }
    else if ((sample_length==24) && (slot_num==3 || slot_num==5 || slot_num==7)){
         for( uint8_t i = 0 ; i< sizeof(codec_para3)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para3[i][0];  
            codec_para[i][1]=codec_para3[i][1];     
         }
         codec_clkin=161280;
    }
    
    if((sr == 22050 || sr == 44100) && sample_length==24 && (slot_num==1 || slot_num==2 || slot_num==4 || slot_num==6 ||  slot_num==8) ){
         for( uint8_t i = 0 ; i< sizeof(codec_para4)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para4[i][0];  
            codec_para[i][1]=codec_para4[i][1];     
         }
         codec_clkin=127008;
    }
    else if ((sr == 22050 || sr == 44100) && sample_length==32 && (slot_num==3 || slot_num==5 || slot_num==7)){
         for( uint8_t i = 0 ; i< sizeof(codec_para5)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para5[i][0];  
            codec_para[i][1]=codec_para5[i][1];     
         }
         codec_clkin=148176;
    }
     else if ((sr == 22050 || sr == 44100) && sample_length==24 && (slot_num==3 || slot_num==5 || slot_num==7)){
         for( uint8_t i = 0 ; i< sizeof(codec_para6)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para6[i][0];  
            codec_para[i][1]=codec_para6[i][1];     
         }
         codec_clkin=111132;
    } 
     else if ((sr == 22050 || sr == 44100) && sample_length==32 && (slot_num==1 || slot_num==2 || slot_num==4 || slot_num==6 ||  slot_num==8)){
         for( uint8_t i = 0 ; i< sizeof(codec_para7)>>1 ; i++ ) { 
            codec_para[i][0]=codec_para7[i][0];  
            codec_para[i][1]=codec_para7[i][1];     
         }
         codec_clkin=169344;
    } 
    
    BCLK= sr * sample_length * slot_num;
    
    for(NDAC=1;NDAC<129;NDAC++) {
        for(MDAC=1;MDAC<129;MDAC++){    
            for(DOSR=1;DOSR<1025;DOSR++){ 
              if( Generate_FCLK(codec_clkin, sr ,NDAC ,MDAC ,DOSR)==1){
                for(BCLK_N_divider=1; BCLK_N_divider <129;  BCLK_N_divider++){
                  if(Generate_BCLK(codec_clkin, BCLK, NDAC, BCLK_N_divider)==1){     
                      flag=1; 
                  }
                  if(flag==1)break;
                }     
              }   
              if(flag==1)break;
            }
            if(flag==1)break;
        }
        if(flag==1)break;
    }
    flag=0; 
    if(DOSR==1024){
        DOSR_H=0;
        DOSR_L=0;
    } else{
        DOSR_H=DOSR/256;
        DOSR_L=DOSR%256;
    }
    NADC  = NDAC ;
    MADC  = MDAC ;
       
    for( i = 0 ; i< sizeof(codec_para)>>1 ; i++ ) {  
        if( (bclk_polarity == 0) && (codec_para[i][0] == 0x1D) ) {
            codec_para[i][1] |= 0x08 ; //blck poarity inverted
        }
        err = I2CWrite_Codec_AIC3204(pSource,codec_para[i][0],codec_para[i][1]);
        if( OS_ERR_NONE != err ){
          return err;
        }
    }   

    uint8_t reg_data[][2] = {
        0x0B,0x80|NDAC,
        0x0C,0x80|MDAC,
        0x0D,DOSR_H,
        0x0E,DOSR_L,
        0x12,0x80|NADC,
        0x13,0x80|MADC,
        0x1E,0x80|BCLK_N_divider
    };
    
    for( i = 0 ; i< sizeof(reg_data)>>1 ; i++ ) {    
        err = I2CWrite_Codec_AIC3204(pSource,reg_data[i][0],reg_data[i][1]);
        if( OS_ERR_NONE != err ){
          return err;
        }
    }
    
    return err;
  
}


CODEC_SETS codec_set_saved;
 
uint8_t Init_CODEC( const DataSource *pSource,CODEC_SETS codec_set ) 

{
    uint8_t err;
    uint8_t i, if_set;
 
    //if( memcmp(&codec_set_saved,&codec_set_saved,sizeof(CODEC_SETS){
    if( (codec_set_saved.sr == codec_set.sr)  &&\
        (codec_set_saved.sample_len == codec_set.sample_len) &&\
        (codec_set_saved.format == codec_set.format) &&\
        (codec_set_saved.slot_num == codec_set.slot_num) &&\
        (codec_set_saved.m_s_sel == codec_set.m_s_sel)   ) {
        APP_TRACE_INFO(("No need Re-Init CODEC\r\n"));
//        return 0;
    } else {
        codec_set_saved = codec_set;  
    }
         
//    Pin_Reset_Codec();
//    OSTimeDly(2000); //test
   
    err = Check_SR_Support( codec_set.sr );
    if( OS_ERR_NONE != err ){
        return err;
    }
    
    if( codec_set.m_s_sel == 0 ) {
        if_set = 0x0C; //master
    } else {
        if_set = 0;    //slave
    }
    
    if( codec_set.sample_len == 16) {
        if_set += 0x00;
//    } else if(codec_set.sample_len == 20) //Not yet support 20/24bit on Audio MCU side
//        if_set += 0x10;
//    } else if(codec_set.sample_len == 24)
//        if_set += 0x20;
    } else if(codec_set.sample_len == 32) {
        if_set += 0x30;
    } else {
        return CODEC_BIT_LEN_NOT_SUPPORT_ERR;
    }
    
    if( codec_set.format == 0 || codec_set.format == 1) { //I2S || TDM-I2S
        if_set += 0x00;
    } else if( codec_set.format == 2){   //PCM DSP
        if_set += 0x40;
    } else {
        return CODEC_FORMAT_NOT_SUPPORT_ERR;
    }
    if( codec_set.format == 1 ) {
        codec_set.slot_num = 2;
    }
//    if( codec_set.sample_len == 16 ) {
//        if( codec_set.slot_num == 2 ) {
//            mode = 0;
//        } else if( codec_set.slot_num == 8 ) {
//            mode = 1;
//        } else {
//            return CODEC_CH_NUM_NOT_SUPPORT_ERR;
//        }
//        
//    } else if( codec_set.sample_len == 32 ) {
//        if( codec_set.slot_num == 8 ) {
//            mode = 2;
//        } else if( codec_set.slot_num == 2 ) {
//            mode = 3;
//        } else if( codec_set.slot_num == 4 ) {
//            mode = 4;
//        } else {
//            return CODEC_CH_NUM_NOT_SUPPORT_ERR;
//        }
//        
//    } else {
//        return CODEC_BIT_LEN_NOT_SUPPORT_ERR;
//        
//    }
       
    for( i = 0 ; i< sizeof(config_aic3204)>>1 ; i++ ) {    
        err = I2CWrite_Codec_AIC3204(pSource,config_aic3204[i][0],config_aic3204[i][1]);
        if( OS_ERR_NONE != err ){
          return err;
        }
    }
    
    err = I2CWrite_Codec_AIC3204(pSource,0,0); //switch to Page0
    if( OS_ERR_NONE != err ){
       return err;
    }
    
    err = I2CWrite_Codec_AIC3204( pSource,27, if_set); //set format
        if( OS_ERR_NONE != err ){
        return err;
    }
    
    err = Set_Codec_PLL( pSource,codec_set.sr, codec_set.sample_len, codec_set.slot_num, codec_set.bclk_polarity );
      
    return err;    
    
}

