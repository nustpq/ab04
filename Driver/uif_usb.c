/*
*********************************************************************************************************
*
*                                               APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename         : uif_usb.c
* Version          : V0.0.1
* Programmer(s)    : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/
     
#include  <ucos_ii.h>
#include "app_cfg.h"
#include "uif_usb.h"
#include "defined.h"



//Ring for USB data endpoint
extern kfifo_t  ep0BulkOut_fifo;
extern kfifo_t  ep0BulkIn_fifo;
extern kfifo_t  ep1BulkOut_fifo;
extern kfifo_t  ep1BulkIn_fifo;

//Ring for USB cmd endpoint
extern kfifo_t  cmdEpBulkOut_fifo;
extern kfifo_t  cmdEpBulkIn_fifo;

extern kfifo_t DBG_USB_Send_kFIFO;

bool volatile restart_audio_0_bulk_out  = false ; 
bool volatile restart_audio_0_bulk_in   = false ; 
bool volatile restart_audio_1_bulk_out  = false ; 
bool volatile restart_audio_1_bulk_in   = false ; 
bool volatile restart_audio_2_bulk_out  = false ; 
bool volatile restart_audio_2_bulk_in   = false ; 
bool volatile restart_log_bulk_in       = true ;
bool volatile restart_cmd_bulk_out      = false ; 
bool volatile restart_cmd_bulk_in       = false ; 
bool volatile audio_run_control         = false ; 
bool volatile padding_audio_0_bulk_out  = false ;
bool volatile padding_audio_1_bulk_out  = false ;
bool volatile audio_play_buffer_ready   = false ; 

//paly
//paly
#if 0
void UsbAudio0DataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining )
{   
    remaining = remaining;
    uint32_t pos = 0,offset = 0;
    uint8_t fillOffset[ 16 ] = { 0 };

    if ( status == USBD_STATUS_SUCCESS ) 
    { 
      if( false == padding_audio_0_bulk_out )
      {
         padding_audio_0_bulk_out = First_Pack_Check_BO( &usbCacheBulkOut0, 128 );
         
         if( true == padding_audio_0_bulk_out )
         {
            kfifo_put( &ssc0_bulkout_fifo, usbCacheBulkOut0 + 128, received-128 );
     
         }
         else
         {
              ; 
         }
      }
      else
          kfifo_put(&ssc0_bulkout_fifo, usbCacheBulkOut0, received );

      restart_audio_0_bulk_out  = true ; 
   
    }  
    else 
    {      
        APP_TRACE_INFO(("\r\nERROR : UsbAudio0DataReceived: Transfer error\r\n" ));
        assert( 0 );
        
    }   
}
#else
void UsbAudio0DataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining )
{   
    remaining = remaining;
    uint32_t pos = 0,offset = 0;
    uint8_t fillOffset[ 16 ] = { 0 };
    const uint8_t pos1 = 128;
    
#if 0
    if ( status == USBD_STATUS_SUCCESS ) 
    { 
      if( false == padding_audio_0_bulk_out )
      {
         padding_audio_0_bulk_out = First_Pack_Check_BO2( &usbCacheBulkOut0, received , &pos );
         
         if( true == padding_audio_0_bulk_out )
         {
            kfifo_put( &ssc0_bulkout_fifo, usbCacheBulkOut0+pos, received-pos );
         }
         else
         {
              ; 
         }
      }
      else
          kfifo_put(&ssc0_bulkout_fifo, usbCacheBulkOut0, received);

      restart_audio_0_bulk_out  = true ; 
   
    }  
    else 
    {      
        APP_TRACE_INFO(("\r\nERROR : UsbAudio0DataReceived: Transfer error\r\n" ));
        assert( 0 );
        
    }
#endif
    
    switch( status )
    {
    case USBD_STATUS_SUCCESS:
      {
            if( false == padding_audio_0_bulk_out )
            {
                padding_audio_0_bulk_out = First_Pack_Check_BO2( &usbCacheBulkOut0, received, &pos );
         
                assert( ( pos % 16 ) == 0 );
                if( true == padding_audio_0_bulk_out )
                {
                    kfifo_put( &ssc0_bulkout_fifo, usbCacheBulkOut0+pos, received-pos );
                }
                else
                {
                    ; 
                }
            }
            else
                kfifo_put(&ssc0_bulkout_fifo, usbCacheBulkOut0, received);

            restart_audio_0_bulk_out  = true ; 
      }
      break;
    case USBD_STATUS_PARTIAL_DONE:
      assert( 0 );
      break;
    default:
      assert( 0 );
      break;
      
    }
}
#endif

void UsbAudio1DataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining )
{   
    remaining = remaining;
    uint32_t pos = 0,offset = 0;
    uint8_t fillOffset[ 16 ] = { 0 };

   if ( status == USBD_STATUS_SUCCESS ) 
    { 
      if( false == padding_audio_1_bulk_out )
      {
         padding_audio_1_bulk_out = First_Pack_Check_BO2( &usbCacheBulkOut1, received , &pos );
         
         if( true == padding_audio_1_bulk_out )
         {
            kfifo_put( &ssc1_bulkout_fifo, usbCacheBulkOut1+pos, received-pos );
         }
         else
         {
              ; 
         }
      }
      else
          kfifo_put(&ssc1_bulkout_fifo, usbCacheBulkOut1, received);

      restart_audio_1_bulk_out  = true ; 
   
    }  
    else 
    {      
        APP_TRACE_INFO(("\r\nERROR : UsbAudio1DataReceived: Transfer error\r\n" ));
        assert( 0 );
        
    }

}

//record
void UsbAudio0DataTransmit(  uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{          
    UIF_LED_On( 3 ); 
    if ( status == USBD_STATUS_SUCCESS  ) 
    {              
        restart_audio_0_bulk_in  = true ;               
    }  
    else 
    {
        CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                1024,
                                (TransferCallback) UsbAudio0DataTransmit,
                                0); 
        
        APP_TRACE_INFO(( "\r\nERROR : UsbAudio0DataTransmit: Rr-transfer hit\r\n" ));  
        assert( 0 );
    } 
    UIF_LED_Off( 3 ); 
    
}

void UsbAudio1DataTransmit(  uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{          
    //Record        
    if ( status == USBD_STATUS_SUCCESS  ) 
    {              
        restart_audio_1_bulk_in  = true ;               
    }  
    else 
    {
        CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                1024,
                                (TransferCallback) UsbAudio1DataTransmit,
                                0); 
        
        APP_TRACE_INFO(( "\r\nERROR : UsbAudio1DataTransmit: Rr-transfer hit\r\n" ));  
        assert( 0 );
    } 
}

void UsbSPIDataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining )
{
    remaining = remaining;
    uint32_t size;
    
    if ( status == USBD_STATUS_SUCCESS ) 
    {     
        kfifo_put( &ep2BulkOut_fifo, usbCacheBulkOut2, received ); 
        
        size = kfifo_get_free_space( &ep2BulkOut_fifo );
        if ( USB_DATAEP_SIZE_64B <= size ) 
        {                    
            CDCDSerialDriver_ReadSPI(    usbCacheBulkOut2,
                                               USB_DATAEP_SIZE_64B,
                                              (TransferCallback) UsbSPIDataReceived,
                                              0);        
        } 
        else 
        { 
          ///Todo:usb out too fast 
          //printf( "\r\nERROR : UsbAudio1DataReceived: Usb Transfer fast\r\n" );
          restart_audio_2_bulk_out  = true ; 
          return;
            
        }     
    
    }  
    else 
    {      
        APP_TRACE_INFO(( "\r\nERROR : UsbSPIDataReceived: Transfer error\r\n" )); 
        
    }      
}



void UsbSPIDataTransmit(      uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{          
    //Record        
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USB_DATAEP_SIZE_64B <= kfifo_get_data_size(  &ep2BulkIn_fifo )  ) 
        { 
          //enough data to send to PC          
            kfifo_get(&ep2BulkIn_fifo, usbCacheBulkIn2, USB_DATAEP_SIZE_64B); 
        
            CDCDSerialDriver_WriteSPI( usbCacheBulkIn2,
                                             USB_DATAEP_SIZE_64B,
                                             (TransferCallback) UsbSPIDataTransmit,
                                             0);       
        } 
        else 
        {                    
            restart_audio_2_bulk_in  = true ;   
            
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_WriteSPI( usbCacheBulkIn2,
                                         USB_DATAEP_SIZE_64B,
                                         (TransferCallback) UsbSPIDataTransmit,
                                         0);  
        APP_TRACE_INFO(( "\r\nERROR : UsbSPIDataTransmit: Rr-transfer hit\r\n" ));  
        
    }        
}
   

void UsbLogDataTransmit(      uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{          
    //Record    
    
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USB_DATAEP_SIZE_64B <= kfifo_get_data_size(  &DBG_USB_Send_kFIFO )  ) 
        { 
          //enough data to send to PC          
            kfifo_get(&DBG_USB_Send_kFIFO, usbCacheBulkIn3, USB_LOGEP_SIZE_256B); 
        
            CDCDSerialDriver_WriteLog( usbCacheBulkIn3,
                                             USB_DATAEP_SIZE_64B,
                                             (TransferCallback) UsbLogDataTransmit,
                                             0);       
        } 
        else 
        {                    
            restart_log_bulk_in  = true ;   
            
        }                   
    }  
    else 
    {
        CDCDSerialDriver_WriteLog( usbCacheBulkIn3,
                                         USB_LOGEP_SIZE_256B,
                                         (TransferCallback) UsbLogDataTransmit,
                                         0);  
        APP_TRACE_INFO(( "\r\nERROR : UsbLogDataTransmit: Rr-transfer hit\r\n" ));  
        
    }        
}
   

void UsbCmdDataReceived(  uint32_t unused,
                          uint8_t status,
                          uint32_t received,
                          uint32_t remaining )
{   
    remaining = remaining;
    
    if ( status == USBD_STATUS_SUCCESS ) 
    {     
        // Check every data package:        
        // LED_CLEAR_DATA;

        kfifo_put(&cmdEpBulkOut_fifo, usbCmdCacheBulkOut, received);         
        
        if ( USB_CMDEP_SIZE_64B <= kfifo_get_free_space( &cmdEpBulkOut_fifo ) ) 
        { 
            //enough free buffer                      
            CDCDSerialDriver_ReadCmd(  usbCmdCacheBulkOut,
                                          USB_CMDEP_SIZE_64B,
                                          (TransferCallback) UsbCmdDataReceived,
                                          0);        
        } 
       else 
       { 
           ///Todo:usb out too fast 
           restart_cmd_bulk_out  = true ;              
       }     
//       total_received += received ; 
     
    }  
    else 
    {      
        APP_TRACE_INFO(( "\r\nERROR : UsbCmdDataReceived: Transfer error\r\n" )); 
        
    }      
    
}

void Check_CMD_BulkOut_Restart( void )
{
    if ( restart_cmd_bulk_out && ( USB_CMDEP_SIZE_64B <= kfifo_get_free_space(&cmdEpBulkOut_fifo)) ) { //               
         restart_cmd_bulk_out = false ;           
          CDCDSerialDriver_ReadCmd(  usbCmdCacheBulkOut,
                                          USB_CMDEP_SIZE_64B,
                                          (TransferCallback) UsbCmdDataReceived,
                                          0);  
     } 
}

void UsbCmdDataTransmit(  uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{             
    
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USB_CMDEP_SIZE_64B <= kfifo_get_data_size(  &cmdEpBulkIn_fifo )  ) 
        {         
            kfifo_get(&cmdEpBulkIn_fifo, usbCmdCacheBulkIn, USB_CMDEP_SIZE_64B); 
        
            CDCDSerialDriver_WriteCmd( usbCmdCacheBulkIn,
                                          USB_DATAEP_SIZE_64B,
                                          (TransferCallback) UsbCmdDataTransmit,
                                          0);       
        } 
        else 
        {                    
            restart_cmd_bulk_in  = true ;   
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_WriteCmd( usbCmdCacheBulkIn,
                                      USB_CMDEP_SIZE_64B,
                                      (TransferCallback) UsbCmdDataTransmit,
                                      0);  
        TRACE_WARNING( "\r\nERROR : UsbCmdDataTransmit: Rr-transfer hit\r\n" );  
        
    }        
}
                             
uint32_t usb_CloseData( uint8_t bEndpoint )
{
  return CDCDSerialDriver_CloseStream( bEndpoint , 1 );  
}


static void ISR_Vbus(const Pin *pPin)
{
    OSIntEnter(); 
    /* Check current level on VBus */
    if (PIO_Get(pPin))
    {
        APP_TRACE_INFO(("VBUS conn\n\r"));
        USBD_Connect();
    }
    else
    {
        APP_TRACE_INFO(("VBUS discon\n\r"));
        USBD_Disconnect();
    }
    OSIntExit();
}

