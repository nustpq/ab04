/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
*
* Filename      : uif_i2s.c
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/
#include "uif_object.h"
#include "defined.h"
#include "uif_i2s.h"
#include "Kfifo.h"
#include "uif_usb.h"

#ifndef USE_UCOS
#include  <ucos_ii.h>
#endif

#define DATA_TRANSMIT_TRACE 0

extern DataSource soruce_ssc0;
extern DataSource source_ssc1;


//static uint8_t mutex = 0;

/* Tx descriptors */
sDmaTransferDescriptor dmaTdSSC0Rx[2];
sDmaTransferDescriptor dmaTdSSC0Tx[2];

sDmaTransferDescriptor dmaTdSSC1Rx[2];
sDmaTransferDescriptor dmaTdSSC1Tx[2];

#ifdef USE_BACKUP_DEBUG_INFO
static const char* DMA_INFO[ DMAD_CANCELED + 1 ] = { "DMAD_OK",
						     "DMAD_BUSY",
						     "DMAD_PARTITAL_DONE",
						     "DMAD_ERROR",
						     "DMAD_CANCELED"
						    };
#endif

extern uint32_t g_portMask;
extern void BSP_LED_On( uint32_t );
extern void BSP_LED_Off( uint32_t );

/*
*********************************************************************************************************
*                                               ISR_HDMA()
*
* Description : DMA interrupt handle
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#ifdef USE_DMA
void ISR_HDMA( void )
{  
    OS_CPU_SR cpu_sr;

    OS_ENTER_CRITICAL();
    DMAD_Handler(&g_dmad);
    OS_EXIT_CRITICAL();

}
#endif

/*
*********************************************************************************************************
*                                               _get_ssc_instance()
*
* Description : get ssc instance handle 
*
* Arguments   : id   :peripheral ID
* Returns     : ptr point to Ssc instance
*
* Note(s)     : none.
*********************************************************************************************************
*/
Ssc * _get_ssc_instance( uint32_t id )
{
#define MAXCHIP_ID 50         //datasheet page 38;

	assert( id < MAXCHIP_ID );

	if( ( id != (uint32_t )ID_SSC0 ) && ( id != (uint32_t )ID_SSC1 ) )
		return NULL;
	
	return ( ID_SSC0 == id ) ?  SSC0 :  SSC1;

}

