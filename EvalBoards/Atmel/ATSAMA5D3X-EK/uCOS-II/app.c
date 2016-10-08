/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2009-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          APPLICATION CODE
*                                         ATMEL ATSAMA5D3X-EK
*
* Filename      : app.c
* Version       : V0.0.1
* Programmer(s) : Leo
* Editor        : ForteMedia SQA
*********************************************************************************************************
* Note(s)       : none.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/


#include  <app_cfg.h>
#include  <lib_mem.h>

#include  <bsp.h>
#include  <bsp_int.h>
#include  <bsp_os.h>
#include  <bsp_cache.h>


#include  <cpu.h>
#include  <cpu_core.h>

#include  <ucos_ii.h>

#include <libnandflash.h>
#include <libpmecc.h>

#include "board.h"
#include "defined.h"
#include "uif_object.h"


#include "codec.h"
#include "sine_table.h"

#include "uif_cmdparse.h"
#include "uif_i2s.h"
#include "uif_usb.h"
#include "uif_spi.h"
#include "uif_twi.h"
#include "uif_usart.h"
#include "uif_nandflash.h"
#include "uif_gpio.h"
#include "uif_led.h"
#include "uif_act8865.h"
#include "uif_dsp.h"

/*
*********************************************************************************************************
*                                        global macro switch
*********************************************************************************************************
*/
#define UIF_COMMAND      1u
#define UIF_NANDFLASH    1u
#define UIF_LED          1u
#define UIF_USB          1u
#define UIF_SSC0         1u
#define UIF_SSC1         1u
#define UIF_SPI0         1u
#define UIF_SPI1         1u
#define UIF_TWI0	 1u
#define UIF_TWI1	 1u
#define UIF_TWI2	 1u
#define UIF_USART1       1u
#define UIF_GPIO         1u


#define UIF_AIC3204      1u
#define UIF_FM36         1u

/*
*********************************************************************************************************
*                                        global object 
*********************************************************************************************************
*/
// all of data sources instance list 
DataSource source_usb;
DataSource source_ssc0;
DataSource source_ssc1;
DataSource source_spi0;
DataSource source_spi1;
DataSource source_twi0;
DataSource source_twi1;
DataSource source_twi2;
DataSource source_usart1;
DataSource source_gpio;

//task sync variable
OS_FLAG_GRP *g_pStartUSBTransfer;      //when one port data ready,notify usb port,
                                       //if all port needed sync ready,transmmit;
OS_EVENT *g_pPortManagerMbox;          //The cmd parse task used Mailbox to control
                                       //other task start or stop;               
//port bitmap
uint8_t g_portMap;

// unmask flag for port 
uint32_t g_portMaskMap;

extern Twid twid[ 3 ];
//twi descriptors for transmmit
 Async async[ MAXTWI ];
 
/*
*********************************************************************************************************
*                                        data struct for nandflash
*********************************************************************************************************
*/

extern uint8_t g_pmeccStatus;


/*
*********************************************************************************************************
*                                        Reimplement SSC with DMA
*********************************************************************************************************
*/
#define TOTAL_Buffers 2                                 //dma descriptor number

sDmad g_dmad;                                           //dma descriptor object



/*
*********************************************************************************************************
*                                        GPIO pin 
*********************************************************************************************************
*/
extern const Pin gpio_pins[ ];

/*
*********************************************************************************************************
*  all of pingpong buffer about ssc declare;                                      
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*  all of pingpong buffer about ssc declare;                                      
*
*********************************************************************************************************
*/
//Ring for ssc 
kfifo_t  ssc0_bulkout_fifo;
kfifo_t  ssc0_bulkin_fifo;
kfifo_t  ssc1_bulkout_fifo;
kfifo_t  ssc1_bulkin_fifo;

//Ring for USB data endpoint
kfifo_t  ep0BulkOut_fifo;
kfifo_t  ep0BulkIn_fifo;
kfifo_t  ep1BulkOut_fifo;
kfifo_t  ep1BulkIn_fifo;

//Ring for USB cmd endpoint
kfifo_t  cmdEpBulkOut_fifo;
kfifo_t  cmdEpBulkIn_fifo;

//Ring for spi 
kfifo_t  spi0_bulkOut_fifo;
kfifo_t  spi0_bulkIn_fifo;
kfifo_t  spi1_bulkOut_fifo;
kfifo_t  spi1_bulkIn_fifo;


/*
*********************************************************************************************************
*                                        buffer copy from uif1.0 define
*Note: Maybe should move all of these defines to a standard-alone file? that read easier;
*********************************************************************************************************
*/

//Buffer Level 1:  USB data stream buffer : 64 B
uint8_t usbCacheBulkOut0[USB_DATAEP_SIZE_64B] ;
uint8_t usbCacheBulkIn0[USB_DATAEP_SIZE_64B] ; 
uint8_t usbCacheBulkOut1[USB_DATAEP_SIZE_64B] ;
uint8_t usbCacheBulkIn1[USB_DATAEP_SIZE_64B] ; 

<<<<<<< HEAD
//Buffer Level 2:  FIFO Loop Data Buffer : 1024 B
uint8_t FIFOBufferBulkOutCmd[ USB_CMD_OUT_BUFFER_SIZE ] ;
uint8_t FIFOBufferBulkInCmd[ USB_CMD_IN_BUFFER_SIZE ]  ;

//--------------------------------spi  buffer --------------------------------//
uint8_t spi0_buffer[2][I2S_OUT_BUFFER_SIZE];
uint8_t spi1_buffer[2][I2S_OUT_BUFFER_SIZE];
uint8_t spi0_ring_buffer[I2S_OUT_BUFFER_SIZE];
uint8_t spi1_ring_buffer[I2S_OUT_BUFFER_SIZE];
=======
//Buffer Level 1:  USB Cmd data stream buffer : 64 B
uint8_t usbCmdCacheBulkOut[ USB_CMDEP_SIZE_64B ] ;             //64B
uint8_t usbCmdCacheBulkIn[ USB_CMDEP_SIZE_64B ]  ;             //64B

////Buffer Level 2:  Ring Data Buffer for usb: 16384 B
uint8_t usbRingBufferBulkOut0[ USB_RINGOUT_SIZE_16K ] ;        //16384B
uint8_t usbRingBufferBulkIn0[ USB_RINGIN_SIZE_16K ] ;          //16384B
uint8_t usbRingBufferBulkOut1[ USB_RINGOUT_SIZE_16K ] ;        //16384B
uint8_t usbRingBufferBulkIn1[ USB_RINGIN_SIZE_16K ] ;          //16384B

//Buffer Level 2:  Ring CMD Buffer : 1024 B
uint8_t usbCmdRingBulkOut[ USB_CMD_RINGOUT_SIZE_1K ] ;         //1024B
uint8_t usbCmdRingBulkIn[ USB_CMD_RINGIN_SIZE_1k ]  ;          //1024B

//Buffer Level 3:  Ring  Data Buffer for audio port include ssc and spi: 16384 B
uint8_t ssc0_RingBulkOut[ USB_RINGOUT_SIZE_16K ] ;             //16384B
uint8_t ssc0_RingBulkIn[ USB_RINGIN_SIZE_16K ] ;               //16384B
uint8_t ssc1_RingBulkOut[ USB_RINGOUT_SIZE_16K ] ;             //16384B
uint8_t ssc1_RingBulkIn[ USB_RINGIN_SIZE_16K ] ;               //16384B

uint16_t spi0_RingBulkOut[ SPI_RINGOUT_SIZE_16K ];
uint16_t spi0_RingBulkIn[ SPI_RINGIN_SIZE_16K];
uint16_t spi1_RingBulkOut[ SPI_RINGOUT_SIZE_16K ];
uint16_t spi1_RingBulkIn[ SPI_RINGIN_SIZE_16K ];

//Buffer Level 4:  PingPong buffer for audio data : MAX 48*2*8*2*2 = 3072 B
//these buffer is private 
uint16_t ssc0_PingPongOut[2][ I2S_PINGPONG_OUT_SIZE_3K ];         // Play 
uint16_t ssc0_PingPongIn[2][ I2S_PINGPONG_IN_SIZE_3K ] ;          // Record
uint16_t ssc1_PingPongOut[2][ I2S_PINGPONG_OUT_SIZE_3K ];         // Play 
uint16_t ssc1_PingPongIn[2][ I2S_PINGPONG_IN_SIZE_3K ] ;          // Record

uint16_t spi0_2MSOut[2][ I2S_PINGPONG_OUT_SIZE_3K ];
uint16_t spi0_2MSIn[2][ I2S_PINGPONG_IN_SIZE_3K ];
uint16_t spi1_2MSOut[2][ I2S_PINGPONG_OUT_SIZE_3K ];
uint16_t spi1_2MSIn[2][ I2S_PINGPONG_IN_SIZE_3K ];

