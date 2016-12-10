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
*                                          Atmel AT91SAM3U4C
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : task_user_if.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#include <includes.h>

#define DBG_UART_BUF_SIZE    256

CPU_INT08U DBG_UART_Send_Buffer[ DBG_UART_Send_Buf_Size ]; //FIFO
CPU_INT08U DBG_USB_Send_Buffer[ DBG_USB_Send_Buf_Size ];   //FIFO

CPU_INT16U debug_uart_fifo_data_max ;
CPU_INT16U debug_usb_fifo_data_max ;
CPU_INT16U debug_uart_fifo_oveflow_counter ;
CPU_INT16U debug_usb_fifo_oveflow_counter ;

kfifo_t DBG_UART_Send_kFIFO;
kfifo_t DBG_USB_Send_kFIFO;

static CPU_CHAR    uart_data[ DBG_UART_BUF_SIZE ] ;

/*
*********************************************************************************************************
*                                         App_TaskDebugInfo()
*
* Description : Check DBG_UART_Send_Buffer[] and Send debug data if kFIFO buffer is not empty
*
* Argument(s) : p_arg       Argument passed to 'App_TaskUserIF()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void  App_TaskDebugInfo (void *p_arg)
{

    CPU_INT16U   i, size ;

    (void)p_arg;

    debug_uart_fifo_data_max = 0;
    debug_usb_fifo_data_max  = 0;
    debug_uart_fifo_oveflow_counter = 0;
    debug_usb_fifo_oveflow_counter  = 0;

    kfifo_init_static( &DBG_UART_Send_kFIFO, (unsigned char *)DBG_UART_Send_Buffer, DBG_UART_Send_Buf_Size);
    kfifo_init_static( &DBG_USB_Send_kFIFO, (unsigned char *)DBG_USB_Send_Buffer, DBG_USB_Send_Buf_Size);

    while ( DEF_TRUE ) {                   /* Task body, always written as an infinite loop.           */

        size  = kfifo_get_data_size( &DBG_UART_Send_kFIFO );
        if( size ) {
            if( size > debug_uart_fifo_data_max ) {
                debug_uart_fifo_data_max = size;
            }
            size = size < DBG_UART_BUF_SIZE ?  size  :  DBG_UART_BUF_SIZE;
            kfifo_get( &DBG_UART_Send_kFIFO, (unsigned char *)uart_data, size ) ;

            for( i = 0; i < size; i++ ){
                BSP_Ser_WrByte( uart_data[i] ) ;
            }

        } else {
            OSTimeDly(1);

        }

#ifdef DBG_USB_LOG_EN         
        if ( USBD_GetState() >= USBD_STATE_CONFIGURED ) { 
            size  = kfifo_get_data_size( &DBG_USB_Send_kFIFO );
            if( restart_log_bulk_in && size > 0 ) {
                restart_log_bulk_in = false;
                if( size > debug_usb_fifo_data_max ) {
                    debug_usb_fifo_data_max = size;
                }
                size = size < USB_LOGEP_SIZE_256B ?  size  :  USB_LOGEP_SIZE_256B;
                kfifo_get(&DBG_USB_Send_kFIFO, usbCacheBulkIn3, size);
                CDCDSerialDriver_WriteLog(       usbCacheBulkIn3,
                                                 size,
                                                 (TransferCallback) UsbLogDataTransmit,
                                                 0); 

            } else {
                OSTimeDly(1);

            }
        }
#endif

    }


}

void Init_Debug_FIFO( void )
{
    kfifo_t *pfifo;

    kfifo_init_static( &DBG_UART_Send_kFIFO, (unsigned char *)DBG_UART_Send_Buffer, DBG_UART_Send_Buf_Size);
    kfifo_init_static( &DBG_USB_Send_kFIFO, (unsigned char *)DBG_USB_Send_Buffer, DBG_USB_Send_Buf_Size);
}


void BSP_Ser_WrStr_To_Buffer( char *p_str )
{    
    CPU_INT16U len;
    CPU_INT16U temp;

    len  = strlen( p_str );
    temp = kfifo_get_free_space( &DBG_UART_Send_kFIFO );

    if( temp < len ) {
        kfifo_release(&DBG_UART_Send_kFIFO, len - temp ) ; //discard old data
        debug_uart_fifo_oveflow_counter++ ;
    }
    kfifo_put( &DBG_UART_Send_kFIFO, (unsigned char *)p_str,  len);

}

void BSP_Ser_WrStr_To_Buffer_USB( char *p_str )
{    
    CPU_INT16U len;
    CPU_INT16U temp;

    len  = strlen( p_str );
    temp = kfifo_get_free_space( &DBG_USB_Send_kFIFO );

    if( temp < len ) {
        kfifo_release(&DBG_USB_Send_kFIFO, len - temp ) ; //discard old data
        debug_usb_fifo_oveflow_counter++ ;
    }
    kfifo_put( &DBG_USB_Send_kFIFO, (unsigned char *)p_str,  len);

}