/*
*********************************************************************************************************
*                                               stop_ssc()
*
* Description : high level ssc port disable with dma
*
* Arguments   : pInstance   :datasource handle
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
extern void kfifo_reset( kfifo_t *fifo );
void stop_ssc( void *pInstance )
{
    pInstance = pInstance;
    DataSource *pSource = ( DataSource *)pInstance;
    
    Ssc *pSSC = ( Ssc * )pSource->dev.instanceHandle;
 
    //step1: stop ssc port
    SSC_DisableTransmitter( pSSC );
    SSC_DisableReceiver( pSSC );
    
    //step2: stop dma channel
    DMAD_StopTransfer(&g_dmad, pSource->dev.rxDMAChannel);
    DMAD_StopTransfer(&g_dmad, pSource->dev.txDMAChannel);
    
    //step3:clear buffer about this port
    memset( pSource->pBufferIn, 0 , sizeof( uint16_t ) * I2S_PINGPONG_IN_SIZE_3K );
    memset( pSource->pBufferOut, 0 , sizeof( uint16_t ) * I2S_PINGPONG_OUT_SIZE_3K );
    kfifo_reset( pSource->pRingBulkIn );
    kfifo_reset( pSource->pRingBulkOut );
    
    //step4:reset port state machine
    pSource->status[ IN ] = ( uint8_t )STOP;
    pSource->status[ OUT ] = ( uint8_t )STOP;
}



/*
*********************************************************************************************************
*                                               register_ssc()
*
* Description : register ssc port to system
*
* Arguments   : mask   :global mask code;
*               type   :one of uif board port used;
* Returns     : mask of datasources
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint32_t register_ssc( uint32_t mask,UIFPORT *type )
{
	assert( NULL != type );
	assert((*type ) < INVALIDPORT );
	
	return mask |= ( 1 << ( *type )); 
}

/*
*********************************************************************************************************
*                                               _config_pins()
*
* Description : config pins for ssc instancr
*
* Arguments   : id     :ssc instance peripheral ID;
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void _config_pins( uint32_t id)
{
  static const Pin pins[] = { PIN_SSC0 };
  static const Pin pins1[] = { PIN_SSC1 };
  
	if( ID_SSC0 == id )
          PIO_Configure(pins, PIO_LISTSIZE( pins ) );
	else 
          PIO_Configure(pins1, PIO_LISTSIZE( pins1 ) );	
}


/*
*********************************************************************************************************
*                                               _SSCx_DmaRxCallback()
*
* Description : callback function for SSCx DMA Rx
*
* Arguments   : status     :reverse
*               pArf       :reverse
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef USE_DMA
void _SSC0_DmaRxCallback( uint8_t status, void *pArg)
{    
    assert( NULL != pArg );
    
    uint32_t temp;
    INT8U error;
    
    DataSource *pSource = ( DataSource *)pArg;

#ifdef ENABLE_PRINT 
      if (status != DMAD_OK) 
      { 
          printf("Rx DMA Status :%s,line:%d\r\n",DMA_INFO[ status ],__LINE__);
          return;
      }
#endif
      
     /*step 1:calculate buffer space */ 
     temp = kfifo_get_free_space( pSource->pRingBulkIn );

     /*step 2:merge buffer according condition */  
     if( temp >= pSource->warmWaterLevel )
     {
       ///Todo: 0xf should be instead with mask;
       source_gpio.buffer_read( &source_gpio, 
                                ( uint8_t * )&source_gpio.pBufferIn[ pSource-> rx_index ], 
                                 10 );            

       kfifo_put( pSource->pRingBulkIn,
                  ( uint8_t * )pSource->pBufferIn[ pSource-> rx_index ],
                  pSource->rxSize );
       
       kfifo_put( pSource->pRingBulkIn,
                  ( uint8_t * )source_gpio.pBufferIn[ pSource-> rx_index ],
                  source_gpio.rxSize );
 
       pSource->rx_index = 1 - pSource->rx_index;
       
       //update state machine of this port;                    
       pSource->status[ IN ] = ( uint8_t )RUNNING;
     }
     else
     {
            if( ( uint8_t )START <= pSource->status[ IN ] )
            {
                pSource->status[ IN ] = ( uint8_t )BUFFERED;
                    //error proccess;
            }
            else
            {
#if DATA_TRANSMIT_TRACE
               printf( "SSC0-Rx:There is No Space in Fifo,space size = (%d) \r\n",temp);
#endif               
               return;
            }
     }
     /*step3:copy data to buffer*/
#if 0     
     CDCDSerialDriver_ReadAudio_0(     usbCacheBulkOut0,                              \
                                USB_DATAEP_SIZE_64B,                           \
                                (TransferCallback) UsbAudio0DataReceived,      \
                                  0);
     CDCDSerialDriver_WriteAudio_0(    usbCacheBulkIn0,                               \
                                 USB_DATAEP_SIZE_64B,                          \
                               (TransferCallback) UsbAudio0DataTransmit,       \
                               0);
#endif     


   
}

/*
*********************************************************************************************************
*                                               _SSC1_DmaRxCallback()
*
* Description : ssc1 port rx dma callback fuction
*
* Arguments   : status    : transmmit of previous result
*               pArg      : instance of ssc
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _SSC1_DmaRxCallback( uint8_t status, void *pArg)
{    
    assert( NULL != pArg );
    INT8U error;
    uint32_t temp = 0;
      
    DataSource *pSource = ( DataSource *)pArg;

#ifdef ENABLE_PRINT 
      if (status != DMAD_OK) 
      { 
          printf("Rx DMA Status :%s,line:%d\r\n",DMA_INFO[ status ],__LINE__);
          return;
      }
#endif
      
     /*step 1:calculate buffer space */ 
     temp = kfifo_get_free_space( pSource->pRingBulkIn );

     /*step 2:merge buffer according condition */  
     if( temp >= pSource->warmWaterLevel )
     {         
       kfifo_put( pSource->pRingBulkIn,
                  ( uint8_t * )pSource->pBufferIn[ pSource-> rx_index ],
                  pSource->rxSize );
 
       pSource->rx_index = 1 - pSource->rx_index; 
                   
       pSource->status[ IN ] = ( uint8_t )RUNNING;
     }
     else
     {
            if( ( uint8_t )START <= pSource->status[ IN ] )
            {
                pSource->status[ IN ] = ( uint8_t )BUFFERED;
                    //error proccess;
            }
            else
            {
#if DATA_TRANSMIT_TRACE              
               printf( "SSC1-Rx:There is No Space in Fifo,space size = (%d) \r\n",temp);
#endif               
               return;
            }
     }      
        
   
}
#endif