// gpio has no private ring buffer, it share with ssc0;
uint16_t gpio_PingPong_bufferOut[2][I2S_PINGPONG_OUT_SIZE_3K];
uint16_t gpio_PingPong_bufferIn[2][I2S_PINGPONG_IN_SIZE_3K];

uint16_t tmpInBuffer[ I2S_PINGPONG_IN_SIZE_3K * 4];
uint16_t tmpOutBuffer[ I2S_PINGPONG_OUT_SIZE_3K *4];
>>>>>>> remotes/origin/0922


//--------------------------------twi  buffer --------------------------------//
uint8_t twi_ring_buffer[ MAXTWI ][ 256 ];

/*
*********************************************************************************************************
*                                      GLOBAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               Init_Bulk_FIFO()
*
* Description : all ring buffer initialize
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void Init_Bulk_FIFO( void )
{   
    kfifo_t *pfifo;
    
    //initialize ring buffer relavent ssc0;
    pfifo = &ssc0_bulkout_fifo;
<<<<<<< HEAD
    kfifo_init_static(pfifo, ssc0_FIFOBufferBulkOut, USB_OUT_BUFFER_SIZE);
    pfifo = &ssc0_bulkin_fifo;
    kfifo_init_static(pfifo, ssc0_FIFOBufferBulkIn, USB_IN_BUFFER_SIZE);
    
    //initialize ring buffer relavent ssc1,extend from old structure;
    pfifo = &ssc1_bulkout_fifo;
    kfifo_init_static(pfifo, ssc1_FIFOBufferBulkOut, USB_OUT_BUFFER_SIZE);
    pfifo = &ssc1_bulkin_fifo;
    kfifo_init_static(pfifo, ssc1_FIFOBufferBulkIn, USB_IN_BUFFER_SIZE);
=======
    kfifo_init_static(pfifo, ssc0_RingBulkOut, USB_RINGOUT_SIZE_16K );
    pfifo = &ssc0_bulkin_fifo;
    kfifo_init_static(pfifo, ssc0_RingBulkIn, USB_RINGIN_SIZE_16K );
    
    //initialize ring buffer relavent ssc1,extend from old structure;
    pfifo = &ssc1_bulkout_fifo;
    kfifo_init_static(pfifo, ssc1_RingBulkOut, USB_RINGOUT_SIZE_16K );
    pfifo = &ssc1_bulkin_fifo;
    kfifo_init_static(pfifo, ssc1_RingBulkIn, USB_RINGIN_SIZE_16K );

    //initialize ring buffer relavent usb cmd ep;    
    pfifo = &cmdEpBulkOut_fifo;
    kfifo_init_static(pfifo, usbCmdRingBulkOut, USB_CMD_RINGOUT_SIZE_1K );
    pfifo = &cmdEpBulkIn_fifo;
    kfifo_init_static(pfifo, usbCmdRingBulkIn, USB_CMD_RINGIN_SIZE_1k );


    //initialize ring buffer relavent spi0;
    pfifo = &spi0_bulkOut_fifo;
    kfifo_init_static(pfifo, ( uint8_t * )spi0_RingBulkOut, SPI_RINGOUT_SIZE_16K );
    pfifo = &spi0_bulkIn_fifo;
    kfifo_init_static(pfifo, ( uint8_t * )spi0_RingBulkIn, SPI_RINGIN_SIZE_16K );
>>>>>>> remotes/origin/0922
    
    //initialize ring buffer relavent spi1;
    pfifo = &spi1_bulkOut_fifo;
    kfifo_init_static(pfifo, ( uint8_t * )spi1_RingBulkOut, SPI_RINGOUT_SIZE_16K );
    pfifo = &spi1_bulkIn_fifo;
    kfifo_init_static(pfifo, ( uint8_t * )spi1_RingBulkIn, SPI_RINGIN_SIZE_16K ); 
    
    //initialize ring buffer relavent usb data ep0;
    pfifo = &ep0BulkOut_fifo;
    kfifo_init_static(pfifo, usbRingBufferBulkOut0, sizeof( usbRingBufferBulkOut0 ) );
    pfifo = &ep0BulkIn_fifo;
    kfifo_init_static(pfifo, usbRingBufferBulkIn0, sizeof( usbRingBufferBulkIn0 ) );
    
    //initialize ring buffer relavent usb data ep1;
    pfifo = &ep1BulkOut_fifo;
    kfifo_init_static(pfifo, usbRingBufferBulkOut1, sizeof( usbRingBufferBulkOut1 ) );
    pfifo = &ep1BulkIn_fifo;
    kfifo_init_static(pfifo, usbRingBufferBulkIn1, sizeof( usbRingBufferBulkIn1 ) );
} 

/*
*********************************************************************************************************
*                                               Dma_configure()
*
* Description : all ports dma configure
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
extern void ISR_HDMA( void );
extern void _SSC0_DmaRxCallback( uint8_t status, void *pArg);
extern void _SSC1_DmaRxCallback( uint8_t status, void *pArg);
extern void _SSC0_DmaTxCallback( uint8_t status, void *pArg);
extern void _SSC1_DmaTxCallback( uint8_t status, void *pArg);
extern void _SPI0_DmaRxCallback( uint8_t status, void* pArg );
extern void _SPI0_DmaTxCallback( uint8_t status, void* pArg );
extern void _SPI1_DmaRxCallback( uint8_t status, void* pArg );
extern void _SPI1_DmaTxCallback( uint8_t status, void* pArg );
extern void _USART1_DmaRxCallback( uint8_t status, void* pArg );
extern void _USART1_DmaTxCallback( uint8_t status, void* pArg );

static void Dma_configure(void)
{
    sDmad *pDmad = &g_dmad;
    uint32_t dwCfg;
    uint8_t iController;
    /* Driver initialize */
    DMAD_Initialize( pDmad, 0 );
    /* IRQ configure */
    IRQ_ConfigureIT( ID_DMAC0, 0, ISR_HDMA );
    IRQ_ConfigureIT( ID_DMAC1, 0, ISR_HDMA );
    IRQ_EnableIT(ID_DMAC0);
    IRQ_EnableIT(ID_DMAC1);

/*----------------------------------------------------------------------------*/    
    // Allocate DMA channels for SSC0 
    source_ssc0.dev.txDMAChannel = DMAD_AllocateChannel( pDmad, DMAD_TRANSFER_MEMORY, ID_SSC0);
    source_ssc0.dev.rxDMAChannel = DMAD_AllocateChannel( pDmad, ID_SSC0, DMAD_TRANSFER_MEMORY);
    if (   source_ssc0.dev.txDMAChannel == DMAD_ALLOC_FAILED || source_ssc0.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }

    // Set RX callback 
    DMAD_SetCallback(pDmad, source_ssc0.dev.rxDMAChannel,
                    (DmadTransferCallback)_SSC0_DmaRxCallback, ( void * )&source_ssc0 );
    // Set TX callback 
    DMAD_SetCallback(pDmad, source_ssc0.dev.txDMAChannel,
                    (DmadTransferCallback)_SSC0_DmaTxCallback, ( void * )&source_ssc0 );
    // Configure DMA RX channel 
    iController = (source_ssc0.dev.rxDMAChannel >> 8);
    dwCfg = 0
            | DMAC_CFG_SRC_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SSC0, DMAD_TRANSFER_RX ))
            | DMAC_CFG_SRC_H2SEL       
            | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_ssc0.dev.rxDMAChannel, dwCfg );
    // Configure DMA TX channel 
    iController = ( source_ssc0.dev.txDMAChannel >> 8);
    dwCfg = 0
            | DMAC_CFG_DST_PER(
               DMAIF_Get_ChannelNumber( iController, ID_SSC0, DMAD_TRANSFER_TX ))
            | DMAC_CFG_DST_H2SEL
            | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_ssc0.dev.txDMAChannel, dwCfg );
