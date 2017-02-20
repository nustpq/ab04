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

bool restart_audio_0_bulk_out  = false ; 
bool restart_audio_0_bulk_in   = false ; 
bool restart_audio_1_bulk_out  = false ; 
bool restart_audio_1_bulk_in   = false ; 
bool restart_audio_2_bulk_out  = false ; 
bool restart_audio_2_bulk_in   = false ; 
bool restart_log_bulk_in       = true ;
bool restart_cmd_bulk_out      = false ; 
bool restart_cmd_bulk_in       = false ; 
bool audio_run_control         = false ; 
bool padding_audio_0_bulk_out  = false ;


void UsbAudio0DataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining )
{   
    remaining = remaining;
    uint32_t size;
    static bool flag = 0;
//      UIF_LED_On( 3 );
    if ( status == USBD_STATUS_SUCCESS ) 
    {         
       // Check every data package:        
       // LED_CLEAR_DATA;
//          flag = First_Pack_Check_BO(&usbCacheBulkOut0, received); 
//          if( flag ) {            
//              if( ! padding_audio_0_bulk_out ) {                            
//                  padding_audio_0_bulk_out = true;
//              }              
//          } else {              
//              if( padding_audio_0_bulk_out ) {
//                  kfifo_put(&ep0BulkOut_fifo, usbCacheBulkOut0, received);  
//              }              
//          }             
       // LED_SET_DATA;
      if( false == padding_audio_0_bulk_out )
         padding_audio_0_bulk_out = First_Pack_Check_BO(&usbCacheBulkOut0, received);      
      else
         if( false == flag )
            flag = true;
         else
            kfifo_put(&ep0BulkOut_fifo, usbCacheBulkOut0, received);  
          
        size = kfifo_get_free_space( &ep0BulkOut_fifo );
        if ( USB_DATAEP_SIZE_64B <=  size ) 
        { 
            //enough free buffer                      
            CDCDSerialDriver_ReadAudio_0(    usbCacheBulkOut0,
                                      USB_DATAEP_SIZE_64B,
                                      (TransferCallback) UsbAudio0DataReceived,
                                      0);        
        } 
       else 
       { 
            //usb out too fast                     
            //printf( "\r\nERROR : UsbAudio0DataReceived: Usb Transfer fast\r\n" );
            ///Todo:maybe send a feedback to pc and let it wait a moment?
           restart_audio_0_bulk_out  = true ;  
           return;           
       }     
//       total_received += received ; 
     
    }  
    else 
    {      
        APP_TRACE_INFO(("\r\nERROR : UsbAudio0DataReceived: Transfer error\r\n" )); 
        
    }
//      UIF_LED_Off( 3 );
    
}

           
void UsbAudio1DataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining )
{
    remaining = remaining;
    uint32_t size;
    
    if ( status == USBD_STATUS_SUCCESS ) 
    {     
         // Check every data package:        
          // LED_CLEAR_DATA;

        kfifo_put( &ep1BulkOut_fifo, usbCacheBulkOut1, received ); 
        
        size = kfifo_get_free_space( &ep1BulkOut_fifo );
        if ( USB_DATAEP_SIZE_64B <= size ) 
        { 
            //enouth free buffer                      
            CDCDSerialDriver_ReadAudio_1(    usbCacheBulkOut1,
                                               USB_DATAEP_SIZE_64B,
                                              (TransferCallback) UsbAudio1DataReceived,
                                              0);        
        } 
        else 
        { 
          ///Todo:usb out too fast 
          //printf( "\r\nERROR : UsbAudio1DataReceived: Usb Transfer fast\r\n" );
          restart_audio_1_bulk_out  = true ; 
          return;
            
        }     
 //       total_received += received ; 
     
    }  
    else 
    {      
        APP_TRACE_INFO(( "\r\nERROR : UsbAudio1DataReceived: Transfer error\r\n" )); 
        
    }      
}