/*
*********************************************************************************************************
*                                               _SSC0_DmaTxCallback()
*
* Description : callback function for SSC0 DMA Tx
*
* Arguments   : status     :reverse
*               pArg       :reverse
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
#ifdef USE_DMA
#define TEST_BUF 0


void _SSC0_DmaTxCallback( uint8_t status, void *pArg)
{
    static uint8_t error;
    uint32_t temp = 0;
    static uint32_t ord = 0;

     
    assert( NULL != pArg );
    
    DataSource *pSource = ( DataSource *)pArg;
    Ssc *pSsc = _get_ssc_instance( pSource->dev.identify );

#ifdef ENABLE_PRINT 
    static int cnt;
    if (status != DMAD_OK) 
    { 
          printf("Tx DMA Status :%d-%s,line:%d,cnt(%d)\r\n",status,DMA_INFO[ status ],__LINE__,cnt++);
          return;
    } 
#endif

            
     pSource->pBufferOut = ( uint16_t * )&ssc0_PingPongOut[ 1 - pSource->tx_index ];
#if 1
     temp = kfifo_get_data_size( pSource->pRingBulkOut );
     if( temp  >=  pSource->txSize ) 
     {
          kfifo_get( pSource->pRingBulkOut,
                      ( uint8_t * )&pSource->pBufferOut[ pSource-> tx_index ],
                      pSource->txSize );
          pSource->tx_index = 1 - pSource->tx_index;
          //update state machine of this port;                    
          pSource->status[ OUT ] = ( uint8_t )RUNNING;
#if DATA_TRANSMIT_TRACE 
                     ord ++;
                     ord %= 10000;
                     printf( "SSC0-Tx( %d ):data size = ( %d ) \r\n",ord,temp);
#endif          
     }
     else
     {
            //if this port was started,but no enough data in buffer,
            //flag this port in buffering state;
            if( ( uint8_t )START == pSource->status[ OUT ] 
                    || ( uint8_t )BUFFERED == pSource->status[ OUT ] )               
            {
                    pSource->status[ OUT ] = ( uint8_t )BUFFERED;
                    //Todo : error proccess--here do nothing
#if DATA_TRANSMIT_TRACE
                     printf( "SSC0-Tx:Data buffering,data size = (%d) \r\n",temp);
#endif                     
                    return;
            }
            else if( ( uint8_t )RUNNING == pSource->status[ OUT ] )
            {       
                  if( temp  >=  pSource->txSize * 2 ) 
                  {
                          kfifo_get( pSource->pRingBulkOut,
                                      ( uint8_t * )&pSource->pBufferOut[ pSource-> tx_index ],
                                      pSource->txSize );
                          pSource->tx_index = 1 - pSource->tx_index;
                                                 
                  }
                  else
                  {
#if DATA_TRANSMIT_TRACE
                    printf( "SSC0-Tx:There is No Data in RingBuffer,data size = (%d) \r\n",temp);
#endif                    
                            ///Todo: error proccess
                            // filled invalid data to ringbuffer and send it to pc that is a tip;
                            return;
                  }
            }
            else
            {       //
#if DATA_TRANSMIT_TRACE
//                    printf( "SSC0-Tx:Port not ready!\n");
#endif                    
                    //port machine state is wrong, firmware has bug;
                    assert( 0 );
                    return;
            }
     }
#endif

     
}

/*
*********************************************************************************************************
*                                               _SSC1_DmaTxCallback()
*
* Description : callback function for SSC1 DMA Tx
*
* Arguments   : status       : reserved
*               pArg         : reserved
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void _SSC1_DmaTxCallback( uint8_t status, void *pArg)
{
      static uint8_t error;
      uint32_t temp = 0;

   
    assert( NULL != pArg );
    
    DataSource *pSource = ( DataSource *)pArg;
    Ssc *pSsc = _get_ssc_instance( pSource->dev.identify );

#ifdef ENABLE_PRINT    
    if (status != DMAD_OK) 
    { 
          printf("Tx DMA Status :%d-%s,line:%d,cnt(%d)\r\n",status,DMA_INFO[ status ],__LINE__,cnt++);
          return;
    } 
#endif

     //step1: switch Ping-Pong buffer to empty part;
     pSource->pBufferOut = ( uint16_t * )ssc1_PingPongOut[ 1 - pSource->tx_index ];  
     //step2: calculate data size of ringbuffer;
     temp = kfifo_get_data_size( pSource->pRingBulkOut );
#if DATA_TRANSMIT_TRACE              
//               printf( "SSC1-Tx:data size = (%d) -------->\r\n",temp);
#endif       
     //step3: copy data to ringbuffer, this will prepare data for usb ringbuffer;
     if( temp  >=  pSource->txSize ) 
     {
          kfifo_get( pSource->pRingBulkOut,
                      ( uint8_t * )pSource->pBufferOut[ pSource-> tx_index ],
                      pSource->txSize  );
          pSource->tx_index = 1 - pSource->tx_index;  
          
          // change port machine states;
          pSource->status[ OUT ] = ( uint8_t )RUNNING;
     }
     else
     {
            //if this port was started,but no enough data in buffer,
            //flag this port in buffering state;
            if( ( uint8_t )START == pSource->status[ OUT ] 
                    || ( uint8_t )BUFFERED == pSource->status[ OUT ] )               
            {
                    pSource->status[ OUT ] = ( uint8_t )BUFFERED;
                    //error proccess;
#if DATA_TRANSMIT_TRACE 
//                     ord ++;
//                     ord %= 10000;
//                     printf( "SSC1-Tx( %d ):Data buffering,data size = (%d) \r\n",ord,temp);
#endif                     
                    return;
            }
            else if( ( uint8_t )RUNNING == pSource->status[ OUT ] )
            { 
#if DATA_TRANSMIT_TRACE              
                    printf( "SSC1-Tx:There is No Data in RingBuffer,data size = (%d) \r\n",temp);
#endif                    
                    return;
            }
            else
            {
#if DATA_TRANSMIT_TRACE              
                    printf( "SSC1-Tx:Port not ready!\n");
#endif                    
                    assert( 0 );
                    return;
            }

     }
#endif          
  
}
#endif


#ifdef USE_DMA

/*
*********************************************************************************************************
*                                               ssc0_buffer_read()
*
* Description : transmmit via ssc0 using dma
*
* Arguments   : pInstance     :datasource object
*               buf           :data
*               len           :size of data in bytes
* Returns     : none
*
* Note(s)     : it is NOT reentrant,and will use this interface to instead SSC0_Recording
*********************************************************************************************************
*/
uint8_t ssc0_buffer_read( void *pInstance,const uint8_t *buf,uint32_t len )
{ 
        assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC0Rx;
        
        Ssc* pSsc = _get_ssc_instance( pSource->dev.identify );
        
        pTds[0].dwSrcAddr = ( uint32_t )&SSC0->SSC_RHR;
        pTds[0].dwDstAddr = ( uint32_t )buf; 
        pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                               | DMAC_CTRLA_SRC_WIDTH_HALF_WORD 
                               | DMAC_CTRLA_DST_WIDTH_HALF_WORD;          
        pTds[0].dwCtrlB   = DMAC_CTRLB_FC_PER2MEM_DMA_FC
                             | DMAC_CTRLB_SRC_INCR_FIXED
                             | DMAC_CTRLB_DST_INCR_INCREMENTING
                             | DMAC_CTRLB_SIF_AHB_IF2
                             | DMAC_CTRLB_DIF_AHB_IF0
                             ;      
        pTds[0].dwDscAddr = (uint32_t) &pTds[1];
        
        pTds[1].dwSrcAddr = ( uint32_t )&SSC0->SSC_RHR;
        pTds[1].dwDstAddr = ( uint32_t )( buf + ( sizeof( ssc0_PingPongIn ) >> 2 ) ); 
        pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                               | DMAC_CTRLA_SRC_WIDTH_HALF_WORD 
                               | DMAC_CTRLA_DST_WIDTH_HALF_WORD;          
        pTds[1].dwCtrlB   = DMAC_CTRLB_FC_PER2MEM_DMA_FC
                             | DMAC_CTRLB_SRC_INCR_FIXED
                             | DMAC_CTRLB_DST_INCR_INCREMENTING
                             | DMAC_CTRLB_SIF_AHB_IF2
                             | DMAC_CTRLB_DIF_AHB_IF0
                             ;      
        pTds[1].dwDscAddr = (uint32_t) &pTds[0];
               
        /* Enable recording(SSC RX) */
        DMAD_PrepareMultiTransfer(&g_dmad, pSource->dev.rxDMAChannel, dmaTdSSC0Rx);
        DMAD_StartTransfer(&g_dmad, pSource->dev.rxDMAChannel);
        
        SSC_EnableReceiver(pSsc); 
        
        return 0;
}


