/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Unified EVM Interface Board 2.0
*
* Filename      : i2s.c
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

extern DataSource soruce_ssc0;
extern DataSource source_ssc1;

extern OS_FLAG_GRP *g_pStartUSBTransfer; 

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
     
//    BSP_LED_On( 3 );
    OS_ENTER_CRITICAL();
    DMAD_Handler(&g_dmad);
    OS_EXIT_CRITICAL();
//    BSP_LED_Off( 3 );
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
void stop_ssc( void *pInstance )
{
    pInstance = pInstance;
    DataSource *pSource = ( DataSource *)pInstance;
    
    Ssc *pSSC = ( Ssc * )pSource->dev.instanceHandle;
    
    SSC_DisableTransmitter( pSSC );
    SSC_DisableReceiver( pSSC );
    
    DMAD_StopTransfer(&g_dmad, pSource->dev.rxDMAChannel);
    DMAD_StopTransfer(&g_dmad, pSource->dev.txDMAChannel);
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
     temp = kfifo_get_data_size( &ssc0_bulkin_fifo );
     /*step 2:check buffer space according condition */  
     if( temp <= pSource->warmWaterLevel )
     {
       //if there has no enough space for data,error
     }
     else
     {
       
     }
     /*step3:copy data to buffer*/
#if 1     
     CDCDSerialDriver_Read(     usbBufferBulkOut0,                             \
                                USBDATAEPSIZE,                                 \
                                (TransferCallback) UsbAudio0DataReceived,      \
                                  0);
     CDCDSerialDriver_Write(    usbBufferBulkIn0,                              \
                                 USBDATAEPSIZE,                                \
                               (TransferCallback) UsbAudio0DataTransmit,       \
                               0);
#endif     
    /*step 4:change current buffer index */
     pSource->rx_index = 1 - pSource->rx_index; 
    /*step 5:send semphone */ 
                   OSFlagPost( 
                    g_pStartUSBTransfer,
                    (OS_FLAGS)(SSC0_IN), 
                    OS_FLAG_SET, 
                    &error
                );   
  
   
}

