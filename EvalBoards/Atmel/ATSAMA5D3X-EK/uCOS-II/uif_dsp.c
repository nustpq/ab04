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
*                                        FM36 DSP Based Signal Path Setup
*
*                                          Atmel AT91SAM7A3
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : uif_dsp.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       : This file copy directly from uif-1.0
*********************************************************************************************************
*/
#include <stdio.h>

#include "defined.h"
#include "uif_twi.h"

#include "uif_dsp.h"
#include "mem_basic.h"
#include "noah_cmd.h"

//Note: This routine do NOT support reentrance
INTERFACE_CFG   Global_UIF_Setting[ UIF_TYPE_CMD_NUM ];

static bool flag_power_lose  = true;
static bool flag_state_pwd   = false;
static uint16_t sr_saved               = 0;
static uint8_t  mic_num_saved          = 0;
static uint8_t  lin_sp_index_saved     = 0;
static uint8_t  start_slot_index_saved = 0;
static uint8_t  bit_length_saved       = 0;
static uint8_t  i2s_tdm_sel_saved      = 0;

#define   GET_BYTE_LOW_4BIT( x )     ( ( x ) & 0x0F ) 
#define   GET_BYTE_HIGH_4BIT( x )    ( ( ( x ) >> 4 ) & 0x0F )


static void Revert_patch_Endien( uint32_t *pData )
{
    
   uint8_t temp, *pChar;    
   
   pChar  = (uint8_t *)pData;   
   temp   = *pChar;
   *pChar = *(pChar+2);
   *(pChar+2) = temp;
   
}



////////////////////////    for FM36-600C on AB03      ////////////////////////

static uint32_t fm36_patch_code_3[] = 
{
    0x003F00,  
    0x3C0065,  
    0x43700A,  
    0x377201,  
    0x17F05E,  
    0x6800A5,  
    0x22629F,  
    0x3C0045,  
    0x43730A,  
    0x377261,  
    0x17F0BE,  
    0x6800A5,  
    0x22629F,  
    0x44000A,  
    0x19E78F,  
    0x81012A,  
    0x877200,  
    0x6800A1,  
    0x977200,  
    0x81013A,  
    0x877220,  
    0x6800A1,  
    0x977220,  
    0x81014A,  
    0x877240,  
    0x6800A1,  
    0x977240,  
    0x81011A,  
    0x19BFCF,  
    0x877210,  
    0x6000A1,  
    0x977210,  
    0x9101DA,  
    0x877230,  
    0x6000A1,  
    0x977230,  
    0x9101EA,  
    0x877250,  
    0x6000A1,  
    0x977250,  
    0x9101FA,  
    0x862450,  
    0x19C27F,  
    0x8A2C72,  
    0x7000AA,  
    0x877260,  
    0x6800A1,  
    0x977260,  
    0x8A2C82,  
    0x7000AA,  
    0x877280,  
    0x6800A1,  
    0x977280,  
    0x810BF1,  
    0x1831DF,  
    0x43F005,  
    0x3C0025,  
    0x3A2D50,  
    0x3B72A2,  
    0x1DC53F,  
    0x43F005,  
    0x19C8CF,  
    0x877270,  
    0x340088,  
    0x6000A1,  
    0x8B72A0,  
    0x7800A2,  
    0x977270,  
    0x877290,  
    0x6000A1,  
    0x8B72B0,  
    0x7800A2,  
    0x977290,  
    0x862550,  
    0x19A30F

};


