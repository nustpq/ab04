/*
*********************************************************************************************************
*
*                                              APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename          :  uif_usart.c
* Version           :  V0.0.2
* Programmer(s)     :  Leo
*********************************************************************************************************
* Note(s)           : usartx communicate implement
*********************************************************************************************************
* History           : 2017-03-28 : add usart receiving timeout parameter;
*********************************************************************************************************
*/

#include "uif_usart.h"



uint8_t usart0Buffer[ 2 ][ USART_BUFFER_SIZE_1K ] = { 0 };
uint8_t usart1Buffer[ 2 ][ USART_BUFFER_SIZE_1K ] = { 0 };

extern sDmad g_dmad;
extern DataSource source_usart0;
extern DataSource source_usart1;

/*
*********************************************************************************************************
*                                               USART_ISR_DMA()
*
* Description : usart1 interrupt function
*
* Arguments   : none
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
#if UNUSED
static void USART_ISR_DMA( void )
{
  
}
#endif

/*
*********************************************************************************************************
*                                               _USART1_DmaTxCallback()
*
* Description : usart1 tx dma callback funtion
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _USART1_DmaTxCallback( uint8_t status, void* pArg )
{
    status = status;
    pArg = pArg;
//    BSP_LED_Toggle( 3 );
    //not sure what should doing,left blank here;
}


/*
*********************************************************************************************************
*                                               _USART0_DmaTxCallback()
*
* Description : usart0 tx dma callback funtion
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _USART0_DmaTxCallback( uint8_t status, void* pArg )
{
    status = status;
    pArg = pArg;
}

/*
*********************************************************************************************************
*                                               _USART1_DmaRxCallback()
*
* Description : usart1 rx dma callback funtion
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _USART1_DmaRxCallback( uint8_t status, void* pArg )
{
    status = status;
    pArg = pArg;
}

/*
*********************************************************************************************************
*                                               _USART0_DmaRxCallback()
*
* Description : usart0 rx dma callback funtion
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _USART0_DmaRxCallback( uint8_t status, void* pArg )
{
    status = status;
    pArg = pArg;
}

/*
*********************************************************************************************************
*                                               usart1_DmaTx()
*
* Description : write data via dma using usart1
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
uint8_t usart1_DmaTx( void *pInstance, const uint8_t *buf, uint32_t len )
{
    sDmaTransferDescriptor td;
    
    assert( NULL != pInstance );
    buf = buf;
    len = len;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;
    
    td.dwSrcAddr = (uint32_t) usart1Buffer[ 1 ];;
    td.dwDstAddr = (uint32_t)&pUsart->US_THR;
    td.dwCtrlA   = USART_BUFFER_SIZE_1K 
                   | DMAC_CTRLA_SRC_WIDTH_BYTE 
                   | DMAC_CTRLA_DST_WIDTH_BYTE;
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR
                   | DMAC_CTRLB_SIF_AHB_IF0 | DMAC_CTRLB_DIF_AHB_IF2
                   | DMAC_CTRLB_FC_MEM2PER_DMA_FC
                   | DMAC_CTRLB_SRC_INCR_INCREMENTING
                   | DMAC_CTRLB_DST_INCR_FIXED;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer(&g_dmad, pSource->dev.txDMAChannel, &td);
    DMAD_StartTransfer(&g_dmad, pSource->dev.txDMAChannel);
    
    return 0;
}


/*
*********************************************************************************************************
*                                               usart0_DmaTx()
*
* Description : write data via dma using usart0
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
uint8_t usart0_DmaTx( void *pInstance, const uint8_t *buf, uint32_t len )
{
    sDmaTransferDescriptor td;
    
    assert( NULL != pInstance );
    buf = buf;
    len = len;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;
    
    td.dwSrcAddr = (uint32_t) usart0Buffer[ 1 ];;
    td.dwDstAddr = (uint32_t)&pUsart->US_THR;
    td.dwCtrlA   = USART_BUFFER_SIZE_1K 
                   | DMAC_CTRLA_SRC_WIDTH_BYTE 
                   | DMAC_CTRLA_DST_WIDTH_BYTE;
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR
                   | DMAC_CTRLB_SIF_AHB_IF0 | DMAC_CTRLB_DIF_AHB_IF2
                   | DMAC_CTRLB_FC_MEM2PER_DMA_FC
                   | DMAC_CTRLB_SRC_INCR_INCREMENTING
                   | DMAC_CTRLB_DST_INCR_FIXED;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer(&g_dmad, pSource->dev.txDMAChannel, &td);
    DMAD_StartTransfer(&g_dmad, pSource->dev.txDMAChannel);
    
    return 0;
}

/*
*********************************************************************************************************
*                                               usart1_DmaRx()
*
* Description : read data via dma using usart1
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
uint8_t usart1_DmaRx( void *pInstance,const uint8_t *buf, uint32_t len )
{
    sDmaTransferDescriptor td;
    
    assert( NULL != pInstance );
    buf = buf;
    len = len;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;

    td.dwSrcAddr = (uint32_t)&pUsart->US_RHR;
    td.dwDstAddr = (uint32_t) usart1Buffer[ 0 ];
    td.dwCtrlA   = USART_BUFFER_SIZE_1K 
                   | DMAC_CTRLA_SRC_WIDTH_BYTE 
                   | DMAC_CTRLA_DST_WIDTH_BYTE;
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR 
                   | DMAC_CTRLB_DST_DSCR
                   | DMAC_CTRLB_SIF_AHB_IF2 
                   | DMAC_CTRLB_DIF_AHB_IF0
                   | DMAC_CTRLB_FC_PER2MEM_DMA_FC
                   | DMAC_CTRLB_SRC_INCR_FIXED
                   | DMAC_CTRLB_DST_INCR_INCREMENTING;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer( &g_dmad, pSource->dev.rxDMAChannel, &td );
    DMAD_StartTransfer( &g_dmad, pSource->dev.rxDMAChannel );
    
    return 0;
}

/*
*********************************************************************************************************
*                                               usart0_DmaRx()
*
* Description : read data via dma using usart0
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
uint8_t usart0_DmaRx( void *pInstance,const uint8_t *buf, uint32_t len )
{
    sDmaTransferDescriptor td;
    
    assert( NULL != pInstance );
    buf = buf;
    len = len;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;

    td.dwSrcAddr = (uint32_t)&pUsart->US_RHR;
    td.dwDstAddr = (uint32_t) usart0Buffer[ 0 ];
    td.dwCtrlA   = USART_BUFFER_SIZE_1K 
                   | DMAC_CTRLA_SRC_WIDTH_BYTE 
                   | DMAC_CTRLA_DST_WIDTH_BYTE;
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR 
                   | DMAC_CTRLB_DST_DSCR
                   | DMAC_CTRLB_SIF_AHB_IF2 
                   | DMAC_CTRLB_DIF_AHB_IF0
                   | DMAC_CTRLB_FC_PER2MEM_DMA_FC
                   | DMAC_CTRLB_SRC_INCR_FIXED
                   | DMAC_CTRLB_DST_INCR_INCREMENTING;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer( &g_dmad, pSource->dev.rxDMAChannel, &td );
    DMAD_StartTransfer( &g_dmad, pSource->dev.rxDMAChannel );
    
    return 0;
}

/*
*********************************************************************************************************
*                                               _usart1_ConfigureDma()
*
* Description : usart1 dma configure
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : Don't use this interface,it is hard to intergrate
*********************************************************************************************************
*/

