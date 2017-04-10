
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"
#include "spi_dma.h"

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/

#define USE_SPI_DMA

#define DMA_SPI_LLI     2

 /*----------------------------------------------------------------------------
 *        Local Variables
 *----------------------------------------------------------------------------*/

#if defined(USE_SPI_DMA)

static uint32_t spiDmaTxChannel;
static uint32_t spiDmaRxChannel;

static sDmaTransferDescriptor dmaTxLinkList[DMA_SPI_LLI];
static sDmaTransferDescriptor dmaRxLinkList[DMA_SPI_LLI];
#endif

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/

#if defined(USE_SPI_DMA)

static void SPID_Rx_Cb(uint32_t dmaStatus, SpiDamon* pArg)
{
    SpidCmd *pSpidCmd = pArg->pCurrentCommand;
    Spi *pSpiHw = pArg->pSpiHw;

    if (dmaStatus == DMAD_PARTIAL_DONE)
        return;

    SPI_Disable ( pSpiHw );
    
    PMC_DisablePeripheral ( pArg->spiId );
    
    SPI_ReleaseCS(pSpiHw);
    
    DMAD_FreeChannel(pArg->pDmad, spiDmaRxChannel);
    DMAD_FreeChannel(pArg->pDmad, spiDmaTxChannel);

    pArg->semaphore++;
        
    if (pSpidCmd && pSpidCmd->callback) {
    
        pSpidCmd->callback(0, pSpidCmd->pArgument);
    }
}


static uint8_t _spid_configureDmaChannels( SpiDamon* pSpid )
{
    uint32_t dwCfg;
    uint8_t iController;


    spiDmaTxChannel = DMAD_AllocateChannel( pSpid->pDmad,
                                            DMAD_TRANSFER_MEMORY, ID_SPI0);
    {
        if ( spiDmaTxChannel == DMAD_ALLOC_FAILED ) 
        {
            return SPID_ERROR;
        }
    }

    spiDmaRxChannel = DMAD_AllocateChannel( pSpid->pDmad,
                                            ID_SPI0, DMAD_TRANSFER_MEMORY);
    {
        if ( spiDmaRxChannel == DMAD_ALLOC_FAILED ) 
        {
            return SPID_ERROR;
        }
    }
    iController = (spiDmaRxChannel >> 8);

    DMAD_SetCallback(pSpid->pDmad, spiDmaRxChannel,
                     (DmadTransferCallback)SPID_Rx_Cb, pSpid);


    dwCfg = 0
           | DMAC_CFG_SRC_PER(
              DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_RX ))
           | DMAC_CFG_DST_PER(
              DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_RX ))
           | DMAC_CFG_SRC_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;

    if (DMAD_PrepareChannel( pSpid->pDmad, spiDmaRxChannel, dwCfg ))
        return SPID_ERROR;

    iController = (spiDmaTxChannel >> 8);

    DMAD_SetCallback(pSpid->pDmad, spiDmaTxChannel, NULL, NULL);


    dwCfg = 0
           | DMAC_CFG_SRC_PER(
              DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_TX ))
           | DMAC_CFG_DST_PER(
              DMAIF_Get_ChannelNumber( iController, ID_SPI0, DMAD_TRANSFER_TX ))
           | DMAC_CFG_DST_H2SEL
           | DMAC_CFG_SOD
           | DMAC_CFG_FIFOCFG_ALAP_CFG;

    if ( DMAD_PrepareChannel( pSpid->pDmad, spiDmaTxChannel, dwCfg ))
        return SPID_ERROR;
    return 0;
}


