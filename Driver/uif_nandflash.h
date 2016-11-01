#ifndef _UIF_NANDFLASH_H_
#define _UIF_NANDFLASH_H_

#include <libnandflash.h>
#include <libpmecc.h>

#define BASIC_PAGESIZE 2048
#define SPARE_SIZE      64
#define PAGESIZE        ( BASIC_PAGESIZE + SPARE_SIZE )
#define BLOCKSIZE       ( 64 * PAGESIZE )
#define PLANESIZE       ( 1024 * BLOCKSIZE )
 
#define FIRMWARE_DOWNLOAD_ADDR  2
#define FIRMWARE_UPDATE_ADDR    20
#define VEC_STORE_ADDR          40

 typedef struct _SmcTestMode 
{
    uint8_t     nfc: 1,    /**< NAND Flash Controller 0: disable, 1: enable */
            nfcSram: 1,    /**< NAND Flash Controller host sram 0: disable, 1: enable */
             xfrDma: 1,    /**< Data transfer with DMA*/
                rev: 5;
} SmcTestMode;

/** Nandflash chip enable pin.*/
extern const Pin nfCePin;
/** Nandflash ready/busy pin.*/
extern const Pin nfRbPin;
//Global Onfi PageParameter instance
extern OnfiPageParam g_OnfiPageParameter;

// Nandflash device structure.
extern struct SkipBlockNandFlash g_skipBlockNf;

// Pmecc instance 
extern PmeccDescriptor g_pmeccDesc;

// Nandflash device structure. 
extern struct NandFlashModel g_modelListfromOnfi;

// Global DMA driver instance for all DMA transfers in application. 
extern sDmad g_nand_dmad;

extern uint8_t g_onfiEccCorrectability;
extern uint8_t g_onficompatible;

extern uint8_t nand_pageBuffer[ 1024 * 1024 ];
extern uint8_t nand_patternBuf[ 1024 * 1024 ];

uint8_t nfc_init( void * parameter );
uint8_t pmecc_init( uint8_t *status );
uint8_t uif_write_backupfirmware( void *buf ,uint32_t len);
uint8_t uif_read_backupfirmware( void *buf,uint32_t len );
uint8_t uif_write_mainfirmware( void *buf ,uint32_t len );
uint8_t uif_read_mainfirmware( void *buf,uint32_t len );
uint8_t uif_write_vec( void *buf,uint32_t len );
uint8_t uif_read_vec( void *buf,uint32_t len );
uint8_t uif_read_sysinfo( void *buf,uint32_t len );
uint8_t uif_write_sysinfo( void *buf,uint32_t len );

uint8_t uif_nandflash_readPage(void *buf,uint16_t blockId,uint16_t pageId);


#endif
