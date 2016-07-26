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
* Version       : V1.00
* Programmer(s) : LEO
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
#include "object.h"

#include "cmdparse.h"
#include "i2s.h"
#include "usb.h"
#include "uif_spi.h"
#include "uif_twi.h"
#include "uif_usart.h"
#include "uif_nandflash.h"

/*
*********************************************************************************************************
*                                        global object 
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

//task sync variable
OS_FLAG_GRP *g_pStartUSBTransfer;      //when one port data ready,notify usb port,
                                       //if all port needed sync ready,transmmit;
OS_EVENT *g_pPortManagerMbox;          //The cmd parse task used Mailbox to control
                                       //other task start or stop;               
//port bitmap
uint8_t g_portMap;

// unmask flag for port 
uint32_t g_portMaskMap;

//twi descriptors for transmmit
 Async async;
 
/*
*********************************************************************************************************
*                                        data struct for nandflash
*********************************************************************************************************
*/

extern uint8_t g_pmeccStatus;
#if 0 
#define BASIC_PAGESIZE 2048
#define SPARE_SIZE      64
#define PAGESIZE        ( BASIC_PAGESIZE + SPARE_SIZE )
#define BLOCKSIZE       ( 64 * PAGESIZE )
#define PLANESIZE       ( 1024 * BLOCKSIZE )
#endif


/*
*********************************************************************************************************
*                                        Reimplement SSC with DMA
*********************************************************************************************************
*/
#define TOTAL_Buffers 2                                 //dma descriptor number

sDmad g_dmad;                                           //dma descriptor object


/*
*********************************************************************************************************
*  all of pingpong buffer about ssc declare;                                      
*
*********************************************************************************************************
*/
uint16_t RxBuffers[2][PINGPONG_SIZE];
uint16_t TxBuffers[2][PINGPONG_SIZE];
uint16_t RxBuffers1[2][PINGPONG_SIZE];
uint16_t TxBuffers1[2][PINGPONG_SIZE];

/*
*********************************************************************************************************
*  all of pingpong buffer about ssc declare;                                      
*
*********************************************************************************************************
*/
//global ring buffers handle
kfifo_t  ssc0_bulkout_fifo;
kfifo_t  ssc0_bulkin_fifo;
kfifo_t  ssc1_bulkout_fifo;
kfifo_t  ssc1_bulkin_fifo;
kfifo_t  bulkout_fifo_cmd;
kfifo_t  bulkin_fifo_cmd;

/*
*********************************************************************************************************
*                                        buffer copy from uif1.0 define
*Note: Maybe should move all of these defines to a standard-alone file? that read easier;
*********************************************************************************************************
*/

//Buffer Level 1:  USB data stream buffer : 64 B
uint8_t usbBufferBulkOut[USBDATAEPSIZE] ;
uint8_t usbBufferBulkIn[USBDATAEPSIZE] ; 

//Buffer Level 2:  FIFO Loop Data Buffer : 16384 B
uint8_t ssc0_FIFOBufferBulkOut[USB_OUT_BUFFER_SIZE] ;
uint8_t ssc0_FIFOBufferBulkIn[USB_IN_BUFFER_SIZE] ;
uint8_t ssc1_FIFOBufferBulkOut[USB_OUT_BUFFER_SIZE] ;
uint8_t ssc1_FIFOBufferBulkIn[USB_IN_BUFFER_SIZE] ;

//Buffer Level 3:  Double-buffer for I2S data : MAX 48*2*8*2*2 = 3072 B
uint16_t ssc0_I2SBuffersOut[2][I2S_OUT_BUFFER_SIZE];  // Play 
uint16_t ssc0_I2SBuffersIn[2][I2S_IN_BUFFER_SIZE] ;   // Record
uint16_t ssc1_I2SBuffersOut[2][I2S_OUT_BUFFER_SIZE];  // Play 
uint16_t ssc1_I2SBuffersIn[2][I2S_IN_BUFFER_SIZE] ;   // Record

//------------------------usb cmd buffer copy from uif1.0 defined-------------// 
//Buffer Level 1:  USB Cmd data stream buffer : 64 B
uint8_t usbCmdBufferBulkOut[ USBCMDDATAEPSIZE ] ;
uint8_t usbCmdBufferBulkIn[ USBCMDDATAEPSIZE ]  ;

