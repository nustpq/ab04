#ifndef _UIF_USB_H
#define _UIF_USB_H

#include "board.h"
#include "kfifo.h"
#include "USBD.h"
#include "CDCDSerialDriver.h"


#define PIN_USB_VBUS      {PIO_PE9, PIOE, ID_PIOE, PIO_INPUT, PIO_PULLUP} 

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
void UsbSPIDataReceived(  uint32_t unused,
                              uint8_t status,
                              uint32_t received,
                              uint32_t remaining );
void UsbCmdDataReceived(  uint32_t unused,
                          uint8_t status,
                          uint32_t received,
                          uint32_t remaining );
void Check_CMD_BulkOut_Restart( void );


void UsbAudio0DataTransmit(  uint32_t unused,
                             uint8_t status,
                             uint32_t transmit,
                             uint32_t remaining );
void UsbAudio1DataTransmit(  uint32_t unused,
                             uint8_t status,
                             uint32_t transmit,
                             uint32_t remaining ); 
void UsbSPIDataTransmit(  uint32_t unused,
                             uint8_t status,
                             uint32_t transmit,
                             uint32_t remaining ); 
void UsbCmdDataTransmit(  uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining );
void UsbLogDataTransmit(      uint32_t unused,
                              uint8_t status,
                              uint32_t transmit,
                              uint32_t remaining );

uint32_t usb_CloseData( uint8_t bEndpoint );

static void ISR_Vbus( const Pin *pPin );
static void VBus_Configure( void );
static void USBPower_Configure( void );
void USBDCallbacks_Initialized( void );
void USBDDriverCallbacks_ConfigurationChanged( uint8_t cfgnum );
void USBDCallbacks_RequestReceived( const USBGenericRequest *request );

void Init_USB( void ); 



extern volatile bool restart_audio_0_bulk_out   ; 
extern volatile bool restart_audio_0_bulk_in    ; 
extern volatile bool restart_audio_1_bulk_out   ; 
extern volatile bool restart_audio_1_bulk_in    ; 
extern volatile bool restart_audio_2_bulk_out   ; 
extern volatile bool restart_audio_2_bulk_in    ; 
extern volatile bool restart_log_bulk_in        ; 
extern volatile bool restart_cmd_bulk_out       ; 
extern volatile bool restart_cmd_bulk_in        ;
extern volatile bool audio_run_control          ;
extern volatile bool padding_audio_0_bulk_out   ;
extern volatile bool padding_audio_1_bulk_out   ;
extern volatile bool flag_stop                  ;
extern bool volatile audio_play_buffer_ready    ;
#endif