/*
*********************************************************************************************************
*                                               ssc1_buffer_read()
*
* Description : transmmit via ssc1 using dma
*
* Arguments   : pInstance     :datasource object
*               buf           :data
*               len           :size of data in bytes
* Returns     : none
*
* Note(s)     : it is NOT reentrant,and will use this interface to instead SSC1_Recording
*********************************************************************************************************
*/
uint8_t ssc1_buffer_read( void *pInstance,const uint8_t *buf,uint32_t len )
{ 
        assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC1Rx;
        
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
        
        pTds[0].dwSrcAddr = ( uint32_t )&SSC1->SSC_RHR;
        pTds[0].dwDstAddr = ( uint32_t )buf; 
        pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                               | DMAC_CTRLA_SRC_WIDTH_HALF_WORD 
                               | DMAC_CTRLA_DST_WIDTH_HALF_WORD;        
        pTds[0].dwCtrlB   = DMAC_CTRLB_FC_PER2MEM_DMA_FC
                             | DMAC_CTRLB_SRC_INCR_FIXED
                             | DMAC_CTRLB_DST_INCR_INCREMENTING
                             | DMAC_CTRLB_SIF_AHB_IF2
                             | DMAC_CTRLB_DIF_AHB_IF0
                             ;      
        pTds[0].dwDscAddr = (uint32_t) &pTds[1];
        
        pTds[1].dwSrcAddr = ( uint32_t )&SSC1->SSC_RHR;
        pTds[1].dwDstAddr = ( uint32_t )( buf + ( len >> 1 ) ); 
        pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                               | DMAC_CTRLA_SRC_WIDTH_HALF_WORD 
                               | DMAC_CTRLA_DST_WIDTH_HALF_WORD;        
        pTds[1].dwCtrlB   = DMAC_CTRLB_FC_PER2MEM_DMA_FC
                             | DMAC_CTRLB_SRC_INCR_FIXED
                             | DMAC_CTRLB_DST_INCR_INCREMENTING
                             | DMAC_CTRLB_SIF_AHB_IF2
                             | DMAC_CTRLB_DIF_AHB_IF0
                             ;      
        pTds[1].dwDscAddr = (uint32_t) &pTds[0];
               
        /* Enable recording(SSC RX) */
        DMAD_PrepareMultiTransfer(&g_dmad, pSource->dev.rxDMAChannel, dmaTdSSC1Rx);
        DMAD_StartTransfer(&g_dmad, pSource->dev.rxDMAChannel);
        
        SSC_EnableReceiver(pSsc); 
        
        return 0;
}
#endif

