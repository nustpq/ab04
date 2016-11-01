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

#ifndef __XMODEM_H__
#define __XMODEM_H__


#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define MAX_RETRANS           5
#define XMODEM_PACK_SIZE      128


#define XMODEM_CANCEL_ERR      0x70
#define XMODEM_SYNC_ERR        0x71
#define XMODEM_TRANS_ERR       0x72
#define XMODEM_TRANS_END_ERR   0x73


extern unsigned char Xmodem_Transmit( unsigned char *src, int srcsz );

#endif