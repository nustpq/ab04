/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include <board.h>
#include <assert.h>
#include "fm1388_spi.h"

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/
extern uint8_t FM1388_SendCommand(
    Fm1388 *pFm1388,
    uint8_t *cmd,
    uint32_t cmdSize,
    uint8_t *pData,
    uint32_t dataSize,
    uint32_t address,
    SpidCallback callback,
    void *pArgument);


void FM1388D_Wait(Fm1388 *pFm1388)
{
    /* Wait for transfer to finish */
    while (FM1388_IsBusy(pFm1388))
    {
    }
}

/*----------------------------------------------------------------------------
 *         Global functions
 *----------------------------------------------------------------------------*/
uint32_t 
FM1388D_ReadWrite( Fm1388 *pFm1388,
                   uint8_t *pData,
                   uint8_t *cmd, 
                   uint32_t rSize, 
                   uint32_t wSize )
{
    assert(pFm1388);

    FM1388_SendCommand( pFm1388, 
                         cmd, wSize,
                         pData, rSize, 
                         0, 0, 0 );    
    /* Wait for transfer to finish */
    FM1388D_Wait( pFm1388 );

    return 0;
}


uint8_t 
SPI_WriteBuffer_API( Fm1388 *pFm1388 ,uint8_t *pdata, uint32_t size )
{
    assert( ( NULL != pFm1388 ) && ( NULL != pdata ) );
    
  return FM1388D_ReadWrite(        pFm1388,
                                   NULL,
                                   pdata, 
                                   0, 
                                   size );
  
}

uint8_t SPI_WriteReadBuffer_API( Fm1388 *pFm1388 ,  
                                 void *buffer_r,  
                                 void *buffer_w, 
                                 uint32_t length_r,  
                                 uint32_t length_w )
{
  assert( ( NULL != pFm1388 ) && ( NULL != buffer_w ) );
  
  return FM1388D_ReadWrite(        pFm1388,
                                   buffer_r,
                                   buffer_w, 
                                   length_r, 
                                   length_w );
  

 
}





