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
#include <includes.h>
#include "ssc.h"

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
extern void Alert_Sound_Gen( uint8_t *pdata, uint32_t size, uint32_t REC_SR_Set );

unsigned int counter_play;
unsigned int counter_rec;

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

   // OS_ENTER_CRITICAL();
    DMAD_Handler(&g_dmad);
  //  OS_EXIT_CRITICAL();

}
#endif


/*
*********************************************************************************************************
*                                               i2s_start_Interrupt()
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
void i2s0_start_Interrupt( void *pPin, void (*isr_handler)( void ) )
{
    uint8_t per_id;
    
    assert( NULL != pPin );
    Pin *pPins = ( Pin * )pPin;

    per_id = ( uint8_t )pPins->id;
    IRQ_DisableIT( per_id );

    pPins->pio->PIO_ISR;
    pPins->pio->PIO_AIMER =   pPins->mask;
    pPins->pio->PIO_IER =     pPins->mask;       //enable int
    pPins->pio->PIO_ESR =     pPins->mask;       //edge int
//    pPins->pio->PIO_REHLSR =  pPins->mask;     //rising edge int
    pPins->pio->PIO_FELLSR =  pPins->mask;       //falling edge int    
    pPins->pio->PIO_IFER =    pPins->mask;       //enable input glitch filter

    IRQ_ConfigureIT( per_id, GPIO_PRIORITY+2, isr_handler );
    IRQ_EnableIT( per_id );

}

void i2s1_start_Interrupt( void *pPin, void (*isr_handler)( void ) )
{
    uint8_t per_id;
    
    assert( NULL != pPin );
    Pin *pPins = ( Pin * )pPin;

    per_id = ( uint8_t )pPins->id;
    IRQ_DisableIT( per_id );

    pPins->pio->PIO_ISR;
    pPins->pio->PIO_AIMER =   pPins->mask;
    pPins->pio->PIO_IER =     pPins->mask;       //enable int
    pPins->pio->PIO_ESR =     pPins->mask;       //edge int
//    pPins->pio->PIO_REHLSR =  pPins->mask;     //rising edge int
    pPins->pio->PIO_FELLSR =  pPins->mask;       //falling edge int    
    pPins->pio->PIO_IFER =    pPins->mask;       //enable input glitch filter

    IRQ_ConfigureIT( per_id, GPIO_PRIORITY+1, isr_handler );
    IRQ_EnableIT( per_id );

}

/*
*********************************************************************************************************
*                                    Disable_i2s_start_Interrupt()
*
* Description :  disable interrupt of special pin
*
* Argument(s) :  pin  : 
*
*
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Disable_i2s_start_Interrupt( Pin *pin )
{
    assert( NULL != pin );

    Pin *pPins = ( Pin * )pin;

    pPins->pio->PIO_IDR = pPins->mask; //enable int

}


void i2s0_isr_handler( void )
{
  extern Pin SSC_Sync_Pin; 
  source_ssc0.buffer_write(  &source_ssc0,
                             ( uint8_t * )ssc0_PingPongOut,                                                
                             source_ssc0.txSize ); 
  source_ssc0.status[ OUT ] = ( uint8_t )START;  
 
  Disable_i2s_start_Interrupt( &SSC_Sync_Pin ); 
 
}

void i2s1_isr_handler( void )
{
  extern Pin SSC_Sync_Pin1; 
 

#if 1  
  source_ssc1.buffer_write(  &source_ssc1,
                             ( uint8_t * )ssc1_PingPongOut,                                                
                             source_ssc1.txSize ); 
  source_ssc1.status[ OUT ] = ( uint8_t )START; 
#endif
  
  Disable_i2s_start_Interrupt( &SSC_Sync_Pin1 );  
  
}


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
    SSC_Stop_Reset( pSSC );

    //step2: stop dma channel
    DMAD_StopTransfer(&g_dmad, pSource->dev.txDMAChannel);
//    DMAD_ClearAuto( &g_dmad, pSource->dev.txDMAChannel );
    OSTimeDly( 4 );
    DMAD_StopTransfer( &g_dmad, pSource->dev.rxDMAChannel );
//    DMAD_ClearAuto( &g_dmad, pSource->dev.rxDMAChannel );
    OSTimeDly( 4 );
    
    //step3:clear buffer about this port
    memset( pSource->pBufferIn, 0 , sizeof( uint16_t ) * I2S_PINGPONG_IN_SIZE_3K );
    memset( pSource->pBufferOut, 0 , sizeof( uint16_t ) * I2S_PINGPONG_OUT_SIZE_3K );
    kfifo_reset( pSource->pRingBulkIn );
    kfifo_reset( pSource->pRingBulkOut );
    
    //step4:reset port state machine
    pSource->rx_index = 0;
    pSource->tx_index = 0;
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

static unsigned int  test_b = 0;
void fill_buf_debug( unsigned char *pChar, unsigned int size) 
{
    unsigned int i;
    unsigned short  *pInt;
    pInt = (unsigned short *)pChar;

    for( i = 0; i< (size>>1); i++ ) { 
       *(pInt+i) =  test_b++; 
    }
    
    
}

extern unsigned char  audio_padding_byte;
void _SSC0_DmaRxCallback( uint8_t status, void *pArg)
{    
    assert( NULL != pArg );
    
    uint32_t temp;
    uint8_t padding[128];
    memset( padding, audio_padding_byte, 128 );
    DataSource *pSource = ( DataSource *)pArg;

    //  UIF_LED_On( LED_HDMI );  
	switch( pSource->status[ IN ] ) 

		{
			case START   :
#if 0                          
//                          First_Pack_Padding_BI( ); 
                          pSource->status[ IN ] == ( uint8_t )RUNNING;
                          temp = kfifo_get_free_space( &ep0BulkIn_fifo );
                          
                          if( temp >= pSource->rxSize )
                          {
                            	kfifo_put( &ep0BulkIn_fifo,//pSource->pRingBulkIn,
                                          ( uint8_t * )&(ssc0_PingPongIn[ pSource-> rx_index ][0]), 
                  					pSource->rxSize );
                                memset( ( uint8_t * )&ssc0_PingPongIn[ pSource-> rx_index ][0], 0, pSource->rxSize );
                                
                                uint32_t counter = kfifo_get_data_size( &ep0BulkIn_fifo  );                                 
                                if(  counter >= pSource->rxSize  && restart_audio_0_bulk_in )  
                                {                    
                                        restart_audio_0_bulk_in = false ;
                                        // ep0 ring --> usb cache
                                        kfifo_get(  &ep0BulkIn_fifo,
                                                    ( uint8_t * )usbCacheBulkIn0,
                                                     pSource->rxSize );         
                
                                        // send ep0 data ---> pc
                                        CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                                                       pSource->rxSize,  //64B size for low delay
                                                                        (TransferCallback)UsbAudio0DataTransmit,
                                                                        ( void * )pSource );

                               } 
                            
                          }
                          
#endif
//                          break;
			case BUFFERED:
			case RUNNING :
				 temp = kfifo_get_free_space( &ep0BulkIn_fifo );//pSource->pRingBulkIn
				 if( temp >= pSource->rxSize )
				 {
                                   
                                      if( pSource->status[ IN ] == ( uint8_t )RUNNING )
                                      {
				 		kfifo_put( &ep0BulkIn_fifo,//pSource->pRingBulkIn,
                  					( uint8_t * )&(ssc0_PingPongIn[ pSource-> rx_index ][0]), 
                  					pSource->rxSize );
//                                                memset( ( uint8_t * )&ssc0_PingPongIn[ pSource-> rx_index ][0], 0, pSource->rxSize );
#if 1
                                                uint32_t counter = kfifo_get_data_size( &ep0BulkIn_fifo  );   
                                                if(  counter >= pSource->rxSize  && restart_audio_0_bulk_in )  
                                                {                    
                                                          restart_audio_0_bulk_in = false ;
                                                          // ep0 ring --> usb cache
                                                          kfifo_get(  &ep0BulkIn_fifo,
                                                                     //( uint8_t * )usbCacheBulkIn0,
                                                                      usbCacheBulkIn0,
                                                                      pSource->rxSize );         
                
                                                // send ep0 data ---> pc
                                                CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                                                               pSource->rxSize,  //64B size for low delay
                                                                               (TransferCallback)UsbAudio0DataTransmit,
                                                                               ( void * )pSource );

                                               }
                                      }
                                      else if( pSource->status[ IN ] == ( uint8_t )START )
                                      {
//                                         First_Pack_Padding_BI( &ep0BulkIn_fifo ); 
//                                         pSource->status[ IN ] = ( uint8_t )RUNNING;
                                         
                                    

                                         kfifo_put( &ep0BulkIn_fifo,//pSource->pRingBulkIn,
                  					( uint8_t * )&(ssc0_PingPongIn[ pSource-> rx_index ][0]), 
                  					pSource->rxSize );
                                         
                                         uint32_t counter = kfifo_get_data_size( &ep0BulkIn_fifo  ); 
                                         
                                         if(  counter >= pSource->rxSize  && restart_audio_0_bulk_in )  
                                                {                    
                                                        restart_audio_0_bulk_in = false ;
                                                        // ep0 ring --> usb cache
                                                        kfifo_get(  &ep0BulkIn_fifo,
                                                                    // ( uint8_t * )usbCacheBulkIn0,
                                                                    usbCacheBulkIn0,
                                                                    pSource->rxSize );         
                
                                                        // send ep0 data ---> pc
                                                        CDCDSerialDriver_WriteAudio_0( usbCacheBulkIn0,
                                                                               pSource->rxSize,  //64B size for low delay
                                                                               (TransferCallback)UsbAudio0DataTransmit,
                                                                               ( void * )pSource );

                                               }
                                      }
#endif		

                                  }
				 else
				 {
						pSource->status[ IN ] = ( uint8_t )BUFFERFULL;
				 }                                  
				break;
			case BUFFERFULL:
				memset( ( uint8_t  * )pSource->pBufferIn[ pSource-> rx_index ],
					     0x10,
					     sizeof( pSource->pBufferIn[ pSource-> rx_index ] ) );
					     
				break;
			case STOP:
 //                              if( pSource->peripheral_stop != NULL )
 //                                            pSource->peripheral_stop( &source_ssc0 ); 
				break;
			default:
				break;

		}
	   pSource->rx_index = 1 - pSource->rx_index;
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
    
    uint32_t temp;
    uint8_t padding[128];
    memset( padding, audio_padding_byte, 128 );
    DataSource *pSource = ( DataSource *)pArg;

	switch( pSource->status[ IN ] ) 

		{
			case START   :
			case BUFFERED:
			case RUNNING :
				 temp = kfifo_get_free_space( &ep1BulkIn_fifo );//pSource->pRingBulkIn
				 if( temp >= pSource->rxSize )
				 {
                                   
                                      if( pSource->status[ IN ] == ( uint8_t )RUNNING )
                                      {
				 		kfifo_put( &ep1BulkIn_fifo,//pSource->pRingBulkIn,
                  					( uint8_t * )&(ssc1_PingPongIn[ pSource-> rx_index ][0]), 
                  					pSource->rxSize );
//                                                memset( ( uint8_t * )&ssc0_PingPongIn[ pSource-> rx_index ][0], 0, pSource->rxSize );
#if 1
                                                uint32_t counter = kfifo_get_data_size( &ep1BulkIn_fifo  );   
                                                if(  counter >= pSource->rxSize  && restart_audio_1_bulk_in )  
                                                {                    
                                                          restart_audio_1_bulk_in = false ;
                                                          // ep0 ring --> usb cache
                                                          kfifo_get(  &ep1BulkIn_fifo,
                                                                     //( uint8_t * )usbCacheBulkIn0,
                                                                      usbCacheBulkIn1,
                                                                      pSource->rxSize );         
                
                                                // send ep0 data ---> pc
                                                CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                                                               pSource->rxSize,  //64B size for low delay
                                                                               (TransferCallback)UsbAudio1DataTransmit,
                                                                               ( void * )pSource );

                                               }
                                      }
                                      else if( pSource->status[ IN ] == ( uint8_t )START )
                                      {
//                                         First_Pack_Padding_BI( &ep1BulkIn_fifo ); 
//                                         pSource->status[ IN ] = ( uint8_t )RUNNING;

                                         kfifo_put( &ep1BulkIn_fifo,//pSource->pRingBulkIn,
                  					( uint8_t * )&(ssc1_PingPongIn[ pSource-> rx_index ][0]), 
                  					pSource->rxSize );
//                                         memset( ( uint8_t * )&ssc0_PingPongIn[ pSource-> rx_index ][0], 0, pSource->rxSize );
                                         
                                         uint32_t counter = kfifo_get_data_size( &ep1BulkIn_fifo  ); 
                                         
                                         if(  counter >= pSource->rxSize  && restart_audio_1_bulk_in )  
                                                {                    
                                                        restart_audio_1_bulk_in = false ;
                                                        // ep0 ring --> usb cache
                                                        kfifo_get(  &ep1BulkIn_fifo,
                                                                    // ( uint8_t * )usbCacheBulkIn0,
                                                                    usbCacheBulkIn1,
                                                                    pSource->rxSize );         
                
                                                        // send ep0 data ---> pc
                                                        CDCDSerialDriver_WriteAudio_1( usbCacheBulkIn1,
                                                                               pSource->rxSize,  //64B size for low delay
                                                                               (TransferCallback)UsbAudio1DataTransmit,
                                                                               ( void * )pSource );

                                               }
                                      }
#endif		

                                  }
				 else
				 {
						pSource->status[ IN ] = ( uint8_t )BUFFERFULL;
				 }                                  
				break;
			case BUFFERFULL:
				memset( ( uint8_t  * )pSource->pBufferIn[ pSource-> rx_index ],
					     0x10,
					     sizeof( pSource->pBufferIn[ pSource-> rx_index ] ) );
					     
				break;
			case STOP:
 //                              if( pSource->peripheral_stop != NULL )
 //                                            pSource->peripheral_stop( &source_ssc0 ); 
				break;
			default:
				break;

		}
	   pSource->rx_index = 1 - pSource->rx_index;
}

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

void _SSC0_DmaTxCallback( uint8_t status, void *pArg)
{
  
    const uint8_t nDelay = 16;
    uint32_t temp = 0;
       
    assert( NULL != pArg );
    
    DataSource *pSource = ( DataSource *)pArg;
    Ssc *pSsc = _get_ssc_instance( pSource->dev.identify );

    UIF_LED_On( LED_RUN ); 
    pSource->pBufferOut = ( uint8_t * )&ssc0_PingPongOut[ 1 - pSource->tx_index ];
     
	switch( pSource->status[ OUT ] )
		{ 
			case START    :
			case BUFFERED :
			case RUNNING  :
				temp = kfifo_get_data_size( pSource->pRingBulkOut );

				if( temp  >=  pSource->txSize )
				{
                                      kfifo_get( pSource->pRingBulkOut,
                                      ( uint8_t * )&(ssc0_PingPongOut[ pSource-> tx_index ][0]),
                                      pSource->txSize );
                                      
                                      uint32_t counter = kfifo_get_free_space( pSource->pRingBulkOut ); 
                                      if(  counter >= source_ssc0.txSize && restart_audio_0_bulk_out  )  
                                      {
                                              restart_audio_0_bulk_out = false ;
                                              // send ep0 data ---> pc
                                              CDCDSerialDriver_ReadAudio_0( usbCacheBulkOut0,
                                                                              source_ssc0.txSize,
                                                                              (TransferCallback)UsbAudio0DataReceived,
                                                                              ( void * )pSource );  
                                      }
				}
				else
				{
					pSource->status[ OUT ] = ( uint8_t )BUFFEREMPTY;
				}
									
				break;
            case BUFFEREMPTY:
                                Alert_Sound_Gen( ( uint8_t * )&pSource->pBufferOut[ pSource-> tx_index ],
                                                  pSource->txSize, 
                                                  16000 );
                                break;
			case STOP     :
				break;
			default:
                          ;
				break;
     
      }
    pSource->tx_index = 1 - pSource->tx_index;

    UIF_LED_Off( LED_RUN );  
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
    uint32_t temp = 0;
       
    assert( NULL != pArg );
    
    DataSource *pSource = ( DataSource *)pArg;
    Ssc *pSsc = _get_ssc_instance( pSource->dev.identify );

    pSource->pBufferOut = ( uint8_t * )&ssc1_PingPongOut[ 1 - pSource->tx_index ];
     
	switch( pSource->status[ OUT ] )
		{ 
			case START    :
			case BUFFERED :
			case RUNNING  :
				temp = kfifo_get_data_size( pSource->pRingBulkOut );

				if( temp  >=  pSource->txSize )
				{
                                      kfifo_get( pSource->pRingBulkOut,
                                      ( uint8_t * )&(ssc1_PingPongOut[ pSource-> tx_index ][0]),
                                      pSource->txSize );
                                      
                                      uint32_t counter = kfifo_get_free_space( pSource->pRingBulkOut ); 
                                      if(  counter >= source_ssc1.txSize && restart_audio_1_bulk_out  )  
                                      {
                                              restart_audio_1_bulk_out = false ;
                                              // send ep0 data ---> pc
                                              CDCDSerialDriver_ReadAudio_1( usbCacheBulkOut1,
                                                                              source_ssc1.txSize,
                                                                              (TransferCallback)UsbAudio1DataReceived,
                                                                              ( void * )pSource );  
                                      }
				}
				else
				{
					pSource->status[ OUT ] = ( uint8_t )BUFFEREMPTY;
				}
									
				break;
            case BUFFEREMPTY:
                                Alert_Sound_Gen( ( uint8_t * )&pSource->pBufferOut[ pSource-> tx_index ],
                                                  pSource->txSize, 
                                                  16000 );
                                break;
			case STOP     :
				break;
			default:
                          ;
				break;
     
      }
    pSource->tx_index = 1 - pSource->tx_index;
}



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
        pTds[1].dwDstAddr = ( uint32_t )( buf + ( sizeof( ssc0_PingPongIn ) >> 1 ) ); 
        pTds[1].dwDstAddr = ( uint32_t )&ssc0_PingPongIn[1];
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
        
        
        //memset( testbuf, 0x55, sizeof(testbuf)  ); 
        
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
        pTds[1].dwDstAddr = ( uint32_t )( buf + ( sizeof( ssc1_PingPongIn ) >> 1 ) ); 
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
	        
	pTds[1].dwSrcAddr = (uint32_t) ( buf + ( sizeof( ssc0_PingPongOut ) >> 1 ) ); 
    //pTds[1].dwSrcAddr = ( uint32_t )&ssc0_PingPongOut[1];
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
        
	pTds[1].dwSrcAddr = (uint32_t) ( buf + ( sizeof( ssc1_PingPongOut ) >> 1 ) ); 
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
* Returns     : error 
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint8_t ssc_txRegister_set( void *instance,void *parameter )
{    
    unsigned char  err;
    unsigned short sample_rate ; //not support 44.1khz now
    unsigned char  channels_play;
    unsigned char  bit_length;
    unsigned char  cki;
    unsigned char  delay;
    unsigned char  start;
    unsigned char  id;
    
    static TCMR tcmr ;
    static TFMR tfmr ;
    assert( ( NULL != instance ) && ( NULL != parameter ) );
    err  = NULL;
    
    DataSource * pSource = ( DataSource * )instance;
    AUDIO_CFG *reg = ( AUDIO_CFG * )parameter;
    Ssc *pSSC = ( Ssc * )pSource->dev.instanceHandle;
    
    channels_play = reg->channel_num ; 
    sample_rate   = reg->sample_rate ;
    bit_length    = reg->bit_length ; 
    cki           = reg->ssc_cki;
    delay         = reg->ssc_delay;
    start         = reg->ssc_start;
    id            = reg->id ;
    
    APP_TRACE_INFO(( "\r\nSSC[%d] [%dth]Play[%dCH - %dHz - %dBit][%d%s - %dDelay - %d%sLeft] ...\r\n",\
             id,counter_play++,channels_play ,sample_rate,bit_length,cki,(cki==0)?"Fall" :"Rise",delay,start,(start==4)?"Low":"High" ));  
    
    if( (channels_play == 0) || (channels_play > 8) ) {        
        err = ERR_TDM_FORMAT ; 
    }  
    if( (bit_length != 16) && (bit_length != 32)  ) {        
        err = ERR_TDM_FORMAT ; 
    }  
    if( NULL != err ) {
        APP_TRACE_INFO(("Init Play Setting Error !\r\n"));
        return err;
    }
          
    pSource->txSize   =  ( sample_rate / 1000 ) * ( bit_length / 8 ) * channels_play * I2S_PINGPONG_BUF_SIZE_MS ; 
    pSource->warmWaterLevel = I2S_PLAY_PRE_BUF_NUM * (pSource->txSize)  ;
    pSource->tx_index = 0;
          
    if( channels_play > 0 ) 
    {        
        tfmr.datnb  = channels_play - 1 ; 
        tfmr.datlen = bit_length-1;
        tfmr.msbf   = 1;       //reg->msbf;
        tfmr.fsedge = 1;       //reg->fsedge;
        
        tcmr.cks = 2;  
        tcmr.cki = cki;
        tcmr.sttdly = delay;
        tcmr.start  = start;
        
        SSC_ConfigureTransmitter( pSSC,  tcmr.value,  tfmr.value );
        SSC_DisableTransmitter( pSSC );
        
        pSource->status[OUT] = ( uint8_t )CONFIGURED; //PQ
        
    } else {
        err =  ERR_TDM_FORMAT ; 
        
    }
    
    //To Do:  DMA set
//    if( bit_length == 16 ) {
//        DMA_CtrA_Reg_Mode  = DMA_CTRA_MODE_16BIT ;
//        DMA_CtrA_Len_Shift = 1;    
//    } else { //32bit
//        DMA_CtrA_Reg_Mode  = DMA_CTRA_MODE_32BIT ;
//        DMA_CtrA_Len_Shift = 2;  
//    }
    
    return err;
         
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
uint8_t ssc_rxRegister_set( void *instance,void *parameter )
{    
    unsigned char  err;
    unsigned short sample_rate ; //not support 44.1khz now
    unsigned char  channels_rec;
    unsigned char  bit_length;
    unsigned char  cki;
    unsigned char  delay;
    unsigned char  start;
    unsigned char  id;
    
    static RCMR rcmr ;
    static RFMR rfmr ;
    assert( ( NULL != instance ) && ( NULL != parameter ) );
    err  = NULL;
    
    DataSource * pSource = ( DataSource * )instance;
    AUDIO_CFG *reg = ( AUDIO_CFG * )parameter;
    Ssc *pSSC = ( Ssc * )pSource->dev.instanceHandle;
    
    channels_rec = reg->channel_num ; 
    sample_rate   = reg->sample_rate ;
    bit_length    = reg->bit_length ; 
    cki           = reg->ssc_cki;
    delay         = reg->ssc_delay;
    start         = reg->ssc_start;
    id            = reg->id ;
    
    APP_TRACE_INFO(( "\r\nSSC[%d] [%dth]Rec[%dCH - %dHz - %dBit][%d%s - %dDelay - %d%sLeft] ...\r\n",\
             id,counter_rec++,channels_rec ,sample_rate,bit_length,cki,(cki==0)?"Fall" :"Rise",delay,start,(start==4)?"Low":"High" ));  
    
    if( (channels_rec == 0) || (channels_rec > 8) ) {        
        err = ERR_TDM_FORMAT ; 
    }  
    if( (bit_length != 16) && (bit_length != 32)  ) {        
        err = ERR_TDM_FORMAT ; 
    }  
    if( NULL != err ) {
        APP_TRACE_INFO(("Init Rec Setting Error !\r\n"));
        return err;
    }
          
    pSource->rxSize   =  ( sample_rate / 1000 ) * ( bit_length / 8 ) * channels_rec * I2S_PINGPONG_BUF_SIZE_MS ; 
    pSource->warmWaterLevel = 0;   //no need pre-buffer for Rec 
    pSource->rx_index = 0;
          
    if( channels_rec > 0 ) 
    {        
        rfmr.datnb  = channels_rec - 1 ; 
        rfmr.datlen = bit_length-1;
        rfmr.msbf   = 1;       //reg->msbf;
        rfmr.fsedge = 1;       //reg->fsedge;
        
        rcmr.cks = 1;        
        rcmr.cki = cki;
        rcmr.sttdly = delay;
        rcmr.start = start;

        SSC_ConfigureReceiver(  pSSC,  rcmr.value , rfmr.value );
        SSC_DisableReceiver( pSSC ); 
        
        pSource->status[IN] = ( uint8_t )CONFIGURED; //PQ
        
    } else {
        err =  ERR_TDM_FORMAT ; 
        
    }
    
    //To Do:  DMA set
//    if( bit_length == 16 ) {
//        DMA_CtrA_Reg_Mode  = DMA_CTRA_MODE_16BIT ;
//        DMA_CtrA_Len_Shift = 1;    
//    } else { //32bit
//        DMA_CtrA_Reg_Mode  = DMA_CTRA_MODE_32BIT ;
//        DMA_CtrA_Len_Shift = 2;  
//    }
    
    return err;    
          
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
    tcmr.sttdly = 1;    //rollback the key parameter
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
    _SSC_Init( pSource->dev.identify ,pSource->dev.direct,0,BOARD_MCK , 8 , 16 );    
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

