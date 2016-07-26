/*
*********************************************************************************************************
*
*                                              APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Unified EVM Interface Board
*
* Filename          :  uif_spi.c
* Version           :  V0.0.1
* Programmer(s)     : Leo
*********************************************************************************************************
* Note(s)           : spi communicate implement
*********************************************************************************************************
*/

#include "uif_spi.h"
#include  <ucos_ii.h>

#define STATE_SLAVE 0
#define STATE_MASTER ( !STATE_SLAVE )
#define SPI_BUFFER_SIZE ( 4096 )

extern sDmad g_dmad;
sDmad spi_dmad;

//const Pin spi0_pins[] = { PINS_SPI0,PIN_SPI0_NPCS0};					 
//const Pin spi1_pins[] = { PINS_SPI1,PIN_SPI1_NPCS0};
const Pin spi0_pins[] = { PINS_SPI0 };					 
const Pin spi1_pins[] = { PINS_SPI1 };                     



/*
*********************************************************************************************************
*                                               _ConfigureTC0()
*
* Description : config TC for spi action per 10ms
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : Configure TC0 to generate an interrupt every 4ms
*********************************************************************************************************
*/
extern void BSP_LED_On (uint32_t led);
extern void BSP_LED_Off (uint32_t led);

void ISR_TC1( void )
{
    volatile static uint8_t state = 0;
  
    Tc *pTc1 = TC1;
    volatile uint32_t status = pTc1->TC_CHANNEL[0].TC_SR;
    
    OS_CPU_SR cpu_sr;
  
    OS_ENTER_CRITICAL();
    if( 0 == state )
    {
//      BSP_LED_Off( 3 );
    }
    else
    {
//      BSP_LED_On( 3 );
    }
  
    state = 1 - state;
    OS_EXIT_CRITICAL();
  
  
}
/*
*********************************************************************************************************
*                                               _ConfigureTC0()
*
* Description : config TC for spi action per 10ms
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : Configure TC0 to generate an interrupt every 4ms
*********************************************************************************************************
*/
void _ConfigureTc1( uint32_t hz )
{
    uint32_t div, tcclks;
    
    assert( 0 != hz );

    /* Enable TC1 peripheral */
    PMC_EnablePeripheral(ID_TC1);
    
    /* Configure TC0 for 250Hz frequency and trigger on RC compare */
    TC_FindMckDivisor(hz, BOARD_MCK, &div, &tcclks, BOARD_MCK);
    TC_Configure(TC1, 0, tcclks | TC_CMR_CPCTRG);
    TC1->TC_CHANNEL[0].TC_RC = (BOARD_MCK / div) / hz;
    
    /* Configure and enable interrupt on RC compare */
    IRQ_ConfigureIT(ID_TC1, 0, ISR_TC1);
    IRQ_EnableIT(ID_TC1);
    
    TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
}


/*
*********************************************************************************************************
*                                               _DmaRxCallback()
*
* Description : spi callback function for DMA receiving
*
* Arguments   : status: dma channel state of perivious transmmit.
*               pArg  : instance of data source
* Returns     : none.
*
* Note(s)     : none
*********************************************************************************************************
*/
void _SPI1_DmaRxCallback( uint8_t status, void* pArg )
{
  
    status = status;
    pArg = pArg;    
}


/*
*********************************************************************************************************
*                                               _DmaRxCallback()
*
* Description : spi callback function for DMA transmitting
*
* Arguments   : status: dma channel state of perivious transmmit.
*               pArg  : instance of data source
* Returns     : none.
*
* Note(s)     : none
*********************************************************************************************************
*/
void  _SPI1_DmaTxCallback( uint8_t status, void* pArg )
{
    status = status;
    pArg = pArg;
    
//    BSP_LED_Toggle( 3 );
}


/*
*********************************************************************************************************
*                                               register_spi()
*
* Description : register spi port to system    
*
* Arguments   : mask: the global mask of system;
*               type  : the current port type;
* Returns     : the new global mask
*
* Note(s)     : none
*********************************************************************************************************
*/
uint32_t register_spi( uint32_t mask, UIFPORT *type )
{
	assert( NULL != type );
	assert((*type ) < INVALIDPORT );
	
	return mask |= ( 1 << ( *type ) ); 


}

/*
*********************************************************************************************************
*                                               _get_spi_instance()
*
* Description : helper function
*
* Arguments   : id    : peripheral ID
*               
* Returns     : the peripheral handle of spi object; 
*
* Note(s)     : none
*********************************************************************************************************
*/
Spi * _get_spi_instance( uint32_t id )
{
#define MAXCHIP_ID 50         //datasheet page 38;
	
        assert( id < MAXCHIP_ID );
	
	if( ( id != (uint32_t )ID_SPI0 ) && ( id != (uint32_t )ID_SPI1 ) )
                return NULL;
		
	return ( ID_SPI0 == id ) ?  SPI0 :  SPI1;
}


/*
*********************************************************************************************************
*                                               start_spi()
*
* Description : start spi port to transmmit
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none 
*
* Note(s)     : none
*********************************************************************************************************
*/
void start_spi( void * pInstance )
{
     assert( NULL != pInstance );
}