/*----------------------------------------------------------------------------*/    
    // Allocate DMA channels for SSC1 
    source_ssc1.dev.txDMAChannel = DMAD_AllocateChannel( pDmad, DMAD_TRANSFER_MEMORY, ID_SSC1);
    source_ssc1.dev.rxDMAChannel = DMAD_AllocateChannel( pDmad, ID_SSC1, DMAD_TRANSFER_MEMORY);
    if (   source_ssc1.dev.txDMAChannel == DMAD_ALLOC_FAILED || source_ssc1.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }

    // Set RX callback 
    DMAD_SetCallback(pDmad, source_ssc1.dev.rxDMAChannel,
                    (DmadTransferCallback)_SSC1_DmaRxCallback,( void * )&source_ssc1 );
    // Set TX callback 
    DMAD_SetCallback(pDmad, source_ssc1.dev.txDMAChannel,
                    (DmadTransferCallback)_SSC1_DmaTxCallback,( void * )&source_ssc1 );
    
    
    // Configure DMA RX channel     
    iController = (source_ssc1.dev.rxDMAChannel >> 8);
    dwCfg = 0
            | DMAC_CFG_SRC_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SSC1, DMAD_TRANSFER_RX ))
            | DMAC_CFG_SRC_H2SEL       
            | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_ssc1.dev.rxDMAChannel, dwCfg );
    
    // Configure DMA TX channel 
    iController = (source_ssc1.dev.txDMAChannel >> 8);
    dwCfg = 0
            | DMAC_CFG_DST_PER(
               DMAIF_Get_ChannelNumber( iController, ID_SSC1, DMAD_TRANSFER_TX ))
            | DMAC_CFG_DST_H2SEL
            | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_ssc1.dev.txDMAChannel, dwCfg );
/*----------------------------------------------------------------------------*/
     // Allocate DMA channels for SPI0 
    source_spi0.dev.txDMAChannel = DMAD_AllocateChannel( pDmad,
                                              DMAD_TRANSFER_MEMORY, ID_SPI0);
    source_spi0.dev.rxDMAChannel = DMAD_AllocateChannel( pDmad,
                                              ID_SPI0, DMAD_TRANSFER_MEMORY);
    if (   source_spi0.dev.txDMAChannel == DMAD_ALLOC_FAILED 
        || source_spi0.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }
    // Set RX callback 
    DMAD_SetCallback(pDmad, source_spi0.dev.rxDMAChannel,
                    (DmadTransferCallback)_SPI0_DmaRxCallback, ( void * )&source_spi0 );
    // Configure DMA RX channel 
    iController = (source_spi0.dev.rxDMAChannel >> 8);
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_RX )& 0x0F)
           | DMAC_CFG_SRC_PER_MSB(
                (DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_RX )& 0xF0) >> 4 )
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_spi0.dev.rxDMAChannel, dwCfg );
    
    // Configure DMA TX channel 
    DMAD_SetCallback(pDmad, source_spi0.dev.txDMAChannel,
                    (DmadTransferCallback)_SPI0_DmaTxCallback, ( void * )&source_spi0 );
    iController = (source_spi0.dev.txDMAChannel >> 8);
    dwCfg = 0           
           | DMAC_CFG_DST_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_TX )& 0x0F)
           | DMAC_CFG_DST_PER_MSB(
                (DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_TX )& 0xF0) >> 4 )
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_spi0.dev.txDMAChannel, dwCfg );    
/*----------------------------------------------------------------------------*/
#if 1   
     // Allocate DMA channels for SPI1 
    source_spi1.dev.txDMAChannel = DMAD_AllocateChannel( pDmad,
                                              DMAD_TRANSFER_MEMORY, ID_SPI1);
    source_spi1.dev.rxDMAChannel = DMAD_AllocateChannel( pDmad,
                                              ID_SPI1, DMAD_TRANSFER_MEMORY);
    if (   source_spi1.dev.txDMAChannel == DMAD_ALLOC_FAILED 
        || source_spi1.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }
    /* Set RX callback */
    DMAD_SetCallback(pDmad, source_spi1.dev.rxDMAChannel,
                    (DmadTransferCallback)_SPI1_DmaRxCallback, ( void * )&source_spi1 );
    /* Configure DMA RX channel */
    iController = (source_spi1.dev.rxDMAChannel >> 8);
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SPI1, DMAD_TRANSFER_RX )& 0x0F)
           | DMAC_CFG_SRC_PER_MSB(
                (DMAIF_Get_ChannelNumber( iController, ID_SPI1, DMAD_TRANSFER_RX )& 0xF0) >> 4 )
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_spi1.dev.rxDMAChannel, dwCfg );
    
    /* Configure DMA TX channel */
    DMAD_SetCallback(pDmad, source_spi1.dev.txDMAChannel,
                    (DmadTransferCallback)_SPI1_DmaTxCallback, ( void * )&source_spi1 );
    iController = (source_spi1.dev.txDMAChannel >> 8);
    dwCfg = 0           
           | DMAC_CFG_DST_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SPI1, DMAD_TRANSFER_TX )& 0x0F)
           | DMAC_CFG_DST_PER_MSB(
                (DMAIF_Get_ChannelNumber( iController, ID_SPI1, DMAD_TRANSFER_TX )& 0xF0) >> 4 )
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_spi1.dev.txDMAChannel, dwCfg );
/*----------------------------------------------------------------------------*/    
    // Allocate DMA channels for USART1 
    source_usart1.dev.txDMAChannel = DMAD_AllocateChannel( &g_dmad,
                                              DMAD_TRANSFER_MEMORY, ID_USART1);
    source_usart1.dev.rxDMAChannel = DMAD_AllocateChannel( &g_dmad,
                                              ID_USART1, DMAD_TRANSFER_MEMORY);
    if (  source_usart1.dev.txDMAChannel == DMAD_ALLOC_FAILED 
        || source_usart1.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }
    // Set RX callback 
    DMAD_SetCallback(&g_dmad, source_usart1.dev.txDMAChannel,
                    (DmadTransferCallback)_USART1_DmaTxCallback, 0);
    DMAD_SetCallback(&g_dmad, source_usart1.dev.rxDMAChannel,
                    (DmadTransferCallback)_USART1_DmaRxCallback, 0);
    // Configure DMA RX channel 
    iController = ( source_usart1.dev.rxDMAChannel >> 8 );
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
              DMAIF_Get_ChannelNumber( iController, ID_USART1, DMAD_TRANSFER_RX ))
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, source_usart1.dev.rxDMAChannel, dwCfg );
    // Configure DMA TX channel 
    iController = ( source_usart1.dev.txDMAChannel >> 8);
    dwCfg = 0 
           | DMAC_CFG_DST_PER(
              DMAIF_Get_ChannelNumber( iController, ID_USART1, DMAD_TRANSFER_TX ))
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, source_usart1.dev.txDMAChannel, dwCfg ); 
/*----------------------------------------------------------------------------*/    
#endif
}