//Buffer Level 2:  FIFO Loop Data Buffer : 1024 B
uint8_t FIFOBufferBulkOutCmd[ USB_CMD_OUT_BUFFER_SIZE ] ;
uint8_t FIFOBufferBulkInCmd[ USB_CMD_IN_BUFFER_SIZE ]  ;

//--------------------------------spi  buffer --------------------------------//
uint8_t spi0_buffer[2][2048];
uint8_t spi1_buffer[2][2048];
uint8_t spi0_ring_buffer[3072];
uint8_t spi1_ring_buffer[3072];


//--------------------------------twi  buffer --------------------------------//
uint8_t twi_ring_buffer[3][256];

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
    kfifo_init_static(pfifo, ssc0_FIFOBufferBulkOut, USB_OUT_BUFFER_SIZE);
    pfifo = &ssc0_bulkout_fifo;
    kfifo_init_static(pfifo, ssc0_FIFOBufferBulkIn, USB_IN_BUFFER_SIZE);
    
    //initialize ring buffer relavent ssc1,extend from old structure;
    pfifo = &ssc1_bulkout_fifo;
    kfifo_init_static(pfifo, ssc1_FIFOBufferBulkOut, USB_OUT_BUFFER_SIZE);
    pfifo = &ssc1_bulkout_fifo;
    kfifo_init_static(pfifo, ssc1_FIFOBufferBulkIn, USB_IN_BUFFER_SIZE);
    
    pfifo = &bulkout_fifo_cmd;
    kfifo_init_static(pfifo, FIFOBufferBulkOutCmd, USB_CMD_OUT_BUFFER_SIZE);
    pfifo = &bulkin_fifo_cmd;
    kfifo_init_static(pfifo, FIFOBufferBulkInCmd, USB_CMD_IN_BUFFER_SIZE);

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
    
    /* Allocate DMA channels for SSC0 */
    source_ssc0.dev.txDMAChannel = DMAD_AllocateChannel( pDmad, DMAD_TRANSFER_MEMORY, ID_SSC0);
    source_ssc0.dev.rxDMAChannel = DMAD_AllocateChannel( pDmad, ID_SSC0, DMAD_TRANSFER_MEMORY);
    if (   source_ssc0.dev.txDMAChannel == DMAD_ALLOC_FAILED || source_ssc0.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }

    /* Set RX callback */
    DMAD_SetCallback(pDmad, source_ssc0.dev.rxDMAChannel,
                    (DmadTransferCallback)_SSC0_DmaRxCallback, ( void * )&source_ssc0 );
    /* Set TX callback */
    DMAD_SetCallback(pDmad, source_ssc0.dev.txDMAChannel,
                    (DmadTransferCallback)_SSC0_DmaTxCallback, ( void * )&source_ssc0 );
    /* Configure DMA RX channel */
    iController = (source_ssc0.dev.rxDMAChannel >> 8);
    dwCfg = 0
            | DMAC_CFG_SRC_PER(
                DMAIF_Get_ChannelNumber( iController, ID_SSC0, DMAD_TRANSFER_RX ))
            | DMAC_CFG_SRC_H2SEL       
            | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_ssc0.dev.rxDMAChannel, dwCfg );
    /* Configure DMA TX channel */
    iController = ( source_ssc0.dev.txDMAChannel >> 8);
    dwCfg = 0
            | DMAC_CFG_DST_PER(
               DMAIF_Get_ChannelNumber( iController, ID_SSC0, DMAD_TRANSFER_TX ))
            | DMAC_CFG_DST_H2SEL
            | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_ssc0.dev.txDMAChannel, dwCfg );
    
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

#if 1   
     /* Allocate DMA channels for SPI1 */
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
                    (DmadTransferCallback)_SPI1_DmaRxCallback, 0);
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
                    (DmadTransferCallback)_SPI1_DmaTxCallback, 0);
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
    
