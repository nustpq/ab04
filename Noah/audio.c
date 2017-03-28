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
AUDIO_CFG  Audio_Configure_Instance[ 2 ];


extern void Init_Audio_Bulk_FIFO( void );

unsigned char  audio_padding_byte;
uint8_t     debug_cnt = 0;




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
      if( audio_padding_byte == *pData++ )
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
        if( audio_padding_byte == *pData++ )
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
    APP_TRACE_INFO(("\r\nPadding USB data [0x%0x]...\r\n",audio_padding_byte));  
    memset( temp, audio_padding_byte, 128 );
    //kfifo_put( &ep0BulkIn_fifo, temp, 128 ) ;
    kfifo_put( pFifo , temp , 128 );
#else
    uint8_t temp[ 1024 ];
    APP_TRACE_INFO(("\r\nPadding USB data [0x%0x]...\r\n",audio_padding_byte));  
    memset( temp1, audio_padding_byte, 1024 );
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
    
    Hold_Task_for_Audio(); 

    Init_Audio_Bulk_FIFO( ); 
    
//    aic3204_init_default( );
    
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
    
    OSTimeDly(10);    
    Destroy_Audio_Path(); 

    OSTimeDly( 4 );
    
//     usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT );
//     usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_0_DATAIN );
    
//    usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_1_DATAOUT );
//    usb_CloseData( CDCDSerialDriverDescriptors_AUDIO_1_DATAIN );       
    
    OSTimeDly(2); 
  
//    reset_fpga( );
    Pin_Reset_Codec( 0 );
    Pin_Reset_Codec( 1 );
    
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
    
    Release_Task_for_Audio();   
   
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
*                                    peripheral_ssc0_recoder()
*
* Description : starting ssc0 recorder alone
*
* Argument(s) : instance  -  keeping align
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void peripheral_ssc0_recoder( void *instance )
{
 
    uint8_t  err = 0; 
    err = Init_CODEC( &source_twi2,
                      Audio_Configure_Instance[0].sample_rate,
                      Audio_Configure_Instance[0].bit_length );    
    
    First_Pack_Padding_BI( &ep0BulkIn_fifo );
    
    if( source_ssc0.buffer_read != NULL )
    {
        source_ssc0.buffer_read(  &source_ssc0,
                                  ( uint8_t * )ssc0_PingPongIn,                                              
                                  source_ssc1.rxSize );
        source_ssc0.status[ IN ]  = ( uint8_t )START;
    }
    
}