/*
*********************************************************************************************************
*                                        GLOBAL VARIABLES FOR USB
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
#if 1
#define TASKLEDPRIORITY    ( 8u ) 
#define TASKUSBPRIORITY    ( 12u )
#define TASKSSC0PRIORITY   ( 6u )
#define CMDPARASEPRIORITY  ( 14u )
#define FIRMWAREVECUPDATE  ( 16u )
#else
#define TASKLEDPRIORITY    ( 14u )     //maybe delay
#define TASKUSBPRIORITY    ( 12u )      //2th priority
#define TASKSSC0PRIORITY   ( 8u )     //change it to dynamic task
#define CMDPARASEPRIORITY  ( 6u )      //hightest priority
#define FIRMWAREVECUPDATE  ( 16u )     //change it to dynamic task
#endif

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  CPU_STK  AppTaskLEDStk[4096u];
static  CPU_STK  AppTaskUSBStk[4096u];
static  CPU_STK  AppTaskSSC0Stk[4096u];
static  CPU_STK  AppTaskCmdParaseStk[4096u];
static  CPU_STK  AppTaskFirmwareVecUpdateStk[4096u];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskLED                (void        *p_arg);
static  void  AppTaskUSB                (void        *p_arg);
static  void  AppTaskSSC0               (void        *p_arg);
static  void  AppTaskCmdParase          (void        *p_arg);
static  void  AppTaskFirmwareVecUpdate  (void        *p_arg);
/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  uint8_t isUsbConnected = 0;
extern void _ConfigureTc1( uint32_t hz );


int main()
{
    CPU_INT08U  os_err;
    uint32_t twi_hz = 100000;

    CODEC_SETS codec_set;


    printf(("\nApplication start!\r\n"));

    CPU_Init();

    Mem_Init();
    
<<<<<<< HEAD
    Init_Bulk_FIFO(  );
=======
    Init_Bulk_FIFO( );
    
    memset( ( void * )tmpInBuffer, 0, sizeof( tmpInBuffer ) );
    memset( ( void * )tmpOutBuffer, 0, sizeof( tmpOutBuffer ) ); 
>>>>>>> remotes/origin/0922

    //Led/Buzzer initialize;
    BSP_LED_Init();
    UIF_LED_Init();
    BSP_LED_Off( 3 ); 
    BSP_BUZZER_Toggle( BUZZER_OFF );
    UIF_LED_On( LED_D3 );
    UIF_LED_Off( LED_D3 );
    UIF_LED_On( LED_D4 );
    UIF_LED_Off( LED_D4 );  
    
    //Misc switch initialize
    UIF_Misc_Init( );
//    UIF_Misc_On( HDMI_UIF_PWR_EN );
    UIF_Misc_On ( CODEC0_RST );
    UIF_Misc_On ( CODEC1_RST );
<<<<<<< HEAD
    UIF_Misc_On ( FAST_PLUS_RST );    
=======
    UIF_Misc_On ( FAST_PLUS_RST );      
>>>>>>> remotes/origin/0922
    
    //test D5
    UIF_LED_On( LED_D5 );
    UIF_LED_Off( LED_D5 );
   
    
#if UIF_USB
    //initialize usb object and it's operation 
    source_usb.init_source = init_usb;
    source_usb.buffer_read = NULL;
    source_usb.buffer_write = NULL;
    source_usb.get_direct = NULL;	

    if(NULL != source_usb.init_source )
        source_usb.init_source(NULL,NULL);
    else
    {
        APP_TRACE_INFO(("this version isn't a release version \r\n"));
	return -1;
    }
#endif
    
#if UIF_SSC0
    //initialize ssc0 object and it's operation 
    memset( ( void * )&source_ssc0, 0 , sizeof( DataSource ) );
    memset( ( void * )ssc0_PingPongOut, 0 , sizeof( ssc0_PingPongOut ) ); 
    memset( ( void * )ssc0_PingPongIn, 0 , sizeof( ssc0_PingPongIn ) );     
    source_ssc0.dev.direct = ( uint8_t )BI;
    source_ssc0.dev.identify = ID_SSC0;
    source_ssc0.dev.instanceHandle = (uint32_t)SSC0;
    source_ssc0.status[ IN ] = ( uint8_t )FREE;
    source_ssc0.status[ OUT ] = ( uint8_t )FREE;    
    source_ssc0.tx_index = 0;
    source_ssc0.rx_index = 0;
    source_ssc0.peripheralParameter = ( void * )Audio_Configure_Instance0;
    source_ssc0.warmWaterLevel = 3072;
    source_ssc0.txSize = 3072;
    source_ssc0.rxSize = 3072;     
    
    source_ssc0.init_source = init_I2S;
    source_ssc0.buffer_write = ssc0_buffer_write;
    source_ssc0.buffer_read  = ssc0_buffer_read;
    source_ssc0.peripheral_stop = stop_ssc;
    
    source_ssc0.pRingBulkOut = &ssc0_bulkout_fifo;
    source_ssc0.pRingBulkIn = &ssc0_bulkin_fifo;
    source_ssc0.pBufferOut = ( uint16_t * )ssc0_PingPongOut;
    source_ssc0.pBufferIn = ( uint16_t * )ssc0_PingPongIn;
    
    if( NULL != source_ssc0.init_source )
        source_ssc0.init_source( &source_ssc0,NULL );
#endif
    
    
#if UIF_SSC1
    //initialize ssc1 object and it's operation 
    memset( ( void * )&source_ssc1, 0 , sizeof( DataSource ) );
    memset( ( void * )ssc1_PingPongOut, 0 , sizeof( ssc1_PingPongOut ) ); 
    memset( ( void * )ssc1_PingPongIn, 0 , sizeof( ssc1_PingPongIn ) );      
    source_ssc1.dev.direct = ( uint8_t )BI;
    source_ssc1.dev.identify = ID_SSC1;
    source_ssc1.dev.instanceHandle = (uint32_t)SSC1;
    source_ssc1.status[ IN ] = ( uint8_t )FREE;
    source_ssc1.status[ OUT ] = ( uint8_t )FREE; 
    source_ssc1.tx_index = 0;
    source_ssc1.rx_index = 0;
    source_ssc1.peripheralParameter = ( void * )Audio_Configure_Instance1;
    source_ssc1.warmWaterLevel = 3072; 
    source_ssc1.txSize = 3072;
    source_ssc1.rxSize = 3072;    
    
    source_ssc1.init_source = init_I2S;
    source_ssc1.buffer_write = ssc1_buffer_write;
    source_ssc1.buffer_read  = ssc1_buffer_read;
    source_ssc0.peripheral_stop = stop_ssc;    
    
    source_ssc1.pRingBulkOut = &ssc1_bulkout_fifo;
    source_ssc1.pRingBulkIn = &ssc1_bulkin_fifo;
    source_ssc1.pBufferOut = ( uint16_t * )ssc1_PingPongOut;
    source_ssc1.pBufferIn = ( uint16_t * )ssc1_PingPongIn;    
    
    if( NULL != source_ssc1.init_source )
       source_ssc1.init_source( &source_ssc1,NULL );
#endif
    
#if UIF_SPI0
    //initialize spi0 object and it's operation
    SPI_CFG spi0_cfg;
    memset( ( void * )&source_spi0, 0 , sizeof( DataSource ) );
    memset( ( void * )&spi0_cfg, 0 , sizeof( SPI_CFG ) );
    source_spi0.dev.direct = ( uint8_t )BI;
    source_spi0.dev.identify = ID_SPI0;
    source_spi0.dev.instanceHandle = (uint32_t)SPI0;    
    source_spi0.status[ IN ] = ( uint8_t )FREE;
    source_spi0.status[ OUT ] = ( uint8_t )FREE; 
    source_spi0.tx_index = 0;
    source_spi0.rx_index = 0;
    source_spi0.privateData = spi0_RingBulkIn;
    spi0_cfg.spi_speed = 10 * 1000 * 1000;
    spi0_cfg.spi_mode = 1;
    source_spi0.warmWaterLevel = 3072; 
    source_spi0.txSize = 3072;
    source_spi0.rxSize = 3072;     
    
    source_spi0.init_source = init_spi;
    source_spi0.peripheral_stop = stop_spi;
    source_spi0.buffer_write = _spiDmaTx;
    source_spi0.buffer_read = _spiDmaRx;
    source_spi0.set_peripheral = spi_register_set;
    
    source_spi0.pRingBulkOut = &spi0_bulkOut_fifo;
    source_spi0.pRingBulkIn = &spi0_bulkIn_fifo;
    source_spi0.pBufferOut = ( uint16_t * )spi0_2MSOut;
    source_spi0.pBufferIn = ( uint16_t * )spi0_2MSIn;  
    
    if( NULL != source_spi0.init_source )
       source_spi0.init_source( &source_spi0,&spi0_cfg );
#endif      
    
#if UIF_SPI1
    //initialize spi1 object and it's operation
    SPI_CFG spi1_cfg;
    memset( ( void * )&source_spi1, 0 , sizeof( DataSource ) );
    memset( ( void * )&spi1_cfg, 0 , sizeof( SPI_CFG ) );
    source_spi1.dev.direct = ( uint8_t )BI;
    source_spi1.dev.identify = ID_SPI1;
    source_spi1.dev.instanceHandle = (uint32_t)SPI1;    
    source_spi1.status[ IN ] = ( uint8_t )FREE;
    source_spi1.status[ OUT ] = ( uint8_t )FREE; 
    source_spi1.tx_index = 0;
    source_spi1.rx_index = 0;
    source_spi1.privateData = spi1_RingBulkIn;
    source_spi1.buffer = ( uint8_t * )spi1_2MSOut;
    spi1_cfg.spi_speed = 10 * 1000 * 1000;
    spi1_cfg.spi_mode  = 1;
    source_spi1.warmWaterLevel = 3072;
    source_spi1.txSize = 3072;
    source_spi1.rxSize = 3072;     
    
    source_spi1.init_source = init_spi;
    source_spi1.peripheral_stop = stop_spi;
    source_spi1.buffer_write = _spiDmaTx;
    source_spi1.buffer_read = _spiDmaRx;
    source_spi1.set_peripheral = spi_register_set;
    
    source_spi1.pRingBulkOut = &spi1_bulkOut_fifo;
    source_spi1.pRingBulkIn = &spi1_bulkIn_fifo;
    source_spi1.pBufferOut = ( uint16_t * )spi1_2MSOut;
    source_spi1.pBufferIn = ( uint16_t * )spi1_2MSIn;      
    
    if( NULL != source_spi1.init_source )
       source_spi1.init_source( &source_spi1,&spi1_cfg );
#endif  


#if UIF_TWI0
    //initialize twi0 object and it's operation     
    TWI_CFG twi0ChipConf[ 2 ];
    memset( ( void * )&twi0ChipConf[ 0 ], 0 ,sizeof( TWI_CFG ) << 1 );
    twi0ChipConf[ 0 ].address = 0xc0 >> 1;
    twi0ChipConf[ 0 ].iaddress = 0;
    twi0ChipConf[ 0 ].isize = 0;
    twi0ChipConf[ 0 ].revers = 0;
    
    memset( ( void * )&source_twi0, 0 , sizeof( DataSource ) );
    source_twi0.dev.direct = ( uint8_t )BI;
    source_twi0.dev.identify = ID_TWI0;
    source_twi0.dev.instanceHandle = (uint32_t)TWI0;	
    source_twi0.status[ IN ] = ( uint8_t )FREE;
    source_twi0.status[ OUT ] = ( uint8_t )FREE; 
    source_twi0.tx_index = 0;
    source_twi0.rx_index = 0;
    source_twi0.privateData = &twi0ChipConf[ 0 ];

    source_twi0.init_source = twi_init_master;
    source_twi0.buffer_write = twi0_uname_write;
    source_twi0.buffer_read = twi0_uname_read;
    
    if( NULL != source_twi0.init_source )
        source_twi0.init_source( &source_twi0,&twi_hz );

#endif
    
    
#if UIF_TWI1
    //initialize twi0 object and it's operation  
    TWI_CFG twi1_chipConf[ 2 ];
    memset( ( void * )&twi1_chipConf[ 0 ], 0 ,sizeof( TWI_CFG ) << 1 );
    twi1_chipConf[ 0 ].address = 0x18;
    twi1_chipConf[ 0 ].iaddress = 0;
    twi1_chipConf[ 0 ].isize = 0;
    twi1_chipConf[ 0 ].revers = 0;
    
    twi1_chipConf[ 1 ].address = 0x18;
    twi1_chipConf[ 1 ].iaddress = 0;
    twi1_chipConf[ 1 ].isize = 0;
    twi1_chipConf[ 1 ].revers = 0;
    
    memset( ( void * )&source_twi1, 0 , sizeof( DataSource ) );
    source_twi1.dev.direct = ( uint8_t )BI;
    source_twi1.dev.identify = ID_TWI1;
    source_twi1.dev.instanceHandle = (uint32_t)TWI1;	
    source_twi1.status[ IN ] = ( uint8_t )FREE;
    source_twi1.status[ OUT ] = ( uint8_t )FREE; 
    source_twi1.tx_index = 0;
    source_twi1.rx_index = 0;
    source_twi1.privateData = &twi1_chipConf[ 0 ];

    source_twi1.init_source = twi_init_master;
    source_twi1.buffer_write = twi1_write;
    source_twi1.buffer_read = twi1_read;
    
    if( NULL != source_twi1.init_source )
        source_twi1.init_source( &source_twi1,&twi_hz );
       
#endif


#if UIF_TWI2
    //initialize twi2 object and it's operation      
    TWI_CFG twi2_chipConf[ 2 ];
    memset( ( void * )&twi2_chipConf[ 0 ], 0 ,sizeof( TWI_CFG ) << 1 );
    twi2_chipConf[ 0 ].address = 0x18;
    twi2_chipConf[ 0 ].iaddress = 0;
    twi2_chipConf[ 0 ].isize = 0;
    twi2_chipConf[ 0 ].revers = 0;
    
    twi2_chipConf[ 1 ].address = 0x18;
    twi2_chipConf[ 1 ].iaddress = 0;
    twi2_chipConf[ 1 ].isize = 0;
    twi2_chipConf[ 1 ].revers = 0;    
    
    memset( ( void * )&source_twi2, 0 , sizeof( DataSource ) );
    source_twi2.dev.direct = ( uint8_t )BI;
    source_twi2.dev.identify = ID_TWI2;
    source_twi2.dev.instanceHandle = (uint32_t)TWI2;	
    source_twi2.status[ IN ] = ( uint8_t )FREE;
    source_twi2.status[ OUT ] = ( uint8_t )FREE; 
    source_twi2.tx_index = 0;
    source_twi2.rx_index = 0;
    source_twi2.privateData = &twi2_chipConf[ 0 ];
    
    source_twi2.init_source = twi_init_master;
    source_twi2.buffer_write = twi2_write;
    source_twi2.buffer_read = twi2_read;
    
    if( NULL != source_twi2.init_source )
        source_twi2.init_source( &source_twi2,&twi_hz );

#endif 

#if UIF_USART1
        //initialize usart1 object and it's operation 
    memset( ( void * )&source_usart1, 0 , sizeof( DataSource ) );
    source_usart1.dev.direct = ( uint8_t )BI;
    source_usart1.dev.identify = ID_USART1;
    source_usart1.dev.instanceHandle = (uint32_t)USART1;
    source_usart1.status[ IN ] = ( uint8_t )FREE;
    source_usart1.status[ OUT ] = ( uint8_t )FREE; 
    source_usart1.tx_index = 0;
    source_usart1.rx_index = 0;
    
    source_usart1.init_source = usart_init;
    source_usart1.buffer_write = usart1_DmaTx;
    source_usart1.buffer_read = usart1_DmaRx;
    
    if( NULL != source_usart1.init_source )
       source_usart1.init_source( &source_usart1,NULL );
#endif 
    
#if UIF_GPIO
    uint16_t keyState[ 8 ] = {0};
    //initialize gpio object and it's operation 
    memset( ( void * )&source_gpio, 0 , sizeof( DataSource ) );
<<<<<<< HEAD
    source_gpio.dev.direct = ( uint8_t )IN;
=======
    
    memset( ( void * )&gpio_PingPong_bufferOut, 
              0 , 
              sizeof( gpio_PingPong_bufferOut ) );    
    memset( ( void * )&gpio_PingPong_bufferIn, 
              0 , 
              sizeof( gpio_PingPong_bufferIn ) ); 
    
        ///todo: this struct should be initialzed after receieved command from usb;
    // here,just for test Cpu performence;
    GPIO_REC_CFG gpio_cfg;
    gpio_cfg.mask = 0xf;
    gpio_cfg.sampleCnt = 4;
    gpio_cfg.gpioscnt = 8;
    gpio_cfg.tdmChannelCnt = 4;
    gpio_cfg.index = 0;

    source_gpio.dev.direct = ( uint8_t )BI;
>>>>>>> remotes/origin/0922
    source_gpio.dev.identify = ID_PIOD;
    source_gpio.dev.instanceHandle = (uint32_t)PIOD;
    source_gpio.status[ IN ] = ( uint8_t )FREE;
    source_gpio.status[ OUT ] = ( uint8_t )FREE;
    source_gpio.privateData = ( void * )gpio_pins;
    source_gpio.peripheralParameter = ( void * )&gpio_cfg;
    source_gpio.tx_index = 0;
    source_gpio.rx_index = 0;
    source_gpio.txSize = 3072;
    source_gpio.rxSize = 3072;
    
    source_gpio.init_source = gpio_Init;
    source_gpio.buffer_write = gpio_Pin_Set;
    source_gpio.buffer_read = gpio_Pin_Get;
    
    source_gpio.pRingBulkOut = &ep0BulkOut_fifo;
    source_gpio.pRingBulkIn = &ep0BulkIn_fifo;
    source_gpio.pBufferOut = ( uint16_t * )gpio_PingPong_bufferOut;
    source_gpio.pBufferIn = ( uint16_t * )gpio_PingPong_bufferIn;    
    
    if( NULL != source_gpio.init_source )
       source_gpio.init_source( &source_gpio,NULL );
    
//    gpio_Pin_Set( &source_gpio, &keyState,63 );
    gpio_Pin_Get( &source_gpio, ( uint8_t *)keyState,0x1f );
#endif

#ifdef UIF_AIC3204
/*----------------------------------------------------*/
<<<<<<< HEAD
    codec_set.sr = 24000;