#if 0   
    /* Allocate DMA channels for USART1 */
    source_usart1.dev.txDMAChannel = DMAD_AllocateChannel( pDmad,
                                              DMAD_TRANSFER_MEMORY, ID_USART1);
    source_usart1.dev.rxDMAChannel = DMAD_AllocateChannel( pDmad,
                                              ID_USART1, DMAD_TRANSFER_MEMORY);
    if (   source_usart1.dev.txDMAChannel == DMAD_ALLOC_FAILED 
        || source_usart1.dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }
    /* Set RX callback */
    DMAD_SetCallback( pDmad, source_usart1.dev.rxDMAChannel, (DmadTransferCallback)_USART1_DmaRxCallback, 0);
    /* Configure DMA RX channel */
    iController = (source_usart1.dev.rxDMAChannel >> 8);
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
             DMAIF_Get_ChannelNumber( iController, ID_USART1, DMAD_TRANSFER_RX ))
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_usart1.dev.rxDMAChannel, dwCfg );
    
    /* Set TX callback */
    DMAD_SetCallback( pDmad, source_usart1.dev.txDMAChannel, (DmadTransferCallback)_USART1_DmaTxCallback, 0);
    /* Configure DMA TX channel */
    iController = (source_usart1.dev.txDMAChannel >> 8);
    dwCfg = 0
           | DMAC_CFG_DST_PER(
             DMAIF_Get_ChannelNumber( iController, ID_USART1, DMAD_TRANSFER_TX ))
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( pDmad, source_usart1.dev.txDMAChannel, dwCfg );
#endif

#if 1
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
    /* Set RX callback */
    DMAD_SetCallback(&g_dmad, source_usart1.dev.txDMAChannel,
                    (DmadTransferCallback)_USART1_DmaTxCallback, 0);
    DMAD_SetCallback(&g_dmad, source_usart1.dev.rxDMAChannel,
                    (DmadTransferCallback)_USART1_DmaRxCallback, 0);
    /* Configure DMA RX channel */
    iController = ( source_usart1.dev.rxDMAChannel >> 8 );
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
              DMAIF_Get_ChannelNumber( iController, ID_USART1, DMAD_TRANSFER_RX ))
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, source_usart1.dev.rxDMAChannel, dwCfg );
    /* Configure DMA TX channel */
    iController = ( source_usart1.dev.txDMAChannel >> 8);
    dwCfg = 0 
           | DMAC_CFG_DST_PER(
              DMAIF_Get_ChannelNumber( iController, ID_USART1, DMAD_TRANSFER_TX ))
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, source_usart1.dev.txDMAChannel, dwCfg );
#endif    
    
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
#define TASKLEDPRIORITY    ( 8u ) 
#define TASKUSBPRIORITY    ( 12u )
#define TASKSSC0PRIORITY   ( 6u )
#define CMDPARASEPRIORITY  ( 14u )
#define FIRMWAREVECUPDATE  ( 16u )


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
extern void _ConfigureTc1( uint32_t hz );

int main()
{
    CPU_INT08U  os_err;
    uint32_t twi_hz = 400000;

    APP_TRACE_INFO(("Application start!\r\n"));

    CPU_Init();

    Mem_Init();
    
    BSP_LED_Init();
    BSP_LED_Off( 3 );   


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
	printf("this version isn't a release version \n");
	return -1;
    }
#endif
    
#if UIF_SSC0
    //initialize ssc0 object and it's operation 
    memset( ( void * )&source_ssc0, 0 , sizeof( DataSource ) );
    source_ssc0.dev.direct = ( uint8_t )BI;
    source_ssc0.dev.identify = ID_SSC0;
    source_ssc0.dev.instanceHandle = (uint32_t)SSC0;
    source_ssc0.status = ( uint8_t )FREE;
    source_ssc0.tx_index = 0;
    source_ssc0.rx_index = 0;
    source_ssc0.peripheralParameter = ( void * )Audio_Configure_Instance0;
    
    source_ssc0.init_source = init_I2S;
    source_ssc0.play = SSC0_Playing;
    source_ssc0.record = SSC0_Recording;
    source_ssc0.buffer_write = NULL;
    
    if( NULL != source_ssc0.init_source )
        source_ssc0.init_source( &source_ssc0,NULL );
