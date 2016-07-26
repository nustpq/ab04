/* ----------------------------------------------------------------------------
 *         SAM Software Package License 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2013, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"

#include "uip.h"
#include "uip_arp.h"

#include "gmac_tapdev.h"

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/
/* Number of buffer for RX */
#define RX_BUFFERS  16
/* Number of buffer for TX */
#define TX_BUFFERS  8

/*----------------------------------------------------------------------------
 *        Variables
 *----------------------------------------------------------------------------*/

/* The PINs for GMAC */
static const Pin gmacPins[] = {BOARD_GMAC_RUN_PINS};
static const Pin gmacModePins[] = {BOARD_GMAC_MODE_PINS};

/* The GMAC driver instance */
static sGmacd gGmacd;

/* The GMACB driver instance */
static GMacb gGmacb;

/** TX descriptors list */
#if defined ( __ICCARM__ ) /* IAR Ewarm */
#pragma data_alignment=8
#pragma location = "region_dma_nocache"
#elif defined (  __GNUC__  ) /* GCC CS3 */
__attribute__((aligned(8), __section__(".region_dma_nocache")))
#endif
static sGmacTxDescriptor gGTxDs[TX_BUFFERS];

/** TX callbacks list */
static fGmacdTransferCallback gGTxCbs[TX_BUFFERS];

/** RX descriptors list */
#if defined ( __ICCARM__ ) /* IAR Ewarm */ 
#pragma data_alignment=8
#pragma location = "region_dma_nocache"
#elif defined (  __GNUC__  ) /* GCC CS3 */
__attribute__((aligned(8), __section__(".region_dma_nocache")))
#endif
static sGmacRxDescriptor gGRxDs[RX_BUFFERS];

/** Send Buffer */
/* Section 3.6 of AMBA 2.0 spec states that burst should not cross 1K Boundaries.
   Receive buffer manager writes are burst of 2 words => 3 lsb bits of the address
   shall be set to 0 */
#if defined ( __ICCARM__ ) /* IAR Ewarm */
#pragma data_alignment=8
#pragma location = "region_dma_nocache"
#elif defined (  __GNUC__  ) /* GCC CS3 */
__attribute__((aligned(8), __section__(".region_dma_nocache")))
#endif
static uint8_t pGTxBuffer[TX_BUFFERS * GMAC_TX_UNITSIZE];

#if defined ( __ICCARM__ ) /* IAR Ewarm */
#pragma data_alignment=8
#pragma location = "region_dma_nocache"
#elif defined (  __GNUC__  ) /* GCC CS3 */
__attribute__((aligned(8), __section__(".region_dma_nocache")))
#endif/** Receive Buffer */
static uint8_t pGRxBuffer[RX_BUFFERS * GMAC_RX_UNITSIZE];

/* MAC address used for demo */
static uint8_t gGMacAddress[6] = {0x00, 0x45, 0x56, 0x78, 0x9a, 0xbc};

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

/**
 * Gmac interrupt handler
 */
static void GMAC_IrqHandler(void)
{
    GMACD_Handler(&gGmacd);
}

/**
 * Set the MAC address of the system.
 * Should only be called before tapdev_init is called.
 */
void gmac_tapdev_setmac(u8_t *addr)
{
    gGMacAddress[0] = addr[0];
    gGMacAddress[1] = addr[1];
    gGMacAddress[2] = addr[2];
    gGMacAddress[3] = addr[3];
    gGMacAddress[4] = addr[4];
    gGMacAddress[5] = addr[5];
}

/**
 * Initialization for GMAC device.
 * Should be called at the beginning of the program to set up the
 * network interface.
 */
void gmac_tapdev_init(void)
{
    sGmacd    *pGmacd = &gGmacd;
    GMacb      *pGmacb = &gGmacb;
    uint32_t  dwErrCount = 0 ;

    /* Init GMAC driver structure */
    GMACD_Init(pGmacd, GMAC, ID_GMAC, 1, 0);
    GMACD_InitTransfer(pGmacd, pGRxBuffer, gGRxDs, RX_BUFFERS, pGTxBuffer, gGTxDs, gGTxCbs, TX_BUFFERS);
    GMAC_SetAddress(gGmacd.pHw, 0, gGMacAddress);

    /* Setup GMAC buffers and interrupts */
    IRQ_ConfigureIT(ID_GMAC, (0x0 << 5), GMAC_IrqHandler);
    IRQ_EnableIT(ID_GMAC);

    /* Hard reset PHY*/
    PMC_EnablePeripheral(ID_PIOB);
    PIO_Configure(gmacModePins, 1);

    /* Init GMACB driver */
    GMACB_Init(pGmacb, pGmacd, BOARD_GMAC_PHY_ADDR);

    /* PHY initialize */
    if (!GMACB_InitPhy(pGmacb, BOARD_MCK,  0, 0,  gmacPins, 1))
    {
        printf( "P: PHY Initialize ERROR!\n\r" ) ;
        return;
    }

    /* Auto Negotiate, work in RMII mode */
    if (!GMACB_AutoNegotiate(pGmacb))
    {
        printf( "P: Auto Negotiate ERROR!\n\r" ) ;
        return;
    }
    printf( "P: Link detected \n\r" ) ;
}

/**
 * Read for GMAC device.
 */
uint32_t gmac_tapdev_read( void )
{
    uint32_t pkt_len = 0 ;
    if ( GMACD_OK != GMACD_Poll( &gGmacd, (uint8_t*)uip_buf, UIP_CONF_BUFFER_SIZE, &pkt_len) )
    {
        pkt_len = 0 ;
    }
    return pkt_len ;
}

/**
 * Send to GMAC device
 */
void gmac_tapdev_send( void )
{
    uint8_t gmac_rc ;

    gmac_rc = GMACD_Send( &gGmacd, (void*)uip_buf, uip_len, NULL) ;
    if ( gmac_rc != GMACD_OK )
    {
        TRACE_ERROR( "E: Send, rc 0x%x\n\r", gmac_rc ) ;
    }
}

