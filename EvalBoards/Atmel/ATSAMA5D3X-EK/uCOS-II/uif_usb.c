/*
*********************************************************************************************************
*
*                                               APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Unified EVM Interface Board
*
* Filename         : usb.c
* Version          : V0.0.1
* Programmer(s)    : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include  <ucos_ii.h>

#include "usb.h"
#include "defined.h"

uint8_t usbBufferBulkOut0[USBDATAEPSIZE];
uint8_t usbBufferBulkOut1[USBDATAEPSIZE];
uint8_t usbBufferBulkIn0[USBDATAEPSIZE];
uint8_t usbBufferBulkIn1[USBDATAEPSIZE];

extern kfifo_t  ssc0_bulkout_fifo;
extern kfifo_t  ssc0_bulkin_fifo;
extern kfifo_t  ssc1_bulkout_fifo;
extern kfifo_t  ssc1_bulkin_fifo;
extern kfifo_t  bulkout_fifo_cmd;
extern kfifo_t  bulkin_fifo_cmd;

void UsbAudio0DataReceived(  unsigned int unused,
                              unsigned char status,
                              unsigned int received,
                              unsigned int remaining )
{
    //Play          
 //   if ( ssc0_dev.status != RUNNING ) 
 //   {
 //       return ;
 //   }
    
    if ( status == USBD_STATUS_SUCCESS ) 
    {     
         // Check every data package:        
          // LED_CLEAR_DATA;

        kfifo_put(&ssc0_bulkout_fifo, usbBufferBulkOut0, received);         
        
        if ( USBDATAEPSIZE <= kfifo_get_free_space( &ssc0_bulkout_fifo ) ) 
        { //enouth free buffer                      
            CDCDSerialDriver_Read(    usbBufferBulkOut0,
                                      USBDATAEPSIZE,
                                      (TransferCallback) UsbAudio0DataReceived,
                                      0);        
        } 
//       else 
//       { //usb out too fast                     
//            bulkout_start  = true ;
            
//       }     
//       total_received += received ; 
     
    }  
    else 
    {      
        printf( "\r\nERROR : UsbAudioDataReceived: Transfer error\r\n" ); 
        
    }
    
    
}


void UsbAudio1DataReceived(  unsigned int unused,
                              unsigned char status,
                              unsigned int received,
                              unsigned int remaining )
{
    //Play          
//    if ( ssc1_dev.status != RUNNING ) 
//    {
//        return ;
//    }
    
    if ( status == USBD_STATUS_SUCCESS ) 
    {     
         // Check every data package:        
          // LED_CLEAR_DATA;

        kfifo_put(&ssc1_bulkout_fifo, usbBufferBulkOut1, received);         
        
        if ( USBDATAEPSIZE <= kfifo_get_free_space( &ssc1_bulkout_fifo ) ) { //enouth free buffer                      
            CDCDSerialDriver_Read(    usbBufferBulkOut1,
                                      USBDATAEPSIZE,
                                      (TransferCallback) UsbAudio1DataReceived,
                                      0);        
        } else { //usb out too fast                     
//            bulkout_start  = true ;
            
        }     
 //       total_received += received ; 
     
    }  
    else 
    {      
        printf( "\r\nERROR : UsbAudio1DataReceived: Transfer error\r\n" ); 
        
    }      
}

void UsbAudio0DataTransmit(  unsigned int unused,
                              unsigned char status,
                              unsigned int transmit,
                              unsigned int remaining )
{          
    //Record    
//    if ( ssc0_dev.status != RUNNING ) 
//    {
//        return ;
//    }
    
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USBDATAEPSIZE <= kfifo_get_data_size(  &ssc0_bulkin_fifo )  ) 
        { //enough data to send to PC
           
            kfifo_get(&ssc0_bulkin_fifo, usbBufferBulkIn0, USBDATAEPSIZE); 
        
            CDCDSerialDriver_Write( usbBufferBulkIn0,
                                    USBDATAEPSIZE,
                                    (TransferCallback) UsbAudio0DataTransmit,
                                    0);       
        } 
        else 
        {                    
//            bulkin_start  = true ;  
            
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_Write( usbBufferBulkIn0,
                                USBDATAEPSIZE,
                                (TransferCallback) UsbAudio0DataTransmit,
                                0);  
        TRACE_WARNING( "\r\nERROR : UsbAudioDataTransmit: Rr-transfer hit\r\n" );  
        
    }    
    
}

void UsbAudio1DataTransmit(  unsigned int unused,
                              unsigned char status,
                              unsigned int transmit,
                              unsigned int remaining )
{          
    //Record    
//    if ( ssc1_dev.status != RUNNING ) 
//    {
//        return ;
//    }
    
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USBDATAEPSIZE <= kfifo_get_data_size(  &ssc1_bulkin_fifo )  ) 
        { //enough data to send to PC
           
            kfifo_get(&ssc1_bulkin_fifo, usbBufferBulkIn1, USBDATAEPSIZE); 
        
            CDCDSerialDriver_Write( usbBufferBulkIn1,
                                    USBDATAEPSIZE,
                                    (TransferCallback) UsbAudio1DataTransmit,
                                    0);       
        } 
        else 
        {                    
//            bulkin_start  = true ;  
            
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_Write( usbBufferBulkIn1,
                                USBDATAEPSIZE,
                                (TransferCallback) UsbAudio1DataTransmit,
                                0);  
        TRACE_WARNING( "\r\nERROR : UsbAudioDataTransmit: Rr-transfer hit\r\n" );  
        
    }        
}

void UsbCmdDataTransmit(  unsigned int unused,
                              unsigned char status,
                              unsigned int transmit,
                              unsigned int remaining )
{             
    
    if ( status == USBD_STATUS_SUCCESS  ) 
    {       
       
        if ( USBDATAEPSIZE <= kfifo_get_data_size(  &ssc1_bulkin_fifo )  ) 
        { //enough data to send to PC
           
            kfifo_get(&bulkin_fifo_cmd, usbCmdBufferBulkIn, USBDATAEPSIZE); 
        
            CDCDSerialDriver_Write( usbCmdBufferBulkIn,
                                    USBDATAEPSIZE,
                                    (TransferCallback) UsbCmdDataTransmit,
                                    0);       
        } 
        else 
        {                    
//            bulkin_start  = true ;  
            
        }              
//        total_transmit += transmit ; 
     
    }  
    else 
    {
        CDCDSerialDriver_Write( usbBufferBulkIn1,
                                USBDATAEPSIZE,
                                (TransferCallback) UsbAudio1DataTransmit,
                                0);  
        TRACE_WARNING( "\r\nERROR : UsbAudioDataTransmit: Rr-transfer hit\r\n" );  
        
    }        
}


static void ISR_Vbus(const Pin *pPin)
{
      OSIntEnter(); 

    /* Check current level on VBus */
    if (PIO_Get(pPin))
    {
        TRACE_INFO("VBUS conn\n\r");
        USBD_Connect();
    }
    else
    {
        TRACE_INFO("VBUS discon\n\r");
        USBD_Disconnect();
    }
        OSIntExit();
}

static void VBus_Configure( void )
{
    TRACE_INFO("VBus configuration\n\r");

    /* Configure PIO */
    PIO_Configure(&pinVbus, 1);
    PIO_ConfigureIt(&pinVbus, ISR_Vbus);
    PIO_EnableIt(&pinVbus);

    /* Check current level on VBus */
    if (PIO_Get(&pinVbus))
    {
        /* if VBUS present, force the connect */
        TRACE_INFO("conn\n\r");
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
    IRQ_ConfigureIT(ID_UDPHS, 0, USBD_IrqHandler);
    IRQ_EnableIT(ID_UDPHS);
}


void USBDDriverCallbacks_ConfigurationChanged(unsigned char cfgnum)
{
    CDCDSerialDriver_ConfigurationChangedHandler(cfgnum);
}

void USBDCallbacks_RequestReceived(const USBGenericRequest *request)
{
    CDCDSerialDriver_RequestHandler(request);
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



