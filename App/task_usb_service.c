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
*                                           TASK PACKAGE
*
*                                          Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : task_usb_service.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/
     
#include <includes.h>     


uint8_t tmpBuffer[ I2S_PINGPONG_IN_SIZE_3K ];

void Init_Audio_Path();
/*
*********************************************************************************************************
*                                    App_TaskUSBService()
*
* Description : Process UART Receive related process between Audio Bridge and PC, Audio Bridge and Ruler.
*               Fetch data from PC in UART receive buffer, check data sanity in DL layer 
*
* Argument(s) : p_arg   Argument passed to 'App_TaskUSBService()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void  App_TaskUSBService ( void *p_arg )
{
    
   (void)p_arg; 
    
    uint8_t  err ;  
    uint8_t  usb_state;
    uint8_t  usb_state_saved ;
    uint32_t counter, counter2;
    
    ListElmt  *e ;
    AUDIOPATH *pPath;      
    
    err = 0;
    usb_state_saved = 0;
    
    
    
    for(;;) 
    {          
        usb_state =   USBD_GetState();         
        
        if( usb_state != usb_state_saved ) {
          
            usb_state_saved = usb_state ;
            if ( usb_state >= USBD_STATE_CONFIGURED ) {                 
                CDCDSerialDriver_ReadCmd(  usbCmdCacheBulkOut,
                                  USB_CMDEP_SIZE_64B ,
                                  (TransferCallback) UsbCmdDataReceived,                                
                                  0);
            }
        }
        
        if ( usb_state >= USBD_STATE_CONFIGURED ) {
          
            UIF_LED_On( LED_USB );    
        } else {
          
            UIF_LED_Off( LED_USB );             
        }
        
        if ( usb_state < USBD_STATE_CONFIGURED ) {  
          
            OSTimeDly(5);
            continue;      
        }
    
        if ( !audio_run_control ) {  
          
            OSTimeDly(5);
            continue;      
        }
 
#if 1
        //////////////////////// proccess up link data /////////////////////////  
        e = portsList.head;         
        while( e != NULL )
        {
            pPath = ( AUDIOPATH * )e->data;
            
            if( CDCDSerialDriverDescriptors_AUDIO_0_DATAIN == pPath->epIn ) {  //SSC0 Rec
                //step1: calculate space of ssc0/spi0/gpio ring buffer.
                counter  = kfifo_get_data_size( pPath->pfifoIn );             
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pInSource->rxSize < counter )
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pInSource->rxSize );
                
                counter = kfifo_get_data_size( &ep0BulkIn_fifo );     
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_in && audio_run_control)  {
                    APP_TRACE_INFO(("\r\nBulk  In 0 start"));
                    restart_audio_0_bulk_in = false ;
                    // ep0 ring --> usb cache
                    kfifo_get( &ep0BulkIn_fifo,
                           ( uint8_t * )usbCacheBulkIn0,
                           USB_DATAEP_SIZE_64B );         
                
                  // send ep0 data ---> pc
                    CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                            USB_DATAEP_SIZE_64B,  //64B size for low delay
                                            (TransferCallback)UsbAudio0DataTransmit,
                                            0);  
                }
                
            } else if( CDCDSerialDriverDescriptors_AUDIO_1_DATAIN == pPath->epIn ) {  //SSC1 Rec
                //step1: calculate space of ssc0/spi0/gpio ring buffer.
                counter  = kfifo_get_data_size( pPath->pfifoIn );             
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pInSource->rxSize < counter )
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pInSource->rxSize );
                
                counter = kfifo_get_data_size( &ep1BulkIn_fifo );     
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_1_bulk_in && audio_run_control)  {
                    APP_TRACE_INFO(("\r\nBulk  In 1 start"));
                    restart_audio_1_bulk_in = false ;
                    // ep0 ring --> usb cache
                    kfifo_get( &ep1BulkIn_fifo,
                           ( uint8_t * )usbCacheBulkIn1,
                           USB_DATAEP_SIZE_64B );         
                
                  // send ep0 data ---> pc
                    CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                            USB_DATAEP_SIZE_64B,  //64B size for low delay
                                            (TransferCallback)UsbAudio1DataTransmit,
                                            0);  
                }
                
            } else if( CDCDSerialDriverDescriptors_SPI_DATAIN == pPath->epIn ) {  //SPI/GPIO Rec
                //step1: calculate space of ssc0/spi0/gpio ring buffer.
                counter  = kfifo_get_data_size( pPath->pfifoIn );             
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pInSource->rxSize < counter )
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pInSource->rxSize );
                
                counter = kfifo_get_data_size( &ep2BulkIn_fifo );     
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_2_bulk_in && audio_run_control)  {
                    APP_TRACE_INFO(("\r\nBulk  In 2 start"));
                    restart_audio_2_bulk_in = false ;
                    // ep0 ring --> usb cache
                    kfifo_get( &ep2BulkIn_fifo,
                           ( uint8_t * )usbCacheBulkIn2,
                           USB_DATAEP_SIZE_64B );         
                
                  // send ep0 data ---> pc
                    CDCDSerialDriver_WriteSPI( usbCacheBulkIn2,
                                            USB_DATAEP_SIZE_64B,  //64B size for low delay
                                            (TransferCallback)UsbSPIDataTransmit,
                                            0);  
                }
                
            } else {
                APP_TRACE_INFO(("\r\nPath Ep not defined : %d",pPath->epIn));             
              
            }              
            e = e -> next;
          
        }
        
        ////////////////////////// proccess down link //////////////////////////        
        e = portsList.head;         
        while( e != NULL )
        {
            pPath = ( AUDIOPATH * )e->data;
            
            if( CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT == pPath->epOut ) { //SSC0 Play
                counter = kfifo_get_free_space( pPath->pfifoIn );
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_out && audio_run_control )  {
                    restart_audio_0_bulk_out = false ;
                    APP_TRACE_INFO(("\r\nBulk Out 0 start"));
                    // send ep0 data ---> pc
                    CDCDSerialDriver_ReadAudio_0( usbCacheBulkOut0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataReceived,
                                        0);  
                }
              
                counter  = kfifo_get_data_size( pPath->pfifoIn );
                counter2 = kfifo_get_free_space( pPath->pfifoOut );
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pOutTarget->txSize <= counter && pPath->pOutTarget->txSize <= counter2 ) {
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pOutTarget->txSize );
                    kfifo_put( pPath->pfifoOut,
                             ( uint8_t * )tmpBuffer,
                             pPath->pOutTarget->txSize );                 
                }               
           
                
            } else if( CDCDSerialDriverDescriptors_AUDIO_1_DATAOUT == pPath->epOut ) {   //SSC1 Play
                counter = kfifo_get_free_space( pPath->pfifoIn );
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_1_bulk_out && audio_run_control )  {
                    restart_audio_1_bulk_out = false ;
                    APP_TRACE_INFO(("\r\nBulk Out 1 start"));
                    // send ep0 data ---> pc
                    CDCDSerialDriver_ReadAudio_1( usbCacheBulkOut1,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio1DataReceived,
                                        0);  
                }
              
                counter  = kfifo_get_data_size( pPath->pfifoIn );
                counter2 = kfifo_get_free_space( pPath->pfifoOut );
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pOutTarget->txSize <= counter && pPath->pOutTarget->txSize <= counter2 ) {
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pOutTarget->txSize );
                    kfifo_put( pPath->pfifoOut,
                             ( uint8_t * )tmpBuffer,
                             pPath->pOutTarget->txSize );                 
                }               
           
                
            } else if( CDCDSerialDriverDescriptors_SPI_DATAOUT == pPath->epOut ) { //SPI/GPIO Play
                counter = kfifo_get_free_space( pPath->pfifoIn );
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_2_bulk_out && audio_run_control )  {
                    restart_audio_2_bulk_out = false ;
                    APP_TRACE_INFO(("\r\nBulk Out 2 start"));
                    // send ep0 data ---> pc
                    CDCDSerialDriver_ReadSPI( usbCacheBulkOut2,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbSPIDataReceived,
                                        0);  
                }
              
                counter  = kfifo_get_data_size( pPath->pfifoIn );
                counter2 = kfifo_get_free_space( pPath->pfifoOut );
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pOutTarget->txSize <= counter && pPath->pOutTarget->txSize <= counter2 ) {
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pOutTarget->txSize );
                    kfifo_put( pPath->pfifoOut,
                             ( uint8_t * )tmpBuffer,
                             pPath->pOutTarget->txSize );                 
                }               
                                      
            } else {
                APP_TRACE_INFO(("\r\nPath Ep not defined : %d",pPath->epOut));
                
            }            
            e = e -> next;
            
        }
#endif     
        OSTimeDly( 1 );
        
    } //for loop
    
}




/*
#if 0
static  void  AppTaskUSB_old (void *p_arg)
{
    uint8_t  err = 0;
    uint8_t  evFlags = 0; 
    uint32_t byteCnt = 0;
    uint32_t byteCnt2 = 0;
    uint8_t  usb_state = 0;
    uint8_t  usb_state_saved = 0;
    
    memset( ( uint8_t * )tmpInBuffer , 0x32 , sizeof( tmpInBuffer ) ); 
    
    for(;;) 
    {
        
        usb_state =   USBD_GetState();         
        
        if( usb_state != usb_state_saved ) { 
            usb_state_saved = usb_state ;
            if ( usb_state >= USBD_STATE_CONFIGURED ) {
                
                CDCDSerialDriver_ReadCmd(  usbCmdCacheBulkOut,
                                  USB_CMDEP_SIZE_64B ,
                                  (TransferCallback) UsbCmdDataReceived,                                
                                  0);
            } 
        }
        
        if ( usb_state >= USBD_STATE_CONFIGURED ) {
            UIF_LED_On( LED_USB );    
        } else {
            UIF_LED_Off( LED_USB );             
        }
        
        if ( usb_state >= USBD_STATE_CONFIGURED ) {  
          
          
#if 0 //for test
 //----------------------------------------------------------------------------
            byteCnt = kfifo_get_free_space( &ep0BulkOut_fifo );
            if(  byteCnt >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_out && audio_run_control )  {
                restart_audio_0_bulk_out = false ;
                APP_TRACE_INFO(("\r\nBulk Out 0 start"));
              // send ep0 data ---> pc
                CDCDSerialDriver_ReadAudio_0( usbCacheBulkOut0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataReceived,
                                        0);  
            } else {
                  byteCnt  = kfifo_get_data_size( &ep0BulkOut_fifo );
                  byteCnt = byteCnt > 256 ?  256 :   byteCnt ;
                  kfifo_get( &ep0BulkOut_fifo,
                          ( uint8_t * )tmpOutBuffer,
                           byteCnt );
                  dump_buf_debug(( uint8_t * )tmpOutBuffer, byteCnt  ) ;
            }
#endif   
        
        
#if 1
 //----------------------------------------------------------------------------
            byteCnt = kfifo_get_free_space( &ep0BulkOut_fifo );
            if(  byteCnt >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_out && audio_run_control )  {
                restart_audio_0_bulk_out = false ;
                APP_TRACE_INFO(("\r\nBulk Out 0 start"));
              // send ep0 data ---> pc
                CDCDSerialDriver_ReadAudio_0( usbCacheBulkOut0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataReceived,
                                        0);  
            }
        
        
 //----------------------------------------------------------------------------
            if( audio_start_flag ) {
                audio_start_flag = false ;
                memset( tmpInBuffer, audio_0_padding , USB_DATAEP_SIZE_64B*4 );
                kfifo_put( &ep0BulkIn_fifo,
                          ( uint8_t * )tmpInBuffer,
                           USB_DATAEP_SIZE_64B*4 );
            }
            //////////
            byteCnt  = kfifo_get_data_size( &ep0BulkOut_fifo );
            byteCnt2 = kfifo_get_free_space( &ep0BulkIn_fifo );
            if( byteCnt >= 1536  && byteCnt2 >= 1536 ) {
               kfifo_get( &ep0BulkOut_fifo,
                          ( uint8_t * )tmpOutBuffer,
                           1536 );        
               //memset( tmpInBuffer, 0x34 , 1536 );
               kfifo_put( &ep0BulkIn_fifo,
                          ( uint8_t * )tmpOutBuffer,
                           1536 );
            }
            //////////
            byteCnt = kfifo_get_data_size( &ep0BulkIn_fifo );     
            if(  byteCnt >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_in && audio_run_control)  {
                APP_TRACE_INFO(("\r\nBulk  In 0 start"));
                restart_audio_0_bulk_in = false ;
                // ep0 ring --> usb cache
                kfifo_get( &ep0BulkIn_fifo,
                          ( uint8_t * )usbCacheBulkIn0,
                           USB_DATAEP_SIZE_64B );         
            
              // send ep0 data ---> pc
                CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataTransmit,
                                        0);  
            }
#endif
               
        }
        OSTimeDlyHMSM( 0,0,0,5 );
        
    } //for loop
}
#endif
*/