//parameters for FM36 signal path on AB03
static uint16_t fm36_para_table_3[][2] = 
{
  //patch settings
  {0x3FA0, 0x9E77},
  {0x3FB0, 0x3F00},
  {0x3FA1, 0x9BFB},
  {0x3FB1, 0x3F0E},
  {0x3FA2, 0x9C26},
  {0x3FB2, 0x3F1C},
  {0x3FA3, 0x831C},
  {0x3FB3, 0x3F2A},
  {0x3FA4, 0x9C8B},
  {0x3FB4, 0x3F36},
  {0x3FA5, 0x9A2F},
  {0x3FB5, 0x3F3D},    

  //////////////////////////////////////////////////////////////
  
  
  ///////////////// SP0 Format  ////////////////// 
  //reset in Config_SPx_Format()
  //{0x2260, 0x78df},//16bit TDM
  //{0x2260, 0x78ff},//32bit TDM
  //{0x2260, 0x7AFF},//32bit TDM,MSB first,Left alignment,8 slot 
  //{0x2260, 0x7AFD},//32bit TDM,MSB first,Left alignment,6 slot 
  // {0x2260, 0x78D9}, //16bit I2S,MSB first,Left alignment,2 slot 
  
  ///////////////// multi-function port  //////////////// 
  {0x2264, 0x01FC}, //use SP0 SP1 PDMI_012 PDMO_012
  
  //////////////////  PDM CLOCK SETTING   /////////////////
  {0x2265, 0x0000}, //ADC clock source, 0: ADC =PLL, 2: ADC=DCA, 3: ADC=1/2 DAC 
  
  //{0x2266, 0x0013}, //3.072Hz
  {0x2266, 0x001B}, //2.048M
  //{0x2266, 0x0033},//1.024

  //ADC
  //{0x2267, 0xBB80}, //3.072Hz
  {0x2267, 0x7d00}, //2.048M //clock to iM401
  //{0x2267, 0x3e80},//1.024

   //DAC
   //{0x226A, 0xBB80}, //3.072Hz
   //{0x226A, 0x7d00}, //2.048M 
   {0x226A, 0x3e80},//1.024   //clock from iM401
   
  ///////////////// SP1 Format  //////////////////
  //reset in Config_SPx_Format()
  //{0x2261, 0x78FF},//32bit TDM,MSB first,Left alignment,8 slot, Left alignment of word inside slot to meet AD1937
  //{0x2260, 0x78D9}, //16bit I2S,MSB first,Left alignment,2 slot 
  //{0x2268, 0xBB80}, 

  {0x2288, 0x0000}, //mic source select : mic_PDM
  {0x2289, 0x7FFF}, //unit gain
  {0x2290, 0x7FFF}, 
  
  {0x22FD, 0x00DE}, //enable ADC/DAC interrupt
  //{0x2274, 0x0001},//mic revert  
  //{0x2303, 0x8000},
  //{0x2304, 0x4000},
  //{0x2305, 0x0000},
  {0x22FC, 0x8000}, //BYPASS ALL SIGNAL PORCESSING
  {0x226E, 0x000C}, //PLL 24.576MHz OSC 
  
  ////////////////////// UpLink Path ////////////////////////////////////
  
  //select data interruption source : [ 0: PDM, 1: SP0, 2: SP1, 3: SP2 ] 
  {0x22B2, 0x0001}, //_lout_des_sel : SP0
  {0x22B3, 0x0001}, //_spkout_des_sel : SP0
  {0x22B4, 0x0001}, //_auxout_des_sel : SP0 
  
    //additional, input
    //////output, aux2 output same as aux1
    ////22C7 = 0x1018 //Aux-in-L
    ////22C8 = 0x1019 //Aux-in-R
    ////22B4 = 1 //SP0
    ////22D5 = 6 //slot6 
    ////22D6 = 7 //slot7 
  
  //select aux2 - lin source 
  {0x229A, 0x0002}, //Aux2-in From SP1
  {0x229B, 0x0008}, //Aux2-in-L in SP1 slot0 
  {0x229C, 0x0009}, //Aux2-in-R in SP1 slot1

  //select output data source slot
  {0x22C1, 0x101A}, //patch
  {0x22C2, 0x101B},
  {0x22C3, 0x101C},
  {0x22C4, 0x101D},
  {0x22C5, 0x101E},
  {0x22C6, 0x101F},
  {0x22C7, 0x1018}, //Aux2-in-L
  {0x22C8, 0x1019}, //Aux2-in-R

  
  //select data dest slot
  //If lineout is from TX0, offset is 0~7
  //If lineout is from TX1, offset is 8~15
  //If lineout is from TX2, offset is 16~23
  //If offset = 0x8000, means Zero output.
  {0x22D7, 0x0000},
  {0x22D8, 0x0001},
  {0x22D9, 0x0002},
  {0x22DA, 0x0003},
  {0x22DB, 0x0004},
  {0x22DC, 0x0005},
  {0x22D5, 0x0006}, 
  {0x22D6, 0x0007}, 
  
  ////////////////////// DownLink Path ////////////////////////////////////
  //mic souce
  {0x2282, 0x0000},
  {0x2283, 0x0001},
  {0x2284, 0x0002},
  {0x2285, 0x0003},
  {0x2286, 0x0004},
  {0x2287, 0x0005},
  
  //set PDM out data
  {0x22B9, 0x3F40}, //PDM output0 source pointer to :  RX0-Slot0 data register  
  {0x22BA, 0x3F42}, //PDM output1 source pointer to :  RX0-Slot1 data register 
  {0x22BB, 0x3F44}, //...
  {0x22BC, 0x3F46},
  {0x22BD, 0x3F48},
  {0x22BE, 0x3F4A}, 

  //{0x22B3, 0}, 
  {0x22FA, 0x00FF}, 
  
  {0x22E5, 0x0020}, //PDM DAC CLOCK As Input
  {0x22EB, 0x0006}, //Actual MIC number in system.
  
  {0x22F1, 0xD800}, //pwd resume mode, enable pwd bypass
  
  {0x22FB, 0x0000}, //run flag
  
  //{0x3FCF, 0x020},//PDMDAC CLOCK As Input
  
  /*******************  MIC Volume Control  ****************
  Volume setting [7:4]:
  0x0 ( 0 dB) 0x4 (-48 dB) 0x8 (-96 dB) 0xc (Not allowed)
  0x1 (-12 dB) 0x5 (-60 dB) 0x9 (-108 dB) 0xd (Not allowed)
  0x2 (-24 dB) 0x6 (-72 dB) 0xa (-120 dB) 0xe (+24 dB)
  0x3 (-36 dB) 0x7 (-84 dB) 0xb (-132 dB) 0xf (+12 dB)
  Volume setting [3:0]:
  0x0 ( 0 dB) 0x4 (-3.00 dB) 0x8 (-6.00 dB) 0xc (-9.00 dB)
  0x1 (-0.75 dB) 0x5 (-3.75 dB) 0x9 (-6.75 dB) 0xd (-9.75 dB)
  0x2 (-1.50 dB) 0x6 (-4.50 dB) 0xa (-7.50 dB) 0xe (-10.50 dB)
  0x3 (-2.25 dB) 0x7 (-5.25 dB) 0xb (-8.25 dB) 0xf (-11.25 dB)
  Real volume = Volume for [7:4] + Volume for [3:0]
  **********************************************************/  
  //{0x3FC6, 0x0000}, //MIC0,1 Volume Gain = 0dB
  //{0x3FC7, 0x0000}, //MIC2,3 Volume Gain = 0dB
  //{0x3FC8, 0x0000}, //MIC4,5 Volume Gain = 0dB
  
  /**************** MEMS Gain Control **********************/
  //{0x3F91, 0x0000}, //MEMS MIC TYPE, SW default setting as MEMS type
  //{0x3F92, 0x0000} //MEMS MIC gain = 0dB
  {0x3F92, 0x0555} //MEMS MIC gain = 6dB
  //{0x3F92, 0x0AAA} //MEMS MIC gain = 12dB //SW default setting
  //{0x3F92, 0x0FFF} //MEMS MIC gain = 18dB

} ;