/*
*********************************************************************************************************
*                                               stop_spi()
*
* Description : stop spi port to transmmit
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void stop_spi( void * pInstance )
{
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Spi *pSpi = ( Spi * )pSource->dev.instanceHandle;
  
    SPI_Disable( pSpi );
    
    DMAD_StopTransfer( &spi_dmad, pSource->dev.rxDMAChannel );
    DMAD_StopTransfer( &spi_dmad, pSource->dev.txDMAChannel );
    
    pSource->status = ( uint8_t )STOP;
}



/*
*********************************************************************************************************
*                                               _ConfigureSpi()
*
* Description : spi port initialize 
*
* Arguments   : pInstance    : data source handle
*               spiState     : role of spi port, master or slave
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
static void _ConfigureSpi( DataSource *pInstance,uint32_t spiState,uint32_t clk )
{
    assert( NULL != pInstance );
    assert( clk > 0 );
  
    uint32_t mode = SPI_MR_MSTR ;
    uint32_t csr0 ;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Spi *pSpi = _get_spi_instance( pSource->dev.identify );

    // spi1 in slave mode 
    if ( spiState == STATE_SLAVE )
    {
        mode &= ( uint32_t ) ( ~( SPI_MR_MSTR ) ) ;
    }
    
    csr0 = SPI_CSR_BITS_8_BIT|SPI_CSR_DLYBS( 0x0 ) ;
    
    // spi clock 
    if ( spiState == STATE_MASTER )
    {
        csr0 |= ( ( BOARD_MCK / clk ) << 8 ) ;
    }
    
    // configure SPI mode 
    SPI_Configure( pSpi, pSource->dev.identify, mode ) ;
    
    // configure SPI csr0
    SPI_ConfigureNPCS( pSpi, 0, csr0 ) ;
    SPI_Enable( pSpi ) ;

}




/*
*********************************************************************************************************
*                                               init_spi()
*
* Description : initialize spi port with dma
*
* Arguments   : pInstance    : data source handle 
*               parameter    : unused
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void init_spi( void *pInstance,void *parameter )
{
    assert( NULL != pInstance );
    
    parameter = parameter;
    DataSource *pSource = ( DataSource * )pInstance;
    VOICE_BUF_CFG *pSpi_Cfg = ( VOICE_BUF_CFG * )parameter;
    
    if( ID_SPI0 == pSource->dev.identify )
      PIO_Configure( spi0_pins, PIO_LISTSIZE( spi0_pins ) ) ;                                              //fill code initialize spi0 pins
    else if( ID_SPI1 == pSource->dev.identify )
      PIO_Configure( spi1_pins, PIO_LISTSIZE( spi1_pins ) ) ;
     
    PMC_EnablePeripheral( pSource->dev.identify );    
    _ConfigureSpi( pSource,pSpi_Cfg->spi_mode ,pSpi_Cfg->spi_speed ) ;
}


/*
*********************************************************************************************************
*                                               ISR_SPI_DMA0()
*
* Description : ISR for DMA0 interrupt
*
* Arguments   : none
*               
* Returns     : none
*
* Note(s)     : unused
*********************************************************************************************************
*/
#ifdef USE_DMA
#if UNUSED
static void ISR_SPI_DMA0( void )
{
    DMAD_Handler( &g_dmad );
}
#endif