=======
    codec_set.sr = 48000;
>>>>>>> remotes/origin/0922
    codec_set.sample_len = 16;
    codec_set.format = 1;
    codec_set.slot_num = 2;
    codec_set.m_s_sel = 0;
    codec_set.bclk_polarity = 1;
    codec_set.flag = 1;
    codec_set.delay = 0;
<<<<<<< HEAD
    Init_CODEC( &source_twi1,codec_set,CODEC1_RST );
=======
    Init_CODEC( &source_twi1,codec_set );
>>>>>>> remotes/origin/0922
    codec_set.sr = 48000;
    codec_set.sample_len = 16;
    codec_set.format = 1;
    codec_set.slot_num = 2;
    codec_set.m_s_sel = 0;
    codec_set.bclk_polarity = 1;
    codec_set.flag = 1;
    codec_set.delay = 0;
    Init_CODEC( &source_twi2,codec_set,CODEC0_RST );
/*---------------------------------------------------*/
#endif
    
#ifdef UIF_FM36
    Init_FM36_AB03( 24000, 
                        1, 
                        0, 
                        0, 
                       16,
                        0,
                       1);
#endif
    
    //config port dma
    Dma_configure( );
    
    //initialize Tc1 interval = 1ms    
    _ConfigureTc1( 1000u );  
  
    OSInit();

