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

static void USART0_IrqHandler( void )
{
  printf( "here, usart1 interrupted\r\n" );
}
  
static void USART1_IrqHandler( void )
{
  printf( "here, usart1 interrupted\r\n" );
}

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
    printf( "here, usart1 interrupted\r\n" );
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
    
    td.dwSrcAddr = (uint32_t) buf;
    td.dwDstAddr = (uint32_t)&pUsart->US_THR;
    td.dwCtrlA   = len 
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
    
    td.dwSrcAddr = (uint32_t) buf;
    td.dwDstAddr = (uint32_t)&pUsart->US_THR;
    td.dwCtrlA   = len 
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


unsigned char UART0_WriteBuffer_API( unsigned char *pdata, unsigned int size )
{
    unsigned char err;

    err = usart0_DmaTx( &source_usart0, (const uint8_t *)pdata, size );

    return err;

}

/*
*********************************************************************************************************
*                                               usartx_Read()
*
* Description : read data via polling method
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
uint16_t usartx_Read( void *pInstance,const uint8_t *buf, uint32_t len )
{  
    assert( NULL != pInstance );
    assert( NULL != buf );
    
    uint32_t cnt = 0;
    const uint32_t timeout = 500;
    uint16_t data = 0;
    
    uint8_t *pBuf = ( uint8_t * )buf;
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;

    for( cnt = 0 ; cnt < len ; cnt ++ )
    {
        data = Custom_USART_Read( pUsart, timeout );     
        
        if( -1 != data )                               //if not timeout
          *pBuf++ = data && 0xff;                      //send data to buffer
        else
        {
          //cnt--;                                       // 
          break;                                       //otherwise,break
        }                                              //and return bytes
    }        
    return cnt;
}

/*
*********************************************************************************************************
*                                               usartx_Read()
*
* Description : read data via polling method
*
* Arguments   : pInstance    : data source handle
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
uint16_t usartx_Write( void *pInstance,const uint8_t *buf, uint32_t len )
{  
    assert( NULL != pInstance );
    assert( NULL != buf );
    
    uint32_t cnt = 0;
    const uint32_t timeout = 500;
    uint16_t ret = 0;

    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;

    for( cnt = 0 ; cnt < len ; cnt ++ )
    {
        if( -1 != Custom_USART_Write( pUsart, *buf++, timeout )  )
           break;
    }        
    return cnt;
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
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;

    td.dwSrcAddr = (uint32_t)&pUsart->US_RHR;
    td.dwDstAddr = ( uint32_t )buf;
    td.dwCtrlA   = len 
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
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;

    td.dwSrcAddr = (uint32_t)&pUsart->US_RHR;
    td.dwDstAddr = ( uint32_t )buf;
    td.dwCtrlA   = len 
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
    const uint16_t receivedTimeOut = 5;
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Usart *pUsart = ( Usart * )pSource->dev.instanceHandle;
    
    uint32_t mode =     US_MR_USART_MODE_NORMAL
//    uint32_t mode =     US_MR_USART_MODE_HW_HANDSHAKING
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
    USART_SetReceivedTimeout( pUsart,receivedTimeOut );    
    USART_EnableReceivedTimeout( pUsart,1 );
    USART_EnableIt(pUsart,( 1 << 8 ) );
    USART_SetReceivedTimeout( pUsart,receivedTimeOut );
    if( pSource->dev.identify == ID_USART0 )
          IRQ_ConfigureIT( ID_USART0, 0, USART0_IrqHandler );
    else if( pSource->dev.identify == ID_USART1 )
         IRQ_ConfigureIT( ID_USART1, 0, USART1_IrqHandler );  
    else
      ;
    
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