#ifdef USE_DMA
/*
*********************************************************************************************************
*                                               ssc0_buffer_write()
*
* Description : buffer write via SSC0 
*
* Arguments   : pInstance     :datasource object handle
*               buf:          :buf will be writed;
*               len:          :bytes of buf;
* Returns     : none
*
* Note(s)     : it is NOT reentrant;
*             : The correct calculation of the length of buffer is the responsibility of the caller,
*             : For the sake of simplicity, i set DMA bit width is 16;
*********************************************************************************************************
*/
uint8_t ssc0_buffer_write( void *pInstance,const uint8_t *buf,uint32_t len )
{
	assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC0Tx;
        
               
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
	/* Setup TD list for TX */
        pTds[0].dwSrcAddr = (uint32_t) buf;               
	pTds[0].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;

	pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                            | DMAC_CTRLA_SRC_WIDTH_HALF_WORD | DMAC_CTRLA_DST_WIDTH_HALF_WORD;
	pTds[0].dwCtrlB   = 0
			    | DMAC_CTRLB_SIF_AHB_IF0
			    | DMAC_CTRLB_DIF_AHB_IF2
			    | DMAC_CTRLB_FC_MEM2PER_DMA_FC
			    | DMAC_CTRLB_SRC_INCR_INCREMENTING
			    | DMAC_CTRLB_DST_INCR_FIXED;
	pTds[0].dwDscAddr = (uint32_t) &pTds[1];
        
	pTds[1].dwSrcAddr = (uint32_t) ( buf + ( sizeof( ssc0_PingPongOut ) >> 2 ) ); 
	pTds[1].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
	pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                          | DMAC_CTRLA_SRC_WIDTH_HALF_WORD | DMAC_CTRLA_DST_WIDTH_HALF_WORD;
	pTds[1].dwCtrlB   = 0
			  | DMAC_CTRLB_SIF_AHB_IF0
			  | DMAC_CTRLB_DIF_AHB_IF2
			  | DMAC_CTRLB_FC_MEM2PER_DMA_FC
			  | DMAC_CTRLB_SRC_INCR_INCREMENTING
			  | DMAC_CTRLB_DST_INCR_FIXED;
	pTds[1].dwDscAddr = (uint32_t) &pTds[0];
               
        DMAD_PrepareMultiTransfer(&g_dmad, pSource->dev.txDMAChannel, dmaTdSSC0Tx);
        DMAD_StartTransfer(&g_dmad, pSource->dev.txDMAChannel);

        SSC_EnableTransmitter( pSsc );
        
        return 0;
}

