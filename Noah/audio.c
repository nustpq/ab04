/*
*********************************************************************************************************
*                                          UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

#include "stdio.h"
#include "stdint.h" 
#include "uif_usb.h"  
#include "defined.h"
#include "audio.h"
#include "uif_i2s.h"
#include "noah_cmd.h"
#include "codec.h"

/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                               on the
*                                    Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : audio.c
* Version       : V0.0.1
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :editor from uif board first version that writed by PQ.
*********************************************************************************************************
*/



unsigned char flag_bypass_fm36; 
volatile unsigned char  Global_SPI_Rec_Start = 0;
volatile unsigned char  Global_SPI_Rec_En = 0;

CODEC_SETS   Codec_Set[2][2]; //2 //2 codec, play/rec
SET_VEC_CFG  Global_VEC_Cfg;

//because AB04 has two ssc port,so extend to 4 sturct;
AUDIO_CFG  Audio_Configure_Instance[ 2 ];


extern void Init_Audio_Bulk_FIFO( void );

uint8_t   global_audio_padding_byte;
uint8_t   debug_cnt = 0;

/*
*********************************************************************************************************
*                                  First_Pack_Check_BO()
*
* Description :  Check if first USB bulk out package is same as padding data.
*
* Argument(s) :  None.
*
*Return(s)   :   true  - check ok.
*                false - check failed.
*
* Note(s)     :  None.
*********************************************************************************************************
*/



bool First_Pack_Check_BO( unsigned char *pData, unsigned int size )
{    
    
    unsigned int i;
    
    for( i = 0; i < size ; i++ )   {
        if( global_audio_padding_byte != *pData++) {
            return false;
        }
    }
    APP_TRACE_INFO(("\r\nSync pos=%d...\r\n",i));  
    return true; 

}


/*
*********************************************************************************************************
*                                  First_Pack_Check_BO1()
*
* Description :  Check if first USB bulk out package is same as padding data.
*
* Argument(s) :  pData: data buffer will be checked
*                size : buffer size
*                pos  : padding data position in the buffer
*       
*Return(s)   :   true  - check ok.
*                false - check failed.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
bool First_Pack_Check_BO1( unsigned char *pData, unsigned int size, uint32_t *pos )
{    
    
    uint32_t i;
    uint32_t cnt = 0;
    
    for( i = 0; i < size ; i++ )
    {
      if( global_audio_padding_byte == *pData++ )
      {
        cnt++;
        if( cnt == 128 )
        {
          *pos = i;
          break;
        }
      }
      else
        cnt = 0;
    }

    return !!cnt; 

}

/*
*********************************************************************************************************
*                                  First_Pack_Check_BO2()
*
* Description :  Check if first USB bulk out package is same as padding data.
*
* Argument(s) :  pData: data buffer will be checked
*                size : buffer size
*                pos  : padding data position in the buffer
*       
*Return(s)   :   true  - check ok.
*                false - check failed.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
bool First_Pack_Check_BO2( unsigned char *pData, unsigned int size, uint32_t *pos )
{
    uint32_t i;
    uint32_t cnt = 0;
    
    for( i = 0; size ; i ++ )
    {
        if( global_audio_padding_byte == *pData++ )
            cnt++;
        else
        {
            if( cnt >= 128 )
            {
                *pos = i;
                break;
            }
            else
                cnt = 0;
        }
    }
     return !!cnt;
}




/*
*********************************************************************************************************
*                                First_Pack_Padding_BI()
*
* Description :  Padding the first USB bulk in package.
*
* Argument(s) :  pFifo:the fifo will be filled padding data.
*
* Return(s)   :  None.
*
* Note(s)     :  Must be called after reset FIFO and before start audio.
*********************************************************************************************************
*/      
void First_Pack_Padding_BI( kfifo_t *pFifo )
{    
#if 1  
    uint8_t temp[ 128 ];
    APP_TRACE_INFO(("\r\nPadding USB data [0x%0x]...\r\n",global_audio_padding_byte));  
    memset( temp, global_audio_padding_byte, 128 );
    //kfifo_put( &ep0BulkIn_fifo, temp, 128 ) ;
    kfifo_put( pFifo , temp , 128 );
#else
    uint8_t temp[ 1024 ];
    APP_TRACE_INFO(("\r\nPadding USB data [0x%0x]...\r\n",global_audio_padding_byte));  
    memset( temp1, global_audio_padding_byte, 1024 );
    kfifo_put( &ep0BulkIn_fifo, temp1, 1024 ) ;
    kfifo_put( pFifo, temp1, 1024 ) ;    
//    kfifo_put(&ep0BulkIn_fifo, temp, USB_DATAEP_SIZE_64B) ;//2 package incase of PID error
#endif
}