void UsbAudio0DataTransmit(  uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{          
    UIF_LED_On( 3 ); 
    if ( status == USBD_STATUS_SUCCESS  ) 
    {              
        if ( USB_DATAEP_SIZE_64B * 2 <= kfifo_get_data_size(  &ep0BulkIn_fifo )  ) 
        { 
            //enough data to send to PC           
            kfifo_get( &ep0BulkIn_fifo, usbCacheBulkIn0, USB_DATAEP_SIZE_64B ); 
        
            CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                    USB_DATAEP_SIZE_64B,
                                    (TransferCallback) UsbAudio0DataTransmit,
                                    0);       
        } 
        else 
        {  
           //has no enough data,exit;
           restart_audio_0_bulk_in  = true ; 
           return; 
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                USB_DATAEP_SIZE_64B,
                                (TransferCallback) UsbAudio0DataTransmit,
                                0);  
        APP_TRACE_INFO(( "\r\nERROR : UsbAudio0DataTransmit: Rr-transfer hit\r\n" ));  
        
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
       
        if ( USB_DATAEP_SIZE_64B <= kfifo_get_data_size(  &ep1BulkIn_fifo )  ) 
        { 
          //enough data to send to PC          
            kfifo_get(&ep1BulkIn_fifo, usbCacheBulkIn1, USB_DATAEP_SIZE_64B); 
        
            CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                             USB_DATAEP_SIZE_64B,
                                             (TransferCallback) UsbAudio1DataTransmit,
                                             0);       
        } 
        else 
        {                    
            restart_audio_1_bulk_in  = true ;   
            
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                         USB_DATAEP_SIZE_64B,
                                         (TransferCallback) UsbAudio1DataTransmit,
                                         0);  
        APP_TRACE_INFO(( "\r\nERROR : UsbAudio1DataTransmit: Rr-transfer hit\r\n" ));  
        
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
         // Check every data package:        
          // LED_CLEAR_DATA;

        kfifo_put( &ep2BulkOut_fifo, usbCacheBulkOut2, received ); 
        
        size = kfifo_get_free_space( &ep2BulkOut_fifo );
        if ( USB_DATAEP_SIZE_64B <= size ) 
        { 
            //enouth free buffer                      
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
 //       total_received += received ; 
     
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
            kfifo_get(&DBG_USB_Send_kFIFO, usbCacheBulkIn3, USB_DATAEP_SIZE_64B); 
        
            CDCDSerialDriver_WriteLog( usbCacheBulkIn3,
                                             USB_DATAEP_SIZE_64B,
                                             (TransferCallback) UsbLogDataTransmit,
                                             0);       
        } 
        else 
        {                    
            restart_log_bulk_in  = true ;   
            
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_WriteLog( usbCacheBulkIn3,
                                         USB_DATAEP_SIZE_64B,
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
        
        if ( USB_DATAEP_SIZE_64B <= kfifo_get_free_space( &cmdEpBulkOut_fifo ) ) 
        { 
            //enough free buffer                      
            CDCDSerialDriver_ReadCmd(  usbCmdCacheBulkOut,
                                          USB_DATAEP_SIZE_64B,
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

void UsbCmdDataTransmit(  uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining )
{             
    
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USB_DATAEP_SIZE_64B <= kfifo_get_data_size(  &cmdEpBulkIn_fifo )  ) 
        {         
            kfifo_get(&cmdEpBulkIn_fifo, usbCmdCacheBulkIn, USB_DATAEP_SIZE_64B); 
        
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
                                      USB_DATAEP_SIZE_64B,
                                      (TransferCallback) UsbCmdDataTransmit,
                                      0);  
        TRACE_WARNING( "\r\nERROR : UsbCmdDataTransmit: Rr-transfer hit\r\n" );  
        
    }        
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
    IRQ_ConfigureIT(ID_UDPHS, 2, USBD_IrqHandler);
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



