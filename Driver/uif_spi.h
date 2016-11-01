#ifndef _UIF_SPI_H_
#define _UIF_SPI_H_

#include "spi.h"
#include "uif_object.h"
#include "defined.h"

void SPI_Handler( void );

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

#endif
static void _ConfigureSpi( DataSource *pInstance,uint32_t mode,uint32_t clk );

#if 0
static void spi_slave_transfer(void *p_tbuf,uint32_t tsize, 
                               void *p_rbuf,uint32_t rsize );
							   
static void spi_master_transfer( void *p_tbuf,uint32_t tsize,
				 void *p_rbuf,uint32_t rsize);
#endif


static void spi_slave_initialize( void );

static void spi_master_initialize( void );

//static void spi_set_clock_configuration( uint8_t configuration );
void spi_register_set( void *instance,void *parameter );
#endif
