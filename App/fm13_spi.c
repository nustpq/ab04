/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <board.h>
#include <assert.h>
#include "fm1388_spi.h"   


/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/

/** SPI clock frequency used in Hz. */
#define SPCK            1000000

/** SPI chip select configuration value. */
#define CSR             (SPI_CSR_NCPHA | \
                         SPI_CSR_CSAAT | \
                         SPID_CSR_DLYBCT(BOARD_MCK, 100) | \
                         SPID_CSR_DLYBS(BOARD_MCK, 10) | \
                         SPID_CSR_SCBR(BOARD_MCK, SPCK))

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

void FM1388_Configure(Fm1388 *pFm1388, SpiDamon *pSpid, uint8_t cs, uint8_t polling)
{
    SpidCmd *pCommand;

    assert(pFm1388);
    assert(pSpid);
    assert(cs < 4);

    SPID_ConfigureCS(pSpid, cs, CSR);

    /* Initialize the FM1388 fields */
    pFm1388->pSpid = pSpid;
//    pFm1388->pDesc = 0;
    pFm1388->pollingMode = polling;

    /* Initialize the command structure */
    pCommand = &(pFm1388->command);
    pCommand->pCmd = (uint8_t *) pFm1388->pCmdBuffer;
    pCommand->callback = 0;
    pCommand->pArgument = 0;
    pCommand->spiCs = cs;
}

uint8_t FM1388_IsBusy(Fm1388 *pFm1388)
{
    if (pFm1388->pollingMode)
    {
        SPID_Handler(pFm1388->pSpid);
        SPID_DmaHandler(pFm1388->pSpid);
    }
    return SPID_IsBusy(pFm1388->pSpid);
}

uint8_t FM1388_SendCommand(
    Fm1388 *pFm1388,
    uint8_t* cmd,
    uint32_t cmdSize,
    uint8_t *pData,
    uint32_t dataSize,
    uint32_t address,
    SpidCallback callback,
    void *pArgument)

{
    SpidCmd *pCommand;

    assert(pFm1388);


    if (FM1388_IsBusy(pFm1388)) {

        return FM1388_ERROR_BUSY;
    }

    /* Update the SPI transfer descriptor */
    pCommand = &(pFm1388->command);
    pCommand->pCmd = cmd;
    pCommand->cmdSize = cmdSize;
    pCommand->pData = pData;
    pCommand->dataSize = dataSize;
    pCommand->callback = callback;
    pCommand->pArgument = pArgument;

    /* Start the SPI transfer */
    if (SPID_SendCommand(pFm1388->pSpid, pCommand)) {

        return FM1388_ERROR_SPI;
    }
    return 0;
}