/*
*********************************************************************************************************
*                                               ISR_SPI_DMA1()
*
* Description : ISR for DMA1 interrupt
*
* Arguments   : none
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
#if UNUSED
static void ISR_SPI_DMA1( void )
{
    DMAD_Handler( &g_dmad );
}
#endif

/*
*********************************************************************************************************
*                                               _spiDmaRx()
*
* Description : spi dma transmmit via dma
*
* Arguments   : pInstance    : data source handle
*               startBuffer  : buffer handle 
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _spiDmaRx( void *pInstance )
{
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Spi *pSpi = _get_spi_instance( pSource->dev.identify );
    
    sDmaTransferDescriptor td;
    td.dwSrcAddr = (uint32_t)&pSpi->SPI_RDR;
    td.dwDstAddr = (uint32_t) pSource->buffer;
    td.dwCtrlA   = SPI_BUFFER_SIZE | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
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
}




/*
*********************************************************************************************************
*                                               _spiDmaTx()
*
* Description : spi dma transmmit via dma
*
* Arguments   : pInstance    : data source handle
*               startBuffer  : buffer handle 
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _spiDmaTx( void *pInstance )
{  
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Spi *pSpi = _get_spi_instance( pSource->dev.identify );
    
    sDmaTransferDescriptor td;

    td.dwSrcAddr = (uint32_t) pSource->privateData;
    td.dwDstAddr = (uint32_t)&pSpi->SPI_TDR;
    td.dwCtrlA   = SPI_BUFFER_SIZE | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR 
                   | DMAC_CTRLB_DST_DSCR
                   | DMAC_CTRLB_SIF_AHB_IF0 
                   | DMAC_CTRLB_DIF_AHB_IF2
                   | DMAC_CTRLB_FC_MEM2PER_DMA_FC
                   | DMAC_CTRLB_SRC_INCR_INCREMENTING
                   | DMAC_CTRLB_DST_INCR_FIXED;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer( &g_dmad, pSource->dev.txDMAChannel, &td );
    DMAD_StartTransfer( &g_dmad, pSource->dev.txDMAChannel );
  
}




/*
*********************************************************************************************************
*                                               _spi_ConfigureDma()
*
* Description : spi dma configure
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
#if UNUSED
static void _spi_ConfigureDma( void *pInstance )
{
    uint32_t dwCfg;
    uint8_t iController;
    
    assert( NULL != pInstance );    
    
    DataSource *pSource = ( DataSource * )pInstance;
    Spi * pSpi = _get_spi_instance( pSource->dev.identify );
    
    /* Driver initialize */
    DMAD_Initialize( &g_dmad, 0 );
    /* IRQ configure */
    IRQ_ConfigureIT( ID_DMAC0, 0, ISR_SPI_DMA0 );
    IRQ_EnableIT( ID_DMAC0 );
    IRQ_ConfigureIT( ID_DMAC1, 0, ISR_SPI_DMA1 );
    IRQ_EnableIT( ID_DMAC1 );

    
    /* Allocate DMA channels for SPI instance */
    pSource->dev.txDMAChannel = DMAD_AllocateChannel( &g_dmad,
                                              DMAD_TRANSFER_MEMORY, pSource->dev.identify );
    pSource->dev.rxDMAChannel = DMAD_AllocateChannel( &g_dmad,
                                              pSource->dev.identify, DMAD_TRANSFER_MEMORY );
    if (   pSource->dev.txDMAChannel == DMAD_ALLOC_FAILED 
        || pSource->dev.rxDMAChannel == DMAD_ALLOC_FAILED )
    {
        printf("DMA channel allocat error\n\r");
        while(1);
    }
    /* Set RX callback */
    DMAD_SetCallback( &g_dmad, pSource->dev.rxDMAChannel,
                    (DmadTransferCallback)_SPI1_DmaRxCallback, 0 );
    /* Configure DMA RX channel */
    iController = ( pSource->dev.rxDMAChannel >> 8 );
    dwCfg = 0
           | DMAC_CFG_SRC_PER(
                DMAIF_Get_ChannelNumber( iController, pSource->dev.identify, DMAD_TRANSFER_RX )& 0x0F)
           | DMAC_CFG_SRC_PER_MSB(
                (DMAIF_Get_ChannelNumber( iController, pSource->dev.identify, DMAD_TRANSFER_RX )& 0xF0) >> 4 )
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, pSource->dev.rxDMAChannel, dwCfg );
    /* Configure DMA TX channel */
    iController = ( pSource->dev.txDMAChannel >> 8 );
    dwCfg = 0           
           | DMAC_CFG_DST_PER(
                DMAIF_Get_ChannelNumber( iController, pSource->dev.identify, DMAD_TRANSFER_TX )& 0x0F )
           | DMAC_CFG_DST_PER_MSB(
                (DMAIF_Get_ChannelNumber( iController, pSource->dev.identify, DMAD_TRANSFER_TX )& 0xF0 ) >> 4 )
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &g_dmad, pSource->dev.txDMAChannel, dwCfg );

}
#endif
#endif



/*
*********************************************************************************************************
*                                               spi_clear_status()
*
* Description : clear spi supend status bit
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void spi_clear_status( void * pInstance )
{
    if( NULL == pInstance )
      return;
    
    DataSource *pSource = ( DataSource * )pInstance;    
    Spi * pSpi = _get_spi_instance( pSource->dev.identify );
    
    pSpi->SPI_RDR ;
}



/*
*********************************************************************************************************
*                                               spi_slave_transfer()
*
* Description : spi transmmit function under slave state
*
* Arguments   : pInstance : device instance handle
*             : p_tbuf    : tx buffer handle
*             : p_rbuf    : rx buffer handle
*             : size     : size of rx buffer 
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
#if UNUSED
static void spi_slave_transfer( void *pInstance,
                                void *p_tbuf,
				void *p_rbuf,
                                uint32_t size )
{


}
#endif							   



/*
*********************************************************************************************************
*                                               spi_master_transfer()
*
* Description : spi transmmit function under Master state
*
* Arguments   : pInstance : device instance handle
*             : p_tbuf    : tx buffer handle
*             : p_rbuf    : rx buffer handle
*             : size     : size of rx buffer 
* Returns     : none
*
* Note(s)     : the size of p_tbuf is equl p_rbuf;
*********************************************************************************************************
*/
#if UNUSED
static void spi_master_transfer( void *pInstance,
                                 void *p_tbuf,
				 void *p_rbuf,
                                 uint32_t size )
{
        assert( ( NULL != pInstance ) );
        assert( ( NULL != p_tbuf ) && ( 0 < size ) );
        assert( ( NULL != p_tbuf ) );
         
        DataSource *pSource = ( DataSource * )pInstance;
        
        spi_clear_status( pSource );
        _spiDmaRx( pSource );
        _spiDmaTx( pSource );

}
#endif