void Pin_Reset_FM36( void )
{
    
//    PIO_Clear(&pinsGpios[11]);
    OSTimeDly(20) ;
//    PIO_Set(&pinsGpios[11]);
    OSTimeDly(50) ;
     
}

//DMIC 4 steps PGA gain
uint8_t DMIC_PGA_Control( uint16_t gain ) 
{
    uint8_t  err  ;
    uint16_t data ;
    uint16_t mute ;
    
    if( flag_state_pwd ) return 0 ;
    
    mute = 0x3F;
    err = NO_ERR;
    
    switch ( gain ){
        case 0 : 
            data = 0x0000;
        break;
        case 6 : 
            data = 0x0555;
        break;
        case 12 : 
            data = 0x0AAA;
        break;
        case 18 : 
            data = 0x0FFF;
        break;
        case 1000 :
            mute = 0x3F3F;
        break;
        default:
            return FM36_DMIC_PGA_GAIN_ERR;
    }  
    
    if( mute == 0x3F ) {
      err = DM_SingleWrite( FM36_I2C_ADDR, 0x3F92, data ) ;
      if( OS_ERR_NONE != err ) {
          return FM36_WR_DM_ERR;;
      }   
    }    
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x3F96, mute ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }    
    
    return err;
    
}


//Setup External Lin data input source
//From : SP0_RX, SP1_RX
static uint8_t Config_Lin( uint8_t lin_sp_index, uint8_t start_slot_index )
{
    
    uint8_t err ;
    
    if( flag_state_pwd ) return 0 ;
     
    APP_TRACE_INFO(("\r\nConfig_Lin sp_index = %d, start_slot_index = %d\r\n", lin_sp_index, start_slot_index));
    
    //Aux-Source
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x229A, lin_sp_index+1 ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }
    
    //Aux-in-L
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x229B, (lin_sp_index<<3) + start_slot_index ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }
    
    //Aux-in-R
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x229C, (lin_sp_index<<3) + start_slot_index + 1 ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }    
   
    return err;
    
}

