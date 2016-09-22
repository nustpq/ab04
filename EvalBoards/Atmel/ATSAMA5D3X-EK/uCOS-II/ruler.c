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
*                                        RULER RELATED OPERATIONS REALIZATION
*
*                                          Atmel AT91SAM3U4C
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : ruler.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <ucos_ii.h>
#include "defined.h"
#include "emb.h"
#include "noah_cmd.h"
#include "codec.h"

/*----------------------------------------------------------------------------*/
//just define these to avoid compile error,they will be redefined according new hardware or abandant

#define AUDIO_UART                  0
#define PC_UART                     1 
#define RULER_UART                  2

#define I2C_MIX_UIF_M        1
#define I2C_MIX_UIF_S        2
#define I2C_MIX_FM36_CODEC   3

#define  APP_CFG_TASK_USER_IF_PRIO  10

//#define LED_P0  PIN_LED_0
#define LED_P0  0

OS_EVENT  *UART_MUX_Sem_lock ;
OS_EVENT  *ACK_Sem_RulerUART ;
OS_EVENT  *Done_Sem_RulerUART ;
OS_EVENT  *EVENT_MsgQ_RulerUART2Noah;
OS_EVENT  *EVENT_MsgQ_Noah2RulerUART;
OS_EVENT  *Load_Vec_Sem_lock;
/*----------------------------------------------------------------------------*/
volatile uint32_t   Global_Mic_Mask[4] ;      //MIC sellection status
volatile uint8_t  Global_Ruler_Index = 0 ;  //the ruler index for UART comm NOW
volatile uint8_t  Global_Bridge_POST = 0 ;  //audio bridge POST status
volatile uint8_t  Global_Ruler_State[4];    //ruler status
volatile uint8_t  Global_Ruler_Type[4];     //ruler type
volatile uint8_t  Global_Mic_State[4];      //MIC (8*4=32) status(calib info error or not)
uint8_t           Audio_Version[20];        //fixed size
uint8_t           Ruler_CMD_Result;
volatile uint8_t  Ruler_Setup_Sync_Data;

extern EMB_BUF   Emb_Buf_Data;
extern EMB_BUF   Emb_Buf_Cmd;

//SET_VEC_CFG  Global_VEC_Cfg; 
//CODEC_SETS    codec_set[2];

uint8_t flag_bypass_fm36;

volatile uint8_t  Global_SPI_Rec_Start = 0;
volatile uint8_t  Global_SPI_Rec_En = 0;

//import global variable
extern uint8_t g_pmeccStatus;             //indicate nfc controller initialized or not
/*
*********************************************************************************************************
*                                           Init_Global_Var()
*
* Description : Initialize Ruler and MIC related global variables to defalut value.
* Argument(s) : None.
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Init_Global_Var( void )
{
    uint8_t ruler_id;    
    
    for( ruler_id = 0; ruler_id < 4; ruler_id++ ) {        
        Global_Ruler_State[ruler_id] = RULER_STATE_DETACHED;
        Global_Ruler_Type[ruler_id]  = 0;
        Global_Mic_State[ruler_id]   = 0 ;
        Global_Mic_Mask[ruler_id]    = 0 ;        
    } 
    
}


/*
*********************************************************************************************************
*                                           Check_Actived_Mic_Number()
*
* Description : Check MIC mask global variable to get the total actived MICs number.
* Argument(s) : None.
* Return(s)   : mic_counter : the total actived MICs number.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static uint8_t Check_Actived_Mic_Number( void )
{
    uint8_t mic_counter = 0;
    uint8_t i, j;    

    for( i = 0; i < 4 ; i++ ) { //scan 4 slots
        for( j = 0; j < 32; j++ ) { //scan max 32mics per slot
            if( (Global_Mic_Mask[i]>>j)&1) {
                mic_counter++;
            }
        }
    } 
   
    return mic_counter;
}


/*
*********************************************************************************************************
*                                           Get_Mask_Num()
*
* Description : Check mask bit number
* Argument(s) : None.
* Return(s)   : mic_counter : the total actived MICs number.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Get_Mask_Num( uint32_t mask )
{
   uint8_t i;
   uint8_t num = 0;
   
   for ( i = 0 ; i<32; i++ ) {
       if( mask & (1<<i) ) {
           num++;
       }
   }
   return num;
    
}


/*
*********************************************************************************************************
*                                           Check_UART_Mixer_Ready()
*
* Description : Check and wait until all data transmission inbuffer for current channel ruler is done .
*               To make sure ruler channels will not be mix up.
*               HW switch is important for this !
* Argument(s) : None.
* Return(s)   : mic_counter : the total actived MICs number.
*
* Note(s)     : If HW switch fast enough, no need this routine.
*********************************************************************************************************
*/
#if OLD_CODE
void Check_UART_Mixer_Ready( void )
{
    uint8_t err; 
    uint32_t  counter;
    
    counter = 0;
    while( OSQGet( EVENT_MsgQ_Noah2RulerUART, &err ) ) {
        OSTimeDly(1);
        counter++;        
    } 
    if( counter) {
        APP_TRACE_INFO(("Check_UART_Mixer_Ready, stage 1 : wait %d ms\r\n",counter));  
    }
        
    counter = 0;
    while( Queue_NData((void*)pUART_Send_Buf[RULER_UART]) ) {
        OSTimeDly(1);
        counter++;  
    } 
    if( counter) {
        APP_TRACE_INFO(("Check_UART_Mixer_Ready, stage 2 : wait %d ms\r\n",counter));  
    }
    OSTimeDly(5);   
    
}
#else
void Check_UART_Mixer_Ready( void )
{
}
#endif