static uint8_t _spid_configureLinkList(Spi *pSpiHw, void *pDmad, SpidCmd *pCommand)
{

    /* Setup RX Link List */
    dmaRxLinkList[0].dwSrcAddr = (uint32_t)&pSpiHw->SPI_RDR;
    dmaRxLinkList[0].dwDstAddr = (uint32_t)pCommand->pCmd;
    dmaRxLinkList[0].dwCtrlA   = pCommand->cmdSize | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmaRxLinkList[0].dwCtrlB   = DMAC_CTRLB_SIF_AHB_IF2 | DMAC_CTRLB_DIF_AHB_IF0 | 
                                 DMAC_CTRLB_FC_PER2MEM_DMA_FC | 
                                 DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;

    /* Setup TX Link List */                           
    dmaTxLinkList[0].dwSrcAddr = (uint32_t)pCommand->pCmd;
    dmaTxLinkList[0].dwDstAddr = (uint32_t)&pSpiHw->SPI_TDR;
    dmaTxLinkList[0].dwCtrlA   = pCommand->cmdSize | DMAC_CTRLA_SRC_WIDTH_BYTE  | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmaTxLinkList[0].dwCtrlB   = DMAC_CTRLB_SIF_AHB_IF0 | DMAC_CTRLB_DIF_AHB_IF2 | 
                                 DMAC_CTRLB_FC_MEM2PER_DMA_FC | 
                                 DMAC_CTRLB_SRC_INCR_INCREMENTING | DMAC_CTRLB_DST_INCR_FIXED;

    /* In case command only */
    if (pCommand->pData == 0) {

        dmaRxLinkList[0].dwDscAddr = 0;
        dmaTxLinkList[0].dwDscAddr = 0;
    }
    /* In case Command & data */
    else {
        dmaRxLinkList[0].dwDscAddr = (uint32_t)&dmaRxLinkList[1];
        dmaRxLinkList[1].dwSrcAddr = (uint32_t)&pSpiHw->SPI_RDR;
        dmaRxLinkList[1].dwDstAddr = (uint32_t)pCommand->pData;
        dmaRxLinkList[1].dwCtrlA   = pCommand->dataSize | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
        dmaRxLinkList[1].dwCtrlB   = DMAC_CTRLB_SIF_AHB_IF2 | DMAC_CTRLB_DIF_AHB_IF0 | 
                                     DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | 
                                     DMAC_CTRLB_FC_PER2MEM_DMA_FC |
                                     DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
        dmaRxLinkList[1].dwDscAddr = 0;
        dmaTxLinkList[0].dwDscAddr = (uint32_t)&dmaTxLinkList[1];
        dmaTxLinkList[1].dwSrcAddr = (uint32_t)pCommand->pData;
        dmaTxLinkList[1].dwDstAddr = (uint32_t)&pSpiHw->SPI_TDR;
        dmaTxLinkList[1].dwCtrlA   = pCommand->dataSize | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
        dmaTxLinkList[1].dwCtrlB   = DMAC_CTRLB_SIF_AHB_IF0 | DMAC_CTRLB_DIF_AHB_IF2 | 
                                     DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | 
                                     DMAC_CTRLB_FC_MEM2PER_DMA_FC | 
                                     DMAC_CTRLB_SRC_INCR_INCREMENTING | DMAC_CTRLB_DST_INCR_FIXED;
        dmaTxLinkList[1].dwDscAddr    = 0;
    }

    
    if ( DMAD_PrepareMultiTransfer( pDmad, spiDmaRxChannel, &dmaRxLinkList[0]))
        return SPID_ERROR;
    if ( DMAD_PrepareMultiTransfer( pDmad, spiDmaTxChannel, &dmaTxLinkList[0]))
        return SPID_ERROR;
    return 0;   
}
#endif

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
uint32_t SPID_Configure( SpiDamon *pSpid ,
                         Spi *pSpiHw , 
                         uint8_t spiId,
                         sDmad *pDmad )
{
    pSpid->pSpiHw = pSpiHw;
    pSpid->spiId  = spiId;
    pSpid->semaphore = 1;
    pSpid->pCurrentCommand = 0;
    pSpid->pDmad = pDmad;

    SPI_Configure ( pSpiHw, pSpid->spiId, SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PCS_Msk );

    PMC_DisablePeripheral (pSpid->spiId );
    return 0;
}


void SPID_ConfigureCS( SpiDamon *pSpid, 
                       uint32_t dwCS, 
                       uint32_t dwCsr)
{
    Spi *pSpiHw = pSpid->pSpiHw;
    
    PMC_EnablePeripheral (pSpid->spiId );

    SPI_ConfigureNPCS( pSpiHw, dwCS, dwCsr );

    PMC_DisablePeripheral (pSpid->spiId );
}

uint32_t SPID_SendCommand( SpiDamon *pSpid, SpidCmd *pCommand)
{
    Spi *pSpiHw = pSpid->pSpiHw;
         
     if (pSpid->semaphore == 0) {
    
         return SPID_ERROR_LOCK;
    }
     pSpid->semaphore--;


    PMC_EnablePeripheral (pSpid->spiId );
    
    SPI_ChipSelect (pSpiHw, 1 << pCommand->spiCs);

#if !defined(USE_SPI_DMA)

    pSpid->pCurrentCommand = pCommand;
    /* Enables the SPI to transfer and receive data. */
    SPI_Enable (pSpiHw );
    {
    uint32_t i;

    for (i = 0; i < pCommand->cmdSize; i ++)
    {
        SPI_Write(pSpiHw, pCommand->spiCs, pCommand->pCmd[i]);
        SPI_Read(pSpiHw);
    }

    for (i = 0; i < pCommand->dataSize; i ++)
    {
        SPI_Write(pSpiHw, pCommand->spiCs, pCommand->pData[i]);
        pCommand->pData[i] = SPI_Read(pSpiHw);
    }
    SPI_ReleaseCS(pSpiHw);
    
    /* Unlock */
    pSpid->semaphore ++;
    
    /* Callback */
    if (pCommand->callback)
    {
        pCommand->callback(0, pCommand->pArgument);
    }
    }
#else
    if (_spid_configureDmaChannels(pSpid) )
        return SPID_ERROR_LOCK;
    if (_spid_configureLinkList(pSpiHw, pSpid->pDmad, pCommand))
        return SPID_ERROR_LOCK;


    pSpid->pCurrentCommand = pCommand;

    SPI_Enable (pSpiHw );

    if (DMAD_StartTransfer( pSpid->pDmad, spiDmaRxChannel )) 
        return SPID_ERROR_LOCK;
    if (DMAD_StartTransfer( pSpid->pDmad, spiDmaTxChannel )) 
        return SPID_ERROR_LOCK;
#endif
    return 0;    
}


void SPID_Handler( SpiDamon *pSpid )
{
    pSpid = pSpid;
}


void SPID_DmaHandler( SpiDamon *pSpid )
{
    DMAD_Handler( pSpid->pDmad );
}

uint32_t SPID_IsBusy(const SpiDamon *pSpid)
{
    if (pSpid->semaphore == 0) {

        return 1;
    }
    else {

        return 0;
    }
}
