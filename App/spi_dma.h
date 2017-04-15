
#ifndef _SPI_DMA_
#define _SPI_DMA_

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"
#include "uif_object.h"   

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/

#define SPID_ERROR          1

#define SPID_ERROR_LOCK     2

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/

#define SPID_CSR_SCBR(mck, spck)    SPI_CSR_SCBR((mck) / (spck))

#define SPID_CSR_DLYBS(mck, delay)  SPI_CSR_DLYBS((((delay) * ((mck) / 1000000)) / 1000) + 1)

#define SPID_CSR_DLYBCT(mck, delay) SPI_CSR_DLYBCT((((delay) / 32 * ((mck) / 1000000)) / 1000) + 1)
   

#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/

/** SPI transfer complete callback. */
typedef void (*SpidCallback)( uint8_t, void* ) ;


typedef struct _SpidCmd
{
    uint8_t *pCmd;

    uint8_t cmdSize;

    uint8_t *pData;

    uint16_t dataSize;

    uint8_t spiCs;

    SpidCallback callback;

    void *pArgument;
} SpidCmd ;


typedef struct _Spid
{
    Spi* pSpiHw ;
    SpidCmd *pCurrentCommand ;
    sDmad* pDmad;
    uint8_t spiId ;
    volatile int8_t semaphore ;
    const DataSource *pSource;                       //embedded an abstract object about port
                                                     //as AB04,it's always spi0,In order to 
                                                     //facilitate the subsequent integration into 
                                                     //the original architecture
} SpiDamon ;

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

extern uint32_t SPID_Configure( SpiDamon* pSpid,
                                Spi* pSpiHw,
                                uint8_t spiId,
                                sDmad* pDmad ) ;

extern void SPID_ConfigureCS( SpiDamon* pSpid, uint32_t dwCS, uint32_t dwCsr ) ;

extern uint32_t SPID_SendCommand( SpiDamon* pSpid, SpidCmd* pCommand ) ;

extern void SPID_Handler( SpiDamon* pSpid ) ;

extern void SPID_DmaHandler( SpiDamon *pSpid );

extern uint32_t SPID_IsBusy( const SpiDamon* pSpid ) ;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _SPI_DMA_ */