static void VBus_Configure( void )
{
    APP_TRACE_INFO(("VBus configuration\n\r"));

    /* Configure PIO */
    PIO_Configure(&pinVbus, 1);
    PIO_ConfigureIt(&pinVbus, ISR_Vbus);
    PIO_EnableIt(&pinVbus);

    /* Check current level on VBus */
    if (PIO_Get(&pinVbus))
    {
        /* if VBUS present, force the connect */
        APP_TRACE_INFO(("conn\n\r"));
        USBD_Connect();
    }
    else
    {
        USBD_Disconnect();
    }
}

static void USBPower_Configure( void )
{
  #ifdef PIN_USB_POWER_ENA
    PIO_Configure(&pinPOnA, 1);
  #endif
  #ifdef PIN_USB_POWER_ENB
    PIO_Configure(&pinPOnB, 1);
  #endif
  #ifdef PIN_USB_POWER_ENC
    PIO_Configure(&pinPOnC, 1);
  #endif
}

/**
 * Invoked after the USB driver has been initialized. By default, configures
 * the UDP/UDPHS interrupt.
 */
void USBDCallbacks_Initialized(void)
{
    IRQ_ConfigureIT(ID_UDPHS, USB_PRIORITY, USBD_IrqHandler);
    IRQ_EnableIT(ID_UDPHS);
}


void USBDDriverCallbacks_ConfigurationChanged( uint8_t cfgnum)
{
    CDCDSerialDriver_ConfigurationChangedHandler( cfgnum );
}

void USBDCallbacks_RequestReceived( const USBGenericRequest *request )
{
    CDCDSerialDriver_RequestHandler( request );
}

void init_usb( void *pParameter,void *dParameter )
{
    extern const USBDDriverDescriptors cdcdSerialDriverDescriptors;

	pParameter = pParameter;
	dParameter = dParameter;

	/* If they are present, configure Vbus & Wake-up pins */
    PIO_InitializeInterrupts(0);

    /* Initialize all USB power (off) */
    USBPower_Configure();

	/* CDC serial driver initialization */
    CDCDSerialDriver_Initialize(&cdcdSerialDriverDescriptors);

    /* connect if needed */
    VBus_Configure();


}