#if UIF_LED
    // create led flash task 
    os_err = OSTaskCreateExt( AppTaskLED,   
                              DEF_NULL,
                             &AppTaskLEDStk[4096 - 1],
                              TASKLEDPRIORITY,
                              TASKLEDPRIORITY,
                             &AppTaskLEDStk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#endif
    
#if UIF_USB   
    // create usb transfer task 
    os_err = OSTaskCreateExt( AppTaskUSB,                                 
                              DEF_NULL,
                             &AppTaskUSBStk[4096 - 1],
                              TASKUSBPRIORITY,
                              TASKUSBPRIORITY,
                             &AppTaskUSBStk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
    
#endif

#if UIF_SSC0   
    // Create the ssc0 data transfer task; 
    os_err = OSTaskCreateExt( AppTaskSSC0,                               
                              DEF_NULL,
                             &AppTaskSSC0Stk[4096 - 1],
                              TASKSSC0PRIORITY,
                              TASKSSC0PRIORITY,
                             &AppTaskSSC0Stk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#endif
    
#if UIF_COMMAND    
    // create cmd parase task 
    os_err = OSTaskCreateExt( AppTaskCmdParase,   
                              DEF_NULL,
                             &AppTaskCmdParaseStk[4096 - 1],
                              CMDPARASEPRIORITY,
                              CMDPARASEPRIORITY,
                             &AppTaskCmdParaseStk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#endif
    
#if UIF_NANDFLASH    
    // create firmware update task 
    os_err = OSTaskCreateExt( AppTaskFirmwareVecUpdate,   
                              DEF_NULL,
                             &AppTaskFirmwareVecUpdateStk[4096 - 1],
                              FIRMWAREVECUPDATE,
                              FIRMWAREVECUPDATE,
                             &AppTaskFirmwareVecUpdateStk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#endif    

    g_pStartUSBTransfer = OSFlagCreate( 0,&os_err );

    OSStart();


    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error starting. OSStart() returned with error %u\r\n", os_err));
    }


    return 0;
}


/*
*********************************************************************************************************
*                                             AppTaskLED()
*
* Description : led flash task.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if UIF_LED
static  void  AppTaskLED ( void *p_arg )
{

    BSP_OS_TmrTickInit(1000u);
    
    memset( spi1_RingBulkIn, 0x55, sizeof( spi1_RingBulkIn ));
    memset( spi1_2MSOut, 0x38, sizeof( spi1_2MSOut ));  
    memset( twi_ring_buffer[ 0 ],0x55,sizeof( twi_ring_buffer[ 0 ] ));
    memset( usartBuffer[ 0 ], 0x55 , 1024 );
    memset( usartBuffer[ 1 ], 0x55 , 1024 );
    memset( spi0_RingBulkOut, 0x55, sizeof( spi0_RingBulkOut ) );    
    
    for(;;) 
    {
<<<<<<< HEAD
        OSTimeDlyHMSM(0, 0, 1, 0);

        UIF_LED_Toggle( LED_D4 );
        
 
        spi_clear_status( &source_spi1 );
        memset( spi1_ring_buffer, 0x55, sizeof( spi1_ring_buffer ) );
        _spiDmaRx( &source_spi1 ,source_spi1.privateData,sizeof( spi1_ring_buffer ));
        _spiDmaTx( &source_spi1 ,source_spi1.privateData,sizeof( spi1_ring_buffer ));
        
        spi_clear_status( &source_spi0 );
        memset( spi0_ring_buffer, 0x55, sizeof( spi0_ring_buffer ) );
        _spiDmaRx( &source_spi0 ,source_spi0.privateData,sizeof( spi0_ring_buffer ));
        _spiDmaTx( &source_spi0 ,source_spi0.privateData,sizeof( spi0_ring_buffer ));
               
        usart1_DmaTx( &source_usart1 , NULL , 0 );
 //       twi0_uname_write( &source_twi0,twi_ring_buffer[0],sizeof( twi_ring_buffer[ 0 ] ) >> 10 );
=======
        OSTimeDlyHMSM(0, 0, 0, 10);  //change this interval about 10ms to fit fm1388
        UIF_LED_Toggle( LED_D5 );
#if 0 
        spi_clear_status( &source_spi1 );
        memset( spi1_RingBulkOut, 0x55, sizeof( spi1_RingBulkOut ) );
        _spiDmaRx( &source_spi1 ,source_spi1.privateData,sizeof( spi1_RingBulkIn ));
        _spiDmaTx( &source_spi1 ,source_spi1.privateData,sizeof( spi1_RingBulkOut ));
#endif  
        uint32_t size = kfifo_get_free_space( &spi0_bulkOut_fifo );
        if( size >= sizeof( spi1_2MSOut ) )
        {
           kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )spi1_2MSOut,
                      size );
           kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )spi1_2MSOut,
                      size );
           kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )spi1_2MSOut,
                      size );
           kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )spi1_2MSOut,
                      size );
        }           
            
        spi_clear_status( &source_spi0 );

        _spiDmaRx( &source_spi0 ,source_spi0.privateData,sizeof( spi0_RingBulkOut ));
        _spiDmaTx( &source_spi0 ,source_spi0.privateData,sizeof( spi0_RingBulkOut ));
               
        usart1_DmaTx( &source_usart1 , NULL , 0 );
//        twi0_uname_write( &source_twi0,twi_ring_buffer[0],sizeof( twi_ring_buffer[ 0 ] ) >> 10 );
//        OSTaskSuspend( OS_PRIO_SELF );
>>>>>>> remotes/origin/0922
    }

}
#endif

/*
*********************************************************************************************************
*                                             AppTaskCmdParase()
*
* Description : parse command that from pc.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if UIF_COMMAND

static  void  AppTaskCmdParase ( void *p_arg )
{
    static uint8_t taskMsg;
//    static uint8_t isUsbConnected = 0;
    
    static uint8_t transmitStart = 0;
    
    g_pPortManagerMbox = OSMboxCreate( (void * )taskMsg );

#ifdef UIF_FM36
    Init_FM36_AB03( 48000, 
                        1, 
                        0, 
                        0, 
                       16,
                        0,
                       1);
#endif    
      
    for(;;) {
        //task alive indicate
        UIF_LED_Toggle( LED_D3 );
        
        /* Device is not configured */
        if (USBD_GetState() < USBD_STATE_CONFIGURED)
        {
            if (isUsbConnected)
            {
                isUsbConnected = 0;
                TC_Stop(TC1, 0);
            }
        }
        else if (isUsbConnected == 0)
        {
            isUsbConnected = 1;
            TC_Start(TC1, 0);
        }
        
        if( isUsbConnected ) {
        
        //step1:get cmd from usb cmd endpoint;
        CDCDSerialDriver_Read_CmdEp(  usbCmdCacheBulkOut,
                                USB_CMDEP_SIZE_64B ,
                                //(TransferCallback) _UsbDataReceived,
                                0,
                                0);
        
         //step2:parse cmd and fill corresponding message via mailbox;
         Audio_State_Control( &taskMsg );
          
         //step3:post message mailbox to start other task;
#if 1         
         if( !transmitStart )
         {
            taskMsg = (SSC0_IN | SSC0_OUT | SSC1_IN | SSC1_OUT );
            OSMboxPost( g_pPortManagerMbox, (void *)taskMsg ); 
            transmitStart = 1;
            taskMsg = 0;
         }
#endif       
         //let this task running periodic per second enough;
         OSTimeDlyHMSM( 0, 0, 0, 500 );
      }
    }
}
#endif

/*
*********************************************************************************************************
*                                             AppTaskSSC0()
*
* Description : audio port data send/received task.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if UIF_SSC0
extern void Alert_Sound_Gen( uint8_t *pdata, uint32_t size, uint32_t REC_SR_Set );
extern void Alert_Sound_Gen1( uint8_t *pdata, uint32_t size, uint32_t REC_SR_Set );

static  void  AppTaskSSC0 ( void *p_arg )
{
    uint8_t err = 0;
    uint8_t receiveTaskMsg = 0; 

<<<<<<< HEAD
//    kfifo_init_static(&ssc0_bulkin_fifo,( uint8_t * )ssc0_FIFOBufferBulkIn,sizeof( ssc0_FIFOBufferBulkIn ) );
    memset( ssc0_I2SBuffersOut, 0, sizeof( ssc0_I2SBuffersOut ) );
    memset( ssc1_I2SBuffersOut, 0, sizeof( ssc1_I2SBuffersOut ) );  
    memset( ssc0_I2SBuffersIn, 0 , sizeof( ssc0_I2SBuffersIn ) );
    memset( ssc1_I2SBuffersIn, 0 , sizeof( ssc1_I2SBuffersIn ) ); 

#if 0    
    Alert_Sound_Gen( ( uint8_t * )ssc0_I2SBuffersOut, 
                      sizeof( ssc0_I2SBuffersOut[ 0 ] ),  
                      48000 );
    
    Alert_Sound_Gen( ( uint8_t * )ssc0_I2SBuffersOut[1], 
                      sizeof( ssc0_I2SBuffersOut[ 1 ] ),  
                      48000 );    
    
    Alert_Sound_Gen1( ( uint8_t * )ssc1_I2SBuffersOut, 
                       sizeof( ssc1_I2SBuffersOut[ 0 ] ),  
                       48000 );
    Alert_Sound_Gen1( ( uint8_t * )ssc1_I2SBuffersOut, 
                       sizeof( ssc1_I2SBuffersOut[ 1 ] ),  
                       48000 );
#endif
    
#if 0
    uint32_t i ;
    uint16_t *pInt = NULL;    
    pInt = ( uint16_t * )ssc0_I2SBuffersOut[0] ;
    for( i = 0; i< ( sizeof( ssc1_I2SBuffersOut ) );  ) 
=======
    
    memset( ssc0_PingPongOut, 0x5555, sizeof( ssc0_PingPongOut ) );
    memset( ssc1_PingPongOut, 0x5555, sizeof( ssc1_PingPongOut ) );  
    memset( ssc0_PingPongIn, 0 , sizeof( ssc0_PingPongIn ) );
    memset( ssc1_PingPongIn, 0 , sizeof( ssc1_PingPongIn ) ); 

#if 1    
    Alert_Sound_Gen( ( uint8_t * )ssc0_PingPongOut, 
                      sizeof( ssc0_PingPongOut[ 0 ] ),  
                      8000 );
    
    Alert_Sound_Gen( ( uint8_t * )ssc0_PingPongOut[1], 
                      sizeof( ssc0_PingPongOut[ 1 ] ),  
                      8000 );    
    
    Alert_Sound_Gen1( ( uint8_t * )ssc1_PingPongOut, 
                       sizeof( ssc1_PingPongOut[ 0 ] ),  
                       8000 );
    Alert_Sound_Gen1( ( uint8_t * )ssc1_PingPongOut, 
                       sizeof( ssc1_PingPongOut[ 1 ] ),  
                       8000 );
#endif
    
#if 0
    uint16_t *pInt = NULL;
    uint32_t i ;
    pInt = ( uint16_t * )ssc0_PingPongOut[0] ;
    for( i = 0; i< ( sizeof( ssc1_PingPongOut ) );  ) 
>>>>>>> remotes/origin/0922
    {        
       *(pInt+i++) = 0x1122 ;      
       *(pInt+i++) = 0x3344 ;
       *(pInt+i++) = 0x5566 ;
       *(pInt+i++) = 0x7788 ;     
       *(pInt+i++) = 0x99aa ;
       *(pInt+i++) = 0xbbcc ;   
       *(pInt+i++) = 0xddee ;
       *(pInt+i++) = 0xff00 ; 
    } 
    
    pInt = ( uint16_t * )ssc1_PingPongOut[0] ;
    for( i = 0; i< ( sizeof( ssc1_PingPongOut ) ); ) 
    {        
       *(pInt+i++) = 0x1122 ;      
       *(pInt+i++) = 0x3344 ;
       *(pInt+i++) = 0x5566 ;
       *(pInt+i++) = 0x7788 ;     
       *(pInt+i++) = 0x99aa ;
       *(pInt+i++) = 0xbbcc ;   
       *(pInt+i++) = 0xddee ;
       *(pInt+i++) = 0xff00 ; 
    }  
#endif    
    
    for(;;) 
    {
          receiveTaskMsg = ( uint32_t )OSMboxPend( g_pPortManagerMbox, 
                                                0, 
                                                &err);

          switch( receiveTaskMsg )
          {
            case ( SSC0_IN | SSC0_OUT | SSC1_IN | SSC1_OUT ):
                    if( ( ( uint8_t )START != source_ssc0.status[ IN ] )  
                        &&( ( uint8_t )BUFFERED != source_ssc0.status[ IN ] ) 
                        &&( ( uint8_t )RUNNING != source_ssc0.status[ IN ] ) )
                    {
//                          OSSchedLock( );
<<<<<<< HEAD
                        if( isUsbConnected )
                        {
                          source_ssc0.buffer_read( &source_ssc0,( uint8_t * )ssc0_I2SBuffersIn,
                                                   sizeof(ssc0_I2SBuffersIn ) >> 1 ); 
 
                          source_ssc0.buffer_write( &source_ssc0,( uint8_t * )ssc0_I2SBuffersOut,
                                                   sizeof(ssc0_I2SBuffersOut ) >> 1 );
                         
                          source_ssc0.status = ( uint8_t )START;
                    
                          source_ssc1.buffer_write( &source_ssc1,( uint8_t * )ssc1_I2SBuffersOut,
                                                   sizeof(ssc1_I2SBuffersOut ) >> 1 );
                          source_ssc1.buffer_read( &source_ssc1,( uint8_t * )ssc1_I2SBuffersIn,
                                                   sizeof(ssc1_I2SBuffersIn ) >> 1 );                            
                          source_ssc1.status = ( uint8_t )START;
                        }
=======
                          source_ssc0.buffer_write( &source_ssc0,( uint8_t * )ssc0_PingPongOut,
                                                   sizeof( ssc0_PingPongOut ) >> 1 );
                          source_ssc0.buffer_read( &source_ssc0,( uint8_t * )ssc0_PingPongIn,
                                                   sizeof( ssc0_PingPongIn ) );                          
                          source_ssc0.status[ IN ] = ( uint8_t )START;
                          source_ssc0.status[ OUT ] = ( uint8_t )START;                          
                    
                          source_ssc1.buffer_write( &source_ssc1,( uint8_t * )ssc1_PingPongOut,
                                                   sizeof( ssc1_PingPongOut ) >> 1 );
                          source_ssc1.buffer_read( &source_ssc1,( uint8_t * )ssc1_PingPongIn,
                                                   sizeof( ssc1_PingPongIn ) >> 1 );                            
                          source_ssc1.status[ IN ] = ( uint8_t )START;
                          source_ssc1.status[ OUT ] = ( uint8_t )START;  
>>>>>>> remotes/origin/0922
//                          OSSchedUnlock( );
//                          OSTaskSuspend( OS_PRIO_SELF );
                    }                 
            break;
            default:
            break;
          }

          OSTimeDlyHMSM(0, 0, 0, 10);  
    }

}
#endif
/*
*********************************************************************************************************
*                                             AppTaskUSB()
*
* Description : usb transfer task.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#ifdef UIF_USB
static  void  AppTaskUSB (void *p_arg)
{
    uint8_t err = 0;
    uint8_t evFlags = 0; 

    memset( ( uint8_t * )tmpInBuffer , 0x32 , sizeof( tmpInBuffer ) ); 
    for(;;) 
    {
<<<<<<< HEAD
//            /* Device is not configured */
//        if (USBD_GetState() < USBD_STATE_CONFIGURED)
//        {
//            if (isUsbConnected)
//            {
//                isUsbConnected = 0;
//            }
//        }
//        else if (isUsbConnected == 0)
//        {
//            isUsbConnected = 1;
//        }      

#if 0               
=======
        UIF_LED_Toggle( LED_D4 );
#if 1               
>>>>>>> remotes/origin/0922
        evFlags = OSFlagQuery( g_pStartUSBTransfer, &err );

        //maybe transfer according events,not waiting for all event happend;
        //
 
/**-------------------proccess up link ----------------------------------------          
            //copy data from ssc0 in ring buffer to usb in ring buffer;
            // ssc0 ring -----> tmpInbuffer
            sizecnt = kfifo_get_data_size( &ssc0_bulkin_fifo );
            kfifo_get( &ssc0_bulkin_fifo,
                      ( uint8_t * )tmpInBuffer,
                       6144 );
//            memset( ( void * )tmpInBuffer, 49, sizeof( tmpInBuffer ) );            
            // tmpInbuffer -----> ep0 ring
            kfifo_put( &ep0BulkIn_fifo,
                      ( uint8_t * )tmpInBuffer,
                       6144 ); 
//============================================================================//            
            //copy data from ssc1 in ring buffer to usb in ring buffer;
            // ssc1 ring -----> tmpInbuffer 
            sizecnt2 = kfifo_get_data_size( &ssc1_bulkin_fifo );
            kfifo_get( &ssc1_bulkin_fifo,
                      ( uint8_t * )tmpInBuffer,
                       6144 );
         
            // tmpInbuffer -----> ep1 ring            
            kfifo_put( &ep1BulkIn_fifo,
                      ( uint8_t * )tmpInBuffer,
                       6144 );
            
            memset( usbRingBufferBulkIn0, 0x33 , sizeof( usbRingBufferBulkIn0 ) );
            // ep0 ring --> usb cache
            kfifo_get( &ep0BulkIn_fifo,
                      ( uint8_t * )usbCacheBulkIn0,
                       USB_DATAEP_SIZE_64B );            
         
            // send ep0 data ---> pc
            CDCDSerialDriver_Write( usbCacheBulkIn0,
                                    6144,
                                    (TransferCallback)UsbAudio0DataTransmit,
                                    0);
            
            memset( usbRingBufferBulkIn1, 0x34 , sizeof( usbRingBufferBulkIn0 ) );
            // ep1 ring --> usb cache            
            kfifo_get( &ep1BulkIn_fifo,
                      ( uint8_t * )usbCacheBulkIn1,
                       USB_DATAEP_SIZE_64B );
            // send ep1 data ---> pc
            CDCDSerialDriver_Write_SecondEp( usbCacheBulkIn1,
                                   6144,
                                  (TransferCallback)UsbAudio1DataTransmit,
                                   0); 
           

---------------------proccess down link ------------------------------------**/
            // usb cache --> ep0 ring 
#if 1   
            CDCDSerialDriver_Read( usbCacheBulkOut0,
                                   USB_DATAEP_SIZE_64B,
                                   (TransferCallback) UsbAudio0DataReceived,
                                    0); 


#if 1  
            //copy data from  usb ring buffer to temp buffer ;
            //ep0 ring ---> tmpOutbuffer;
            if( source_ssc0.txSize == kfifo_get( &ep0BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize ) ) 
            //tmpOutBuffer --> ssc0 ring
            kfifo_put( &ssc0_bulkout_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize );
        
#endif            
            //copy data from  usb ring buffer to spi0 in ring buffer ;
            //ep0 ring ---> tmpOutBuffer;
#if 1            
            kfifo_get( &ep0BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize  ); 
            //tmpOutBuffer ---> spi0 ring
            kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize  );             
