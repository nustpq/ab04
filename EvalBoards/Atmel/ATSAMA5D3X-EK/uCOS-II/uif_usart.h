/*
*********************************************************************************************************
*
*                                              APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                on the
*                                      Unified EVM Interface Board
*
* Filename          :  uif_usart.h
* Version           :  V0.0.1
* Programmer(s)     :  Leo
*********************************************************************************************************
* Note(s)           : usart1 communicate header file
*********************************************************************************************************
*/

#ifndef _UIF_USART_H_
#define _UIF_USART_H_

#include "defined.h"
#include "dmac.h"

extern uint8_t usartBuffer[ 2 ][ 1024 ];

uint8_t usart1_DmaRx( void *pInstance, const uint8_t *buf, uint32_t len );
uint8_t usart1_DmaTx( void *pInstance, const uint8_t *buf, uint32_t len );
void usart_init( void *pInstance, void *parameter );

static void _ConfigureDma( void* pInstance );
static void _configureUsart( void *pInstance );

#endif