void _SSC1_DmaRxCallback( uint8_t status, void *pArg)
{    
    assert( NULL != pArg );
    INT8U error;
      
    DataSource *pSource = ( DataSource *)pArg;

#ifdef ENABLE_PRINT 
      if (status != DMAD_OK) 
      { 
          printf("Rx DMA Status :%s,line:%d\r\n",DMA_INFO[ status ],__LINE__);
          return;
      }
#endif
        
    /*step 1:get current buffer index */ 
      
    /*step 2:copy buffer */ 
      
    /*step 3:change current buffer index */
    pSource->rx_index = 1 - pSource->rx_index;  
      
    /*step 4:send semphone */
                OSFlagPost( 
                    g_pStartUSBTransfer,
                    (OS_FLAGS)(SSC1_IN), 
                    OS_FLAG_SET, 
                    &error
                );
  
   
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
#define TEST_BUF 1

#ifndef USE_EVENTGROUP
extern OS_FLAG_GRP *g_pStartUSBTransfer;
#endif

extern uint16_t TxBuffers[2][PINGPONG_SIZE];
extern uint16_t TxBuffers1[2][PINGPONG_SIZE];
void _SSC0_DmaTxCallback( uint8_t status, void *pArg)
{
      static uint8_t error;
      uint16_t temp = 0;

   
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

#if 1   
     //step 1:get current buffer index 
    
     //step 2:copy buffer to ring buffer 
     temp = kfifo_get_data_size( pSource->usbBulkOut );
     if( pSource->warmWaterLevel <= temp ) 
     {
              //update buffer point;
#if TEST_BUF
       pSource->i2sBufferOut = ( uint8_t * )&TxBuffers[ pSource->tx_index ];

#else
       pSource->i2sBufferOut = ( uint8_t * )&ssc0_I2SBuffersOut[ pSource->tx_index ];
#endif              
              //get data from buffer;
              kfifo_get( pSource->usbBulkOut, 
                         pSource->i2sBufferOut,
//                        ( uint8_t * )&ssc0_I2SBuffersOut[ pSource->tx_index ],
//                         pSource->warmWaterLevel 
                           192
                           ); 
              //update state machine of this port;
#if 1
                      CDCDSerialDriver_Read(  usbBufferBulkOut0,               \
                                USBCMDDATAEPSIZE ,                             \
                                (TransferCallback)UsbAudio0DataReceived,       \
                                0);
                      
                      CDCDSerialDriver_Write(  usbBufferBulkIn0,                \
                                 USBDATAEPSIZE,                                 \
                                 (TransferCallback) UsbAudio0DataTransmit,      \
                                 0);
#endif                      
              pSource->status = ( uint8_t )RUNNING;
     }
     else
     {
            //if this port was started,but no enough data in buffer,
            //flag this port in buffering state;
            if( ( uint8_t )START <= pSource->status )
            {
                    pSource->status = ( uint8_t )BUFFERED;
                    //error proccess;
            }
     }
#endif
     
      
     //step 3:change current buffer index 
     pSource->tx_index = 1 - pSource->tx_index;
      
     //step 4:send semphone       
#ifndef USE_EVENTGROUP
     OSFlagPost(    //send group event
                    g_pStartUSBTransfer,
                    (OS_FLAGS)(SSC0_OUT), //
                    OS_FLAG_SET, //
                    &error
                );
#endif  
}

void _SSC1_DmaTxCallback( uint8_t status, void *pArg)
{
      static uint8_t error;
      uint16_t temp = 0;

   
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

#if 1   
     //step 1:get current buffer index 
    
     //step 2:copy buffer to ring buffer 
     temp = kfifo_get_data_size( pSource->usbBulkOut );
     if( pSource->warmWaterLevel <= temp ) 
     {
              //update buffer point;
#if TEST_BUF
       pSource->i2sBufferOut = ( uint8_t * )&TxBuffers1[ pSource->tx_index ];

#else
       pSource->i2sBufferOut = ( uint8_t * )&ssc1_I2SBuffersOut[ pSource->tx_index ];
#endif              
              //get data from buffer;
              kfifo_get( pSource->usbBulkOut, 
                         pSource->i2sBufferOut,
//                        ( uint8_t * )&ssc0_I2SBuffersOut[ pSource->tx_index ],
//                         pSource->warmWaterLevel 
                           192
                           ); 
              //update state machine of this port;
#if 1
                      CDCDSerialDriver_Read(  usbBufferBulkOut0,               \
                                USBCMDDATAEPSIZE ,                             \
                                (TransferCallback)UsbAudio1DataReceived,       \
                                0);
                      
                      CDCDSerialDriver_Write(  usbBufferBulkIn1,                \
                                 USBDATAEPSIZE,                                 \
                                 (TransferCallback) UsbAudio1DataTransmit,      \
                                 0);
#endif                      
              pSource->status = ( uint8_t )RUNNING;
     }
     else
     {
            //if this port was started,but no enough data in buffer,
            //flag this port in buffering state;
            if( ( uint8_t )START <= pSource->status )
            {
                    pSource->status = ( uint8_t )BUFFERED;
                    //error proccess;
            }
     }
#endif
     
      
     //step 3:change current buffer index 
     pSource->tx_index = 1 - pSource->tx_index;
      
     //step 4:send semphone       
#ifndef USE_EVENTGROUP
     OSFlagPost(    //send group event
                    g_pStartUSBTransfer,
                    (OS_FLAGS)( SSC1_OUT ), //
                    OS_FLAG_SET, //
                    &error
                );
#endif  
}
#endif
/*
*********************************************************************************************************
*                                               SSCx_Recording()
*
* Description : SSCx recording DMA parameter config 
*
* Arguments   : pInstance     :datasource object
* Returns     : none
*
* Note(s)     : it is NOT reentrant;
*********************************************************************************************************
*/

#ifdef USE_DMA
void SSC0_Recording( void *pInstance )
{ 
        assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC0Rx;
        
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
        
        pTds[0].dwSrcAddr = ( uint32_t )&SSC0->SSC_RHR;
        pTds[0].dwDstAddr = ( uint32_t )ssc0_I2SBuffersIn[ 0 ]; 
        pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE(I2S_IN_BUFFER_SIZE)
                             | DMAC_CTRLA_SRC_WIDTH_BYTE
                             | DMAC_CTRLA_DST_WIDTH_BYTE;
        pTds[0].dwCtrlB   = DMAC_CTRLB_FC_PER2MEM_DMA_FC
                             | DMAC_CTRLB_SRC_INCR_FIXED
                             | DMAC_CTRLB_DST_INCR_INCREMENTING
                             | DMAC_CTRLB_SIF_AHB_IF2
                             | DMAC_CTRLB_DIF_AHB_IF0
                             ;      
        pTds[0].dwDscAddr = (uint32_t) &pTds[1];
        
        pTds[1].dwSrcAddr = ( uint32_t )&SSC0->SSC_RHR;
        pTds[1].dwDstAddr = ( uint32_t )ssc0_I2SBuffersIn[ 1 ]; 
        pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE(I2S_IN_BUFFER_SIZE)
                             | DMAC_CTRLA_SRC_WIDTH_BYTE
                             | DMAC_CTRLA_DST_WIDTH_BYTE;
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
}

void SSC1_Recording( void *pInstance )
{ 
        assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC1Rx;
        
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
        
        pTds[0].dwSrcAddr = ( uint32_t )&SSC1->SSC_RHR;
        pTds[0].dwDstAddr = ( uint32_t )ssc1_I2SBuffersIn[ 0 ]; 
        pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE(I2S_IN_BUFFER_SIZE)
                             | DMAC_CTRLA_SRC_WIDTH_BYTE
                             | DMAC_CTRLA_DST_WIDTH_BYTE;
        pTds[0].dwCtrlB   = DMAC_CTRLB_FC_PER2MEM_DMA_FC
                             | DMAC_CTRLB_SRC_INCR_FIXED
                             | DMAC_CTRLB_DST_INCR_INCREMENTING
                             | DMAC_CTRLB_SIF_AHB_IF2
                             | DMAC_CTRLB_DIF_AHB_IF0
                             ;      
        pTds[0].dwDscAddr = (uint32_t) &pTds[1];
        
        pTds[1].dwSrcAddr = ( uint32_t )&SSC1->SSC_RHR;
        pTds[1].dwDstAddr = ( uint32_t )ssc1_I2SBuffersIn[ 1 ]; 
        pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE(I2S_IN_BUFFER_SIZE)
                             | DMAC_CTRLA_SRC_WIDTH_BYTE
                             | DMAC_CTRLA_DST_WIDTH_BYTE;
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
}
#endif

/*
*********************************************************************************************************
*                                               SSCx_Playing()
*
* Description : SSC0 Tx DMA parameter config 
*
* Arguments   : pInstance     :datasource object
* Returns     : none
*
* Note(s)     : it is NOT reentrant;
*********************************************************************************************************
*/

#ifdef USE_DMA
void SSC0_Playing( void *pInstance )
{
	assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC0Tx;
        
        memset( TxBuffers,0x5555,sizeof( TxBuffers ) );
//        memset( ssc0_I2SBuffersOut, 0x5555, sizeof( ssc0_I2SBuffersOut ));
                
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
		/* Setup TD list for TX */
#if   TEST_BUF      
		pTds[0].dwSrcAddr = (uint32_t) TxBuffers[0];
#else
                pTds[0].dwSrcAddr = (uint32_t) ssc0_I2SBuffersOut[ 0 ];
#endif
                
		pTds[0].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
#if   TEST_BUF
		pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( PINGPONG_SIZE )
#else
                pTds[0].dwCtrlA  = DMAC_CTRLA_BTSIZE( I2S_OUT_BUFFER_SIZE )
#endif
						  | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
		pTds[0].dwCtrlB   = 0
						  | DMAC_CTRLB_SIF_AHB_IF0
						  | DMAC_CTRLB_DIF_AHB_IF2
						  | DMAC_CTRLB_FC_MEM2PER_DMA_FC
						  | DMAC_CTRLB_SRC_INCR_INCREMENTING
						  | DMAC_CTRLB_DST_INCR_FIXED;
		pTds[0].dwDscAddr = (uint32_t) &pTds[1];
#if   TEST_BUF	
		pTds[1].dwSrcAddr = (uint32_t) TxBuffers[1];
#else
                pTds[0].dwSrcAddr = (uint32_t) ssc0_I2SBuffersOut[ 1 ];
#endif
		pTds[1].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
#if   TEST_BUF
		pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( PINGPONG_SIZE )
#else
                pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( I2S_OUT_BUFFER_SIZE )
#endif
						  | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
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
}

void SSC1_Playing( void *pInstance )
{
#define TEST_BUF 1
	assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC1Tx;
        
        memset( TxBuffers1,0x5555,sizeof( TxBuffers1 ) );
//        memset( ssc1_I2SBuffersOut, 0x5555, sizeof( ssc1_I2SBuffersOut ));
                
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
		/* Setup TD list for TX */
#if   TEST_BUF      
		pTds[0].dwSrcAddr = (uint32_t) TxBuffers1[0];
#else
                pTds[0].dwSrcAddr = (uint32_t) ssc1_I2SBuffersOut[ 0 ];
#endif
                
		pTds[0].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
#if   TEST_BUF
		pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( PINGPONG_SIZE )
#else
                pTds[0].dwCtrlA  = DMAC_CTRLA_BTSIZE( I2S_OUT_BUFFER_SIZE )
#endif
						  | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
		pTds[0].dwCtrlB   = 0
						  | DMAC_CTRLB_SIF_AHB_IF0
						  | DMAC_CTRLB_DIF_AHB_IF2
						  | DMAC_CTRLB_FC_MEM2PER_DMA_FC
						  | DMAC_CTRLB_SRC_INCR_INCREMENTING
						  | DMAC_CTRLB_DST_INCR_FIXED;
		pTds[0].dwDscAddr = (uint32_t) &pTds[1];
#if   TEST_BUF	
		pTds[1].dwSrcAddr = (uint32_t) TxBuffers1[1];
#else
                pTds[0].dwSrcAddr = (uint32_t) ssc0_I2SBuffersOut[ 1 ];
#endif
		pTds[1].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
#if   TEST_BUF
		pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( PINGPONG_SIZE )
#else
                pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( I2S_OUT_BUFFER_SIZE )
#endif
						  | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
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
}

/*
*********************************************************************************************************
*                                               buffer_write()
*
* Description : buffer write via SSC0 
*
* Arguments   : pInstance     :datasource object handle
*               buf:          buf will be writed;
*               len:          bytes of buf;
* Returns     : none
*
* Note(s)     : unused,it is NOT reentrant;
*********************************************************************************************************
*/
void buffer_write( void *pInstance,const uint8_t *buf,uint32_t len )
{
  	assert( NULL != pInstance );
		
	DataSource *pSource = (DataSource *)pInstance;
	sDmaTransferDescriptor *pTds = dmaTdSSC0Tx;
        
        memset( ( void * )buf,0x5555,len ); //PINGPONG_SIZE
//        memset( ssc0_I2SBuffersOut, 0x5555, sizeof( ssc0_I2SBuffersOut ));
                
        Ssc* pSsc = _get_ssc_instance(pSource->dev.identify);
		/* Setup TD list for TX */
#if   TEST_BUF      
		pTds[0].dwSrcAddr = (uint32_t) buf[0];
#else
                pTds[0].dwSrcAddr = (uint32_t) ssc0_I2SBuffersOut[ 0 ];
#endif
                
		pTds[0].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
#if   TEST_BUF
		pTds[0].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
#else
                pTds[0].dwCtrlA  = DMAC_CTRLA_BTSIZE( len >> 1 )
#endif
						  | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
		pTds[0].dwCtrlB   = 0
						  | DMAC_CTRLB_SIF_AHB_IF0
						  | DMAC_CTRLB_DIF_AHB_IF2
						  | DMAC_CTRLB_FC_MEM2PER_DMA_FC
						  | DMAC_CTRLB_SRC_INCR_INCREMENTING
						  | DMAC_CTRLB_DST_INCR_FIXED;
		pTds[0].dwDscAddr = (uint32_t) &pTds[1];
#if   TEST_BUF	
		pTds[ 1 ].dwSrcAddr = (uint32_t) ( buf[ 0 ]+len >> 1 );
#else
                pTds[ 1 ].dwSrcAddr = (uint32_t)( buf[ 0 ]+len >> 1 );
#endif
		pTds[1].dwDstAddr = (uint32_t)	&pSsc->SSC_THR;
#if   TEST_BUF
		pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
#else
                pTds[1].dwCtrlA   = DMAC_CTRLA_BTSIZE( len >> 1 )
#endif
						  | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
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
    
    if( reg->channels > 0 ) 
    {        
        tfmr.datnb  = reg->channels - 1 ; 
        tfmr.datlen = reg->bit_length-1;  
        tcmr.cki = reg->cki;
        tcmr.sttdly = reg->delay;
        tcmr.start = reg->start;
        
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
    
    if( reg->channels > 0 ) 
    {        
        rfmr.datnb  = reg->channels - 1 ; 
        rfmr.datlen = reg->bit_length-1;  
        rcmr.cki = reg->cki;
        rcmr.sttdly = reg->delay;
        rcmr.start = reg->start;

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
                       
static void _SSC_Init( unsigned int id,
                       unsigned int slave,
                       unsigned int bitrate,
                       unsigned int mclk, 
                       unsigned char slot_num, 
                       unsigned char slot_len )
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

    
    SSC_Configure(  pSSC,
                    bitrate,  //0:slave not gen clk 1:gen clk
                    mclk 
                 );    
      
    tcmr.cks    = 0 ;   // 0:MCK 1:RK 2:TK
    rcmr.cks    = 1 ;   // 0:MCK 1:TK 2:RK
    
    tcmr.cko    = 1 ;   // 0:input only 1:continus 2:only transfer
    rcmr.cko    = 0 ;   // 0:input only 1:continus 2:only transfer
    
    tcmr.cki    = 0;    // 0: falling egde send
    rcmr.cki    = 1;    // 1: rising edge lock  
    
    tcmr.start  = 4;    // 4: falling edge trigger for low left, 5: rising edge trigger for high left,
    rcmr.start  = 4;    //0:continuous 1:transmit 2:RF_LOW 3:RF_HIGH 4:RF_FAILLING
    					//5:RF_RISING 6:RF_LEVEL 7:RF_EDGE 8:CMP_0
    tcmr.sttdly = 1;
    rcmr.sttdly = 1;   
	
    tcmr.period = 15;   // period ;  slave not use 0-->15
    rcmr.period = 0;    // period ;  slave not use
    
    tcmr.ckg    = 0 ;   //slave not use
    rcmr.ckg    = 0 ;   //slave not use
       
    tfmr.fsos   = 1 ;   //input only
    rfmr.fsos   = 0 ;   //input only
    
    tfmr.datnb  = slot_num-1;	//5 ; //6 slot TDM
    rfmr.datnb  = slot_num-1;	//5 ; 
    
    tfmr.datlen = slot_len-1;	//31 ; //32bits
    rfmr.datlen = slot_len-1;	//31 ;
    
    tfmr.fslen  = 15 ; 	//frame sync is not used
    rfmr.fslen  = 0 ; 	//frame sync is not used
       
    tfmr.fsedge = 1 ;
    rfmr.fsedge = 1 ;
          
    tfmr.msbf   = 1 ;
    rfmr.msbf   = 1 ;   

    tfmr.datdef = 0 ;
    tfmr.fsden  = 0 ;
    
    rfmr.loop   = 1 ; //0:normal 1:loop 
    
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
    _SSC_Init( pSource->dev.identify ,pSource->dev.direct,1520000,BOARD_MCK , 8 , 16 );    
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

	pSource->status = (uint8_t)CONFIGURED;
        
}