void Init_Audio_Path()
{
    AUDIO_CFG in,out;
    
    in.bit_length = 16;
    in.channel_num = 2;
    in.ssc_delay = 0;
    in.sample_rate = 16000;
    in.ssc_cki = 0;
    in.ssc_start = 0;
    
    out.bit_length = 16;
    out.channel_num = 2;
    out.ssc_delay = 0;
    out.sample_rate = 16000;
    out.ssc_cki = 1;
    out.ssc_start = 0;
    
    AUDIOPATH g_audio_path;
  
    g_audio_path.createAudioPath = createPath;
    g_audio_path.findPort        = findPort;
    
    
    g_audio_path.createAudioPath(  "ep1->ssc0",
                                   ( void * )&in,
                                   "ep2<-ssc0",
                                   ( void * )&out
                                     );
    /*
    g_audio_path.createAudioPath(  "ep1->spi0",
                                   ( void * )&in,
                                   "ep2<-spi0",
                                   ( void * )&out
                                     );
    g_audio_path.createAudioPath(  "ep5->ssc1",
                                   ( void * )&in,
                                   "ep6<-ssc1",
                                   ( void * )&out
                                     );    
    g_audio_path.createAudioPath(  "ep1->ssc0",
                                   ( void * )&in,
                                   "ep2<-ssc0",
                                   ( void * )&out
                                     ); 
    g_audio_path.createAudioPath(  "ep1->ssc0",
                                   ( void * )&in,
                                   "ep2<-ssc0",
                                   ( void * )&out
                                     ); 
    g_audio_path.createAudioPath(  "ep5->ssc1",
                                   ( void * )&in,
                                   "ep6<-ssc1",
                                   ( void * )&out
                                     );
    g_audio_path.createAudioPath(  "ep1->spi0",
                                   ( void * )&in,
                                   "ep2<-spi0",
                                   ( void * )&out
                                     ); 
    g_audio_path.createAudioPath(  "ep1->gpio",
                                   ( void * )&in,
                                   "ep2<-spi0",
                                   ( void * )&out
                                     );
    g_audio_path.createAudioPath(  "ssc0->ssc1",
                                   ( void * )&in,
                                   "ssc1<-ssc0",
                                   ( void * )&out
                                     );    
    */
  
}
 