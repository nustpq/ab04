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
*                           __            _                          _ _       
                      / _| ___  _ __| |_ ___ _ __ ___   ___  __| (_) __ _ 
                     | |_ / _ \| '__| __/ _ \ '_ ` _ \ / _ \/ _` | |/ _` |
                     |  _| (_) | |  | ||  __/ | | | | |  __/ (_| | | (_| |
                     |_|  \___/|_|   \__\___|_| |_| |_|\___|\__,_|_|\__,_|
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


#include <includes.h>


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
#define UIF_TWI0	     1u
#define UIF_TWI1	     1u
#define UIF_TWI2	     1u
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


//--------------------------------twi  buffer --------------------------------//
uint8_t twi_ring_buffer[ MAXTWI ][ 256 ];

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

void Dma_configure(void)
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

static  CPU_STK  AppTaskLEDStk[4096u];
static  CPU_STK  AppTaskUSBStk[4096u];
static  CPU_STK  AppTaskSSC0Stk[4096u];
static  CPU_STK  AppTaskCmdParaseStk[4096u];
static  CPU_STK  AppTaskFirmwareVecUpdateStk[4096u];

static  OS_STK       App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       App_TaskUserIF_Stk[APP_CFG_TASK_USER_IF_STK_SIZE];
static  OS_STK       App_TaskJoyStk[APP_CFG_TASK_JOY_STK_SIZE];
static  OS_STK       App_TaskGenieShellStk[APP_CFG_TASK_SHELL_STK_SIZE];

static  OS_STK       App_TaskUART_RxStk[APP_CFG_TASK_UART_RX_STK_SIZE];
static  OS_STK       App_TaskUART_TxStk[APP_CFG_TASK_UART_TX_STK_SIZE];
//static  OS_STK       App_TaskUART_TxRulerStk[APP_CFG_TASK_UART_TX_RULER_STK_SIZE];
//static  OS_STK       App_TaskNoahStk[APP_CFG_TASK_NOAH_STK_SIZE];
//static  OS_STK       App_TaskNoahRulerStk[APP_CFG_TASK_NOAH_RULER_STK_SIZE];
static  OS_STK       App_TaskCMDParseStk[APP_CFG_TASK_CMD_PARSE_STK_SIZE];
static  OS_STK       App_TaskDebugInfoStk[APP_CFG_TASK_DBG_INFO_STK_SIZE];    

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
static  void  App_BufferCreate (void);
static  void  App_EventCreate (void);
static  void  App_TaskStart             (void        *p_arg);
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

int main()
{
  
    CPU_INT08U  os_err;
                              
    CPU_Init();
                    
    OSInit(); 
    
    OSTaskCreateExt((void (*)(void *)) App_TaskStart,           /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_START_PRIO,
                    (INT16U          ) APP_CFG_TASK_START_PRIO,
                    (OS_STK         *)&App_TaskStartStk[0],
                    (INT32U          ) APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &os_err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */
  
    return (1);

}
    

/*
*********************************************************************************************************
*                                          AppTaskStart()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument(s) : p_arg       Argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*
*               (2) Interrupts are enabled once the task starts because the I-bit of the CCR register was
*                   set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/
static  void  App_TaskStart (void *p_arg)
{
    (void)p_arg;
    CPU_INT32U  counter;
    CPU_INT08U  os_err;
    
    BSP_Init();      
    Mem_Init(); 
    
#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif
    
    App_BufferCreate();  
    App_EventCreate(); 
    
#if UIF_LED
    // create led flash task 
    os_err = OSTaskCreateExt( AppTaskLED,   
                              DEF_NULL,
                             &AppTaskLEDStk[APP_CFG_TASK_LED_STK_SIZE - 1],
                              TASKLEDPRIORITY,
                              TASKLEDPRIORITY,
                             &AppTaskLEDStk[0],
                              APP_CFG_TASK_LED_STK_SIZE,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(TASKLEDPRIORITY, "AppTaskLED", &os_err);
#endif 
#endif
   
    
#if UIF_USB   
    // create usb transfer task 
    os_err = OSTaskCreateExt( AppTaskUSB,                                 
                              DEF_NULL,
                             &AppTaskUSBStk[APP_CFG_TASK_USB_STK_SIZE - 1],
                              TASKUSBPRIORITY,
                              TASKUSBPRIORITY,
                             &AppTaskUSBStk[0],
                              APP_CFG_TASK_USB_STK_SIZE,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(TASKUSBPRIORITY, "AppTaskUSB", &os_err);
#endif    
#endif

#if UIF_SSC0   
    // Create the ssc0 data transfer task; 
    os_err = OSTaskCreateExt( AppTaskSSC0,                               
                              DEF_NULL,
                             &AppTaskSSC0Stk[APP_CFG_TASK_SSC0_STK_SIZE - 1],
                              TASKSSC0PRIORITY,
                              TASKSSC0PRIORITY,
                             &AppTaskSSC0Stk[0],
                              APP_CFG_TASK_SSC0_STK_SIZE,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(TASKSSC0PRIORITY, "AppTaskSSC0", &os_err);
#endif 
#endif
    
//#if UIF_COMMAND    
//    // create cmd parase task 
//    os_err = OSTaskCreateExt( AppTaskCmdParase,   
//                              DEF_NULL,
//                             &AppTaskCmdParaseStk[4096 - 1],
//                              CMDPARASEPRIORITY,
//                              CMDPARASEPRIORITY,
//                             &AppTaskCmdParaseStk[0],
//                              4096u,
//                              0u,
//                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
//
//    if(os_err != OS_ERR_NONE) {
//        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
//    }
//#endif
    
//#if UIF_NANDFLASH    
//    // create firmware update task 
//    os_err = OSTaskCreateExt( AppTaskFirmwareVecUpdate,   
//                              DEF_NULL,
//                             &AppTaskFirmwareVecUpdateStk[4096 - 1],
//                              FIRMWAREVECUPDATE,
//                              FIRMWAREVECUPDATE,
//                             &AppTaskFirmwareVecUpdateStk[0],
//                              4096u,
//                              0u,
//                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
//
//    if(os_err != OS_ERR_NONE) {
//        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
//    }
//#endif    

    
////////////////////////////////////////////////////////////////////////////////
    
    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskCMDParse,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskCMDParseStk[APP_CFG_TASK_CMD_PARSE_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_CMD_PARSE_PRIO,
                    (INT16U          ) APP_CFG_TASK_CMD_PARSE_PRIO,
                    (OS_STK         *)&App_TaskCMDParseStk[0],
                    (INT32U          ) APP_CFG_TASK_CMD_PARSE_STK_SIZE,
                    (void *)0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_CMD_PARSE_PRIO, "CMD_Parse", &os_err);
#endif   
    
    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskUART_Rx,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskUART_RxStk[APP_CFG_TASK_UART_RX_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_UART_RX_PRIO,
                    (INT16U          ) APP_CFG_TASK_UART_RX_PRIO,
                    (OS_STK         *)&App_TaskUART_RxStk[0],
                    (INT32U          ) APP_CFG_TASK_UART_RX_STK_SIZE,
                    (void *)0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_UART_RX_PRIO, "Uart_rx", &os_err);
#endif 
    
    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskUART_Tx,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskUART_TxStk[APP_CFG_TASK_UART_TX_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_UART_TX_PRIO,
                    (INT16U          ) APP_CFG_TASK_UART_TX_PRIO,
                    (OS_STK         *)&App_TaskUART_TxStk[0],
                    (INT32U          ) APP_CFG_TASK_UART_TX_STK_SIZE,
                    (void *)0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_UART_TX_PRIO, "Uart_tx", &os_err);
#endif  

    
    OSTaskCreateExt((void (*)(void *)) App_TaskUserIF,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskUserIF_Stk[APP_CFG_TASK_USER_IF_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_USER_IF_PRIO,
                    (INT16U          ) APP_CFG_TASK_USER_IF_PRIO,
                    (OS_STK         *)&App_TaskUserIF_Stk[0],
                    (INT32U          ) APP_CFG_TASK_USER_IF_STK_SIZE,
                    (void *)0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_USER_IF_PRIO, "User I/F", &os_err);
#endif
   
////////////////////////////////////////////////////////////////////////////////
    
    g_pStartUSBTransfer = OSFlagCreate( 0,&os_err );

    OSStart();
    
    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error starting. OSStart() returned with error %u\r\n", os_err));
    }
    
    
    for (;;) {
      
        counter++;
        
        if(counter&0xFF) {
            UIF_LED_On( LED_RUN ); 
        }
        
        if(counter&0x3F) {
            UIF_LED_Off( LED_RUN ); 
        }
        
        OSTimeDly(10);  
    }

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

    
    memset( spi1_RingBulkIn, 0x55, sizeof( spi1_RingBulkIn ));
    memset( spi1_2MSOut, 0x38, sizeof( spi1_2MSOut ));  
    memset( twi_ring_buffer[ 0 ],0x55,sizeof( twi_ring_buffer[ 0 ] ));
    memset( usartBuffer[ 0 ], 0x55 , 1024 );
    memset( usartBuffer[ 1 ], 0x55 , 1024 );
    memset( spi0_RingBulkOut, 0x55, sizeof( spi0_RingBulkOut ) );    
    
    for(;;) 
    {
        OSTimeDlyHMSM(0, 0, 0, 10);  //change this interval about 10ms to fit fm1388
      
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


    
//#if UIF_COMMAND
//
//static  void  AppTaskCmdParase ( void *p_arg )
//{
//    static uint8_t taskMsg;
//    static uint8_t isUsbConnected = 0;
//    
//    static uint8_t transmitStart = 0;
//    
//    g_pPortManagerMbox = OSMboxCreate( (void * )taskMsg );
//
//  
//    while(1);
//    
//    for(;;) {
//        //task alive indicate
//        UIF_LED_Toggle( LED_D3 );
//        
//        /* Device is not configured */
//        if (USBD_GetState() < USBD_STATE_CONFIGURED)
//        {
//            if (isUsbConnected)
//            {
//                isUsbConnected = 0;
//                TC_Stop(TC1, 0);
//                
//                //stop all port;
//            }
//        }
//        else if (isUsbConnected == 0)
//        {
//            isUsbConnected = 1;
//            TC_Start(TC1, 0);
//        }
//        
//        //step1:get cmd from usb cmd endpoint;
//        CDCDSerialDriver_Read_CmdEp(  usbCmdCacheBulkOut,
//                                USB_CMDEP_SIZE_64B ,
//                                (TransferCallback) UsbCmdDataReceived,                                
//                                0);
//        
//         //step2:parse cmd and fill corresponding message via mailbox;
//         Audio_State_Control( &taskMsg );
//          
//         //step3:post message mailbox to start other task;
//#if 1         
//         if( !transmitStart )
//         {
//            taskMsg = (SSC0_IN | SSC0_OUT | SSC1_IN | SSC1_OUT );
//            OSMboxPost( g_pPortManagerMbox, (void *)taskMsg ); 
//            transmitStart = 1;
//            taskMsg = 0;
//         }
//#endif       
//         //let this task running periodic per second enough;
//         OSTimeDlyHMSM( 0, 0, 0, 500 );
//    }
//
//}
//#endif

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
                          source_ssc0.buffer_write( &source_ssc0,( uint8_t * )ssc0_PingPongOut,
                                                   sizeof( ssc0_PingPongOut ) >> 1 );
                          source_ssc0.buffer_read( &source_ssc0,( uint8_t * )ssc0_PingPongIn,
                                                   sizeof( ssc0_PingPongIn ) );                          
                          source_ssc0.status[ IN ]  = ( uint8_t )START;
                          source_ssc0.status[ OUT ] = ( uint8_t )START;                          
                    
                          source_ssc1.buffer_write( &source_ssc1,( uint8_t * )ssc1_PingPongOut,
                                                   sizeof( ssc1_PingPongOut ) >> 1 );
                          source_ssc1.buffer_read( &source_ssc1,( uint8_t * )ssc1_PingPongIn,
                                                   sizeof( ssc1_PingPongIn ) >> 1 );                            
                          source_ssc1.status[ IN ]  = ( uint8_t )START;
                          source_ssc1.status[ OUT ] = ( uint8_t )START;  
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
    uint8_t  err = 0;
    uint8_t  evFlags = 0; 
    uint32_t byteCnt = 0;
    uint32_t byteCnt2 = 0;
    uint8_t  usb_state = 0;
    uint8_t  usb_state_saved = 0;
    
    memset( ( uint8_t * )tmpInBuffer , 0x32 , sizeof( tmpInBuffer ) ); 
    
    for(;;) 
    {
        
        usb_state =   USBD_GetState();         
        
        if( usb_state != usb_state_saved ) { 
            usb_state_saved = usb_state ;
            if ( usb_state >= USBD_STATE_CONFIGURED ) {
                
                CDCDSerialDriver_Read_CmdEp(  usbCmdCacheBulkOut,
                                  USB_CMDEP_SIZE_64B ,
                                  (TransferCallback) UsbCmdDataReceived,                                
                                  0);
            } 
        }
        
        if ( usb_state >= USBD_STATE_CONFIGURED ) {
            UIF_LED_On( LED_USB );    
        } else {
            UIF_LED_Off( LED_USB );             
        }
        
        if ( usb_state >= USBD_STATE_CONFIGURED ) {        
#if 0 //test
 //----------------------------------------------------------------------------
            byteCnt = kfifo_get_free_space( &ep0BulkOut_fifo );
            if(  byteCnt >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_out && audio_run_control )  {
                restart_audio_0_bulk_out = false ;
                APP_TRACE_INFO(("\r\nBulk Out 0 start"));
              // send ep0 data ---> pc
                CDCDSerialDriver_Read( usbCacheBulkOut0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataReceived,
                                        0);  
            } else {
                  byteCnt  = kfifo_get_data_size( &ep0BulkOut_fifo );
                  byteCnt = byteCnt > 256 ?  256 :   byteCnt ;
                  kfifo_get( &ep0BulkOut_fifo,
                          ( uint8_t * )tmpOutBuffer,
                           byteCnt );
                  dump_buf_debug(( uint8_t * )tmpOutBuffer, byteCnt  ) ;
            }
#endif   
        
        
#if 1
 //----------------------------------------------------------------------------
            byteCnt = kfifo_get_free_space( &ep0BulkOut_fifo );
            if(  byteCnt >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_out && audio_run_control )  {
                restart_audio_0_bulk_out = false ;
                APP_TRACE_INFO(("\r\nBulk Out 0 start"));
              // send ep0 data ---> pc
                CDCDSerialDriver_Read( usbCacheBulkOut0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataReceived,
                                        0);  
            }
        
        
 //----------------------------------------------------------------------------
            if( audio_start_flag ) {
                audio_start_flag = false ;
                memset( tmpInBuffer, audio_0_padding , USB_DATAEP_SIZE_64B*4 );
                kfifo_put( &ep0BulkIn_fifo,
                          ( uint8_t * )tmpInBuffer,
                           USB_DATAEP_SIZE_64B*4 );
            }
            //////////
            byteCnt  = kfifo_get_data_size( &ep0BulkOut_fifo );
            byteCnt2 = kfifo_get_free_space( &ep0BulkIn_fifo );
            if( byteCnt >= 1536  && byteCnt2 >= 1536 ) {
               kfifo_get( &ep0BulkOut_fifo,
                          ( uint8_t * )tmpOutBuffer,
                           1536 );        
               //memset( tmpInBuffer, 0x34 , 1536 );
               kfifo_put( &ep0BulkIn_fifo,
                          ( uint8_t * )tmpOutBuffer,
                           1536 );
            }
            //////////
            byteCnt = kfifo_get_data_size( &ep0BulkIn_fifo );     
            if(  byteCnt >= USB_DATAEP_SIZE_64B && restart_audio_0_bulk_in && audio_run_control)  {
                APP_TRACE_INFO(("\r\nBulk  In 0 start"));
                restart_audio_0_bulk_in = false ;
                // ep0 ring --> usb cache
                kfifo_get( &ep0BulkIn_fifo,
                          ( uint8_t * )usbCacheBulkIn0,
                           USB_DATAEP_SIZE_64B );         
            
              // send ep0 data ---> pc
                CDCDSerialDriver_Write( usbCacheBulkIn0,
                                        USB_DATAEP_SIZE_64B,
                                        (TransferCallback)UsbAudio0DataTransmit,
                                        0);  
            }
#endif
       
        
        
        
 //----------------------------------------------------------------------------       
#if 0               
        //evFlags = OSFlagQuery( g_pStartUSBTransfer, &err );

        //maybe transfer according events,not waiting for all event happend;
        //
 
//-------------------proccess up link ----------------------------------------          
            //copy data from ssc0 in ring buffer to usb in ring buffer;
            // ssc0 ring -----> tmpInbuffer
            sizecnt = kfifo_get_data_size( &ssc0_bulkin_fifo );
            kfifo_get( &ssc0_bulkin_fifo,
                      ( uint8_t * )tmpInBuffer,
                       6144 );
            memset( ( void * )tmpInBuffer, 0x12, sizeof( tmpInBuffer ) );            
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
           
#endif
//---------------------proccess down link ------------------------------------
            // usb cache --> ep0 ring 
#if 0   
            CDCDSerialDriver_Read( usbCacheBulkOut0,
                                   USB_DATAEP_SIZE_64B,
                                   (TransferCallback) UsbAudio0DataReceived,
                                    0); 


 
            //copy data from  usb ring buffer to temp buffer ;
            //ep0 ring ---> tmpOutbuffer;
            if( source_ssc0.txSize == kfifo_get( &ep0BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize ) ) 
            //tmpOutBuffer --> ssc0 ring
            kfifo_put( &ssc0_bulkout_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize );
        
            
            //copy data from  usb ring buffer to spi0 in ring buffer ;
            //ep0 ring ---> tmpOutBuffer;
           
            kfifo_get( &ep0BulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize  ); 
            //tmpOutBuffer ---> spi0 ring
            kfifo_put( &spi0_bulkOut_fifo,
                      ( uint8_t * )tmpOutBuffer,
                       source_ssc0.txSize  );             
#endif
           
#if 0           
            CDCDSerialDriver_Read_SecondEp( usbCacheBulkOut1,
                                            USB_DATAEP_SIZE_64B,
                                            (TransferCallback) UsbAudio1DataReceived,
                                            0); 
           
           //copy data from  usb ring buffer to ssc1 in ring buffer ; 
            //ep1 ring ---> tmpOutbuffer; 
            byteCnt = kfifo_get( &ep1BulkOut_fifo,
                                 ( uint8_t * )tmpOutBuffer,
                                 source_ssc1.txSize );
            if( source_ssc1.txSize == byteCnt )
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
       



       
        }
        OSTimeDlyHMSM( 0,0,0,5 );
        
    } //for loop
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
        
        OSTimeDly(2);
      
    }  
    
}
#endif



/*
*********************************************************************************************************
*                                      App_BufferCreate()
*
* Description : Create the application uart buffer
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TasStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void  App_BufferCreate (void)
{
  
    CPU_INT08U  err;
    
    //APP_TRACE_INFO(("Creating Application Buffer...\r\n"));
    
#if (OS_MEM_EN > 0)
    
     pMEM_Part_MsgUART = OSMemCreate( MemPartition_MsgUART, MsgUARTQueue_SIZE, MsgUARTBody_SIZE, &err );
     if(OS_ERR_NONE != err) {
        while(1);  
     }   
     
#if (OS_MEM_NAME_EN > 0)
   OSMemNameSet(pMEM_Part_MsgUART, "MEM_Part_MsgUART", &err); 
#endif
   
#endif           
       
}


static void  App_EventCreate (void)
{
    
    //APP_TRACE_INFO(("Creating Application Events...\r\n"));
      
#if (OS_EVENT_NAME_EN  > 0 )
    CPU_INT08U  err;
#endif
    
    App_UserIF_Mbox     = OSMboxCreate((void *)0);   /* Create MBOX for comm between App_TaskUserIF() and App_TaskJoy()    */
   // App_Noah_Ruler_Mbox = OSMboxCreate((void *)0);   /* Create MBOX for comm App_TaskUserIF()to App_TaskNoah_Ruler()       */    
//    ACK_Sem_PCUART      = OSSemCreate(0);            /* Create Sem for the ACK from PC, after UART data sent               */    
//    ACK_Sem_RulerUART   = OSSemCreate(0);            /* Create Sem for the ACK from Ruler, after UART data sent            */    
//    Done_Sem_RulerUART  = OSSemCreate(0);            /* Create Sem for the Ruler operation caller, after operation done    */    
    EVENT_MsgQ_PCUART2Noah     = OSQCreate(&MsgQ_PCUART2Noah[0],MsgUARTQueue_SIZE);             /* Message queue from PC   */  
    EVENT_MsgQ_Noah2PCUART     = OSQCreate(&MsgQ_Noah2PCUART[0],MsgUARTQueue_SIZE);             /* Message queue to PC     */  
//    EVENT_MsgQ_RulerUART2Noah  = OSQCreate(&MsgQ_RulerUART2Noah[0],MsgUARTQueue_SIZE);          /* Message queue from Ruler*/  
//    EVENT_MsgQ_Noah2RulerUART  = OSQCreate(&MsgQ_Noah2RulerUART[0],MsgUARTQueue_SIZE);          /* Message queue to Ruler  */  
    EVENT_MsgQ_Noah2CMDParse   = OSQCreate(&MsgQ_Noah2CMDParse[0],MsgUARTQueue_SIZE);   /* Message queue to Task CMD Prase */ 
//    Bsp_Ser_Tx_Sem_lock = OSSemCreate(1); 
//    Bsp_Ser_Rx_Sem_lock = OSSemCreate(1); 
//    DBGU_Tx_Sem_lock    = OSSemCreate(1); 
//    DBGU_Tx_Sem_lock    = OSSemCreate(1);     
//    GPIO_Sem_I2C_Mixer  = OSSemCreate(1);  //sem for I2C mixer
//    UART_MUX_Sem_lock   = OSSemCreate(1); 
//    Load_Vec_Sem_lock   = OSSemCreate(1); //sem for MCU_Load_Vec() in im501_comm.c
//    //ruler UART MUX //if error then halt MCU
//    if( NULL == UART_MUX_Sem_lock )  while(1); //last Event creat failure means OS_MAX_EVENTS is not enough

#if (OS_EVENT_NAME_EN > 0)    
    
//   OSEventNameSet(App_UserIF_Mbox,      "Joy->UserI/F Mbox",   &err);
//  // OSEventNameSet(App_Noah_Ruler_Mbox,  "UserI/F->NoahRulerMbox",     &err);
//   OSEventNameSet(ACK_Sem_PCUART,       "PCUART_Tx_ACK_Sem",    &err);  
//   OSEventNameSet(ACK_Sem_RulerUART,    "RulerUART_Tx_ACK_Sem", &err); 
//   OSEventNameSet(Done_Sem_RulerUART,   "Done_Sem_RulerUART",   &err);
//   OSEventNameSet(EVENT_MsgQ_PCUART2Noah,      "EVENT_MsgQ_PCUART2Noah",      &err);
//   OSEventNameSet(EVENT_MsgQ_Noah2PCUART,      "EVENT_MsgQ_Noah2PCUART",      &err); 
//   OSEventNameSet(EVENT_MsgQ_RulerUART2Noah,   "EVENT_MsgQ_RulerUART2Noah",   &err);
//   OSEventNameSet(EVENT_MsgQ_Noah2RulerUART,   "EVENT_MsgQ_Noah2RulerUART",   &err); 
//   OSEventNameSet(EVENT_MsgQ_Noah2CMDParse,    "EVENT_MsgQ_Noah2CMDParse",    &err);
//   OSEventNameSet(Bsp_Ser_Tx_Sem_lock,  "Bsp_Ser_Tx_Sem_lock",  &err);
//   OSEventNameSet(Bsp_Ser_Rx_Sem_lock,  "Bsp_Ser_Tx_Sem_lock",  &err);
//   OSEventNameSet(DBGU_Tx_Sem_lock,     "DBGU_Tx_Sem_lock",     &err);
//   OSEventNameSet(DBGU_Rx_Sem_lock,     "DBGU_Rx_Sem_lock",     &err);
//   OSEventNameSet(UART_MUX_Sem_lock,    "UART_MUX_Sem_lock",    &err);
   
#endif
   
}