/*
*********************************************************************************************************
*                                    Audio_Start()
*
* Description :  Start USB data transfer.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Audio_Start( void )
{  
    
    //Hold_Task_for_Audio(); 

    Init_Audio_Bulk_FIFO( ); 
    
    source_ssc0.status[ IN ] == ( uint8_t )CONFIGURED;
    source_ssc0.status[ OUT ] == ( uint8_t )CONFIGURED;
    source_ssc1.status[ IN ] == ( uint8_t )CONFIGURED;
    source_ssc1.status[ OUT ] == ( uint8_t )CONFIGURED;
    OSTimeDly( 2 );
    
    audio_play_buffer_ready  = false ;
    audio_run_control        = true ;
//    padding_audio_0_bulk_out = false  ;
    
    restart_audio_0_bulk_out = true  ; 
    restart_audio_0_bulk_in  = true  ;
    restart_audio_1_bulk_out = true  ;
    restart_audio_1_bulk_in  = true  ; 
    restart_audio_2_bulk_out = true  ;
    restart_audio_2_bulk_in  = true  ;
    debug_cnt = 0;
    
//    OSTaskResume ( APP_CFG_TASK_USB_SEV_PRIO );
  
}


/*
*********************************************************************************************************
*                                    Audio_Stop()
*
* Description :  Stop USB data transfer.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Audio_Stop( void )
{  
        
    printf( "\r\nStop Play & Rec..."); 
    OSTimeDly( 8 );
    audio_run_control        = false  ; 
//    OSTimeDly( 4 );
//    OSTaskSuspend ( APP_CFG_TASK_USB_SEV_PRIO );
    
    OSTimeDly(50);    
    Destroy_Audio_Path(); 

    OSTimeDly( 50 );
    
//     usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT );
//     usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_0_DATAIN );
//    
//    usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_1_DATAOUT );
//    usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_1_DATAIN );       
    
    OSTimeDly(10); 
  

    
    Init_Audio_Bulk_FIFO( ); 
    memset( usbCacheBulkOut0 , 0 , sizeof( usbCacheBulkOut0 ));
    memset( usbCacheBulkOut1 , 0 , sizeof( usbCacheBulkOut1 )); 
    memset( usbCacheBulkIn0, 0 , sizeof( usbCacheBulkIn0 ) );
    memset( usbCacheBulkIn1, 0 , sizeof( usbCacheBulkIn0 ) );
    memset( ssc0_PingPongOut,0 , sizeof( ssc0_PingPongOut ) );        
    memset( ssc0_PingPongIn, 0 , sizeof( ssc0_PingPongIn ) );        
    memset( ssc1_PingPongOut,0 , sizeof( ssc1_PingPongOut ) );    
    memset( ssc1_PingPongIn, 0 , sizeof( ssc1_PingPongIn ) );   
    
    padding_audio_0_bulk_out = false  ; 
    padding_audio_1_bulk_out = false  ;
    
    restart_audio_0_bulk_out = false  ; 
    restart_audio_0_bulk_in  = false  ;
    restart_audio_1_bulk_out = false  ;
    restart_audio_1_bulk_in  = false  ; 
    restart_audio_2_bulk_out = false  ;
    restart_audio_2_bulk_in  = false  ;                       
 
    audio_play_buffer_ready  = false  ;         
    
    //Release_Task_for_Audio();   
   
}

/*
*********************************************************************************************************
*                                    Audio_State_Control()
*
* Description : Process command from Host MCU via USB.
*
* Argument(s) : msg: send control command to other task;
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
//void Audio_State_Control( void )
//{    
//    unsigned char err ;
//    unsigned int  temp ;    
//    
//    err = 0 ;
//
//   
//        switch( audio_cmd_index ) {
//            
//            case AUDIO_CMD_START_REC :                
//                if( audio_state_check != 0 ) {
//                    Audio_Stop(); 
//                    //Rec_Voice_Buf_Stop(); 
//                    Stop_CMD_Miss_Counter++;
//                }                 
//                bulkout_trigger = true; //trigger paly&rec sync
//                err = Audio_Start_Rec();  
//                time_start_test = second_counter ;
//                audio_state_check = 1;
//            break;
//
//            case AUDIO_CMD_START_PLAY :                
//                if( audio_state_check != 0 ) {
//                    Audio_Stop(); 
//                    Rec_Voice_Buf_Stop(); 
//                    Stop_CMD_Miss_Counter++;
//                }                     
//                err = Audio_Start_Play();  
//                time_start_test = second_counter ;
//                audio_state_check = 2; 
//            break;
//            
//            case AUDIO_CMD_START_PALYREC :                
//                if( audio_state_check != 0 ) {
//                    Audio_Stop();
//                    Rec_Voice_Buf_Stop(); 
//                    Stop_CMD_Miss_Counter++;
//                }                                         
//                err = Audio_Start_Play();
//                if( err == 0 ) {                    
//                  delay_ms(1);  //make sure play and rec enter interruption in turns 2ms              
//                  err = Audio_Start_Rec(); 
//                }
//                time_start_test = second_counter ;
//                audio_state_check = 3; 
//            break;
//
//            case AUDIO_CMD_STOP : 
//                if( audio_state_check != 0 ) {
//                  Audio_Stop(); 
//                  err = Rec_Voice_Buf_Stop(); 
//                  if( err != 0 ) {
//                      printf("Rec_Voice_Buf_Stop failed : %d", err);
//                  }
//                  printf("\r\nThis cycle test time cost: ");
//                  Get_Run_Time(second_counter - time_start_test);   
//                  printf("\r\n\r\n");
//                  time_start_test = 0 ;
//                  audio_state_check = 0; 
//                }
//            break;   
//        
//            case AUDIO_CMD_CFG: 
//                if( Audio_Configure[1].bit_length == 16 ) {
//                    temp = Audio_Configure[1].sample_rate / 1000 *  Audio_Configure[1].channel_num * 2 * 2;
//                } else { //32
//                    temp = Audio_Configure[1].sample_rate / 1000 *  Audio_Configure[1].channel_num * 2 * 4;        
//                }            
//                if( (temp * PLAY_BUF_DLY_N) > USB_OUT_BUFFER_SIZE ) { //play pre-buffer must not exceed whole play buffer
//                    err = ERR_AUD_CFG;
//                }              
//            break;
//            
//            case AUDIO_CMD_VERSION: 
//                USART_WriteBuffer( AT91C_BASE_US0,(void *)fw_version, sizeof(fw_version) );  //Version string, no ACK 
//            break;         
//            
////            case AUDIO_CMD_RESET:                 
////                printf("\r\nReset USB EP...");   
////                if( audio_state_check != 0 ) { //in case of error from repeat Stop CMD 
////                    Toggle_PID_BI =  Check_Toggle_State();
////                }
////                //Reset Endpoint Fifos
////                AT91C_BASE_UDPHS->UDPHS_EPTRST = 1<<CDCDSerialDriverDescriptors_AUDIODATAOUT;
////                AT91C_BASE_UDPHS->UDPHS_EPTRST = 1<<CDCDSerialDriverDescriptors_AUDIODATAIN; 
////                delay_ms(10);
////                AT91C_BASE_UDPHS->UDPHS_EPT[CDCDSerialDriverDescriptors_AUDIODATAOUT].UDPHS_EPTCLRSTA = 0xFFFF; //AT91C_UDPHS_NAK_OUT | AT91C_UDPHS_TOGGLESQ | AT91C_UDPHS_FRCESTALL;                  
////                AT91C_BASE_UDPHS->UDPHS_EPT[CDCDSerialDriverDescriptors_AUDIODATAIN].UDPHS_EPTCLRSTA  = 0xFFFF;//AT91C_UDPHS_TOGGLESQ | AT91C_UDPHS_FRCESTALL;
////                AT91C_BASE_UDPHS->UDPHS_EPT[CDCDSerialDriverDescriptors_AUDIODATAIN].UDPHS_EPTSETSTA  = AT91C_UDPHS_KILL_BANK ;
////                printf("Done.\r\n");
////                delay_ms(10); 
////                printf("\r\nReset USB EP...");
//    
//            break;  
//            
//            case AUDIO_CMD_READ_VOICE_BUF : 
//                if( Audio_Configure[0].channel_num != 1 ) { //make sure  no other channel recording while SPI recording
//                    err = ERR_AUD_CFG;
//                } else {
//                    Rec_Voice_Buf_Start();
//                }
//            break;          
//            
//            default:         
//                err = ERR_CMD_TYPE;
//            break;
//        
//        }
        
     
    
//}

/*
*********************************************************************************************************
*                                    Get_Run_Time()
*
* Description : Format ms tick to [day:hour:min:sec:0.sec], and print.
*
* Argument(s) : time  -  number of 100ms tick from Timer #2 , using global : second_counter .
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Get_Run_Time( uint32_t time )
{
    
    uint8_t  msec, sec, min, hour;
    uint32_t   day;

    msec = time % 10;
    sec  = time /10 % 60 ;
    min  = time / 600 %60 ;
    hour = time / 36000 %24 ; 
    day  = time / 36000 /24 ;
    printf("[%d:%02d:%02d:%02d.%d]", day, hour, min, sec, msec ); 
    
}



      

/*
*********************************************************************************************************
*                                    Audio_Manager()
*
* Description : Audio Port Manager .
*
* Argument(s) : confif data
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Audio_Manager( unsigned char cfg_data )
{

    APP_TRACE_INFO(( "\r\nAudio Manager: config data = 0X%0X ]", cfg_data ));

        
        if( (cfg_data & SSC0_IN) && ( source_ssc0.status[IN] >= CONFIGURED ) ) {

            source_ssc0.buffer_read(   &source_ssc0,
                                      ( uint8_t * )ssc0_PingPongIn,                                              
                                      source_ssc0.rxSize );
            source_ssc0.status[ IN ]  = ( uint8_t )START;
        } 
        if( (cfg_data & SSC1_IN) && ( source_ssc1.status[IN] >= CONFIGURED ) ) {

            source_ssc1.buffer_read(   &source_ssc1,
                                     ( uint8_t * )ssc1_PingPongIn,                                              
                                      source_ssc1.rxSize );
            source_ssc1.status[ IN ]  = ( uint8_t )START;
        } 
        
        OSTimeDly(2); 
        
        if ( (cfg_data & SSC0_OUT) && ( source_ssc0.status[OUT] >= CONFIGURED ) ){
            source_ssc0.buffer_write(  &source_ssc0,
                                       ( uint8_t * )ssc0_PingPongOut,                                                
                                       source_ssc0.txSize ); 
            source_ssc0.status[ OUT ] = ( uint8_t )START;
        }      
        if ( (cfg_data & SSC1_OUT) && ( source_ssc1.status[OUT] >= CONFIGURED ) ){
            source_ssc1.buffer_write(  &source_ssc1,
                                       ( uint8_t * )ssc1_PingPongOut,                                                
                                       source_ssc1.txSize ); 
            source_ssc1.status[ OUT ] = ( uint8_t )START;
        }
               
      
}
    

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
unsigned char Setup_Audio( AUDIO_CFG *pAudioCfg )
{

    unsigned char err;
    unsigned char mic_num;
    unsigned char data     = 0xFF;
    unsigned char lin_ch   = 0;

    //unsigned char buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_SET_AUDIO_CFG };
    
    //APP_TRACE_INFO(("Setup_Audio[%d] [%s]:[%d SR]:[%d CH]: %s\r\n",pAudioCfg->id,(pAudioCfg->type == 0) ? "REC " : "PLAY", pAudioCfg->sr, pAudioCfg->channels,((pAudioCfg->type == 0) && (pAudioCfg->lin_ch_mask == 0)) ? "LIN Disabled" : "LIN Enabled"));
    if( pAudioCfg->type == 0 ) {
        if ( pAudioCfg->lin_ch_mask != 0 ) {
            lin_ch = 2;// 2 channel LINE IN
            pAudioCfg->channel_num += lin_ch;
            //APP_TRACE_INFO(("LIN 2CH added up to %d CH\r\n",pAudioCfg->channel_num));
        }
        APP_TRACE_INFO(("\r\nSetup_Audio[%d] [REC ]:[%d SR]:[%d CH]:[%d-Bit]:[LIN %d]", pAudioCfg->id, pAudioCfg->sample_rate, pAudioCfg->channel_num, pAudioCfg->bit_length, lin_ch));
    } else if( pAudioCfg->type == 1 ){
        APP_TRACE_INFO(("\r\nSetup_Audio[%d] [PLAY]:[%d SR]:[%d CH]:[%d-Bit]", pAudioCfg->id, pAudioCfg->sample_rate, pAudioCfg->channel_num, pAudioCfg->bit_length ));
    } else {
        APP_TRACE_INFO(("\r\nSetup_Audio[%d] ERROR: Unsupported pAudioCfg->type, %d\r\n",pAudioCfg->id, pAudioCfg->type));
        return AUD_CFG_ERR;
    }

    err = Check_SR_Support( pAudioCfg->sample_rate );
    if( err != NO_ERR ) {
        APP_TRACE_INFO(("\r\nSetup_Audio[%d] ERROR: Unsupported sample rate!\r\n",pAudioCfg->id));
        return err;
    }
    if( pAudioCfg->id == 0 ) { //I2S0 path
        mic_num = Check_Actived_Mic_Number();
        if( mic_num > 6 ) {
            APP_TRACE_INFO(("\r\nERROR: Check_Actived_Mic_Number = %d > 6\r\n",mic_num));
            return AUD_CFG_MIC_NUM_MAX_ERR;//if report err, need UI support!
        }
        //check rec mic num
        if( (pAudioCfg->type == 0) && ( mic_num != pAudioCfg->channel_num ) ) {
            APP_TRACE_INFO(("WARN:(Setup_Audio Rec)pAudioCfg->channel_num(%d) !=  Active MICs Num(%d)\r\n",pAudioCfg->channel_num,mic_num));
            //buf[4] = mic_num;
            return AUD_CFG_MIC_NUM_DISMATCH_ERR;
        }  
        if( (pAudioCfg->type == 0) && (pAudioCfg->channel_num == 0) && (pAudioCfg->lin_ch_mask == 0) ) {
            APP_TRACE_INFO(("WARN:(Setup_Audio Rec)pAudioCfg->channel_num + ch_lin =  0\r\n" ));
            //return AUD_CFG_PLAY_CH_ZERO_ERR; UI not support
        }
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

#ifdef BOARD_TYPE_AB04 //BOARD_TYPE_UIF
    if( pAudioCfg->type == 0) {
        mic_num = pAudioCfg->channel_num ;
        Global_Mic_Mask[0] = mic_num;
    } else {
        mic_num = Global_Mic_Mask[0]; //save mic num to ruler0
    }

#endif


#ifdef BOARD_TYPE_AB04 //BOARD_TYPE_UIF
    if ( pAudioCfg->type == 0 ) {

        pAudioCfg->gpio_rec_num  = Get_Mask_Num( pAudioCfg->gpio_rec_bit_mask ); //gpio num
        pAudioCfg->gpio_rec_start_index = pAudioCfg->channel_num;  //gpio start index
        pAudioCfg->channel_num += pAudioCfg->gpio_rec_num; //add gpio num to channel
        if(  pAudioCfg->channel_num > 10 ) {
            APP_TRACE_INFO(("ERROR:(Setup_Audio Rec)Mic+Lin+GPIO+SPI Rec channel num(=%d) > 10 NOT allowed for UIF\r\n", pAudioCfg->channel_num));
            return AUD_CFG_MIC_NUM_MAX_ERR ;
        }

        pAudioCfg->spi_rec_num   = Get_Mask_Num( pAudioCfg->spi_rec_bit_mask );  //spi num
        Global_SPI_Rec_En = 0; //clear flag as No SPI rec
        if( pAudioCfg->spi_rec_num  != 0 ){
            if( pAudioCfg->channel_num != 0 ) {
                APP_TRACE_INFO(("ERROR:(Setup_Audio Rec)Mic+Lin+GPIO Rec conflict with SPI Rec\r\n"));
                //return AUD_CFG_SPI_REC_CONFLICT ;
            }
            pAudioCfg->channel_num = pAudioCfg->spi_rec_num;  //use spi num for rec channel
            Global_SPI_Rec_En = 1; //set flag for SPI rec
        }
    }
#endif
    if( (pAudioCfg->spi_rec_num != 0) && (pAudioCfg->channel_num > pAudioCfg->slot_num) )   {
         APP_TRACE_INFO(("\r\nSetup_Audio ERROR: channel_num > slot_num\r\n"));
         return AUD_CFG_ERR ;
    }
    //check channel num
    if( (pAudioCfg->type == 1) && (pAudioCfg->channel_num == 0) ) {
        APP_TRACE_INFO(("WARN:(Setup_Audio Play)pAudioCfg->channel_num =  0\r\n" ));
        //return AUD_CFG_PLAY_CH_ZERO_ERR;  UI not support
    }
    if( (pAudioCfg->type == 0) && (pAudioCfg->channel_num == 0) ) {
        APP_TRACE_INFO(("WARN:(Setup_Audio Rec)pAudioCfg->channel_num  =  0\r\n" ));
        //return AUD_CFG_PLAY_CH_ZERO_ERR; UI not support
    }


    //ssc_cki =  0: falling egde send for sending, 1: rising edge lock for receiving
    if ( pAudioCfg->type == 0 ) { //rec
        pAudioCfg->ssc_cki  =   pAudioCfg->bclk_polarity == 0 ? 1 : 0 ;
    } else { //play
        pAudioCfg->ssc_cki  =   pAudioCfg->bclk_polarity == 0 ? 0 : 1 ;
    }
    switch(  pAudioCfg->format ) {
        case 1 :  //PDM
            flag_bypass_fm36 = 0;       //Not bypass FM36 for PDM mode
            pAudioCfg->ssc_start = 4;  //falling edge trigger for low left
            pAudioCfg->slot_num  = 8; //make sure 8 slots enabled when used FM36 to record PDM
        break;
        case 2 : //I2S or I2S-TDM
            flag_bypass_fm36 = 1;      //bypass FM36
            pAudioCfg->ssc_start = 4;  //falling edge trigger for low left
            if( pAudioCfg->ssc_delay != 1 ) {  //one clock delay is needed in I2S
                return CODEC_FORMAT_NOT_SUPPORT_ERR;
            }
        break;
        case 3 : //PCM or PCM-TDM
            flag_bypass_fm36 = 1;      //bypass FM36
            pAudioCfg->ssc_start = 5;  //rising edge trigger for high left
        break;
        default:
            return CODEC_FORMAT_NOT_SUPPORT_ERR;
        break;
    }
    //Dump_Data(buf, sizeof(buf));
    //UART2_Mixer(3);
    //USART_SendBuf( AUDIO_UART, buf, sizeof(buf)) ;
    //USART_SendBuf( AUDIO_UART, (unsigned char *)pAudioCfg, sizeof(AUDIO_CFG)) ;
    //err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM);
    //if( err != NO_ERR ) {
    //    APP_TRACE_INFO(("\r\nSetup_Audio ERROR: timeout\r\n"));
    //    return err;
    //}
    //if( data != NO_ERR ) {
    //    APP_TRACE_INFO(("\r\nSetup_Audio ERROR: %d\r\n ",data));
    //    return data;
    //}

    
    
    err = Add_Audio_Path( getPathName( pAudioCfg->id *2 + pAudioCfg->type ) , pAudioCfg );
    if( err != NO_ERR ) {         
        return err;
    }  
    
    Codec_Set[pAudioCfg->id][pAudioCfg->type].flag          = 1;  //cfg received
    Codec_Set[pAudioCfg->id][pAudioCfg->type].sr            = pAudioCfg->sample_rate;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].sample_len    = pAudioCfg->bit_length;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].format        = pAudioCfg->format;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].slot_num      = pAudioCfg->slot_num;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].m_s_sel       = pAudioCfg->master_slave;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].delay         = pAudioCfg->ssc_delay;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].bclk_polarity = pAudioCfg->bclk_polarity;
    Codec_Set[pAudioCfg->id][pAudioCfg->type].id            = pAudioCfg->id;              
    
    
    return 0 ;
}



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
unsigned char Update_Audio( unsigned char id )
{
    unsigned char err;
    unsigned char index ;
    const DataSource data_source[] = {source_twi2, source_twi1};
    
    APP_TRACE_INFO(("\r\nUpdate_Audio[id] : [REC] = %d [PLAY] = %d\r\n", Codec_Set[id][0].flag, Codec_Set[id][1].flag));
    err = 0;
    
    if( (Codec_Set[id][0].flag == 1)  && (Codec_Set[id][1].flag == 1) ) {
        if( (Codec_Set[id][0].sr !=  Codec_Set[id][1].sr) ||
            (Codec_Set[id][0].slot_num != Codec_Set[id][1].slot_num) ||
            (Codec_Set[id][0].format !=  Codec_Set[id][1].format)  ||
            (Codec_Set[id][0].m_s_sel !=  Codec_Set[id][1].m_s_sel) ||
            (Codec_Set[id][0].sample_len !=  Codec_Set[id][1].sample_len) ||
            (Codec_Set[id][0].bclk_polarity !=  Codec_Set[id][1].bclk_polarity) ) {
            err = AUD_CFG_ERR;
            APP_TRACE_INFO(("\r\nERROR: [REC] and [PLAY] audio settings conflicts!\r\n"));
        }
        index = 0;
    } else if( Codec_Set[id][0].flag == 1 ) {
        index = 0;
    } else if( Codec_Set[id][1].flag == 1 ) {
        index = 1;
    } else {
        err = AUD_CFG_ERR;
    }
    Codec_Set[id][0].flag = 0; //reset Cfg flag
    Codec_Set[id][1].flag = 0;

    if( err != NO_ERR ) {
        return err;
    }
    APP_TRACE_INFO(("\r\n\############## BCLK POLARITY = %d\r\n", Codec_Set[id][index].bclk_polarity));
    //I2C_Mixer(I2C_MIX_FM36_CODEC);
    err = Init_CODEC( &data_source[id], Codec_Set[id][index] );
    memcpy((void*)&Audio_Configure_Instance[id], (void*)&Codec_Set[id][index], sizeof(AUDIO_CFG) );
    //I2C_Mixer(I2C_MIX_UIF_S);
    if( err != NO_ERR ) {
        APP_TRACE_INFO(("\r\nUpdate_Audio Init_CODEC ERROR: %d\r\n",err));
        return err;
    }

    if( 0 == id ) {  //FM36 connected to SSC0 
        if( flag_bypass_fm36 == 0 ) {
            err = Init_FM36_AB03( Codec_Set[0][index].sr, Global_Mic_Mask[0], 1, 0, Codec_Set[0][index].sample_len, Codec_Set[0][index].slot_num==2 ? 0:1, 0 ); //Lin from SP1_RX, slot0~1
        } else{
            err = FM36_PWD_Bypass();
        }
         
        if( err != NO_ERR ) {
            APP_TRACE_INFO(("\r\nUpdate_Audio ReInit_FM36 ERROR: %d\r\n",err));
            return err;
        }
    }
    
    return 0 ;

}


/*
*********************************************************************************************************
*                                           Start_Audio()
*
* Description : Send command to start USB audio play/record.
* Argument(s) : cmd_type : record£¨== 1£©/play£¨== 2£©/record & play £¨== 3£©
*               padding :  used for usb audio BI/BO first package padding
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.
*
* Note(s)     : None.
*********************************************************************************************************
*/