#endif
//========================================================================
//instead ep1 when linux scripts can't use ep1   
#if 1  
            //copy data from  usb ring buffer to temp buffer ;
            //ep0 ring ---> tmpOutbuffer;
            if( source_ssc0.txSize == kfifo_get( &ep0BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize ) ) 
            //tmpOutBuffer --> ssc0 ring
            kfifo_put( &ssc0_bulkout_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize );
        
#endif            
            //copy data from  usb ring buffer to spi0 in ring buffer ;
            //ep0 ring ---> tmpOutBuffer;
#if 1            
            kfifo_get( &ep0BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize  ); 
            //tmpOutBuffer ---> spi0 ring
            kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize  );             
#endif            
//========================================================================            
#if 0            
            CDCDSerialDriver_Read_SecondEp( usbCacheBulkOut1,
                                            USB_DATAEP_SIZE_64B,
                                            (TransferCallback) UsbAudio1DataReceived,
                                            0); 
           
           //copy data from  usb ring buffer to ssc1 in ring buffer ; 
            //ep1 ring ---> tmpOutbuffer;            
            if( source_ssc1.txSize == kfifo_get( &ep1BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                        source_ssc1.txSize ) )
            //tmpOutBuffer --> ssc1 ring            
            kfifo_put( &ssc1_bulkout_fifo,
                      ( uint8_t * )tmpOutBuffer,
                        source_ssc1.txSize );             
