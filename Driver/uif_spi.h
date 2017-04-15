/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
*
* Filename      : uif_spi.h
* Version       : V0.0.2
* Programmer(s) : Leo
* Modifie       : Leo
*********************************************************************************************************
* History(s)    : reconstruct this code for FM1388
*
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#ifndef _UIF_SPI_H_
#define _UIF_SPI_H_

#include "spi.h"
#include "uif_object.h"
#include "defined.h"


/** An unspecified error has occured.*/
#define SPID_ERROR          1

/** SPI driver is currently in use.*/
#define SPID_ERROR_LOCK     2

/** SPI clock frequency used in Hz. */
#define SPCK            1000000

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
void SPI_Handler( void );
extern sDmad g_dmad;
/*--------------public interface ----------------*/
uint32_t register_spi( uint32_t mask, UIFPORT *type );
Spi * _get_spi_instance( uint32_t id );
void start_spi( void * pInstance );
void stop_spi( void * pInstance );
void spi_clear_status( void * pInstance );

void init_spi(void *pInstance,void *parameter );

/*---------------private interface---------------*/
#ifdef USE_DMA
//static void ISR_SPI_DMA( void );
uint8_t _spiDmaRx( void *pInstance, const uint8_t *buf,uint32_t len  );
uint8_t _spiDmaTx( void *pInstance, const uint8_t *buf,uint32_t len  );
//static void _spi_ConfigureDma( void *pInstance );
void _SPI0_DmaRxCallback( uint8_t status, void* pArg );
void _SPI0_DmaTxCallback( uint8_t status, void* pArg );
void _SPI1_DmaRxCallback( uint8_t status, void* pArg );
void _SPI1_DmaTxCallback( uint8_t status, void* pArg );
#endif
static void _ConfigureSpi( DataSource *pInstance,uint32_t mode,uint32_t clk, uint32_t format );


#endif