#endif
    
    
#if UIF_SSC1
    //initialize ssc1 object and it's operation 
    memset( ( void * )&source_ssc1, 0 , sizeof( DataSource ) );
    source_ssc1.dev.direct = ( uint8_t )BI;
    source_ssc1.dev.identify = ID_SSC1;
    source_ssc1.dev.instanceHandle = (uint32_t)SSC1;
    source_ssc1.status = ( uint8_t )FREE;
    source_ssc1.tx_index = 0;
    source_ssc1.rx_index = 0;
    source_ssc1.peripheralParameter = ( void * )Audio_Configure_Instance1;
    
    source_ssc1.init_source = init_I2S;
    source_ssc1.play = SSC1_Playing;
    source_ssc1.record = SSC1_Recording;
    source_ssc1.buffer_write = NULL;
    
    if( NULL != source_ssc1.init_source )
       source_ssc1.init_source( &source_ssc1,NULL );
#endif
    
#if UIF_SPI0
    //initialize spi0 object and it's operation
    VOICE_BUF_CFG spi0_cfg;
    memset( ( void * )&source_spi0, 0 , sizeof( DataSource ) );
    memset( ( void * )&spi0_cfg, 0 , sizeof( VOICE_BUF_CFG ) );
    source_spi0.dev.direct = ( uint8_t )BI;
    source_spi0.dev.identify = ID_SPI0;
    source_spi0.dev.instanceHandle = (uint32_t)SPI0;    
    source_spi0.status = ( uint8_t )FREE;
    source_spi0.tx_index = 0;
    source_spi0.rx_index = 0;
    source_spi0.privateData = spi0_ring_buffer;
    spi0_cfg.spi_speed = 4 * 1000 * 1000;
    spi0_cfg.spi_mode = 1;
    
    
    source_spi0.init_source = init_spi;
    source_spi0.play = NULL;
    source_spi0.record = NULL;
    source_spi0.peripheral_stop = stop_spi;
    source_spi0.buffer_write = NULL;
    
    if( NULL != source_spi0.init_source )
       source_spi0.init_source( &source_spi0,&spi0_cfg );
#endif      
    
#if UIF_SPI1
    //initialize spi1 object and it's operation
    VOICE_BUF_CFG spi1_cfg;
    memset( ( void * )&source_spi1, 0 , sizeof( DataSource ) );
    memset( ( void * )&spi1_cfg, 0 , sizeof( VOICE_BUF_CFG ) );
    source_spi1.dev.direct = ( uint8_t )BI;
    source_spi1.dev.identify = ID_SPI1;
    source_spi1.dev.instanceHandle = (uint32_t)SPI1;    
    source_spi1.status = ( uint8_t )FREE;
    source_spi1.tx_index = 0;
    source_spi1.rx_index = 0;
    source_spi1.privateData = spi1_ring_buffer;
    source_spi1.buffer = spi1_buffer[ 1 ];
    spi1_cfg.spi_speed = 40 * 1000 * 1000;
    spi1_cfg.spi_mode  = 1;
    
    
    source_spi1.init_source = init_spi;
    source_spi1.play = NULL;
    source_spi1.record = NULL;
    source_spi1.peripheral_stop = stop_spi;
    source_spi1.buffer_write = NULL;
    
    if( NULL != source_spi1.init_source )
       source_spi1.init_source( &source_spi1,&spi1_cfg );
#endif  


#if UIF_TWI0
    //initialize twi0 object and it's operation     
    OPTIONPARAMETER twi0ChipConf[ 2 ];
    memset( ( void * )&twi0ChipConf[ 0 ], 0 ,sizeof( OPTIONPARAMETER ) << 1 );
    twi0ChipConf[ 0 ].address = 0xc0;
    twi0ChipConf[ 0 ].iaddress = 0;
    twi0ChipConf[ 0 ].isize = 0;
    twi0ChipConf[ 0 ].revers = 0;
    
    memset( ( void * )&source_twi0, 0 , sizeof( DataSource ) );
    source_twi0.dev.direct = ( uint8_t )BI;
    source_twi0.dev.identify = ID_TWI0;
    source_twi0.dev.instanceHandle = (uint32_t)TWI0;	
    source_twi0.status = ( uint8_t )FREE;
    source_twi0.tx_index = 0;
    source_twi0.rx_index = 0;
    source_twi0.privateData = &twi0ChipConf[ 0 ];

    source_twi0.init_source = twi_init_master;
    source_twi0.play = NULL;
    source_twi0.record = NULL;
    source_twi0.buffer_write = twi_uname_write;
    source_twi0.buffer_read = twi_uname_read;
    
    if( NULL != source_twi0.init_source )
        source_twi0.init_source( &source_twi0,&twi_hz );