/*
*********************************************************************************************************
*                                           Setup_Audio()
*
* Description : Send command to configure USB audio.
* Argument(s) : pAudioCfg : pointer to AUDIO_CFG type data.
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
//We assume :
//1. there must be a REC setup and a PLAY setup for each audio configuration
//2. REC setup comes first and then PLAY setup 

#if OLD_CODE
uint8_t Setup_Audio( AUDIO_CFG *pAudioCfg )
{
    uint8_t err; 
    uint8_t mic_num; 
    uint8_t data  = 0xFF;
    uint8_t format;
    uint8_t polarity;
    
    uint8_t buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_SET_AUDIO_CFG  };
        
    //APP_TRACE_INFO(("Setup_Audio [%s]:[%d SR]:[%d CH]: %s\r\n",(pAudioCfg->type == 0) ? "REC " : "PLAY", pAudioCfg->sr, pAudioCfg->channels,((pAudioCfg->type == 0) && (pAudioCfg->lin_ch_mask == 0)) ? "LIN Disabled" : "LIN Enabled"));
    if( pAudioCfg->type == 0 ) {
        APP_TRACE_INFO(("\r\nSetup_Audio [REC ]:[%d SR]:[%d CH]:[%d-Bit]", pAudioCfg->sr, pAudioCfg->channels, pAudioCfg->bit_length));
    } else if( pAudioCfg->type == 1 ){
        APP_TRACE_INFO(("\r\nSetup_Audio [PLAY]:[%d SR]:[%d CH]:[%d-Bit]", pAudioCfg->sr, pAudioCfg->channels, pAudioCfg->bit_length ));
    } else {
        APP_TRACE_INFO(("\r\nSetup_Audio ERROR: Unsupported pAudioCfg->type, %d\r\n",pAudioCfg->type));
        return AUD_CFG_ERR;
    }
    
    err = Check_SR_Support( pAudioCfg->sr );
    if( err != NO_ERR ) { 
        APP_TRACE_INFO(("\r\nSetup_Audio ERROR: Unsupported sample rate!\r\n")); 
        return err;
    }        

    //check sample rate
    //No add here!
    
#ifdef BOARD_TYPE_AB03   
    //check play ch num
    if(  (pAudioCfg->type == 1) && ( pAudioCfg->channel_num > 4 ) ) { //for AB03
        APP_TRACE_INFO(("ERROR:(Setup_Audio Play)pAudioCfg->channel_num(=%d) > 4 NOT allowed for AB03\r\n",pAudioCfg->channel_num));
        return AUD_CFG_PLAY_CH_ERR ;
    }
#endif
    
#ifdef BOARD_TYPE_UIF
    if( pAudioCfg->type == 0) {
        mic_num = pAudioCfg->channel_num ;
        Global_Mic_Mask[0] = mic_num;
    } else {
        mic_num = Global_Mic_Mask[0]; //save mic num to ruler0
    } 
#endif   
    if ( (pAudioCfg->type == 0) && (pAudioCfg->lin_ch_mask != 0) ) {         
        pAudioCfg->channels += 2; //add 2 channel for LINE IN
        APP_TRACE_INFO(("Lin 2 channels added...%d\r\n",pAudioCfg->channels)); 
    }

#ifdef BOARD_TYPE_UIF    
    if ( pAudioCfg->type == 0 ) {
        
        pAudioCfg->gpio_rec_num  = Get_Mask_Num( pAudioCfg->gpio_rec_bit_mask ); //gpio num
        pAudioCfg->gpio_rec_start_index = pAudioCfg->channel_num;  //gpio start index
        pAudioCfg->channel_num += pAudioCfg->gpio_rec_num; //add gpio num to channel 
        
        pAudioCfg->spi_rec_num   = Get_Mask_Num( pAudioCfg->spi_rec_bit_mask );  //spi num    
        
        Global_SPI_Rec_En = 0; //clear flag as No SPI rec
        if( pAudioCfg->spi_rec_num  != 0 ){
            if( pAudioCfg->channel_num != 0 ) {
                APP_TRACE_INFO(("ERROR:(Setup_Audio Rec)Mic+Lin+GPIO Rec conflict with SPI Rec\r\n"));
                return AUD_CFG_SPI_REC_CONFLICT ;
            }
            pAudioCfg->channel_num = pAudioCfg->spi_rec_num;  //use spi num for rec channel  
            Global_SPI_Rec_En = 1; //set flag for SPI rec            
        }      
        if(  pAudioCfg->channel_num > 8 ) {
            APP_TRACE_INFO(("ERROR:(Setup_Audio Rec)Mic+Lin+GPIO+SPI Rec channel num(=%d) > 8 NOT allowed for AB03\r\n", pAudioCfg->channel_num));
            return AUD_CFG_MIC_NUM_MAX_ERR ;
        }  
        
    }
#endif
    //check channel num    
    if( (pAudioCfg->type == 1) && (pAudioCfg->channels == 0) ) {
        APP_TRACE_INFO(("WARN:(Setup_Audio Play)pAudioCfg->channels =  0\r\n" ));        
        //return AUD_CFG_PLAY_CH_ZERO_ERR;  UI not support
    }  
    if( (pAudioCfg->type == 0) && (pAudioCfg->channels == 0) ) {
        APP_TRACE_INFO(("WARN:(Setup_Audio Rec)pAudioCfg->channels  =  0\r\n" ));        
        //return AUD_CFG_PLAY_CH_ZERO_ERR; UI not support
    }
    
    //Dump_Data(buf, sizeof(buf)); 
    UART2_Mixer(3); 
    USART_SendBuf( AUDIO_UART, buf, sizeof(buf)) ; 
    USART_SendBuf( AUDIO_UART, (uint8_t *)pAudioCfg, sizeof(AUDIO_CFG)) ; 
    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM);
    if( err != NO_ERR ) { 
        APP_TRACE_INFO(("\r\nSetup_Audio ERROR: timeout\r\n")); 
        return err;
    }
    if( data != NO_ERR ) {
        APP_TRACE_INFO(("\r\nSetup_Audio ERROR: %d\r\n ",data)); 
        return data; 
    }
    
    flag_bypass_fm36 = 1;  //bypass FM36
    if( pAudioCfg->format == 1 && pAudioCfg->channels == 2 ){ //I2S
        format = 0 ;//I2S
    } else if( pAudioCfg->format == 2 ){ //PDM       
        format = 1; //I2S-TDM
        pAudioCfg->channels = 8; //make sure 8 slots enabled when used FM36 to record PDM
        flag_bypass_fm36 = 0; //not bypass FM36 for PDM mode
    } else if( pAudioCfg->format == 3 ) { //PCM/TDM
        format = 2; //PCM/TDM
    } else {
        return CODEC_FORMAT_NOT_SUPPORT_ERR;
    }
        
    if( (pAudioCfg->type + pAudioCfg->cki) != 1 ) {
        return CODEC_FORMAT_NOT_SUPPORT_ERR;
    }
    if( pAudioCfg->start==4 ) {    
        polarity = 0;
    } else if( pAudioCfg->start==5 ){
        polarity = 1;    
    } else {
        return CODEC_FORMAT_NOT_SUPPORT_ERR;
    }    
  
    codec_set[pAudioCfg->type].flag       = 1;  //cfg received
    codec_set[pAudioCfg->type].sr         = pAudioCfg->sr;
    codec_set[pAudioCfg->type].sample_len = pAudioCfg->bit_length;
    codec_set[pAudioCfg->type].format     = format;
    codec_set[pAudioCfg->type].slot_num   = pAudioCfg->channels;
    codec_set[pAudioCfg->type].m_s_sel    = pAudioCfg->master_or_slave;
    codec_set[pAudioCfg->type].delay      = pAudioCfg->delay;
    codec_set[pAudioCfg->type].bclk_polarity = polarity;
    return err ; 
}
#else
uint8_t Setup_Audio( AUDIO_CFG *pAudioCfg )
{
  return 0;
}
#endif


/*
*********************************************************************************************************
*                                           Update_Audio()
*
* Description : Update Audio path setting(CODEC and FM36) based on Setup_Audio() command .
* Argument(s) : None.
*             
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Update_Audio( void )
{
  /*
    uint8_t err;
    uint8_t index ;  
      
    APP_TRACE_INFO(("\r\nUpdate_Audio : [REC] = %d [PLAY] = %d\r\n", codec_set[0].flag, codec_set[1].flag));
    err = 0;     
    if( (codec_set[0].flag == 1)  && (codec_set[1].flag == 1) ) {
        if( (codec_set[0].sr !=  codec_set[1].sr) ||
            (codec_set[0].sample_len !=  codec_set[1].sample_len) ||
            (codec_set[0].format !=  codec_set[1].format)  ||
            (codec_set[0].m_s_sel !=  codec_set[1].m_s_sel)  ) {
            err = AUD_CFG_ERR;
            APP_TRACE_INFO(("\r\nERROR: [REC] conflict [PLAY] audio settings!\r\n"));
        }
        if( codec_set[0].slot_num >= codec_set[1].slot_num ) {
           index = 0; 
        } else {
           index = 1;
        }
    } else if( codec_set[0].flag == 1 ) {
        index = 0;
    } else if( codec_set[1].flag == 1 ) {
        index = 1;
    } else {        
        err = AUD_CFG_ERR;
    }    
    codec_set[0].flag = 0; //reset Cfg flag
    codec_set[1].flag = 0;
    if( err != NO_ERR ) {
        return err;
    }
    
    //codec_set[index].
    I2C_Mixer(I2C_MIX_FM36_CODEC);    
    err = Init_CODEC( &source_twi2,codec_set[index] );     //-------------temp by leo 
    //err = Init_CODEC( pAudioCfg->sr,  pAudioCfg->bit_length, i2s_tdm_sel, buf[4], buf[15]);
    I2C_Mixer(I2C_MIX_UIF_S);
    if( err != NO_ERR ) {
        APP_TRACE_INFO(("\r\nUpdate_Audio Init_CODEC ERROR: %d\r\n",err)); 
        return err;
    } 
#ifdef BOARD_TYPE_AB03  
    err = Init_FM36_AB03( codec_set[index].sr, Global_Mic_Mask[0], 1, 0, 1, 0 ); //Lin from SP1_RX, slot0~1
#elif defined BOARD_TYPE_UIF
    I2C_Mixer(I2C_MIX_FM36_CODEC);
    if( flag_bypass_fm36 == 0 ) {
        err = Init_FM36_AB03( codec_set[index].sr, Global_Mic_Mask[0], 1, 0, codec_set[index].sample_len, codec_set[index].format, 0 ); //Lin from SP1_RX, slot0~1
    } else{
        err = FM36_PWD_Bypass(); 
    }
    I2C_Mixer(I2C_MIX_UIF_S);
#endif
    if( err != NO_ERR ) {
        APP_TRACE_INFO(("\r\nUpdate_Audio ReInit_FM36 ERROR: %d\r\n",err)); 
        return err;
    }
        
//    if ( pAudioCfg->lin_ch_mask != 0 ) {
//        err = Set_AIC3204_DSP_Offset( mic_num );
//        if( err != NO_ERR ) {
//            APP_TRACE_INFO(("\r\nSetup_Audio Init AIC3204 ERROR: %d\r\n",err)); 
//        }
//    }
//    if( buf[16] != 0) {
//        Global_SPI_Record = 1; //set flag for SPI rec
//    }
    ////////////////////////////////////////////////////////////////////////////
    
    return 0 ; 
 */ 
    return 0;
}