/*
*********************************************************************************************************
*                                               ssc1_buffer_write()
*
* Description : buffer write via SSC1
*
* Arguments   : pInstance     :datasource object handle
*               buf:          :buf will be writed;
*               len:          :bytes of buf;
* Returns     : none
*
* Note(s)     : it is NOT reentrant;
*********************************************************************************************************
*/
uint8_t ssc1_buffer_write( void *pInstance,const uint8_t *buf,uint32_t len )
{
	assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC1Tx;
       
                
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
	/* Setup TD list for TX */
        pTds[0].dwSrcAddr = (uint32_t) buf;               
	pTds[0].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
	pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                            | DMAC_CTRLA_SRC_WIDTH_HALF_WORD 
                            | DMAC_CTRLA_DST_WIDTH_HALF_WORD;        
	pTds[0].dwCtrlB   = 0
			    | DMAC_CTRLB_SIF_AHB_IF0
			    | DMAC_CTRLB_DIF_AHB_IF2
			    | DMAC_CTRLB_FC_MEM2PER_DMA_FC
			    | DMAC_CTRLB_SRC_INCR_INCREMENTING
			    | DMAC_CTRLB_DST_INCR_FIXED;
	pTds[0].dwDscAddr = (uint32_t) &pTds[1];
        
	pTds[1].dwSrcAddr = (uint32_t) ( buf + ( len  >> 1) );
	pTds[1].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
	pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
                          | DMAC_CTRLA_SRC_WIDTH_HALF_WORD 
                          | DMAC_CTRLA_DST_WIDTH_HALF_WORD;        
	pTds[1].dwCtrlB   = 0
			  | DMAC_CTRLB_SIF_AHB_IF0
			  | DMAC_CTRLB_DIF_AHB_IF2
			  | DMAC_CTRLB_FC_MEM2PER_DMA_FC
			  | DMAC_CTRLB_SRC_INCR_INCREMENTING
			  | DMAC_CTRLB_DST_INCR_FIXED;
	pTds[1].dwDscAddr = (uint32_t) &pTds[0];
               
        DMAD_PrepareMultiTransfer(&g_dmad, pSource->dev.txDMAChannel, dmaTdSSC1Tx);
        DMAD_StartTransfer(&g_dmad, pSource->dev.txDMAChannel);

        SSC_EnableTransmitter( pSsc );
        
        return 0;
}

