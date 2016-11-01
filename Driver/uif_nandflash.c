/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : uif_nandflash.c
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :firmware update and vec store function implement file. Because these are critical operate 
*                for whole system, and the modifier must be very aware of the memory layout, for simple and   
*                safety,i used FIX and SPECIFIED funtion interface.                       
*********************************************************************************************************
*/


#include "uif_nandflash.h"

#include <string.h>

static const uint16_t main_firmware = 2;
static const uint16_t backup_firmware = 20;
static const uint16_t vec_store_block = 40;
static const uint16_t sysinfo_store_block = 60;

//pmecc is initialized or not
uint8_t g_pmeccStatus;

//Global Onfi PageParameter instance
OnfiPageParam g_OnfiPageParameter;

// Nandflash device structure.
struct SkipBlockNandFlash g_skipBlockNf;

// Pmecc instance 
PmeccDescriptor g_pmeccDesc;

// Nandflash device structure. 
struct NandFlashModel g_modelListfromOnfi;

// Global DMA driver instance for all DMA transfers in application. 
sDmad g_nand_dmad;

// Nandflash chip enable pin.
const Pin nfCePin = {0, 0, 0, 0, 0};
// Nandflash ready/busy pin.
const Pin nfRbPin = {0, 0, 0, 0, 0};

uint8_t g_onfiEccCorrectability = 0xFF;
uint8_t g_onficompatible = 0;

//buffer for nandflash operation.the size is 1MB
uint8_t nand_pageBuffer[ 1024 * 1024 ];
uint8_t nand_patternBuf[ 1024 * 1024 ];


