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

Pin SSC_Sync_Pin = PIN_SSC_RF;
Pin SSC_Sync_Pin1 = PIN_SSC1_RF;
uint8_t tmpBuffer[ USB_RINGOUT_SIZE_16K];
//uint8_t tmpBuffer1[ I2S_PINGPONG_IN_SIZE_3K ];
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
void i2s0_isr_handler( void );
void i2s1_isr_handler( void );
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
    extern uint8_t debug_cnt;
    
    OS_CPU_SR cpu_sr;  

 
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
        
//        if ( usb_state >= USBD_STATE_CONFIGURED ) {
          
//            UIF_LED_On( LED_USB );    
//        } else {
          
//            UIF_LED_Off( LED_USB );             
//        }
        
        if ( usb_state < USBD_STATE_CONFIGURED ) {            
            OSTimeDly( 2 );
            continue;      
        }
    
        if ( audio_run_control == false) {            
            OSTimeDly( 2 );
            continue;      
        }
       
        e = portsList.head;         
        while( e != NULL )
        {
            pPath = ( AUDIOPATH * )e->data;
            
            if( CDCDSerialDriverDescriptors_AUDIO_0_DATAIN == pPath->ep ) {  //SSC0 Rec      
#if 0                
                counter = kfifo_get_data_size( pPath->pfifoOut );  //&ep0BulkIn_fifo   
                if(  counter >= USB_DATAEP_SIZE_64B  && restart_audio_0_bulk_in )  {
                    //APP_TRACE_INFO(("\r\nAudio 0 BulkIn start"));
                    restart_audio_0_bulk_in = false ;
                    // ep0 ring --> usb cache
                    kfifo_get( pPath->pfifoOut,
                           ( uint8_t * )usbCacheBulkIn0,
                           USB_DATAEP_SIZE_64B );         
                
                    // send ep0 data ---> pc
                    CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                            USB_DATAEP_SIZE_64B,  //64B size for low delay
                                            (TransferCallback)UsbAudio0DataTransmit,
                                            0);  
                }         
#endif               
            } else if( CDCDSerialDriverDescriptors_AUDIO_1_DATAIN == pPath->ep ) {  //SSC1 Rec       
#if 0                
                counter = kfifo_get_data_size( pPath->pfifoOut );     
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_1_bulk_in )  {
//                    APP_TRACE_INFO(("\r\nAudio 1 BulkIn start"));
                    restart_audio_1_bulk_in = false ;
                    // ep0 ring --> usb cache
                    kfifo_get( pPath->pfifoOut,
                           ( uint8_t * )usbCacheBulkIn1,
                           USB_DATAEP_SIZE_64B );         
                
                  // send ep0 data ---> pc
                    CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                            USB_DATAEP_SIZE_64B,  //64B size for low delay
                                            (TransferCallback)UsbAudio1DataTransmit,
                                            0);  
                }                       