#endif


/*
*********************************************************************************************************
*                                               ssc_txRegister_set()
*
* Description : config ssc hardware tx registers
*
* Arguments   : Details in datasheet
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void ssc_txRegister_set( void *instance,void *parameter )
{    
    assert( ( NULL != instance ) && ( NULL != parameter ) );
    
    static TCMR tcmr ;
    static TFMR tfmr ;
    
    DataSource * pSource = ( DataSource * )instance;
    AUDIO_CFG *reg = ( AUDIO_CFG * )parameter;
    Ssc *pSSC = ( Ssc * )pSource->dev.instanceHandle;
    
    if( reg->channel_num > 0 ) 
    {        
        tfmr.datnb  = reg->channel_num - 1 ; 
        tfmr.datlen = reg->bit_length-1;  
        tcmr.cki = reg->ssc_cki;
        tcmr.sttdly = reg->ssc_delay;
        tcmr.start = reg->ssc_start;
        
        SSC_ConfigureTransmitter( pSSC,  tcmr.value,  tfmr.value );
        SSC_DisableTransmitter( pSSC );
        
    }       
         
}


/*
*********************************************************************************************************
*                                               ssc_rxRegister_set()
*
* Description : lconfig ssc hardware rx registers
*
* Arguments   : Details in datasheet
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void ssc_rxRegister_set( void *instance,void *parameter )
{    
    assert( ( NULL != instance ) && ( NULL != parameter ) );
    
    static RCMR rcmr ;
    static RFMR rfmr ;
    
    DataSource * pSource = ( DataSource * )instance;
    AUDIO_CFG *reg = ( AUDIO_CFG * )parameter;
    Ssc *pSSC = ( Ssc * )pSource->dev.instanceHandle;
    
    if( reg->channel_num > 0 ) 
    {        
        rfmr.datnb  = reg->channel_num - 1 ; 
        rfmr.datlen = reg->bit_length-1;  
        rcmr.cki = reg->ssc_cki;
        rcmr.sttdly = reg->ssc_delay;
        rcmr.start = reg->ssc_start;

        SSC_ConfigureReceiver(  pSSC,  rcmr.value , rfmr.value );
        SSC_DisableReceiver( pSSC ); 
        
    }       
         
}

/*
*********************************************************************************************************
*                                               _SSC_Init()
*
* Description : low level ssc initialize,config ssc hardware registers
*
* Arguments   : Details in datasheet
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
                       
static void _SSC_Init( uint32_t id,
                       uint32_t slave,
                       uint32_t bitrate,
                       uint32_t mclk, 
                       uint8_t slot_num, 
                       uint8_t slot_len )
{
    assert( ( (uint32_t )ID_SSC0 == id ) || ( (uint32_t )ID_SSC1 == id ) );
    
    static TCMR tcmr ;
    static TFMR tfmr ;
    static RCMR rcmr ;
    static RFMR rfmr ;
    
    slave = slave;
	
    Ssc *pSSC = _get_ssc_instance( id );

    if( NULL == pSSC )
	return;

    bitrate = 0;//1228810
    SSC_Configure(  pSSC,
                    bitrate,  //0:slave not gen clk 1:gen clk
                    mclk 
                 );    
      
    tcmr.cks    = 2 ;   // 0:MCK 1:RK 2:TK
    rcmr.cks    = 1 ;   // 0:MCK 1:TK 2:RK  0-->1
    
    tcmr.cko    = 0 ;   // 0:input only 1:continus 2:only transfer
    rcmr.cko    = 0 ;   // 0:input only 1:continus 2:only transfer
    
    tcmr.cki    = 0;    // 0: falling egde send
    rcmr.cki    = 1;    // 1: rising edge lock  
    
    tcmr.start  = 4;    // 4: falling edge trigger for low left, 5: rising edge trigger for high left,
    rcmr.start  = 4;    // 0: continuous 1:transmit 2:RF_LOW 3:RF_HIGH 4:RF_FAILLING
    			// 5: RF_RISING 6:RF_LEVEL 7:RF_EDGE 8:CMP_0
    tcmr.sttdly = 1;
    rcmr.sttdly = 1;   
	
    tcmr.period = 0;   // period ;  slave not use 0-->15
    rcmr.period = 0;   // period ;  slave not use
    
    tcmr.ckg    = 0 ;   //slave not use
    rcmr.ckg    = 0 ;   //slave not use
       
    tfmr.fsos   = 0 ;   //input only
    rfmr.fsos   = 0 ;   //input only
    
    tfmr.datnb  = slot_num-1;	//8 ; 
    rfmr.datnb  = slot_num-1;	//8 ; 
    
    tfmr.datlen = slot_len-1;	//31 ; //32bits
    rfmr.datlen = slot_len-1;	//31 ;
    
    tfmr.fslen  = 0 ; 	//frame sync is not used 0-->15
    rfmr.fslen  = 0 ; 	//frame sync is not used
       
    tfmr.fsedge = 1 ;
    rfmr.fsedge = 1 ;
          
    tfmr.msbf   = 1 ;
    rfmr.msbf   = 1 ;   

    tfmr.datdef = 0 ;
    tfmr.fsden  = 0 ;
    
    rfmr.loop   = 0 ; //0:normal 1:loop 
    
    SSC_ConfigureTransmitter( pSSC,  tcmr.value,  tfmr.value );
    SSC_DisableTransmitter( pSSC );
    SSC_ConfigureReceiver(  pSSC,  rcmr.value , rfmr.value );
    SSC_DisableReceiver( pSSC );    

}



/*
*********************************************************************************************************
*                                               _init_I2S()
*
* Description : middle level ssc initialize,
*
* Arguments   : pInstance    :datasource instance
*               dParameter   :reverse
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void _init_I2S( void *pInstance,void *dParameter )
{
    printf("\r\nInit I2S ...\r\n"); 

    assert( NULL != pInstance );
    dParameter = dParameter;
  
    DataSource *pSource = ( DataSource * )pInstance;
    Ssc *pSSC = _get_ssc_instance( pSource->dev.identify );

    _config_pins( pSource->dev.identify );

//    PMC_EnablePeripheral( pSource->dev.identify );
    IRQ_DisableIT( pSource->dev.identify );

    /* initialize ssc port to default state,if other config,invoke set_parameter interface */
    _SSC_Init( pSource->dev.identify ,pSource->dev.direct,0,BOARD_MCK , 2 , 16 );    
}

                                                  
/*
*********************************************************************************************************
*                                               init_I2S()
*
* Description : middle level ssc initialize,
*
* Arguments   : pInstance    :datasource instance
*               dParameter   :reverse
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void init_I2S(void *pParameter,void *dParameter)
{
	/* reserved this interface for update */
	dParameter = dParameter;
	
	assert( NULL != pParameter );
	DataSource *pSource = ( DataSource * )pParameter;

	assert( pSource->dev.direct < INVALIDDIR );

        /* Configure SSC port */
        _init_I2S( ( void * )pSource,NULL );

        if( pSource->dev.direct == ( uint8_t )IN )
            pSource->status[ IN ] = (uint8_t)CONFIGURED;
        else if( pSource->dev.direct == ( uint8_t )OUT )
            pSource->status[ OUT ] = (uint8_t)CONFIGURED;
        else
        {
            pSource->status[ IN ] = (uint8_t)CONFIGURED;
            pSource->status[ OUT ] = (uint8_t)CONFIGURED;
        }
        
}