#endif
    
    
#if UIF_TWI1
    //initialize twi0 object and it's operation  
    OPTIONPARAMETER twi1_chipConf[ 2 ];
    memset( ( void * )&twi1_chipConf[ 0 ], 0 ,sizeof( OPTIONPARAMETER ) << 1 );
    twi1_chipConf[ 0 ].address = 0xc0;
    twi1_chipConf[ 0 ].iaddress = 0;
    twi1_chipConf[ 0 ].isize = 0;
    twi1_chipConf[ 0 ].revers = 0;
    
    twi1_chipConf[ 1 ].address = 0x30;
    twi1_chipConf[ 1 ].iaddress = 0;
    twi1_chipConf[ 1 ].isize = 0;
    twi1_chipConf[ 1 ].revers = 0;
    
    memset( ( void * )&source_twi1, 0 , sizeof( DataSource ) );
    source_twi1.dev.direct = ( uint8_t )BI;
    source_twi1.dev.identify = ID_TWI1;
    source_twi1.dev.instanceHandle = (uint32_t)TWI1;	
    source_twi1.status = ( uint8_t )FREE;
    source_twi1.tx_index = 0;
    source_twi1.rx_index = 0;
    source_twi1.privateData = &twi1_chipConf[ 0 ];

    source_twi1.init_source = twi_init_master;
    source_twi1.play = NULL;
    source_twi1.record = NULL;
    source_twi1.buffer_write = twi_uname_write;
    source_twi1.buffer_read = twi_uname_read;
    
    if( NULL != source_twi1.init_source )
        source_twi1.init_source( &source_twi1,&twi_hz );

#endif


#if UIF_TWI2
    //initialize twi2 object and it's operation      
    OPTIONPARAMETER twi2_chipConf[ 2 ];
    memset( ( void * )&twi2_chipConf[ 0 ], 0 ,sizeof( OPTIONPARAMETER ) << 1 );
    twi2_chipConf[ 0 ].address = 0xc0;
    twi2_chipConf[ 0 ].iaddress = 0;
    twi2_chipConf[ 0 ].isize = 0;
    twi2_chipConf[ 0 ].revers = 0;
    
    twi2_chipConf[ 1 ].address = 0x30;
    twi2_chipConf[ 1 ].iaddress = 0;
    twi2_chipConf[ 1 ].isize = 0;
    twi2_chipConf[ 1 ].revers = 0;    
    
    memset( ( void * )&source_twi2, 0 , sizeof( DataSource ) );
    source_twi2.dev.direct = ( uint8_t )BI;
    source_twi2.dev.identify = ID_TWI2;
    source_twi2.dev.instanceHandle = (uint32_t)TWI2;	
    source_twi2.status = ( uint8_t )FREE;
    source_twi2.tx_index = 0;
    source_twi2.rx_index = 0;
    source_twi2.privateData = &twi2_chipConf[ 0 ];
    
    source_twi2.init_source = twi_init_master;
    source_twi2.play = NULL;
    source_twi2.record = NULL;
    source_twi2.buffer_write = twi_fm36_write;
    source_twi2.buffer_read = twi_fm36_read;
    
    if( NULL != source_twi2.init_source )
        source_twi2.init_source( &source_twi2,&twi_hz );

#endif 

#if UIF_USART1
        //initialize ssc1 object and it's operation 
    memset( ( void * )&source_usart1, 0 , sizeof( DataSource ) );
    source_usart1.dev.direct = ( uint8_t )BI;
    source_usart1.dev.identify = ID_USART1;
    source_usart1.dev.instanceHandle = (uint32_t)USART1;
    source_usart1.status = ( uint8_t )FREE;
    source_usart1.tx_index = 0;
    source_usart1.rx_index = 0;