/*
*********************************************************************************************************
*                                    nfc_init()
*
* Description :  initialize nfc control for nandflash
*
* Argument(s) :  parameter  : not used, reserve;
*		
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t nfc_init( void *parameter )
{
      volatile static uint8_t isNfcInitResult = 0;
      
      uint8_t const nandflash_bus_width = 8;
      BOARD_ConfigureNandFlash( nandflash_bus_width );
//    Smc_OpConfigure( 0 );
      
    if ( !NandEbiDetect() ) 
    {
        printf( "\tDevice Unknown\n\r" );
        return 0;
    }
    
    if ( NandGetOnfiPageParam ( &g_OnfiPageParameter ) )
    {
            printf( "\tOpen NAND Flash Interface (ONFI)-compliant\n\r" );
            g_modelListfromOnfi.deviceId = g_OnfiPageParameter.manufacturerId;
            g_modelListfromOnfi.options = g_OnfiPageParameter.onfiBusWidth? NandFlashModel_DATABUS16:NandFlashModel_DATABUS8;
            g_modelListfromOnfi.pageSizeInBytes = g_OnfiPageParameter.onfiPageSize;
            g_modelListfromOnfi.spareSizeInBytes = g_OnfiPageParameter.onfiSpareSize;
            g_modelListfromOnfi.deviceSizeInMegaBytes = ( g_OnfiPageParameter.onfiPagesPerBlock \
                                                     * g_OnfiPageParameter.onfiBlocksPerLun
                                                     * g_OnfiPageParameter.onfiPageSize ) / 1024 /1024;
            g_modelListfromOnfi.blockSizeInKBytes = ( g_OnfiPageParameter.onfiPagesPerBlock * g_OnfiPageParameter.onfiPageSize )/ 1024;
            g_onfiEccCorrectability = g_OnfiPageParameter.onfiEccCorrectability;
            
            switch (g_OnfiPageParameter.onfiPageSize) 
            {
                case 256: g_modelListfromOnfi.scheme = &nandSpareScheme256; break;
                case 512: g_modelListfromOnfi.scheme = &nandSpareScheme512; break;
                case 2048: g_modelListfromOnfi.scheme = &nandSpareScheme2048; break;
                case 4096: g_modelListfromOnfi.scheme = &nandSpareScheme4096; break;
                default: g_modelListfromOnfi.scheme = &nandSpareScheme512; break;
            }
            g_onficompatible = 1;
    }
    
    NandDisableInternalEcc( );
    
    memset( &g_skipBlockNf, 0, sizeof( g_skipBlockNf ) );
    
    if (SkipBlockNandFlash_Initialize( &g_skipBlockNf,
                                         ( g_onficompatible ? &g_modelListfromOnfi: 0 ),
                                         BOARD_NF_COMMAND_ADDR,
                                         BOARD_NF_ADDRESS_ADDR,
                                         BOARD_NF_DATA_ADDR,
                                         nfCePin,
                                         nfRbPin ) ) 
    {
        printf("-E- Device Unknown\n\r");
        return 0;
    } 
    
    if( NULL != parameter )
        *(uint8_t * )parameter = 1;
    
    return 0;
}


/*
*********************************************************************************************************
*                                    pmecc_init()
*
* Description :  initialize pmecc control
*
* Argument(s) :  status  :  indicate the pmecc initialized or not ,maybe other 
*		            funtion will use this infomation;
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t pmecc_init( uint8_t *status )
{
	uint8_t ret = 0;
	
	const uint8_t sectorSize = 0;
	const uint8_t eccErrorBit = 4;
	const uint32_t pageSize = 2048;
	const uint32_t spareSize = 64;
	const uint8_t eccOffset = 36;
	
	ret = PMECC_Initialize( &g_pmeccDesc,sectorSize,
				eccErrorBit, 
				pageSize, 
				spareSize, 
				eccOffset, 
				0 );
        
        if( NULL != status )
        {
          *status = ( ( 0 == ret ) ? 1 : 0 );
        }
        
        return ret;
}


/*
*********************************************************************************************************
*                                    uif_write_backupfirmware()
*
* Description :  write firmware data to firmware backup region
*
* Argument(s) :  buf  :  where firmware data from
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_write_backupfirmware( void *buf ,uint32_t len )
{

	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );
        
        ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                            backup_firmware, 
                                            NORMAL_ERASE );

	if( !ret )
	{
		ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                                      backup_firmware + 1, 
                                                      NORMAL_ERASE );
	}
	
	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      backup_firmware,  
                                                      (uint8_t *)buf); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      backup_firmware+1,  
                                                      (uint8_t *)buf + (len >> 1)); 
	}
	return ret;
}


/*
*********************************************************************************************************
*                                    uif_read_backupfirmware()
*
* Description :  read firmware data to memory
*
* Argument(s) :  buf  :  where firmware data to
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t uif_read_backupfirmware( void *buf,uint32_t len )
{
	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    backup_firmware,  
                                                    (uint8_t *)buf ); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    backup_firmware+1,  
                                                    (uint8_t *)buf + (len >> 1) ); 
	}
        
	return ret;
}


/*
*********************************************************************************************************
*                                    uif_write_mainfirmware()
*
* Description :  write firmware data to firmware main region
*
* Argument(s) :  buf  :  where firmware data from
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_write_mainfirmware( void *buf ,uint32_t len )
{

	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );
        
        ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                            main_firmware, 
                                            NORMAL_ERASE );

	if( !ret )
	{
		ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                                      main_firmware + 1, 
                                                      NORMAL_ERASE );
	}
	
	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      main_firmware,  
                                                      (uint8_t *)buf); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      main_firmware+1,  
                                                      (uint8_t *)buf + (len >> 1)); 
	}
	return ret;
}


/*
*********************************************************************************************************
*                                    uif_read_mainfirmware()
*
* Description :  write firmware data to memory
*
* Argument(s) :  buf  :  where firmware data to
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_read_mainfirmware( void *buf,uint32_t len )
{
	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    main_firmware,  
                                                    (uint8_t *)buf ); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    main_firmware+1,  
                                                    (uint8_t *)buf + (len >> 1) ); 
	}
	return ret;

}

/*
*********************************************************************************************************
*                                    uif_write_vec()
*
* Description :  write vec data to special region
*
* Argument(s) :  buf  :  where vec data from
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_write_vec( void *buf,uint32_t len )
{
  	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );
        
        ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                            vec_store_block, 
                                            NORMAL_ERASE );

	if( !ret )
	{
		ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                                      vec_store_block + 1, 
                                                      NORMAL_ERASE );
	}
	
	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      vec_store_block,  
                                                      (uint8_t *)buf); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      vec_store_block+1,  
                                                      (uint8_t *)buf + (len >> 1)); 
	}
	return ret;
}                                                     


/*
*********************************************************************************************************
*                                    uif_read_vec()
*
* Description :  read vec data to memory
*
* Argument(s) :  buf  :  where vec data to
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_read_vec( void *buf,uint32_t len )
{
  	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    vec_store_block,  
                                                    (uint8_t *)buf ); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    vec_store_block+1,  
                                                    (uint8_t *)buf + (len >> 1) ); 
	}
	return ret;
}


/*
*********************************************************************************************************
*                                    uif_write_sysinfo()
*
* Description :  write info data to special region
*
* Argument(s) :  buf  :  where info data from
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_write_sysinfo( void *buf,uint32_t len )
{
  	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );
        
        ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                            sysinfo_store_block, 
                                            NORMAL_ERASE );

	if( !ret )
	{
		ret = SkipBlockNandFlash_EraseBlock( &g_skipBlockNf, 
                                                      sysinfo_store_block + 1, 
                                                      NORMAL_ERASE );
	}
	
	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      sysinfo_store_block,  
                                                      (uint8_t *)buf); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_WriteBlock( &g_skipBlockNf, 
                                                      sysinfo_store_block+1,  
                                                      (uint8_t *)buf + (len >> 1)); 
	}
	return ret;
}                                                     


/*
*********************************************************************************************************
*                                    uif_read_sysinfo()
*
* Description :  read info data to memory
*
* Argument(s) :  buf  :  where info data from
*		 len  :  length of buf in bytes
*                
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t uif_read_sysinfo( void *buf,uint32_t len )
{
  	uint8_t ret = 0;
        
        assert( ( NULL != buf ) || ( 0 < len ) );

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    sysinfo_store_block,  
                                                    (uint8_t *)buf ); 
	}

	if( !ret )
	{
		ret = SkipBlockNandFlash_ReadBlock( &g_skipBlockNf, 
                                                    sysinfo_store_block+1,  
                                                    (uint8_t *)buf + (len >> 1) ); 
	}
	return ret;
}


/*
*********************************************************************************************************
*                                    uif_nandflash_readPage()
*
* Description :  read  data to memory, it's a unimplement and unused interface;
*
* Argument(s) :  buf  :  where info data from
*		 blockid  :  nandflash block id
*                pageId   :  page id in block
*
* Return(s)   :  error code 0:success otherwise failed.
*
* Note(s)     : Unused£»
*********************************************************************************************************
*/                                                     
uint8_t uif_nandflash_readPage(void *buf,uint16_t blockId,uint16_t pageId)
{
    uint8_t ret = 0;
    
    return ret;
}


