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
//because AB04 has two ssc port,so extend to 4 sturct;
AUDIO_CFG  Audio_Configure_Instance0[ 2 ];
AUDIO_CFG  Audio_Configure_Instance1[ 2 ];

extern void Init_Audio_Bulk_FIFO( void );

unsigned char  audio_padding_byte;


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
        if( audio_padding_byte != *pData++) {
            return false;
        }
    }
     //printf("\r\nSync\r\n");
    return true; 

}

/*
*********************************************************************************************************
*                                First_Pack_Padding_BI()
*
* Description :  Padding the first USB bulk in package.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     :  Must be called after reset FIFO and before start audio.
*********************************************************************************************************
*/      
void First_Pack_Padding_BI( void )
{
    unsigned char temp[ USB_DATAEP_SIZE_64B ];
    
    APP_TRACE_INFO(("\r\nPadding USB data [0x%0x]...\r\n",audio_padding_byte));  
    memset( temp, audio_padding_byte, USB_DATAEP_SIZE_64B );
    kfifo_put(&ep0BulkIn_fifo, temp, USB_DATAEP_SIZE_64B) ;
    kfifo_put(&ep0BulkIn_fifo, temp, USB_DATAEP_SIZE_64B) ;//2 package incase of PID error
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
    
    Hold_Task_for_Audio();                      
                            
    First_Pack_Padding_BI( ); 
    
    
    //Audio_Manager( SSC0_IN | SSC0_OUT );  //start audio
    
    //OSTimeDly(1);    
    audio_play_buffer_ready  = false ;
    audio_run_control        = true ;
    
    restart_audio_0_bulk_out = true  ; 
    restart_audio_0_bulk_in  = true  ;
    restart_audio_1_bulk_out = true  ;
    restart_audio_1_bulk_in  = true  ; 
    restart_audio_2_bulk_out = true  ;
    restart_audio_2_bulk_in  = true  ;

  
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
    
    audio_run_control        = false  ; 
    OSTimeDly(20);
//    delay_ms(20); //wait until DMA interruption done
//    
// 
//    
//    SSC_Play_Stop();  
//    SSC_Record_Stop(); 
//    
//    delay_ms(10);  
//    
//    printf( "\r\nEnd Audio Transfer..."); 
//    //End_Audio_Transfer(); 
//    delay_ms(10); 
//    
//    printf("\r\nReset USB EP...");
//    if( audio_state_check != 0 ) { //in case of error from repeat Stop CMD 
//        Toggle_PID_BI =  Check_Toggle_State();
//    }
    
    //Reset Endpoint Fifos
    //AT91C_BASE_UDPHS->UDPHS_EPTRST = 1<<CDCDSerialDriverDescriptors_AUDIODATAOUT;
    //AT91C_BASE_UDPHS->UDPHS_EPTRST = 1<<CDCDSerialDriverDescriptors_AUDIODATAIN; 
    //delay_ms(50);

    
    Destroy_Audio_Path();
    
    
    
    UDPHS->UDPHS_EPTRST = 1<<CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT;
    UDPHS->UDPHS_EPTRST = 1<<CDCDSerialDriverDescriptors_AUDIO_0_DATAIN; 
    OSTimeDly(10);
    //SSC_Reset(); //I2S_Init();        
     
    Dma_configure();
    
    //LED_Clear( USBD_LEDDATA );
    
    
    
    padding_audio_0_bulk_out = false  ; 
    
    restart_audio_0_bulk_out = false  ; 
    restart_audio_0_bulk_in  = false  ;
    restart_audio_1_bulk_out = false  ;
    restart_audio_1_bulk_in  = false  ; 
    restart_audio_2_bulk_out = false  ;
    restart_audio_2_bulk_in  = false  ;                       
 
    audio_play_buffer_ready  = false  ;         
       
    //Init_Audio_Bulk_FIFO(); 
    
    Release_Task_for_Audio(); 
    

    
    //End_Audio_Transfer();
    
    
//    //reset debug counters
//    BO_free_size_max      = 0 ;
//    BI_free_size_min      = 100 ; 
//    total_received        = 0 ;
//    total_transmit        = 0 ;
//    error_bulkout_full    = 0 ;
//    error_bulkout_empt    = 0 ;
//    error_bulkin_full     = 0 ;
//    error_bulkin_empt     = 0 ;    
//    debug_trans_counter1  = 0 ;
//    debug_trans_counter2  = 0 ;
//    debug_trans_counter3  = 0 ;
//    debug_trans_counter4  = 0 ; 
//    debug_usb_dma_IN      = 0 ;
//    debug_usb_dma_OUT     = 0 ;    
//    test_dump             = 0 ;

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
    