/*
*********************************************************************************************************
*                                           Start_Audio()
*
* Description : Send command to start USB audio play/record.
* Argument(s) : cmd_type : record��== 1��/play��== 2��/record & play ��== 3��
*               padding :  used for usb audio BI/BO first package padding
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t Start_Audio( START_AUDIO start_audio )
{   
    uint8_t err;  
    uint8_t data  = 0xFF; 
    uint8_t ruler_id;    
    uint8_t buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_START_AUDIO, start_audio.type&0x03, start_audio.padding }; 
    
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 

    APP_TRACE_INFO(("\r\nStart_Audio : type = [%d], padding = [0x%X]\r\n", start_audio.type, start_audio.padding));    
    
    UART2_Mixer(3); 
    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf) );    
    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM );  
    if( err != NO_ERR  || data != 0  ) {         
        APP_TRACE_INFO(("\r\nStart_Audio ERROR: timeout = %d, ack data = %d\r\n",err, data));        
        return data; 
        
    } else {
        
        OS_ENTER_CRITICAL(); 
        for( ruler_id = 0 ; ruler_id < 4 ; ruler_id++ ) {       
            if( Global_Ruler_State[ruler_id] ==  RULER_STATE_SELECTED ) {//given: if mic selected, then ruler used
                Global_Ruler_State[ruler_id] = RULER_STATE_RUN;                 
            }      
        }
        OS_EXIT_CRITICAL();  
        
    }
    
    return 0 ;   
}
#else
uint8_t Start_Audio( START_AUDIO start_audio )
{
  return 0;
}
#endif


/*
*********************************************************************************************************
*                                           Stop_Audio()
*
* Description : Send command to stop USB audio play/record.
* Argument(s) : None.
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Stop_Audio( void )
{  
    uint8_t err   = 0xFF;  
    uint8_t data  = 0xFF;
    uint8_t ruler_id;     
    uint8_t buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_STOP_AUDIO };
    
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
#if OLD_CODE
    APP_TRACE_INFO(("\r\nStop_Audio\r\n"));
    UART2_Mixer(3); 
    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf)) ;    
    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM); 
    if( err != NO_ERR ) { 
        APP_TRACE_INFO(("\r\nStop_Audio ERROR: timeout\r\n")); 
        return err;
    }
    if( data != NO_ERR ) {
        APP_TRACE_INFO(("\r\nStop_Audio ERROR: %d\r\n ",data)); 
        return data; 
    } 
    
    //check if it is in SPI recording mode
    if( Global_SPI_Rec_Start == 1 ) {
        Global_SPI_Rec_Start = 0;
        Enable_SPI_Port();
    }
    Disable_Interrupt_For_iM501_IRQ();
    
   
//    err = Init_CODEC( 0 );
//    if( err != NO_ERR ) {
//        APP_TRACE_INFO(("\r\nStop_Audio Power Down CODEC ERROR: %d\r\n",err)); 
//    }
    
    OS_ENTER_CRITICAL(); 
    for( ruler_id = 0 ; ruler_id < 4 ; ruler_id++ ) {       
        if( Global_Ruler_State[ruler_id] ==  RULER_STATE_RUN ) {//given: if mic selected, then ruler used
            Global_Ruler_State[ruler_id] = RULER_STATE_SELECTED;            
        }                   
    }
    OS_EXIT_CRITICAL();
      
//clear mic toggle after each audio stop to avoid issues in scripts test using USBTEST.exe
#ifdef FOR_USE_USBTEST_EXE    
    for( ruler_id = 0; ruler_id < 4; ruler_id++ ) {
        Global_Mic_Mask[ruler_id] = 0 ;
    }
#endif
#else
    //here don't need stop audio via usart
#endif
    
    return 0 ;    
}


/*
*********************************************************************************************************
*                                           Reset_Audio()
*
* Description : Send command to reset USB audio data stream.
* Argument(s) : None.
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t Reset_Audio( void )
{  
    uint8_t err   = 0xFF;  
    uint8_t data  = 0xFF;    
    uint8_t buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_RESET_AUDIO };
    
    APP_TRACE_INFO(("Reset_Audio\r\n"));
    UART2_Mixer(3); 
    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf)) ;    
    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM); 
    if( err != NO_ERR ) { 
        APP_TRACE_INFO(("\r\nReset_Audio ERROR: timeout\r\n")); 
        return err;
    }
    if( data != NO_ERR ) {
        APP_TRACE_INFO(("\r\nReset_Audio ERROR: %d\r\n ",data)); 
        return data; 
    } 
     
    return 0 ;    
}
#else
uint8_t Reset_Audio( void )
{
  uint8_t err = 0;
  
  return err;
}
#endif

/*
*********************************************************************************************************
*                                       Get_Audio_Version()
*
* Description : Get USB audio MCU firmware version info, and stored in a global variable.
* Argument(s) : None.
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Get_Audio_Version( void )
{  
#if OLD_CODE
    uint8_t err;      
    uint8_t buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_GET_AUDIO_VERSION };
   
    UART2_Mixer(3); 
    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf)) ;    
    err = USART_Read_Timeout( AUDIO_UART, &Audio_Version, sizeof(Audio_Version), TIMEOUT_AUDIO_COM); 
    if( err != NO_ERR ) { 
        APP_TRACE_INFO(("\r\nGet_Audio_Version ERROR: timeout\r\n")); 
        return err;        
    } else {        
        APP_TRACE_INFO(("\r\nUSB Audio FW Version: %s\r\n ",Audio_Version));
    } 
#else
    //here read version info from flash directly
    if( g_pmeccStatus )
      return -1;
    
    //here read info from flash directly and don't need via usart;
#endif    
    return 0 ;   
}


    
uint8_t SPI_Rec_Start( VOICE_BUF_CFG *pSpi_rec_cfg )      // SPI_REC_CFG
{   
    uint8_t err   = 0xFF;  
    uint8_t data  = 0xFF;
    
    uint8_t buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_START_RD_VOICE_BUF };
           
    if( pSpi_rec_cfg->gpio_irq < 2 ) {
        APP_TRACE_INFO(("\r\nIRQ gpio support: UIF_GPIO_2 ~ UIF_GPIO_9 only!\r\n ",data)); 
        return AUD_CFG_SPI_REC_CONFLICT;
    } 
    if( Global_SPI_Rec_En == 0 ) {
       APP_TRACE_INFO(("\r\nGlobal_SPI_Rec channel = 0\r\n ",data)); 
       return AUD_CFG_SPI_REC_CONFLICT;
    }
    pSpi_rec_cfg->gpio_irq -= 2 ;//'cause UIF_GPIO connecting to Host is differnt from Audio
    
    APP_TRACE_INFO(("\r\nSPI_Rec_Start : Chip ID = %d, IRQ = GPIO[%d], spi.mode = %d, spi.speed = %d MHz\r\n",\
                         pSpi_rec_cfg->chip_id, pSpi_rec_cfg->gpio_irq, pSpi_rec_cfg->spi_mode, pSpi_rec_cfg->spi_speed / 1000000 ));
    
    Disable_SPI_Port(); //disabled host mcu SPI;
    
    UART2_Mixer(3); 
    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf) ); 
    USART_SendBuf( AUDIO_UART, ( uint8_t *)pSpi_rec_cfg, sizeof( VOICE_BUF_CFG ) ) ; 
    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM );  
    if( err != NO_ERR  || data != 0  ) {         
        APP_TRACE_INFO(("\r\nSPI_Rec_Start ERROR: timeout = %d, ack data = %d\r\n",err, data));  
        Enable_SPI_Port(); //Enabled host mcu SPI if failed to get resp from audio mcu        
        return data; 
    }
    
    Global_SPI_Rec_Start = 1; //set flag for SPI rec
    
    return 0 ;
}
     
/*
*********************************************************************************************************
*                                       Init_Ruler()
*
* Description : Communicate with ruler to check connected or not
*    
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   ruler connected
*               others :   =error code . ruler connection error,           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Init_Ruler( uint8_t ruler_slot_id ) //0 ~ 3
{
    uint8_t err ;

#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
      
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {      
        return RULER_STATE_ERR ;         
    } 
    OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    } 
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_EST, NULL, 0, 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Init ruler[%d] timeout!\r\n",ruler_slot_id));            
        } else {
            err = Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Init_Ruler[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));        
    }    
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}
                

/*
*********************************************************************************************************
*                                       Setup_Ruler()
*
* Description : Send ruler slot id to ruler for identification.
*             
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code . ruler connection error,           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Setup_Ruler( uint8_t ruler_slot_id ) //0 ~ 3
{    
    uint8_t err ;
    EMB_BUF        *pEBuf_Data; 
    uint8_t buf[] = { RULER_CMD_SET_RULER, ruler_slot_id };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    pEBuf_Data  = &Emb_Buf_Data; //Golbal var
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {      
        return RULER_STATE_ERR ;         
    } 
    
    OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Setup_Ruler[%d] timeout\r\n",ruler_slot_id));
        } else {            
            Ruler_Setup_Sync_Data = pEBuf_Data->data[0] ;
            APP_TRACE_INFO(("Get Ruler_Setup_Sync_Data : 0x%X\r\n",Ruler_Setup_Sync_Data));
            err = Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Setup_Ruler[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }    
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}


/*
*********************************************************************************************************
*                                       Get_Ruler_Type()
*
* Description : Get the specified ruler's type, and stored in a global variable, in which
*               bit7: 0-ruler, 1- handset. Other bits reserved.
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Get_Ruler_Type(  uint8_t ruler_slot_id )
{  
    uint8_t err ;
    EMB_BUF        *pEBuf_Data; 
    uint8_t buf[] = { RULER_CMD_GET_RULER_TYPE };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    pEBuf_Data  = &Emb_Buf_Data; //Golbal var
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {      
        return RULER_STATE_ERR ;         
    } 

    OSSemPend( UART_MUX_Sem_lock, 0, &err ); 
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Ruler_Type[%d] timeout\r\n",ruler_slot_id));
        } else {
            Global_Ruler_Type[ruler_slot_id] =  pEBuf_Data->data[0] ;
            err = Ruler_CMD_Result; //exe result from GACK 
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Get_Ruler_Type[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }          
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));        
    }       
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}


/*
*********************************************************************************************************
*                                       Read_Ruler_Status()
*
* Description : Get back specified ruler's POST status.
*             
* Argument(s) : ruler_slot_id: 0~ 3.
*               status_data:   pointer to the address that store the read status data 
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Read_Ruler_Status( uint8_t ruler_slot_id, unsigned short *status_data )
{    
    uint8_t err ;
    EMB_BUF        *pEBuf_Data; 
    uint8_t buf[] = { RULER_CMD_RAED_RULER_STATUS };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    pEBuf_Data  = &Emb_Buf_Data; //Golbal var
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {      
        return RULER_STATE_ERR ;         
    } 
    OSSemPend( UART_MUX_Sem_lock, 0, &err ); 
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Ruler_Status[%d] timeout\r\n",ruler_slot_id));
        } else {
            *status_data = (pEBuf_Data->data[1] << 8) + pEBuf_Data->data[0] ;       
            err = Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Read_Ruler_Status[%d] err = %d\r\n",ruler_slot_id,err));
            } 
        }   
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }        
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}


/*
*********************************************************************************************************
*                                       Read_Ruler_Info()
*
* Description : Get back specified ruler's infomation data.
*               And the read back data is stored in global varies : Emb_Buf_Data
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code . ruler connection error,           
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Read_Ruler_Info( uint8_t ruler_slot_id )
{    
    uint8_t  err ; 
    uint8_t  buf[] = { RULER_CMD_RAED_RULER_INFO }; 
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
 
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    }    
    OSSemPend( UART_MUX_Sem_lock, 0, &err );  
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Ruler_Info[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Ruler_CMD_Result;
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Read_Ruler_Info[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }    
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}


/*
*********************************************************************************************************
*                                       Write_Ruler_Info()
*
* Description : Write infomation data to specified ruler.
*               And before this function is called, the data to be written need have been stored in global varies : Emb_Buf_Cmd
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .       
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Write_Ruler_Info( uint8_t ruler_slot_id )
{
    uint8_t   err;
    unsigned short  data_length;
    uint8_t   temp;
    uint8_t  *pdata;
    uint8_t   buf[4];  
    EMB_BUF        *pEBuf_Cmd;        
  
 #if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    pEBuf_Cmd  = &Emb_Buf_Cmd; //Golbal var
    buf[0] =  RULER_CMD_WRITE_RULER_INFO;
    buf[1] =  EMB_DATA_FRAME;  
    buf[2] = (pEBuf_Cmd->length) & 0xFF;    
    buf[3] = ((pEBuf_Cmd->length)>>8) & 0xFF;     

    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    } 
    OSSemPend( UART_MUX_Sem_lock, 0, &err );    
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }  
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE != err ) { return err ; }
    pdata = pEBuf_Cmd->data;
    data_length = pEBuf_Cmd->length;
    while( data_length > 0 ){ 
        temp = data_length > (NOAH_CMD_DATA_MLEN-1) ? (NOAH_CMD_DATA_MLEN-1) : data_length ;  
        err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, pdata, temp, 0, buf, 1 ) ; 
        if( OS_ERR_NONE != err ) { break;}
        OSTimeDly(50); //wait for ruler operation
        data_length -= temp;
        pdata += temp;
    }
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Write_Ruler_Info[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Write_Ruler_Info[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }
    OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
    
}


/*
*********************************************************************************************************
*                                       Read_Mic_Cali_Data()
*
* Description : Get back specified ruler specified mic's calibration data.
*               And the read back data is stored in global varies : Emb_Buf_Data
* Argument(s) : ruler_slot_id : 0~ 3.
*               mic_id        : 0~ 7
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Read_Mic_Cali_Data(uint8_t ruler_slot_id, uint8_t mic_id)
{    
    uint8_t  err ; 
    uint8_t  buf[] = { RULER_CMD_READ_MIC_CALI_DATA, mic_id }; 
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
 
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    }   
   
    OSSemPend( UART_MUX_Sem_lock, 0, &err );   
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Mic_Cali_Data[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Ruler_CMD_Result; 
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Read_Mic_Cali_Data[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }      
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}


/*
*********************************************************************************************************
*                                       Write_Mic_Cali_Data()
*
* Description : Write calibration data to specified ruler specified mic.
*               And before this function is called, the data to be written need have been stored in global varies : Emb_Buf_Cmd
* Argument(s) : ruler_slot_id : 0~ 3.
*               mic_id        : 0~ 7
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .    
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Write_Mic_Cali_Data(uint8_t ruler_slot_id, uint8_t mic_id)
{    
    uint8_t   err;
    unsigned short  data_length;
    uint8_t   temp;
    uint8_t  *pdata;
    uint8_t   buf[5];  
    EMB_BUF        *pEBuf_Cmd;        
  
 #if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    pEBuf_Cmd  = &Emb_Buf_Cmd; //Golbal var
    buf[0] =  RULER_CMD_WRITE_MIC_CALI_DATA; 
    buf[1] =  mic_id;  
    buf[2] =  EMB_DATA_FRAME;  
    buf[3] = (pEBuf_Cmd->length) & 0xFF;    
    buf[4] = ((pEBuf_Cmd->length)>>8) & 0xFF;     

    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    } 
    OSSemPend( UART_MUX_Sem_lock, 0, &err ); 
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    } 
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE != err ) { return err ; }
    pdata = pEBuf_Cmd->data;
    data_length = pEBuf_Cmd->length;
    while( data_length > 0 ){ 
        temp = data_length > (NOAH_CMD_DATA_MLEN-2) ? (NOAH_CMD_DATA_MLEN-2) : data_length ;  
        err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, pdata, temp, 0, buf, 2 ) ; 
        if( OS_ERR_NONE != err ) { break;}
        OSTimeDly(50); //wait for ruler operation       
        data_length -= temp;
        pdata += temp;        
    }
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Write_Mic_Cali_Data[%d][%d] timeout\r\n",ruler_slot_id, mic_id));
        } else {
            err = Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Write_Mic_Cali_Data[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }
    OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
    
}


/*
*********************************************************************************************************
*                                       Update_Mic_Mask()
*
* Description : Update specified ruler's all mic's active state.
* Argument(s) : ruler_slot_id : 0~ 3.
*               mic_mask      : bit[0..31]. 0 - deactive, 1 - active.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .    
*
* Note(s)     : Support: Ruler(8Mic) for Ruler
*                        Handset(16Mic) for H01/H02/H02A
*                        Handset(18Mic) for H03
*********************************************************************************************************
*/
uint8_t Update_Mic_Mask( uint8_t ruler_slot_id, uint32_t mic_mask )
{    
    uint8_t err ;
    uint8_t buf_size_send ;
    uint8_t buf[] = { RULER_CMD_TOGGLE_MIC, mic_mask&0xFF, (mic_mask>>8)&0xFF,
                            (mic_mask>>16)&0xFF,  (mic_mask>>24)&0xFF };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    }  
    
    OSSemPend( UART_MUX_Sem_lock, 0, &err );  
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    if( Global_Ruler_Type[ruler_slot_id] == RULER_TYPE_H03 ) {
        buf_size_send = 5; //H03 cmd data size = 1+4 for 16> mic
    } else {
        buf_size_send = 3; //Default cmd data size = 1+2 for <16 mic
    }
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, buf_size_send, 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Update_Mic_Mask for Ruler[%d] timeout\r\n",ruler_slot_id));
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
}