// Set additional data besides MIC data
// M0..M5 + X0 + X1
// Max 8 slots
// Make sure last 2 slots folowed mic data closely
static uint8_t Config_SP0_Out( uint8_t mic_num )
{
    
    uint8_t err ;
    
    if( flag_state_pwd ) return 0 ;
    APP_TRACE_INFO(("\r\nConf FM36 Mic num = %d\r\n", mic_num));
    
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x22EB, mic_num ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }  
    
    //select output data source slot
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x22C1 + mic_num, 0x1018 ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }       
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x22C2 + mic_num, 0x1019 ) ;
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }     
  
    return err;
    
}


static uint8_t Config_SPx_Format( uint8_t bit_length, uint8_t i2s_tdm_sel )
{
    
    uint8_t  err ;
    uint16_t temp;
    
    if( flag_state_pwd ) return 0 ;
    APP_TRACE_INFO(("\r\nConf FM36 Bit length = %d, %d\r\n", bit_length, i2s_tdm_sel));
    
    if( i2s_tdm_sel == 0 ) { //I2S
        if( bit_length == 32 ) {          
              //temp = 0x78FF;//32bit I2S, 16bit data, MSB first,Left alignment
              APP_TRACE_INFO(("\r\nConf FM36 I2S to 32bit, not support!\r\n"));
              return FM36_WR_DM_ERR;
        } else {//16
              temp = 0x78D9;//16bit I2S,16bit data, MSB first,Left alignment 
        }       
        
    } else { //TDM
        if( bit_length == 32 ) {          
              temp = 0x78FF;//32bit TDM, 16bit data, MSB first,Left alignment,8 slot 
        } else { //16
              temp = 0x78DF;//16bit TDM,16bit data, MSB first,Left alignment,8 slot 
        }
    }
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x2260, temp ) ; //SP0
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }       
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x2261, temp ) ; //SP1
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    } 
    
    return err;
    
}   