#endif
 /*  
           // spi1 connect to fpga in AB04,so it does not participate in data transmission
            
           //copy data from  usb ring buffer to spi1 in ring buffer ;
            //ep1 ring ---> tmpOutbuffer; 
        
            kfifo_get( &ep1BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       sizeof( source_spi1.txSize )  << 1); 
            //tmpOutBuffer --> spi1 ring                 
            kfifo_put( &spi1_bulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       sizeof( source_spi1.txSize ) << 1 );  
*/            
#endif        

#endif

       OSTimeDlyHMSM( 0,0,0,1 );
    }
      
}
#endif

/*
*********************************************************************************************************
*                                             AppTaskFirmwareVecUpdate()
*
* Description : firmware update and vec store task.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if UIF_NANDFLASH

static  void  AppTaskFirmwareVecUpdate  ( void        *p_arg )
{
#define UPDATE_FIRMWARE  1
#define BACKUP_FIRMWARE  2
#define RESTORE_FIRMWARE 3 
#define STORE_VEC        4
#define READ_VEC         5
#define WRITE_SYSINFO    6
#define READ_SYSINFO     7
  
    uint8_t type = 0;
    uint8_t ret = 0;
    
    //step1:initialize hardware about nandflash
    nfc_init( NULL );
    pmecc_init( &g_pmeccStatus );
  
    //step2:initialize memory for firmware or vec
    memset( nand_pageBuffer, 0 , sizeof( nand_pageBuffer ) );
    memset( nand_patternBuf, 0 , sizeof( nand_patternBuf ) );
    
    //step3: infinite cycle of task;
    for( ; ; )
    {
        //received control command from protocol parse task;
        //type = 
//        OSTaskSuspend( OS_PRIO_SELF );
        switch( type )
        {
          case UPDATE_FIRMWARE:
            //1.stop all port
            //2.write firmware to backup region
            ret = uif_write_backupfirmware( nand_pageBuffer ,sizeof( nand_pageBuffer ) );
            //3.read back to compare
            if( 0 == ret )
            {
              uif_read_backupfirmware( nand_patternBuf,sizeof( nand_patternBuf ) );
            }            
            //4.write to main firmware region            
            if( ( 0 == ret )
              &&( 0 == memcmp( nand_pageBuffer,nand_patternBuf,sizeof( nand_pageBuffer ) ) ) ) 
            {
                ret = uif_write_mainfirmware( nand_pageBuffer ,sizeof( nand_pageBuffer ) );
            }
            //5.reset device
            type = 0;
          break;
          case BACKUP_FIRMWARE:
            //1.stop all port
            //2.read main firmware to backup buffer
            memset( nand_pageBuffer, 0 , sizeof( nand_pageBuffer ) ); 
            ret  = uif_read_mainfirmware( nand_pageBuffer ,sizeof( nand_pageBuffer ) );

            //3.write to backup firmware region
            if( 0 == ret )
            {
                ret  = uif_read_backupfirmware( nand_pageBuffer ,sizeof( nand_pageBuffer ) );
            }
            //4.reset device
          break;          
          case RESTORE_FIRMWARE:
            //1.stop all port
            //2.read back backup region to buffer
            memset( nand_pageBuffer, 0 , sizeof( nand_pageBuffer ) ); 
            ret  = uif_read_backupfirmware( nand_pageBuffer ,sizeof( nand_pageBuffer ) );            
            //3.write to main firmware
            if( 0 == ret )
            {
                 ret  = uif_write_mainfirmware( nand_pageBuffer ,sizeof( nand_pageBuffer ) ); 
            }
            else
            {
              //process error;
            }
            //4.reset device
          break;
          case STORE_VEC:
            //1.stop all port
            //2.initialize buffer
            memset( nand_pageBuffer, 0 , sizeof( nand_pageBuffer ) );
            //3.copy data from usb buffer to vec buffer
            //4.write vec to vec region
            ret  = uif_write_vec( nand_pageBuffer ,sizeof( nand_pageBuffer ) );
            //5.proccess error
            if( 0 != ret )
                  APP_TRACE_INFO(("Store VEC failed!\r\n"));
              
          break;
          case READ_VEC:
            //1.stop all port
            //2.read vec region to buffer
             ret  = uif_read_vec( nand_pageBuffer ,sizeof( nand_pageBuffer ) );
            //3.proccess error
             if( 0 != ret )
                  APP_TRACE_INFO(("Read VEC failed!\r\n"));  
             //4.copy data to target if needed
          break;
          case WRITE_SYSINFO:
            {
              SYSINFO info;
              memset( ( void * )&info, 0 , sizeof( SYSINFO ) );
              sprintf( ( char * )info.date,"2016-07-28" );
              sprintf( ( char * )info.firmware_version , "AB04-f-0704-0.01" );
              sprintf( ( char * )info.adaptor_soft_version , "Tuner-0704-1.0.0" );
              sprintf( ( char * )info.hardware_version , "AB04-h-0801-0.0.1" );
              uif_write_sysinfo( ( void * )&info,sizeof( SYSINFO ) );
            }
          break;
          case READ_SYSINFO:
            {
              SYSINFO info;
              memset( ( void * )&info, 0 , sizeof( SYSINFO ) );
              uif_read_sysinfo( ( void * )&info,sizeof( SYSINFO ) ); 
            }
          break;
         default:
           //do nothing;
          break;
        }
      
    }  
    
}
#endif