/*
*********************************************************************************************************
*                                       Ruler_Active_Control()
*
* Description : Active/Deactive ruler(LED)when play and record start/stop.  
* Argument(s) : active_state : 0 - deactive ruler (LED)
*                              1 - active ruler (LED).
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .    
*
* Note(s)     : Support Ruler(8Mic) and Handset(16Mic)
*********************************************************************************************************
*/
uint8_t Ruler_Active_Control( uint8_t active_state )  
{    
    uint8_t err ;
    uint8_t ruler_id;
    uint8_t buf[] = { RULER_CMD_ACTIVE_CTR, active_state };

#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    err = 0;        
    
    for( ruler_id = 0 ; ruler_id < 4 ; ruler_id++ ) {   
        //check ruler connection state 
        if( //RULER_TYPE_MASK(Global_Ruler_Type[ruler_id]) == RULER_TYPE_HANDSET ||
            Global_Ruler_State[ruler_id] < RULER_STATE_CONFIGURED || 
            Global_Mic_Mask[ruler_id] == 0 ) {      
            continue;       
        } 
        APP_TRACE_INFO(("Ruler[%d]_Active_Control : [%d]\r\n",ruler_id,active_state));      
        OSSemPend( UART_MUX_Sem_lock, 0, &err );
        if( Global_Ruler_Index != ruler_id ) {
            Check_UART_Mixer_Ready();
            //OS_ENTER_CRITICAL(); 
            Global_Ruler_Index = ruler_id ; //for ruler status switch in TX/RX/Noah 
            //OS_EXIT_CRITICAL();  
            UART1_Mixer( ruler_id );
        }   
        err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
        if( OS_ERR_NONE == err ) {
            OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
            if( OS_ERR_TIMEOUT == err ) {
                APP_TRACE_INFO(("Ruler[%d]_Active_Control timeout\r\n",ruler_id));
            } else {
                err = Ruler_CMD_Result; //exe result from GACK
                if(OS_ERR_NONE != err ){
                    APP_TRACE_INFO(("Ruler[%d]_Active_Control err = %d\r\n",ruler_id,err));
                }
            }
            
        } else {
            APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_id,err));
        }    
        OSSemPost( UART_MUX_Sem_lock );
        if( err != NO_ERR ) {
            break;
        }
    }        
    return err ;    
}