static uint8_t Config_SR( uint16_t sr )
{
    
    uint8_t err ;
    uint16_t temp ;
    
    if( flag_state_pwd ) return 0 ;
    APP_TRACE_INFO(("\r\nConf FM36 SR = %dkHz\r\n", sr));
      
    switch ( sr ) {
        case 8000 :
            temp = 0x1F40;
        break;        
        case 16000 :
            temp = 0x3E80;
        break;
        case 22050 :
            temp = 0x5622;
        break;
        case 24000 :
            temp = 0x5DC0;
        break;
        case 32000 :
            temp = 0x7D00;
        break;
        case 44100 :
            temp = 0xAC44;
        break;
        default: //48000 :
            temp = 0xBB80;
        break;   
    }
    
    //select output data source slot
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x2268, temp ) ; //Output Clock for ADC DPLL, PDM input sampling rate.
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }       
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x2269, temp ) ;// reference Clock for DAC DPLL,PDM output sampling rate.
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }     
  
    return err;
    
}  



uint8_t FM36_PWD_Bypass( void )
{
    
    uint8_t  err ;  
    
    if( flag_state_pwd ) return 0 ;
    APP_TRACE_INFO(("\r\nPower down FM36 to bypass SP0<-->SP1\r\n"));
     
    err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FEF, 0x2000 ) ; //pwd
    //err = DM_SingleWrite( FM36_I2C_ADDR, 0x22F9, 1 ) ; //pwd
    if( OS_ERR_NONE != err ) {
        return FM36_WR_DM_ERR;;
    }  
    
    flag_state_pwd = true ;
    
    return err;
    
}  


//onoff       : 0 - turn off PDM clock, 1 - turn on PDM clock
//fast_switch : 0 - normal usage,  1 - fast onoff switch, not care pop sound
uint8_t FM36_PDMADC_CLK_OnOff( uint8_t onoff, uint8_t fast_switch )
{
    uint8_t  err ;  
    
    if( flag_state_pwd ) return 0 ;
     
    APP_TRACE_INFO(("\r\nEnable/Disbale FM36 ADC PDM CLK: %d",onoff));    

    if( onoff ) {  //PDMCLK_ON, for normal operation 
        
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FCF, 0x0020 ) ;  //turn on clk
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;
        }          
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3F90, 0x0000 ) ;  //power up MIC0-5
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;
        }  
        if( fast_switch == 0 ) {
          OSTimeDly(5);  //depop time
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3F96, 0x003F ) ;  //unmute MIC0-5
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;
        }  
        
    } else { //PDMCLK_OFF
        
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3F96, 0x3F3F ) ;  //mute MIC0-5
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;
        }  
        if( fast_switch == 0 ) {
            OSTimeDly(10); //wait data 0 to cyclebuffer
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3F90, 0x003F ) ;  //power down MIC0-5 
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;
        } 
        if( fast_switch == 0 ) {
            OSTimeDly(5);
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FCF, 0x0024 ) ;  //turn off clk
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;
        } 
        
    }
    
    return err;
    
}  