unsigned char Start_Audio( START_AUDIO start_audio )
{
    unsigned char err;
    unsigned char data  = 0xFF;
    unsigned char ruler_id;
 

#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;
#endif

    APP_TRACE_INFO(("\r\nStart_Audio : type = [%d], padding = [0x%X]\r\n", start_audio.type, start_audio.padding));
    
    
//    UART2_Mixer(3);
//    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf) );
//    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM );
//    if( err != NO_ERR  || data != 0  ) {
//        APP_TRACE_INFO(("\r\nStart_Audio ERROR: timeout = %d, ack data = %d\r\n",err, data));
//        return data;
//
//    } else {

        OS_ENTER_CRITICAL();
        for( ruler_id = 0 ; ruler_id < 4 ; ruler_id++ ) {
            if( Global_Ruler_State[ruler_id] ==  RULER_STATE_SELECTED ) {//given: if mic selected, then ruler used
                Global_Ruler_State[ruler_id] = RULER_STATE_RUN;
            }
        }
        OS_EXIT_CRITICAL();

//    }
     
    global_audio_padding_byte = start_audio.padding;
    //Audio_Manager();
    Audio_Start();
  
    
    return 0 ;
}


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
unsigned char Stop_Audio( void )
{
    unsigned char err   = 0xFF;
    unsigned char data  = 0xFF;
    unsigned char ruler_id;
    unsigned char buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_STOP_AUDIO };
    
    Audio_Stop();
    
     /*
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;                                 // Storage for CPU status register
#endif
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
     */     
    
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
unsigned char Reset_Audio( void )
{
    unsigned char err   = 0xFF;
    unsigned char data  = 0xFF;
    unsigned char buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_RESET_AUDIO };
    /*
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
    */
    return 0 ;
}