/*
*********************************************************************************************************
*                                    peripheral_ssc1_recorder()
*
* Description : starting ssc1 recorder alone
*
* Argument(s) : instance  -  keeping align
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void peripheral_ssc1_recorder( void *instance )
{
 
    uint8_t err = Init_CODEC( &source_twi1,
                      Audio_Configure_Instance[1].sample_rate,
                      Audio_Configure_Instance[1].bit_length );
    
    First_Pack_Padding_BI( &ep1BulkIn_fifo );
    
    if( source_ssc1.buffer_read != NULL )
    {
        source_ssc1.buffer_read(  &source_ssc1,
                                  ( uint8_t * )ssc1_PingPongIn,                                              
                                  source_ssc1.rxSize );
        source_ssc1.status[ IN ]  = ( uint8_t )START;
    }
    
}

/*
*********************************************************************************************************
*                                    peripheral_ssc0_start()
*
* Description : starting ssc0 playing and recording.
*
* Argument(s) : instance  -  keeping align.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void peripheral_ssc0_start( void *instance )
{
extern Pin SSC_Sync_Pin;  
    uint8_t  err = 0; 
    err = Init_CODEC( &source_twi2,
                      Audio_Configure_Instance[0].sample_rate,
                      Audio_Configure_Instance[0].bit_length );    
    
    First_Pack_Padding_BI( &ep0BulkIn_fifo );
    
    if( source_ssc0.buffer_read != NULL )
    {
        source_ssc0.buffer_read(  &source_ssc0,
                                  ( uint8_t * )ssc0_PingPongIn,                                              
                                  source_ssc1.rxSize );
        source_ssc0.status[ IN ]  = ( uint8_t )START;
    }
 
    while( !PIO_Get( &SSC_Sync_Pin ) );
    while( PIO_Get( &SSC_Sync_Pin ) );
    
    if( source_ssc0.buffer_write != NULL )
    {                          
        source_ssc0.buffer_write(  &source_ssc0,
                                   ( uint8_t * )ssc0_PingPongOut,                                                
                                   source_ssc0.txSize ); 
        source_ssc0.status[ OUT ] = ( uint8_t )START;  
                          
    } 
    
}

/*
*********************************************************************************************************
*                                    peripheral_ssc1_start()
*
* Description : starting ssc1 playing and recording.
*
* Argument(s) : instance  -  keeping align .
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void peripheral_ssc1_start( void *instance )
{
extern Pin SSC_Sync_Pin1;  
 
    uint8_t err = Init_CODEC( &source_twi1,
                      Audio_Configure_Instance[1].sample_rate,
                      Audio_Configure_Instance[1].bit_length );
    
    First_Pack_Padding_BI( &ep1BulkIn_fifo );
    
    if( source_ssc1.buffer_read != NULL )
    {
        source_ssc1.buffer_read(  &source_ssc1,
                                  ( uint8_t * )ssc1_PingPongIn,                                              
                                  source_ssc1.rxSize );
        source_ssc1.status[ IN ]  = ( uint8_t )START;
    }
 
    while( !PIO_Get( &SSC_Sync_Pin1 ) );
    while( PIO_Get( &SSC_Sync_Pin1 ) );
    
    if( source_ssc1.buffer_write != NULL )
    {                          
        source_ssc1.buffer_write(  &source_ssc1,
                                   ( uint8_t * )ssc1_PingPongOut,                                                
                                   source_ssc1.txSize ); 
        source_ssc1.status[ OUT ] = ( uint8_t )START;  
                          
    } 
    
}

/*
*********************************************************************************************************
*                                    peripheral_sync_start()
*
* Description : starting ssc0 and ssc1 playing and recording at same time.
*
* Argument(s) : instance  -  keeping align.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void peripheral_sync_start( void *instance )
{
    extern Pin SSC_Sync_Pin;
    uint8_t err = 0;

//    err = Init_CODEC( &source_twi2,
//                      Audio_Configure_Instance[0].sample_rate,
//                      Audio_Configure_Instance[0].bit_length );
//                      
                      
    err = Init_CODEC( &source_twi2,48000, 16 );                 
                          
    First_Pack_Padding_BI( &ep0BulkIn_fifo );
    First_Pack_Padding_BI( &ep1BulkIn_fifo ); 
                          
    if( source_ssc0.buffer_read != NULL )
    {
        source_ssc0.buffer_read(   &source_ssc0,
                                   ( uint8_t * )ssc0_PingPongIn,                                              
                                   source_ssc0.rxSize );
        source_ssc0.status[ IN ]  = ( uint8_t )START;
    }
                          
    if( source_ssc1.buffer_read != NULL )
    {
        source_ssc1.buffer_read(  &source_ssc1,
                                  ( uint8_t * )ssc1_PingPongIn,                                              
                                  source_ssc1.rxSize );
        source_ssc1.status[ IN ]  = ( uint8_t )START;
    }
 
    while( !PIO_Get( &SSC_Sync_Pin ) );
    while( PIO_Get( &SSC_Sync_Pin ) );
                        
    if( source_ssc0.buffer_write != NULL )
    {
        source_ssc0.buffer_write(  &source_ssc0,
                                   ( uint8_t * )ssc0_PingPongOut,                                                
                                   source_ssc0.txSize ); 
        source_ssc0.status[ OUT ] = ( uint8_t )START;

    }
                          
    if( source_ssc1.buffer_write != NULL )
    {                          
        source_ssc1.buffer_write(  &source_ssc1,
                                   ( uint8_t * )ssc1_PingPongOut,                                                
                                   source_ssc1.txSize ); 
        source_ssc1.status[ OUT ] = ( uint8_t )START;  
                          
    }    
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
* Note(s)     :    bit7    |bit6   |bit5     |bit4     |bit3   |bit2   |bit1     | bit0 
*                  SYNC    | X     |ssc0_P   |ssc0_R   |  X    |X      |ssc0_P   | ssc0_R
*
*                  Here, just install function according sence not starting peripheral really.
*********************************************************************************************************
*/
void Audio_Manager( unsigned char cfg_data )
{

    APP_TRACE_INFO(( "\r\nAudio Manager: config data = 0X%0X ]", cfg_data ));
    
    const uint8_t ssc0_recorder_bit = cfg_data & ( 1 << 0 );
    const uint8_t ssc0_play_bit = cfg_data & ( 1 << 1 );
    const uint8_t ssc1_recorder_bit = cfg_data & ( 1 << 4 );
    const uint8_t ssc1_play_bit = cfg_data & ( 1 << 5 );
//    const uint8_t ssc_sync_bit = cfg_data & ( 1 << 7 ); 
    
    const uint8_t ssc_sync_bit =  1;

        if( ssc_sync_bit )                          // play and record sync
        {
          source_ssc0.peripheral_start = peripheral_sync_start;
          source_ssc1.peripheral_start = NULL;
        }
        else                                       // asynchronous
        {
           if( ssc0_recorder_bit && ssc0_play_bit )
           {
              source_ssc0.peripheral_start = peripheral_ssc0_start;
           }
           else if( ssc0_recorder_bit && ( !ssc0_play_bit ) )
           {
             source_ssc0.peripheral_record_alone = peripheral_ssc0_recorder;
           }
          
           if( ssc1_recoder_bit && ssc1_play_bit )
           {
              source_ssc1.peripheral_start = peripheral_ssc1_start;
           }
           else
           {
             source_ssc1.peripheral_record_alone = peripheral_ssc1_recorder;
           }         
          
        }
     
}
    

