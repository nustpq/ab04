/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Unified EVM Interface Board 2.0
*
* Filename      : i2s.h
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#ifndef _I2S_H_
#define _I2S_H_

#include "board.h"
#include "chip.h"
#include "defined.h"
#include "kfifo.h"
#include "ssc.h"
#include "uif_spi.h"


#define PIN_SSC0      PINS_SSC_CODEC
#define PIN_SSC1      PINS_SSC_HDMI       //need redefine according hardware

#define DMA_CTRA_MODE_8BIT( size )     ( DMAC_CTRLA_SRC_WIDTH_BYTE       \
                                        | DMAC_CTRLA_DST_WIDTH_BYTE      \
                                        | DMAC_CTRLA_BTSIZE( size  ) ); 

#define DMA_CTRA_MODE_16BIT( size )     ( DMAC_CTRLA_SRC_WIDTH_HALF_WORD \
                                        | DMAC_CTRLA_DST_WIDTH_HALF_WORD \
                                        | DMAC_CTRLA_BTSIZE( size >> 1 ) );                             

#define DMA_CTRA_MODE_32BIT( size )     ( DMAC_CTRLA_SRC_WIDTH_WORD \
                                        | DMAC_CTRLA_SRC_WIDTH_WORD \
                                        | DMAC_CTRLA_BTSIZE( size >> 2 ) );  



extern sDmad g_dmad;

extern kfifo_t  ssc0_bulkout_fifo;
extern kfifo_t  ssc0_bulkin_fifo;
extern kfifo_t  ssc1_bulkout_fifo;
extern kfifo_t  ssc1_bulkin_fifo;
extern kfifo_t  bulkout_fifo_cmd;
extern kfifo_t  bulkin_fifo_cmd;

#ifdef USE_DMA
/* SSC devices dma descriptors */
extern sDmaTransferDescriptor dmaTdSSC0Rx[2];
extern sDmaTransferDescriptor dmaTdSSC0Tx[2];

extern sDmaTransferDescriptor dmaTdSSC1Rx[2];
extern sDmaTransferDescriptor dmaTdSSC1Tx[2];
#endif


#ifdef USE_DMA
void _SSC0_DmaRxCallback( uint8_t status, void *pArg);
void _SSC0_DmaTxCallback( uint8_t status, void *pArg);
#endif

/*------------------public interface -------------*/
uint32_t register_ssc( uint32_t mask,UIFPORT *type );
uint32_t read_ssc( void );
Ssc *_get_ssc_instance( uint32_t id );
void stop_ssc( void *instance );
void init_I2S( void *pParameter,void *dParameter );

/*-----------------private interface -------------*/
#ifdef USE_DMA
void ISR_HDMA( void );
#endif

static void _set_perpheral_instance( DataSource *pSource );
static void _config_pins( uint32_t id );

#ifdef USE_DMA
uint8_t ssc0_buffer_read( void *pInstance,const uint8_t *buf,uint32_t len );
uint8_t ssc0_buffer_write( void *pInstance,const uint8_t *buf,uint32_t len );
uint8_t ssc1_buffer_read( void *pInstance,const uint8_t *buf,uint32_t len );
uint8_t ssc1_buffer_write( void *pInstance,const uint8_t *buf,uint32_t len );
#if UNUSED  //these interface will be instead with xx_buffer_xx
void SSC0_Playing( void *pInstance );                        
void SSC1_Playing( void *pInstance );
void SSC0_Recording( void *pInstance );
void SSC1_Recording( void *pInstance );
#endif
void _ssc_ConfigureDma( void* instance );
#endif

void ssc_rxRegister_set( void *instance,void *parameter );
void ssc_txRegister_set( void *instance,void *parameter );

static void _SSC_Init( uint32_t id,
		uint32_t slave,
                unsigned int bitrate,
		uint32_t mclk, 
		uint8_t slot_num, 
		uint8_t slot_len );
static void _init_I2S( void *pInstance,void *dParameter );


#endif //