uint8_t FM36_PDM_CLK_Set( uint8_t pdm_dac_clk, uint8_t pdm_adc_clk, uint8_t type )
{
    
    uint8_t  err ;
    uint16_t data1,data2,data3 ;
    
    if( flag_state_pwd ) return 0 ;
     
    APP_TRACE_INFO(("\r\nConf FM36 PDMADC Clock = %dMHz\r\n", pdm_adc_clk));      
    switch ( pdm_adc_clk ) {
        case 4 : //4.096
            data3 = 0x0F;            
            data1 = 0xFA00;
        break;  
        case 3 : //3.072
            data3 = 0x13;           
            data1 = 0xBB80;
        break;
        case 2 : //2.048
            data3 = 0x1B;           
            data1 = 0x7D00;
        break;
        case 1 : //1.024
            data3 = 0x33;         
            data1 = 0x3E80;
        break; 
        default ://2.048
            APP_TRACE_INFO(("Not supported PDM CLK, reset to default 2.048MHz\r\n"));
            data3 = 0x1B;          
            data1 = 0x7D00;
        break;
    }
    
    APP_TRACE_INFO(("\r\nConf FM36 PDMDAC Clock = %dMHz\r\n", pdm_dac_clk));
    switch ( pdm_dac_clk ) {
        case 4 : //4.096           
            data2 = 0xFA00;            
        break;  
        case 3 : //3.072           
            data2 = 0xBB80;            
        break;
        case 2 : //2.048           
            data2 = 0x7D00;;            
        break;
        case 1 : //1.024           
            data2 = 0x3E80;          
        break; 
        default ://1.024
            APP_TRACE_INFO(("Not supported PDM CLK, reset to default 1.024MHz\r\n"));            
            data2 = 0x3E80;           
        break;
    }
    
    if( type == 0 ) { //para set before run
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x2267, data1 ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x226A, data2 ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x2266, data3 ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }
        
    } else { //on the fly check
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FD5, data1 ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FCD, data2 ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FED, data3 ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }
        
        err = DM_LegacyRead( FM36_I2C_ADDR, 0x3FE4,(uint8_t *)&data1 ) ;
        if( OS_ERR_NONE != err ) {
            err = FM36_RD_DM_ERR;
            return err ;
        }     
        err = DM_SingleWrite( FM36_I2C_ADDR, 0x3FE4, data1) ;// must rewrite 3fe4 to setting PLL.
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }     
    }
    
    return err;
    
}


//TDM mode, in addition, bypass RX0.0 and RX0.1 to TX1.0 and TX1.1
static uint16_t fm36_para_table_tdm_aec[][2] =   
{
       //select aux2 - lin source 
      {0x229A, 0x0001}, //Aux2-in From SP0
      {0x229B, 0x0000}, //Aux2-in-L in SP0 slot0 
      {0x229C, 0x0001}, //Aux2-in-R in SP0 slot1 
      {0x22D5, 0x0008}, //Aux2-out-L to SP1 slot0
      {0x22D6, 0x0009}  //Aux2-out-R to SP1 slot1
};


//I2S mode, in addition, bypass RX0.0 and RX0.1 to TX1.0 and TX1.1
static uint16_t fm36_para_table_i2s_aec[][2] =   
{
       //select aux2 - lin source 
      {0x229A, 0x0001}, //Aux2-in From SP0
      {0x229B, 0x0000}, //Aux2-in-L in SP0 slot0 
      {0x229C, 0x0001}, //Aux2-in-R in SP0 slot1 
      {0x22D5, 0x0008}, //Aux2-out-L to SP1 slot0
      {0x22D6, 0x0009}  //Aux2-out-R to SP1 slot1
};

static uint8_t Config_SP0IN_to_SP1Out( void )
{
    
    uint8_t  err ;
    uint32_t   i ;

    if( flag_state_pwd ) return 0 ;
    APP_TRACE_INFO(("\r\nConf FM36 SP0 Rx to SP1 Tx\r\n"));
    
    for(i = 0; i < sizeof(fm36_para_table_tdm_aec)/4; i++) {  
        err = DM_SingleWrite( FM36_I2C_ADDR, fm36_para_table_tdm_aec[i][0], fm36_para_table_tdm_aec[i][1] ) ;
        if( OS_ERR_NONE != err ) {
            return FM36_WR_DM_ERR;;
        }      
    }
  
    return err;
    
}