/*
*********************************************************************************************************
*                                       Get_Ruler_Version()
*
* Description : Get back specified ruler's version info.
*               And the version data is stored in global varies : Emb_Buf_Data
* Argument(s) : ruler_slot_id : 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Get_Ruler_Version( uint8_t ruler_slot_id )
{  
    uint8_t err ;
    uint8_t buf[] = { RULER_CMD_GET_RULER_VERSION };
    EMB_BUF      *pEBuf_Data;         
      
    pEBuf_Data  = &Emb_Buf_Data;  //Global var   
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {      
        return RULER_STATE_ERR ;         
    }  
    
    OSSemPend( UART_MUX_Sem_lock, 0, &err ); 
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Get_Ruler_Version[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Ruler_CMD_Result;
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Get_Ruler_Version[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        if(err == OS_ERR_NONE ) {
            APP_TRACE_INFO(("Ruler[%d] FW Version: %s\r\n",ruler_slot_id, pEBuf_Data->data)); 
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }    
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;        
}
  


/*
*********************************************************************************************************
*                                       FLASHD_Write_Safe()
*
* Description : Add code area protection for FLASHD_Write()
*               Writes a data buffer in the internal flash. This function works in polling
*               mode, and thus only returns when the data has been effectively written.    
* Argument(s) :  address  Write address.
*                pBuffer  Data buffer.
*                size     Size of data buffer in bytes.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t FLASHD_Write_Safe( uint32_t address, const void *pBuffer,  uint32_t size)
{
    uint8_t err;
    if( size == 0 ) {
        return 0;
    }
    if( address < (AT91C_IFLASH + FLASH_HOST_FW_BIN_MAX_SIZE) ) {
        APP_TRACE_INFO(("ERROR: this operation wanna flush code area!\r\n"));  
        return FW_BIN_SAVE_ADDR_ERR;
    }
    err = FLASHD_Write(  address, pBuffer, size );
    return err;  
    
}
#else
uint8_t FLASHD_Write_Safe( uint32_t address, const void *pBuffer,  uint32_t size)
{
  uint8_t err = 0;
  
  return err;
}
#endif

/*
*********************************************************************************************************
*                                       Read_Flash_State()
*
* Description : Save ruler FW bin file to flash
*               
* Argument(s) : *pFlash_Info : pointer to FLASH_INFO type data where to save read data
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Read_Flash_State( FLASH_INFO  *pFlash_Info, uint32_t flash_address )
{
    
    *pFlash_Info = *(FLASH_INFO *)flash_address;    
    
}


/*
*********************************************************************************************************
*                                       Write_Flash_State()
*
* Description : Save ruler FW bin file to flash
*               
* Argument(s) : *pFlash_Info : pointer to FLASH_INFO type data need to be saved
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t Write_Flash_State( FLASH_INFO   *pFlash_Info, uint32_t flash_address )
{
    
    uint8_t err;   
    //save state to flash
    pFlash_Info->s_w_counter++ ;
    err = FLASHD_Write_Safe( flash_address, pFlash_Info, AT91C_IFLASH_PAGE_SIZE); 
    if(err != NO_ERR ) {                     
        APP_TRACE_INFO(("ERROR: Write flash state failed!\r\n"));  
    }
    
    return err;
    
}
#else
uint8_t Write_Flash_State( FLASH_INFO   *pFlash_Info, uint32_t flash_address )  //this fun need reimplement
{
  uint8_t err = 0;
  
  return err;
}
#endif


/*
*********************************************************************************************************
*                                       Save_Ruler_FW()
*
* Description : Save ruler FW bin file to flash
*               
* Argument(s) :  cmd  :  1~ 3.
*               *pBin : pointer to bin file data packge to be wriiten to flash
*               *pStr : pointer to file name string
*                size : bin package file size 
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t Save_Ruler_FW( uint32_t cmd, uint8_t *pBin, uint8_t *pStr, uint32_t size )
{  
    uint8_t err; 
    static uint32_t flash_addr = FLASH_ADDR_FW_BIN;
 
    FLASH_INFO    flash_info;
    
    err = NO_ERR;
    Read_Flash_State(&flash_info, FLASH_ADDR_FW_STATE);
     
    switch( cmd ) {
        case FW_DOWNLAD_CMD_START :
            APP_TRACE_INFO(("Start loading ruler bin file to AB01 flash ... \r\n"));
            flash_addr = FLASH_ADDR_FW_BIN;                
            flash_info.f_w_state = FW_DOWNLAD_STATE_UNFINISHED ;
            flash_info.bin_size  = 0;
        break;   
        case FW_DOWNLAD_CMD_DOING :
            APP_TRACE_INFO(("> ")); 
            if( flash_info.f_w_state != FW_DOWNLAD_STATE_UNFINISHED ) {
                APP_TRACE_INFO(("ERROR: flash state not match!\r\n"));
                err  =  FW_BIN_STATE_0_ERR;                
            } 
        break;
        case FW_DOWNLAD_CMD_DONE :
            APP_TRACE_INFO((">\r\n")); 
            if( flash_info.f_w_state != FW_DOWNLAD_STATE_UNFINISHED ) {
                APP_TRACE_INFO(("ERROR: flash state not match!\r\n"));
                err  =  FW_BIN_STATE_1_ERR;
                break;
            }
            flash_info.f_w_state = FW_DOWNLAD_STATE_FINISHED ;
            flash_info.f_w_counter++;            
         break;
         
         default:
            APP_TRACE_INFO(("ERROR:  Save ruler FW bad cmd!\r\n"));
            err = FW_BIN_SAVE_CMD_ERR;    
         break;
        
    }
    if( err != NO_ERR ) {
        return err;
    }    
    Buzzer_OnOff(1);               
    LED_Toggle(LED_DS2);    
    err = FLASHD_Write_Safe( flash_addr, pBin, size ); 
//    Buzzer_OnOff(0);
    BSP_BUZZER_Toggle( 0 );
    if(err != NO_ERR ) {                     
        APP_TRACE_INFO(("ERROR: Write MCU flash failed!\r\n"));
        return err;
    }
    flash_addr += size;
    flash_info.bin_size   = flash_addr - FLASH_ADDR_FW_BIN ;
    strcpy(flash_info.bin_name, (char const*)pStr);  
    if( cmd != FW_DOWNLAD_CMD_DOING ) {        
        err = Write_Flash_State( &flash_info, FLASH_ADDR_FW_STATE ); 
        if( err == NO_ERR && cmd == FW_DOWNLAD_CMD_DONE ) { 
              APP_TRACE_INFO(("Bin file[%d Btyes] saved successfully!\r\n",flash_info.bin_size));     
        }   
    } 
    return err;      
}
#else
uint8_t Save_Ruler_FW( uint32_t cmd, uint8_t *pBin, uint8_t *pStr, uint32_t size )
{
  uint8_t err = 0;

  
  return err;  
}
#endif

/*
*********************************************************************************************************
*                                       Save_DSP_VEC()
*
* Description : Save ruler FW bin file to flash
*               
* Argument(s) :  cmd  :  1~ 3.
*               *pBin : pointer to bin file data packge to be wriiten to flash
*               *pStr : pointer to file name string
*                size : bin package file size 
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : Vec size usually not exceed 2kB, so one emb package should be ok.
*********************************************************************************************************
*/
#if OLD_CODE
uint8_t Save_DSP_VEC( MCU_FLASH *p_dsp_vec )
{  
    uint8_t err; 
    uint32_t flash_addr;
    uint32_t index;
    FLASH_INFO   flash_info;
    
    
    if( (p_dsp_vec->addr_index == 0 ) || (p_dsp_vec->addr_index > FLASH_ADDR_FW_VEC_NUM) || (p_dsp_vec->data_len > FLASH_ADDR_FW_VEC_SIZE )  ) {
        return MCU_FLASH_OP_ERR;
    }
    
    err   = NO_ERR;
    index = p_dsp_vec->addr_index;
    flash_addr = FLASH_ADDR_FW_VEC + index * FLASH_ADDR_FW_VEC_SIZE;
    
    Read_Flash_State(&flash_info, FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * index );
             
    Buzzer_OnOff(1);               
    LED_Toggle(LED_DS2);    
    err = FLASHD_Write_Safe( flash_addr, p_dsp_vec->pdata, p_dsp_vec->data_len ); 
    Buzzer_OnOff(0); 
    if(err != NO_ERR ) {                     
        APP_TRACE_INFO(("ERROR: Write MCU flash failed!\r\n"));
        return err;
    }
    if( flash_info.flag != 0x55 ) {
        flash_info.flag = 0x55;
        flash_info.f_w_counter = 0;
        flash_info.s_w_counter = 0;
    }
    flash_info.f_w_state = FW_DOWNLAD_STATE_FINISHED ;
    flash_info.f_w_counter++;
    flash_info.s_w_counter++;
    flash_info.bin_size  = p_dsp_vec->data_len ;
    strcpy(flash_info.bin_name, (char const*)(p_dsp_vec->pStr)); 
          
    err = Write_Flash_State( &flash_info,  FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * index ); 
    if( err == NO_ERR  ) { 
        APP_TRACE_INFO(("Vec file[%d][%s][%d Btyes] saved successfully!\r\n", index, flash_info.bin_name, flash_info.bin_size));     
    }   
     
    return err;  
    
}
#else
uint8_t Save_DSP_VEC( MCU_FLASH *p_dsp_vec )
{
  uint8_t err = 0;
  
  return err;
}
#endif
/*
*********************************************************************************************************
*                                       Set_DSP_VEC()
*
* Description : set config setting for load DSP��vector from flash
*               
* Argument(s) :  *p_dsp_vec_cfg :  .
*              
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : 
*********************************************************************************************************
*/
 
