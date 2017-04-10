
#ifndef FM1388_SPI_H
#define FM1388_SPI_H

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <board.h>
#include "uif_object.h"
#include "spi_dma.h"

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/

/** Device is protected, operation cannot be carried out. */
#define FM1388_ERROR_PROTECTED        1
/** Device is busy executing a command. */
#define FM1388_ERROR_BUSY             2
/** There was a problem while trying to program page data. */
#define FM1388_ERROR_PROGRAM          3
/** There was an SPI communication error. */
#define FM1388_ERROR_SPI              4

/** Device ready/busy status bit. */
#define FM1388_STATUS_RDYBSY          (1 << 0)
/** Device is ready. */
#define FM1388_STATUS_RDYBSY_READY    (0 << 0)
/** Device is busy with internal operations. */
#define FM1388_STATUS_RDYBSY_BUSY     (1 << 0)

/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/

//Aplication object for Fm1388
typedef struct _Fm1388 {

    SpiDamon *pSpid;                                     //bind machinsm

    SpidCmd command;                                 //as a send buffer

    uint32_t pCmdBuffer[2];

    uint32_t pollingMode;                            //this is DMA parameter

} Fm1388; 

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

extern void FM1388_Configure(Fm1388 *pFm1388,
                           SpiDamon *pSpid,
                           uint8_t cs,
                           uint8_t polling);

extern uint8_t FM1388_SendCommand(
    Fm1388 *pFm1388,
    uint8_t* cmd,               
    uint32_t cmdSize,
    uint8_t *pData,
    uint32_t dataSize,
    uint32_t address,
    SpidCallback callback,
    void *pArgument);

extern uint8_t FM1388_IsBusy(Fm1388 *pFm1388);



#endif /*#ifndef FM1388_SPI_H */