/*
*********************************************************************************************************
*                                       Init_FM36_AB03()
*
* Description : Initialize FM36 DSP on AB03 board.
* Argument(s) : sr        : sample rate : 8000 ~ 48000 
*               bit_length: 16bit/32bit mode
*               mic_num   : 0~6
*               lin_sp_index  : line in data source: 0 ~ 1
*               start_slot_index: line in data slot: 0 ~ 7
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Init_FM36_AB03( uint16_t sr, 
                              uint8_t mic_num, 
                              uint8_t lin_sp_index, 
                              uint8_t start_slot_index, 
                              uint8_t bit_length,
                              uint8_t i2s_tdm_sel,
                              uint8_t force_reset)
{
    uint32_t   i;
    uint16_t temp, temp2 ;
    uint16_t addr, val; 
    uint8_t  err ;     
    
    if( sr               == sr_saved  &&  \
        mic_num          == mic_num_saved && \
        lin_sp_index     == lin_sp_index_saved && \
        start_slot_index == start_slot_index_saved && \
        bit_length       == bit_length_saved && \
        i2s_tdm_sel      == i2s_tdm_sel_saved && \
        force_reset      == 0  ) 
    {    
        APP_TRACE_INFO(("No need Re-Init FM36\r\n"));
        return NO_ERR;        
    } else {
        sr_saved = sr ;
        mic_num_saved = mic_num ;
        lin_sp_index_saved = lin_sp_index;
        start_slot_index_saved = start_slot_index ;
        bit_length_saved = bit_length;
        i2s_tdm_sel_saved = i2s_tdm_sel;
    }   
    
    Pin_Reset_FM36();
    flag_state_pwd  = false ;

    //check chip type by rom id 
    err = CM_LegacyRead( FM36_I2C_ADDR, 0x2FFF,(uint8_t *)&temp ) ;
    if( OS_ERR_NONE != err ) {
        err = FM36_RD_CM_ERR;
        return FM36_RD_CM_ERR ;
    }
    APP_TRACE_INFO(("ROM ID = 0x%X\r\n", temp));
    
    if( temp != FM36_ROM_ID_2 ) { 
        return FM36_CHECK_ROMID_ERR; //dsp type error
    }    
    
    //load FM36-600C patch code
    if( flag_power_lose ) { //only need download patch once after one power cycle            
        flag_power_lose = false;
        addr = (uint16_t)fm36_patch_code_3[0];
        for( i = 1; i < sizeof(fm36_patch_code_3)/4; i++) { 
            Revert_patch_Endien(&fm36_patch_code_3[i]);    
            err = PM_SingleWrite( FM36_I2C_ADDR, addr++, (uint8_t*)&fm36_patch_code_3[i],0 ) ;
              if( OS_ERR_NONE != err ) {
                  err = FM36_WR_PM_ERR;
                  return err ;
              }         
        }
    } 
    
    //load parameters
    for( i = 0; i < sizeof(fm36_para_table_3)/4; i++) {            
        addr = fm36_para_table_3[i][0];
        val  = fm36_para_table_3[i][1];
        
        if( addr == 0x22FB ) {  //hit run flag para, do other inits before write it            
            err = Config_SP0_Out( mic_num );
            if( OS_ERR_NONE != err ) {
                return err ;
            }
            err = Config_Lin( lin_sp_index, start_slot_index );
            if( OS_ERR_NONE != err ) {
                return err ;
            }
            err = Config_SPx_Format( bit_length, i2s_tdm_sel );
            if( OS_ERR_NONE != err ) {
                return err ;
            }
            err = Config_SR( sr );
            if( OS_ERR_NONE != err ) {
                return err ;
            }
            err = FM36_PDM_CLK_Set( GET_BYTE_HIGH_4BIT(Global_UIF_Setting[ UIF_TYPE_FM36_PDMCLK - 1 ].attribute), GET_BYTE_LOW_4BIT(Global_UIF_Setting[ UIF_TYPE_FM36_PDMCLK - 1 ].attribute), 0 );
            if( OS_ERR_NONE != err ) {
                return err ;
            }
            err = Config_SP0IN_to_SP1Out();
            if( OS_ERR_NONE != err ) {
                return err ;
            }
        } 
        
        err = DM_SingleWrite( FM36_I2C_ADDR, addr, val ) ;
        if( OS_ERR_NONE != err ) {
            err = FM36_WR_DM_ERR;
            return err ;
        }
        if( addr == 0x22FB ) {  //if run chip, delay
            OSTimeDly(100);            
        }
    } 
          
    //check running status
    err = DM_LegacyRead( FM36_I2C_ADDR, 0x2306,(uint8_t *)&temp2 ) ;
    if( OS_ERR_NONE != err ) {
        err = FM36_RD_DM_ERR;
        return err ;
    }
    APP_TRACE_INFO(("0x2306 = 0x%X\r\n", temp2));
    OSTimeDly(100);
    err = DM_LegacyRead( FM36_I2C_ADDR, 0x22FB,(uint8_t *)&temp ) ;
    if( OS_ERR_NONE != err ) {
        err = FM36_RD_DM_ERR;
        return err ;
    }
    APP_TRACE_INFO(("0x22FB = 0x%X\r\n", temp));
    if( temp != 0x5A5A ) {   
        return FM36_CHECK_FLAG_ERR;
    }
    err = DM_LegacyRead( FM36_I2C_ADDR, 0x2306,(uint8_t *)&temp ) ;
    if( OS_ERR_NONE != err ) {
        err = FM36_RD_DM_ERR;
        return err ;
    }
    APP_TRACE_INFO(("0x2306 = 0x%X\r\n", temp));
    if( temp == temp2 ) {
        APP_TRACE_INFO(("FM36 frame counter stopped !"));
        return FM36_CHECK_COUNTER_ERR;
    } 
    
//    if( Global_UIF_Setting[ UIF_TYPE_FM36_PATH - 1 ].attribute == ATTRI_FM36_PATH_PWD_BP ) {
//        FM36_PWD_Bypass();
//        
//    }
    
    return err;
    
}


//Force reset and init FM36 using previous setting
uint8_t Init_FM36_AB03_Preset( void )
{
    return ( Init_FM36_AB03( sr_saved, mic_num_saved, lin_sp_index_saved, start_slot_index_saved, bit_length_saved, i2s_tdm_sel_saved, 1 ) );   
}


static uint8_t para_table_pdm_pa[][2] = 
{
    {0x01, 0x00},
    {0x02, 0x00},
    {0x03, 0x00},
    {0x04, 0x00},
    {0x10, 0x00},
    {0x11, 0x00},
    {0x12, 0x00},
    {0x13, 0x00},
    {0x14, 0x00},
    {0x15, 0x00},
    {0x16, 0x00},
    {0x17, 0x00},
    {0x18, 0x00},
    {0x19, 0x00},
    {0x1A, 0x00},
    {0x20, 0x00},
    {0x21, 0x00},
    {0x22, 0x00},
    {0x23, 0x02},
    {0x24, 0x80},
    {0x25, 0x10},
    {0x26, 0x04},
    {0x27, 0x88},
    {0x28, 0x88},
    {0x30, 0x03},
    {0x31, 0x00},
    {0x32, 0x00},
    {0x33, 0x01},
    {0x34, 0x01},
    {0x35, 0x02},
    {0x36, 0x00},
    {0x37, 0x00},
    {0x38, 0x00},
    {0x39, 0x00},
    {0x40, 0x01},
    {0x41, 0x00}     
};


uint8_t Config_PDM_PA( void )
{
    
    uint8_t  err ;
    uint32_t   i ;
    uint8_t  buf[3] ;  

//has problem 
    OPTIONPARAMETER *option = ( OPTIONPARAMETER * )source_twi2.privateData;
    option->address = MAX98504 >> 1;
    option->iaddress = 0;
    option->isize = 0;

   
    APP_TRACE_INFO(("\r\nConf PDM PA\r\n"));
    buf[0] = 0;
    
    for(i = 0; i < sizeof(para_table_pdm_pa)/2; i++) 
    {  
        buf[1] = para_table_pdm_pa[i][0];
        buf[2] = para_table_pdm_pa[i][1];
        
//        err = TWID_Write( MAX98504>>1, 0, 0, buf, sizeof(buf), NULL);
        err = twi_fm36_write( &source_twi2,buf,sizeof( buf ) );
        
        if( OS_ERR_NONE != err ) 
        {
            return FM36_WR_DM_ERR;;
        }      
    }
  
    return err;
    
}