#endif                
            } else if( CDCDSerialDriverDescriptors_SPI_DATAIN == pPath->ep ) {  //SPI Rec       
                
                counter = kfifo_get_data_size( &ep2BulkIn_fifo );     
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_2_bulk_in )  {
                    APP_TRACE_INFO(("\r\nAudio 2 BulkIn start"));
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
          
                counter  = kfifo_get_data_size( pPath->pfifoIn );
                counter2 = kfifo_get_free_space( pPath->pfifoOut );
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pSource->rxSize <= counter && pPath->pSource->rxSize <= counter2 ) {
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pSource->rxSize );
                    kfifo_put( pPath->pfifoOut,
                             ( uint8_t * )tmpBuffer,
                             pPath->pSource->rxSize );                 
                } 
        
             ///////////////////////////////////////////////////////////////////                  
            } else if( CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT == pPath->ep ) { //SSC0 Play
                if( source_ssc0.status[ OUT ] == ( uint8_t )CONFIGURED )
                {
                      counter = kfifo_get_free_space( pPath->pfifoOut ); 
                      if(  counter > source_ssc0.txSize && restart_audio_0_bulk_out  )  {
                            restart_audio_0_bulk_out = false ;
                            // send ep0 data ---> pc
                            CDCDSerialDriver_ReadAudio_0( usbCacheBulkOut0,
                                                        source_ssc0.txSize,
                                                        (TransferCallback)UsbAudio0DataReceived,
                                                        0);  
                        }
                     //counter = kfifo_get_free_space( pPath->pfifoOut );
                     else if( ( counter  <= source_ssc0.txSize )
                        && ( source_ssc0.status[ IN ] < ( uint8_t )START ) )    
                      {   
                          err = Init_CODEC( &source_twi2,48000, 16 );
                          
                          First_Pack_Padding_BI( &ep0BulkIn_fifo );
                          First_Pack_Padding_BI( &ep1BulkIn_fifo ); 

                          source_ssc0.buffer_read(   &source_ssc0,
                                                  ( uint8_t * )ssc0_PingPongIn,                                              
                                                  source_ssc0.rxSize );
                          source_ssc0.status[ IN ]  = ( uint8_t )START;
                          

                          source_ssc1.buffer_read(   &source_ssc1,
                                                  ( uint8_t * )ssc1_PingPongIn,                                              
                                                  source_ssc1.rxSize );
                          source_ssc1.status[ IN ]  = ( uint8_t )START;
 
                          while( !PIO_Get( &SSC_Sync_Pin ) );
                           while( PIO_Get( &SSC_Sync_Pin ) );
#if 1                         
                          if( source_ssc0.buffer_write != NULL )
                          {
                              source_ssc0.buffer_write(  &source_ssc0,
                                                        ( uint8_t * )ssc0_PingPongOut,                                                
                                                        source_ssc0.txSize ); 
                              source_ssc0.status[ OUT ] = ( uint8_t )START;

                          }
#endif
                          
#if 1                                                                       
                          source_ssc1.buffer_write(  &source_ssc1,
                                                    ( uint8_t * )ssc1_PingPongOut,                                                
                                                    source_ssc1.txSize ); 
                          source_ssc1.status[ OUT ] = ( uint8_t )START;  

#endif                           
 
                           
                      }  
                  }
            } else if( CDCDSerialDriverDescriptors_AUDIO_1_DATAOUT == pPath->ep ) {   //SSC1 Play
                if( source_ssc1.status[ OUT ] == ( uint8_t )CONFIGURED )
                {
                      counter = kfifo_get_free_space( pPath->pfifoOut ); 
                      if(  counter > source_ssc1.txSize && restart_audio_1_bulk_out  )  {
                            restart_audio_1_bulk_out = false ;
                            // send ep0 data ---> pc
                            CDCDSerialDriver_ReadAudio_1( usbCacheBulkOut1,
                                                        source_ssc1.txSize,
                                                        (TransferCallback)UsbAudio1DataReceived,
                                                        0);  
                        }
                     else if( ( counter  <= source_ssc1.txSize )
                        && ( source_ssc1.status[ IN ] < ( uint8_t )START ) )    
                      { 
#if 0                        
//                          err = Init_CODEC( &source_twi1,48000, 16 );
                          First_Pack_Padding_BI( &ep1BulkIn_fifo ); 
                          source_ssc1.buffer_read(   &source_ssc1,
                                                  ( uint8_t * )ssc1_PingPongIn,                                              
                                                  source_ssc1.rxSize );
                          source_ssc1.status[ IN ]  = ( uint8_t )START;
 
                    

                          
                          source_ssc1.buffer_write(  &source_ssc1,
                                                    ( uint8_t * )ssc1_PingPongOut,                                                
                                                    source_ssc1.txSize ); 
                          source_ssc1.status[ OUT ] = ( uint8_t )START;  

#endif                          
                      }  
                  }                           
            } else if( CDCDSerialDriverDescriptors_SPI_DATAOUT == pPath->ep ) { //SPI/GPIO Play
                counter = kfifo_get_free_space( pPath->pfifoIn );
                if(  counter >= USB_DATAEP_SIZE_64B && restart_audio_2_bulk_out  )  {
                    restart_audio_2_bulk_out = false ;
                    APP_TRACE_INFO(("\r\nAudio 2 BulkOut start"));
                    // send ep0 data ---> pc
                    CDCDSerialDriver_ReadSPI( usbCacheBulkOut2,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbSPIDataReceived,
                                        0);  
                }
              
                counter  = kfifo_get_data_size( pPath->pfifoIn );
                counter2 = kfifo_get_free_space( pPath->pfifoOut );
                //step2: get data from ssc0/spi0/gpio ring buffer to temp buffer.
                if( pPath->pSource->txSize <= counter && pPath->pSource->txSize <= counter2 ) {
                    kfifo_get( pPath->pfifoIn,
                             ( uint8_t * )tmpBuffer,
                             pPath->pSource->txSize );
                    kfifo_put( pPath->pfifoOut,
                             ( uint8_t * )tmpBuffer,
                             pPath->pSource->txSize );                 
                }               
                                      
            } else {
                APP_TRACE_INFO(("\r\nPath not defined[EP%d] !",pPath->ep ));                
            }            
            e = e -> next;            
        }         
        OSTimeDly( 1 );
//        OS_Sched();          
    } //for loop
    
}







 