unsigned char SPI_Rec_Start( SPI_PLAY_REC_CFG *pSpi_rec_cfg )
{
    unsigned char err   = 0xFF;
    unsigned char data  = 0xFF;
  /*
    unsigned char buf[] = { CMD_DATA_SYNC1, CMD_DATA_SYNC2, RULER_CMD_START_RD_VOICE_BUF };

    if( pSpi_rec_cfg->gpio_irq < 2 ) {
        APP_TRACE_INFO(("\r\nIRQ gpio support: UIF_GPIO_2 ~ UIF_GPIO_9 only!\r\n ",data));
        return AUD_CFG_SPI_REC_CONFLICT;
    }
    if( Global_SPI_Rec_En == 0 ) {
       APP_TRACE_INFO(("\r\nGlobal_SPI_Rec channel = 0\r\n ",data));
       return AUD_CFG_SPI_REC_CONFLICT;
    }
    pSpi_rec_cfg->gpio_irq -= 2 ;//'cause UIF_GPIO connecting to Host is differnt from Audio

    APP_TRACE_INFO(("\r\nSPI_Rec_Start : Chip ID = %d, IRQ = GPIO[%d], spi.mode = %d, spi.speed = %d kHz\r\n",\
                         pSpi_rec_cfg->chip_id, pSpi_rec_cfg->gpio_irq, pSpi_rec_cfg->spi_mode, pSpi_rec_cfg->spi_speed / 1000 ));

    Disable_SPI_Port(); //disabled host mcu SPI;

    UART2_Mixer(3);
    USART_SendBuf( AUDIO_UART, buf,  sizeof(buf) );
    USART_SendBuf( AUDIO_UART, (unsigned char *)pSpi_rec_cfg, sizeof(SPI_PLAY_REC_CFG)) ;
    err = USART_Read_Timeout( AUDIO_UART, &data, 1, TIMEOUT_AUDIO_COM );
    if( err != NO_ERR  || data != 0  ) {
        APP_TRACE_INFO(("\r\nSPI_Rec_Start ERROR: timeout = %d, ack data = %d\r\n",err, data));
        Enable_SPI_Port(); //Enabled host mcu SPI if failed to get resp from audio mcu
        return data;
    }

    Global_SPI_Rec_Start = 1; //set flag for SPI rec
   */
    return 0 ;
}


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
unsigned char Save_DSP_VEC( MCU_FLASH *p_dsp_vec )
{
    unsigned char err;
    unsigned int flash_addr;
    unsigned int index;
    FLASH_INFO   flash_info;
     /*

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
       */
    return err;

}






/*
*********************************************************************************************************
*                                       Set_DSP_VEC()
*
* Description : set config setting for load DSP¡¡vector from flash
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

unsigned char Set_DSP_VEC( SET_VEC_CFG *p_dsp_vec_cfg )
{
    unsigned char err;
    OS_SEM_DATA  sem_data;
    /*
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
            OSTimeDlyResume(APP_CFG_TASK_USER_IF_PRIO);
            OSSemPend( Load_Vec_Sem_lock, 0, &err );
            OSSemPost( Load_Vec_Sem_lock );
        }
        I2C_Mixer(I2C_MIX_FM36_CODEC);
        err = FM36_PDMADC_CLK_OnOff(1,0); //enable PDM clock
        I2C_Mixer(I2C_MIX_UIF_S);
    }
    */
    return err;

}


void Debug_Audio( void )
{

   AUDIO_CFG    AudioCfg;
   START_AUDIO  start_audio;

   AudioCfg.channel_num = 8;
   AudioCfg.bit_length = 32;
   AudioCfg.sample_rate = 16000;
   AudioCfg.type = 1; //play

   start_audio.type = 2; //play

   Setup_Audio(&AudioCfg);
   Start_Audio( start_audio ); //play

}