#if UNUSED
static void _ConfigureDma( void* pInstance )
{
    uint32_t dwCfg;
    uint8_t iController;
    
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    
    /* Driver initialize */
    DMAD_Initialize( &g_dmad, 0 );
    /* IRQ configure */
    IRQ_ConfigureIT( ID_DMAC0, 0, USART_ISR_DMA );
    IRQ_EnableIT( ID_DMAC0 );

    /* Allocate DMA channels for USART */
    pSource->dev.txDMAChannel = DMAD_AllocateChannel( &g_dmad, DMAD_TRANSFER_MEMORY, pSource->dev.identify );
    pSource->dev.rxDMAChannel = DMAD_AllocateChannel( &g_dmad, pSource->dev.identify, DMAD_TRANSFER_MEMORY);
    if (   pSource->dev.txDMAChannel == DMAD_ALLOC_FAILED || pSource->dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }
    /* Set RX callback */
    DMAD_SetCallback(&g_dmad, pSource->dev.rxDMAChannel,(DmadTransferCallback)_USART1_DmaRxCallback, 0);

    /* Configure DMA RX channel */
    iController = ( pSource->dev.rxDMAChannel >> 8);
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
              DMAIF_Get_ChannelNumber( iController, pSource->dev.identify, DMAD_TRANSFER_RX ))
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, pSource->dev.rxDMAChannel, dwCfg );

    /* Configure DMA TX channel */
    iController = ( pSource->dev.txDMAChannel >> 8 );
    dwCfg = 0 
           | DMAC_CFG_DST_PER(
              DMAIF_Get_ChannelNumber( iController, pSource->dev.identify, DMAD_TRANSFER_TX ))
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, pSource->dev.txDMAChannel, dwCfg );
}
#endif

/*
*********************************************************************************************************
*                                               _configureUsart()
*
* Description : usart1  configure
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
static void _configureUsart( void *pInstance )
{ 
    const uint16_t receivedTimeOut = 500;
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;
    
    uint32_t mode =     US_MR_USART_MODE_NORMAL
                        | US_MR_USCLKS_MCK
                        | US_MR_CHRL_8_BIT
                        | US_MR_PAR_NO
                        | US_MR_NBSTOP_1_BIT
                        | US_MR_CHMODE_NORMAL ;

    /* Enable the peripheral clock in the PMC*/
    PMC_EnablePeripheral( pSource->dev.identify );

    /* Configure the USART in the desired mode @115200 bauds*/
    USART_Configure( pUsart, mode, 115200, BOARD_MCK ) ;

    /* Configure the interrupt */
    /* Processed in DMA callback, no USART interrupt */
    USART_SetReceivedTimeout( pUsart,receivedTimeOut );
    /* Enable receiver & transmitter*/
    USART_SetTransmitterEnabled( pUsart, 1 ) ;
    USART_SetReceiverEnabled( pUsart, 1 ) ;

}

/*
*********************************************************************************************************
*                                               usart_init()
*
* Description : usart  initialize
*
* Arguments   : pInstance    : data source handle
*               parameter    : unused
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void usart_init( void *pInstance , void *parameter )
{
      assert( NULL != pInstance );
      
      parameter = parameter;
      
      static const Pin pins0[] = { PINS_USART0 };
      static const Pin pins1[] = { PINS_USART1 };
      
      DataSource *pSource = ( DataSource * )pInstance;
      
      if( pSource->dev.identify == ID_USART0 )
      {
          /* Configure pins*/
          PIO_Configure( pins0, PIO_LISTSIZE( pins0 ) ) ;
      }
      else
      {
          PIO_Configure( pins1, PIO_LISTSIZE( pins1 ) ) ;
      } 
      
      _configureUsart( pInstance );           
}