uint8_t Set_DSP_VEC( SET_VEC_CFG *p_dsp_vec_cfg )
{  
    uint8_t err; 
    OS_SEM_DATA  sem_data;
    
    err = NO_ERR;
    
    if( (p_dsp_vec_cfg->vec_index_a > 7) || (p_dsp_vec_cfg->vec_index_b > 7) || (p_dsp_vec_cfg->delay > 65535 ) ) {
        Global_VEC_Cfg.flag        = 0; //means error
        return FW_VEC_SET_CFG_ERR;
    }
    Global_VEC_Cfg.vec_index_a = p_dsp_vec_cfg->vec_index_a;
    Global_VEC_Cfg.vec_index_b = p_dsp_vec_cfg->vec_index_b;
    Global_VEC_Cfg.delay       = p_dsp_vec_cfg->delay;  
    Global_VEC_Cfg.type        = p_dsp_vec_cfg->type;
    Global_VEC_Cfg.gpio        = p_dsp_vec_cfg->gpio;
    Global_VEC_Cfg.flag        = 0x55; //means cfg ok    
    Global_VEC_Cfg.trigger_en  = p_dsp_vec_cfg->trigger_en; 
    Global_VEC_Cfg.pdm_clk_off = p_dsp_vec_cfg->pdm_clk_off; 
    Global_VEC_Cfg.if_type     = p_dsp_vec_cfg->if_type; 
        
    if( Global_VEC_Cfg.trigger_en ) {
        err = MCU_Load_Vec(1);
    } else {
        //check if it's in MCU_Load_Vec() now
        OSSemQuery(Load_Vec_Sem_lock, &sem_data);
        if( sem_data.OSCnt == 0 ) {
            OSTimeDlyResume( APP_CFG_TASK_USER_IF_PRIO );
            OSSemPend( Load_Vec_Sem_lock, 0, &err );
            OSSemPost( Load_Vec_Sem_lock );
        }
        I2C_Mixer(I2C_MIX_FM36_CODEC);
        err = FM36_PDMADC_CLK_OnOff(1,0); //enable PDM clock
        I2C_Mixer( I2C_MIX_UIF_S ); 
    }
    return err;  
    
}