//    source_usart1.peripheralParameter = ( void * )Audio_Configure_Instance1;
    
    source_usart1.init_source = usart_init;
    source_usart1.play = NULL;
    source_usart1.record = NULL;
    source_usart1.buffer_write = usart1_DmaTx;
    source_usart1.buffer_read = usart1_DmaRx;
    
    if( NULL != source_usart1.init_source )
       source_usart1.init_source( &source_usart1,NULL );
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
    
    memset( spi1_ring_buffer, 0x55, sizeof( spi1_ring_buffer ));
    memset( spi1_buffer[ 1 ], 0, sizeof( spi1_ring_buffer ));  
    memset( twi_ring_buffer[ 0 ],0x55,sizeof( twi_ring_buffer[ 0 ] ));
    memset( usartBuffer[ 0 ], 0x55 , 1024 );
    memset( usartBuffer[ 1 ], 0x55 , 1024 );
    
    for(;;) 
    {
        OSTimeDlyHMSM(0, 0, 1, 0);

        BSP_LED_Toggle( 1 );
        
 
        spi_clear_status( &source_spi1 );
        memset( spi1_buffer[ 1 ], 0, sizeof( spi1_ring_buffer ) );
        _spiDmaRx( &source_spi1 );
        _spiDmaTx( &source_spi1 );
               
        usart1_DmaTx( &source_usart1 , NULL , 0 );
//       twi_uname_write( &source_twi0,twi_ring_buffer[0],sizeof( twi_ring_buffer[ 0 ] ) );
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
    static uint8_t isUsbConnected = 0;
    
    g_pPortManagerMbox = OSMboxCreate( (void * )taskMsg );
      
    for(;;) {
     
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
        
        //step1:get cmd from usb cmd endpoint;
        CDCDSerialDriver_Read_CmdEp(  usbCmdBufferBulkOut,
                                USBCMDDATAEPSIZE ,
                                //(TransferCallback) _UsbDataReceived,
                                0,
                                0);
        
         //step2:parse cmd and fill corresponding message via mailbox;
         Audio_State_Control( &taskMsg );
          
         //step3:post message mailbox to start other task;
         taskMsg = (SSC0_IN | SSC0_OUT | SSC1_IN | SSC1_OUT );
         OSMboxPost( g_pPortManagerMbox, (void *)taskMsg );     
       
         //let this task running periodic per second enough;
         OSTimeDlyHMSM( 0, 0, 0, 500 );
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
static  void  AppTaskSSC0 ( void *p_arg )
{
    uint8_t err = 0;
    uint8_t receiveTaskMsg = 0; 
    
    for(;;) 
    {
          receiveTaskMsg = ( uint32_t )OSMboxPend( g_pPortManagerMbox, 
                                                0, 
                                                &err);

          switch( receiveTaskMsg )
          {
            case ( SSC0_IN | SSC0_OUT | SSC1_IN | SSC1_OUT ):
                    if( ( ( uint8_t )START != source_ssc0.status )  
                        &&( ( uint8_t )BUFFERED != source_ssc0.status ) 
                        &&( ( uint8_t )RUNNING != source_ssc0.status ) )
                    {
//                          OSSchedLock( );
                          source_ssc0.play( &source_ssc0 );
                          source_ssc0.record( &source_ssc0 );
                          source_ssc0.status = ( uint8_t )START;
                          source_ssc1.play( &source_ssc1 );
                          source_ssc1.record( &source_ssc1 );
                          source_ssc1.status = ( uint8_t )START;
//                          OSSchedUnlock( );
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
    
   
    for(;;) 
    {

#if 1               
        evFlags = OSFlagQuery( g_pStartUSBTransfer, &err );

        //maybe transfer according events,not waiting for all event happend;
        //
        if( 0x0f == evFlags )  
        {                  
            CDCDSerialDriver_Write( usbBufferBulkIn0,
                                   USBDATAEPSIZE,
                                  (TransferCallback)UsbAudio0DataTransmit,
                                   0);
            CDCDSerialDriver_Write_SecondEp( usbBufferBulkIn1,
                                   USBDATAEPSIZE,
                                  (TransferCallback)UsbAudio1DataTransmit,
                                   0); 
            CDCDSerialDriver_Read( usbBufferBulkOut0,
                                  USBDATAEPSIZE,
                                  (TransferCallback) UsbAudio0DataReceived,
                                  0);        

            CDCDSerialDriver_Read_SecondEp( usbBufferBulkOut1,
                                     USBDATAEPSIZE,
                                     (TransferCallback) UsbAudio1DataReceived,
                                         0);        
        }
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
         default:
           //do nothing;
          break;
        }
      
    }  
    
}
#endif

