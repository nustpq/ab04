#ifndef _USB_H
#define _USB_H

#include "board.h"
#include "kfifo.h"
#include "USBD.h"
#include "CDCDSerialDriver.h"


#define PIN_USB_VBUS      {PIO_PE9, PIOE, ID_PIOE, PIO_INPUT, PIO_PULLUP} 

#define USBDATAEPSIZE   64

extern void USBD_IrqHandler(void);

extern kfifo_t  bulkout_fifo;
extern kfifo_t  bulkin_fifo;
extern kfifo_t  bulkout_fifo_cmd;
extern kfifo_t  bulkin_fifo_cmd;

static const Pin pinVbus = PIN_USB_VBUS;



void UsbAudio0DataReceived(  uint32_t unused,
                             uint8_t status,
                             uint32_t received,
                             uint32_t remaining );
void UsbAudio1DataReceived(  uint32_t unused,
                             uint8_t status,
                             uint32_t received,
                             uint32_t remaining );

void UsbAudio0DataTransmit(  uint32_t unused,
                             uint8_t status,
                             uint32_t transmit,
                             uint32_t remaining );
void UsbAudio1DataTransmit(  uint32_t unused,
                             uint8_t status,
                             uint32_t transmit,
                             uint32_t remaining );


static void ISR_Vbus( const Pin *pPin );
static void VBus_Configure( void );
static void USBPower_Configure( void );
void USBDCallbacks_Initialized( void );
void USBDDriverCallbacks_ConfigurationChanged( uint8_t cfgnum );
void USBDCallbacks_RequestReceived( const USBGenericRequest *request );

void init_usb( void *pParameter,void *dParameter ); 

#endif