/*
*********************************************************************************************************
*                                       Update_Ruler_FW()
*
* Description :  Write firmware to specified ruler's MCU flash
*               
* Argument(s) :  ruler_slot_id :  0~ 3.     
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : Do not care if ruler is attached or not.Because host can not detect ruler if FW was crashed.
*********************************************************************************************************
*/
uint8_t Update_Ruler_FW( uint8_t ruler_slot_id )
{
    uint8_t err;
    uint32_t  flash_addr; 
    FLASH_INFO   *pFlash_Info;
    uint8_t Buf[9];
    uint8_t i;
    
    err = NO_ERR;
    flash_addr  = FLASH_ADDR_FW_BIN;
    pFlash_Info = (FLASH_INFO *)FLASH_ADDR_FW_STATE ;
        
    if( pFlash_Info->f_w_state != FW_DOWNLAD_STATE_FINISHED ) {
        APP_TRACE_INFO(("ERROR: FW bin file missed!\r\n"));        
        return FW_BIN_STATE_ERR;
    }
    
    APP_TRACE_INFO(("Start updating ruler[%d] firmware to \"%s\" version ...\r\n",ruler_slot_id,pFlash_Info->bin_name)); 
    memset(Buf,'d',sizeof(Buf)); //send 'd' to start download  
    Ruler_Power_Switch(0);   //power off ruler  
    OSTimeDly(200);   
    for( i = 0; i < 4; i++ ) {
        Global_Ruler_State[i] = RULER_STATE_DETACHED ;
    }
    UART_Init(RULER_UART,  NULL,  115200 );   //Init Ruler to inquire mode
    Port_Detect_Enable(0); //disable ruler detect
    
    OSSemPend( UART_MUX_Sem_lock, 0, &err ); 
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    Ruler_Power_Switch(1);   //power on ruler
    OSTimeDly(200);
    err = USART_SendBuf( RULER_UART, Buf,  sizeof(Buf));
    if( OS_ERR_NONE == err ) {     
        OSTimeDly(800);
        err = USART_Read_Timeout( RULER_UART, Buf, 3, 5000 );
        if( OS_ERR_NONE == err && ( Buf[0] == 'c' || Buf[0] == 'C' )) {
            Global_Ruler_State[ruler_slot_id] = RULER_STATE_RUN ;
            err = Xmodem_Transmit( (uint8_t *)flash_addr, pFlash_Info->bin_size );
            Global_Ruler_State[ruler_slot_id] = RULER_STATE_DETACHED ;            
        }         
    }
    if( OS_ERR_NONE != err ) {
        APP_TRACE_INFO(("\r\nFailed to init ruler bootloader. Err Code = [0x%X]\r\n", err));        
    } else {
        APP_TRACE_INFO(("\r\nUpdate ruler[%d] firmware successfully!\r\n", ruler_slot_id));   
    }
    Port_Detect_Enable(1); //enable ruler detect
#if OLD_CODE
    UART_Init( RULER_UART,  ISR_Ruler_UART,  115200 );  //Init Ruler back to interuption mode
#endif
    Ruler_Power_Switch(0);   //power off ruler  
    OSTimeDly(500);    
    Ruler_Power_Switch(1);   //power on ruler
    OSSemPost( UART_MUX_Sem_lock ); 
    return err ;    
    
}
//uint8_t Update_Ruler_FW( uint8_t ruler_slot_id )
//{
//    uint8_t err;
//    uint32_t  flash_addr; 
//    FLASH_INFO   *pFlash_Info;
//    
//    err = NO_ERR;
//    flash_addr  = FLASH_ADDR_FW_BIN;
//    pFlash_Info = (FLASH_INFO *)FLASH_ADDR_FW_STATE ;
//        
//    if( pFlash_Info->f_w_state != FW_DOWNLAD_STATE_FINISHED ) {
//        APP_TRACE_INFO(("ERROR: FW bin file missed!\r\n"));        
//        return FW_BIN_STATE_ERR;
//    }
//    
//    APP_TRACE_INFO(("Start updating MCU FW to [%s] on ruler[%d]...\r\n",pFlash_Info->bin_name,ruler_slot_id)); 
//    
//    UART_Init(RULER_UART,  NULL,  115200 );    //Init Ruler as no ISR  
//    
//    UART1_Mixer( ruler_slot_id );
//    Check_UART_Mixer_Ready();
//    if( USART_Start_Ruler_Bootloader() ) {  
//        APP_TRACE_INFO(("Failed to init ruler bootloader!\r\n"));     
//    }
//    
//    err = Xmodem_Transmit( (uint8_t *)flash_addr, pFlash_Info->bin_size);      
//         
//    UART_Init(RULER_UART,  ISR_Ruler_UART,  115200 );    //Init Ruler back to ISR 
//    
//    return err ;    
//    
//}


/*
*********************************************************************************************************
*                                       Toggle_Mic()
*
* Description : Toggle specified mic's active state by sending command to related ruler and updating 
*               FPGA mic signal switch array. 
*               One mic One time.
* Argument(s) : pdata : pointer to TOGGLE_MIC structure data
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Toggle_Mic(  TOGGLE_MIC *pdata )
{  
#ifdef BOARD_TYPE_UIF 
    return 0;
#else
    
    uint8_t  err ;
    uint8_t  id;
    uint32_t   mic_mask;  
    uint32_t   fpga_mask;
    
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    fpga_mask = 0;
    //check ruler connection state 
    if( Global_Ruler_State[pdata->ruler_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    }  
    APP_TRACE_INFO(("Toggle Ruler[%d]-Mic[%d] : %d  : ", pdata->ruler_id, pdata->mic_id, pdata->on_off )); 
    OS_ENTER_CRITICAL(); 
    mic_mask = Global_Mic_Mask[pdata->ruler_id];
    OS_EXIT_CRITICAL();  
    mic_mask &= ~( 1<<(pdata->mic_id));
    mic_mask |=  (pdata->on_off&0x01)<<( pdata->mic_id);
    err = Update_Mic_Mask( pdata->ruler_id, mic_mask );
    APP_TRACE_INFO((" %s [0x%X]\r\n", err == OS_ERR_NONE ? "OK" : "FAIL" , err )); 
    if( OS_ERR_NONE != err ) {        
        return err;    
    }
    OS_ENTER_CRITICAL(); 
    Global_Mic_Mask[pdata->ruler_id] = mic_mask; 
    //APP_TRACE_INFO(("Update Ruler[%d] Mic_Mask:  %d\r\n",pdata->ruler_id,Global_Mic_Mask[pdata->ruler_id]));   
    if( mic_mask == 0 ) {      
        Global_Ruler_State[pdata->ruler_id] = RULER_STATE_CONFIGURED;         
    } else {
        Global_Ruler_State[pdata->ruler_id] = RULER_STATE_SELECTED;  
    }
    OS_EXIT_CRITICAL();
    if( RULER_TYPE_MASK( Global_Ruler_Type[pdata->ruler_id] ) == RULER_TYPE_RULER ) { //ruler
        for( id = 0; id < 4; id++ ) {
            fpga_mask += (Global_Mic_Mask[id]&0xFF) << (id<<3);
        }
    } else { //handset
       fpga_mask = 0x3F << ((pdata->ruler_id)<<3);
    }
    Init_FPGA(fpga_mask);
    return err; 
#endif
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
uint8_t Set_Volume(  const DataSource *pSource,SET_VOLUME *pdata )
{  
  /*
    uint8_t  err ;
    
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
    err = CODEC_Set_Volume( pSource,pdata->spk, pdata->lout, pdata->lin );
    if( OS_ERR_NONE != err ) {    
        APP_TRACE_INFO(( "FAIL [0x%X]\r\n", err )); 
        I2C_Mixer(I2C_MIX_UIF_S); 
        return err;    
    }
    APP_TRACE_INFO(( "OK\r\n" )); 
    
    I2C_Mixer(I2C_MIX_UIF_S); 
    
    return err; 
  */
}



/*
*********************************************************************************************************
*                                       Reset_Mic_Mask()
*
* Description : Reset all mics to deactived state on the specified rulers and update FPGA mic signal switch array.
* Argument(s) : pInt : pointer to a int data, the 4 bytes of wihch control 4 ruler's all mic need be 
*               reset to deactive state or not.
*                      1 - deactive all mics on this ruler
*                      0 - do nothing. ignore the reset operation
* Return(s)   : NO_ERR :   execute successfully
*               others :   = error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Reset_Mic_Mask(  uint32_t *pInt )
{  
    uint8_t  err ;
    uint8_t  id;
    uint8_t  *pChar;  
    uint32_t   fpga_mask;
    
    fpga_mask = 0;    
    pChar     = (uint8_t *)pInt;
    err       = 0;

    for( id = 0; id < 4; id++ ) {        
        if( *(pChar+id) == 0 ) { 
            continue;
        }
        if( Global_Ruler_State[id] < RULER_STATE_CONFIGURED ) { //why not RULER_STATE_SELECTED  ? Because UI need reset mic in any case
            continue;
        }
        Global_Ruler_State[id] = RULER_STATE_CONFIGURED ;
        err = Update_Mic_Mask( id, 0 );
        if( OS_ERR_NONE != err ) {        
            return err;    
        } 
        Global_Mic_Mask[id] = 0;  
        if( RULER_TYPE_MASK( Global_Ruler_Type[id] ) == RULER_TYPE_RULER ) {//ruler 
            fpga_mask += (Global_Mic_Mask[id]&0xFF) << (id<<3);
            
        } else {
            fpga_mask += 0x3F << (id<<3); //handset choose the lowest slot H01
        }        
    }
    
    Init_FPGA(fpga_mask);    
    return err;  
}


/*
*********************************************************************************************************
*                                       Ruler_Port_LED_Service()
*
* Description : Control the ruler port identify LED state:  
*               turn on LED after ruler configured, blink LED during recording
* Argument(s) : None.
* Return(s)   : None.
* Note(s)     : None.
*********************************************************************************************************
*/
void Ruler_Port_LED_Service( void )
{    
    static uint32_t counter; 
    static uint32_t counter_buz;    
    uint8_t ruler_id;
    uint8_t ruler_state;    
    uint8_t LED_Freq;
    uint8_t post_err_flag;

    LED_Freq      = 0x3F; 
    post_err_flag = 0;
    
    for( ruler_id = 0 ; ruler_id < 4 ; ruler_id++ ) {
      
        ruler_state = Global_Ruler_State[ruler_id];  
        if( Global_Bridge_POST != NO_ERR ) { //if POST err, start all LED 
            ruler_state = RULER_STATE_RUN ;
            post_err_flag = 1;
        }
        switch( ruler_state ) {
          
            case RULER_STATE_DETACHED :
            case RULER_STATE_ATTACHED :
                LED_Clear( LED_P0 + ruler_id );
            break;            
            case RULER_STATE_CONFIGURED :
            case RULER_STATE_SELECTED :  
                LED_Set( LED_P0 + ruler_id );
            break;            
            case RULER_STATE_RUN :
                if( (counter & LED_Freq) == 0 ) {
                    LED_Toggle( LED_P0 + ruler_id );   
                    if( post_err_flag== 1 && ruler_id == 0 && (counter_buz++ < 6 ) ) {
                        //Buzzer_Toggle(); //buzzer off id POST err 
                        Buzzer_OnOff( counter_buz&0x01 );   //fix long buz issue in some case                  
                    }
                }
            
            default:              
            break;
        }
  
    }    
    counter++;    
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
uint8_t AB_POST( const DataSource *pSource )
{
  /*
    uint8_t  err;  
    APP_TRACE_INFO(("\r\nStart Audio Bridge POST :\r\n"));    
    //Enable_FPGA();
    
    ///////////////////////////
    APP_TRACE_INFO(("\r\n1. CODEC... \r\n"));
    I2C_Mixer(I2C_MIX_FM36_CODEC);
    codec_set[0].sr = SAMPLE_RATE_DEF;
    codec_set[0].sample_len = SAMPLE_LENGTH;
    codec_set[0].format = 0; //I2S
    codec_set[0].slot_num = 8; //8 channels
    codec_set[0].m_s_sel = 0; //master
    codec_set[0].flag = 0; //reset Cfg flag
    codec_set[1].flag = 0;
    err = Init_CODEC( pSource,codec_set[0] );  
    I2C_Mixer(I2C_MIX_UIF_S);
    if( err != NO_ERR ) {
        Global_Bridge_POST = POST_ERR_CODEC;
        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
        return Global_Bridge_POST;
    } else {
        APP_TRACE_INFO(("\r\n---OK\r\n"));
    }
    ////////////////////////////
    APP_TRACE_INFO(("\r\n2. FM36 DSP... \r\n"));
#ifdef BOARD_TYPE_AB03   
    err = Init_FM36_AB03( SAMPLE_RATE_DEF, 0, 1, 0, 0, 1, 0 ); //Lin from SP1.Slot0
#elif defined BOARD_TYPE_UIF
    I2C_Mixer(I2C_MIX_FM36_CODEC); 
    err = Init_FM36_AB03( SAMPLE_RATE_DEF, 0, 1, 0, SAMPLE_LENGTH, 1, 1  ); //force reset FM36, Lin from SP1.Slot0
    I2C_Mixer(I2C_MIX_UIF_S);
#else 
    err = Init_FM36( SAMPLE_RATE_DEF );
#endif
    if( err != NO_ERR ) {
        Global_Bridge_POST = POST_ERR_FM36;
        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
        return Global_Bridge_POST;
    } else {
        APP_TRACE_INFO(("\r\n---OK\r\n"));
    }  
    //////////////////////////////
    APP_TRACE_INFO(("\r\n3. AUDIO MCU... \r\n"));
    err = Get_Audio_Version();
    if( err != NO_ERR ) {
        Global_Bridge_POST = POST_ERR_AUDIO;
        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
        return Global_Bridge_POST;
    } else {
        APP_TRACE_INFO(("\r\n---OK\r\n"));
    }
//    err = Stop_Audio();
//    if( err != NO_ERR ) {
//        Global_Bridge_POST = POST_ERR_AUDIO;
//        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
//        return Global_Bridge_POST;
//    }    
   
    //Config_PDM_PA();
    
    
//    APP_TRACE_INFO(("\r\n4. external CODEC... \r\n"));
//    err = Init_CODEC_AIC3204( SAMPLE_RATE_DEF );    
//    if( err != NO_ERR ) {
//        Global_Bridge_POST = POST_ERR_CODEC;
//        APP_TRACE_INFO(("\r\n---Error : %d\r\n",err));
//        return ;
//    } else {
//        APP_TRACE_INFO(("\r\n---OK\r\n"));
//    }
    
    //Disable_FPGA(); 
    //Ruler_Power_Switch(1); 
    
//    err = Init_CODEC( 0 );
//    if( err != NO_ERR ) {
//        Global_Bridge_POST = POST_ERR_CODEC ;
//        APP_TRACE_INFO(("\r\nPower Down CODEC ERROR: %d\r\n",err)); 
//    }
    
    return err;
*/    
}



/*
*********************************************************************************************************
*                                       Ruler_POST()
*
* Description : Get back specified ruler Power-On-Self-Test status. 
*
* Argument(s) : ruler_id :  0~ 3
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code . 
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t Ruler_POST( uint8_t ruler_id )
{
    uint8_t  err;
    unsigned short result;   
    
    APP_TRACE_INFO(("\r\nRuler[%d] POST status check... \r\n",ruler_id)); 

    err = Read_Ruler_Status( ruler_id, &result);
    if( err == RULER_STATE_ERR ) { //no ruler attached
        return err;;
    }      
    if( err != NO_ERR ) {
       return err;
    }
    if( result != 0 ) {
        if( result != 0x8000 ) {        
            APP_TRACE_INFO(("\r\n---Error Ruler[%d]: %d-0x%X\r\n",ruler_id,err,result));
            return 1; 
        } else {
            APP_TRACE_INFO(("\r\n---WARNING Ruler[%d]: Mic calibration data NOT Initialized!\r\n",ruler_id));  
        }
    } 
    APP_TRACE_INFO(("\r\n---OK\r\n"));  
         
    return err;
}


/*
*********************************************************************************************************
*                                       simple_test_use()
*
* Description : debug use.
*
* Argument(s) : None.
* Return(s)   : None.
* Note(s)     : None.
*********************************************************************************************************
*/
void simple_test_use( void )
{      
    APP_TRACE_INFO(("\r\nHi,man. Simple play/rec test triggered...\r\n"));   
    
#if 0  
    
 //R01      
    TOGGLE_MIC toggle_mic[6] = {    
                                    {0, 6, 1 }, {0, 7, 1 }, {0, 8, 1 },
                                    {0, 12, 1 }, {0, 13, 1 }, {0, 14, 1 }  
                                }; 
  
    for (uint8_t i = 0; i< 6 ; i++ ) {
        Toggle_Mic(&toggle_mic[i]); 
    } 
    
#else
    
//H01
    Update_Mic_Mask( 0, 0x3f); 
    Init_FPGA(0x3F);
    Global_Ruler_State[0] = RULER_STATE_RUN; 
        
    AUDIO_CFG audio_config_play = {SAMPLE_RATE_DEF, AUDIO_TYPE_PLAY, 6 };
    AUDIO_CFG audio_config_rec  = {SAMPLE_RATE_DEF, AUDIO_TYPE_REC,  6 };
    Setup_Audio( &audio_config_play );                   
    Setup_Audio( &audio_config_rec );                 
    //Start_Audio( AUDIO_START_PALYREC ); 
    
#endif
    
}


uint8_t Ruler_Setup_Sync( uint8_t ruler_slot_id )
{
    uint8_t err ;
    uint8_t buf[] = { RULER_CMD_SETUP_SYNC, Ruler_Setup_Sync_Data, ruler_slot_id };

#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {      
        return RULER_STATE_ERR ;         
    } 
    
    OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    
    err = pcSendDateToBuf( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Ruler_Setup_Sync[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Ruler_Setup_Sync[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        
    } else {
        APP_TRACE_INFO(("Ruler[%d] pcSendDateToBuf failed: %d\r\n",ruler_slot_id,err));
    }    
    OSSemPost( UART_MUX_Sem_lock );    
    return err ;    
        
}


void Debug_Audio( void ) 
{
    
   AUDIO_CFG    AudioCfg;
   START_AUDIO  start_audio;
   
   AudioCfg.channels = 8;
   AudioCfg.bit_length = 32;
   AudioCfg.sr = 16000;
   AudioCfg.type = 1; //play
   
   start_audio.type = 2; //play
   
   Setup_Audio(&AudioCfg);      
   Start_Audio( start_audio ); //play
   
}
