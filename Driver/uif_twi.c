/*
*********************************************************************************************************
*
*                                              APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename          :  uif_twi.c
* Version           :  V0.0.1
* Programmer(s)     :  Leo
*********************************************************************************************************
* Note(s)           : twi communicate implement
*********************************************************************************************************
*/
#include "app_cfg.h"
#include "defined.h"
#include "uif_twi.h"

#include <ucos_ii.h>

//for compatiblility£¬this structure unused£»
/*
typedef struct i2c_device
{
  DataSource *pI2c;
  TWI_CFG *first; 
  TWI_CFG *second;
  OS_EVENT *lock;
}FM36,CODEC;
*/

#define UIF_PINS_TWI_TWI0  PINS_TWI0
#define UIF_PINS_TWI_CODEC PINS_TWI1
#define UIF_PINS_TWI_FM36  PINS_TWI2

/** Pio pins to configure. */
static const Pin pins_uname[] = {UIF_PINS_TWI_TWI0};
static const Pin pins_codec[] = {UIF_PINS_TWI_CODEC};
static const Pin pins_fm36[] = {UIF_PINS_TWI_FM36};

/** buffer declare for twi */
extern uint8_t twi_ring_buffer[ MAXTWI ][ 256 ];


/** TWI driver instance.*/
Twid twid[ PMIC ];

/** Twi Async descriptor */
static Async async[ PMIC ];


/*
*********************************************************************************************************
*                                    twiCallback()
*
* Description :  twi read/write callback routine shared with all twi port;
*
* Argument(s) :  none
*			    
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void twiCallback( void )
{
    //do nothing here
}


/*
*********************************************************************************************************
*                                    TWI1_IrqHandler()
*
* Description :  twi interrupt serves routine
*
* Argument(s) :  none
*			    
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static void TWI0_IrqHandler( void )
{
    OS_CPU_SR cpu_sr;
    
    OS_ENTER_CRITICAL( );
    TWID_Handler( &twid[ UNAMED ] );
    OS_EXIT_CRITICAL();
}

static void TWI1_IrqHandler( void )
{
    OS_CPU_SR cpu_sr;
    
    OS_ENTER_CRITICAL( );
    TWID_Handler( &twid[ CODEC1 ] );
    OS_EXIT_CRITICAL();
}

static void TWI2_IrqHandler( void )
{
    OS_CPU_SR cpu_sr;
    
    OS_ENTER_CRITICAL( );
    TWID_Handler( &twid[ FM36 ] );
    OS_EXIT_CRITICAL();
}


/*
*********************************************************************************************************
*                                    twi0_init_master()
*
* Description :  Initialize twix as master
*
* Argument(s) :  freq      : twi clock frequency;
*		 
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void twi0_init_master( uint32_t freq ) 
{
    // Configure TWI pins. 
    PIO_Configure( pins_uname , PIO_LISTSIZE( pins_uname ));
    // Enable TWI peripheral clock 
    PMC_EnablePeripheral(ID_TWI0);

    // Configure TWI 
    TWI_ConfigureMaster( TWI0, freq, BOARD_MCK );
    TWID_Initialize(&twid[ UNAMED ], TWI0 );

    // Configure TWI interrupts 
    IRQ_ConfigureIT( ID_TWI0, 0, TWI0_IrqHandler );
    IRQ_EnableIT( ID_TWI0 );
}

/*
*********************************************************************************************************
*                                    twi1_init_master()
*
* Description :  Initialize twix as master
*
* Argument(s) :  freq      : twi clock frequency;
*		 
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void twi1_init_master( uint32_t freq ) 
{
    // Configure TWI pins. 
    PIO_Configure( pins_codec , PIO_LISTSIZE( pins_codec ));
    // Enable TWI peripheral clock 
    PMC_EnablePeripheral(ID_TWI1);

    // Configure TWI 
    TWI_ConfigureMaster( TWI1, freq, BOARD_MCK );
    TWID_Initialize(&twid[ CODEC1 ], TWI1 );

    // Configure TWI interrupts 
    IRQ_ConfigureIT( ID_TWI1, 0, TWI1_IrqHandler );
    IRQ_EnableIT( ID_TWI1 );
}

/*
*********************************************************************************************************
*                                    twi2_init_master()
*
* Description :  Initialize twix as master
*
* Argument(s) :  freq      : twi clock frequency;
*		 
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void twi2_init_master( uint32_t freq ) 
{
    // Configure TWI pins. 
    PIO_Configure( pins_fm36 , PIO_LISTSIZE( pins_fm36 ));
    // Enable TWI peripheral clock 
    PMC_EnablePeripheral(ID_TWI2);

    // Configure TWI 
    TWI_ConfigureMaster( TWI2, freq, BOARD_MCK );
    TWID_Initialize(&twid[ FM36 ], TWI2 );

    // Configure TWI interrupts 
    IRQ_ConfigureIT( ID_TWI2, 0, TWI2_IrqHandler );
    IRQ_EnableIT( ID_TWI2 );
}



/*
*********************************************************************************************************
*                                    twi_init_master()
*
* Description :  Initialize twix as master
*
* Argument(s) :  pInstance  : data source instance;
*		 pFreq      : twi clock frequency;
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void twi_init_master( void *pInstance, void * pFreq )
{
  
    assert( ( NULL != pInstance ) && ( NULL != pFreq ) );
    
    DataSource *pSource = ( DataSource * )pInstance;
//    Twi *pTwi = ( Twi * )pSource->dev.instanceHandle;
    
    uint32_t hz = *( uint32_t * )pFreq;   
    assert( 0 != hz );
 
    if( ID_TWI0 == pSource->dev.identify )
    {
        twi0_init_master( hz ); 
    }
    else if( ID_TWI1 == pSource->dev.identify )
    {
        twi1_init_master( hz ); 
    }
    else if( ID_TWI2 == pSource->dev.identify )
    {
        twi2_init_master( hz ); 
    }
    else
    {
        APP_TRACE_INFO(("ALERT:error deive used,please check it!\n\t"));
    }
    
}


/*
*********************************************************************************************************
*                                    twi_init_slave()
*
* Description :  Initialize twix as slave
*
* Argument(s) :  pInstance  : data source instance;
*		pSlave      : slave address;
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void twi_init_slave( void *pInstance, void* pSlave )
{
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Twi *pTwi = ( Twi * )pSource->dev.instanceHandle;
    
    uint32_t slave_address = *( uint32_t * )pSlave;   
    assert( 0 != slave_address );

    /* Configure TWI pins. no other choice,so hardcode here*/
	if( ID_TWI0 == pSource->dev.identify )
	{
              PIO_Configure( pins_uname, PIO_LISTSIZE( pins_uname ) );
	}
	else if( ID_TWI1 == pSource->dev.identify )
	{
              PIO_Configure( pins_codec, PIO_LISTSIZE( pins_codec ) );
	}
	else
	{
		PIO_Configure( pins_fm36, PIO_LISTSIZE( pins_fm36 ) );	
	}
    /* Enable TWI peripheral clock */
    PMC_EnablePeripheral( pSource->dev.identify );
    TWI_ConfigureSlave( pTwi, slave_address ) ;

    /* Clear receipt buffer */
    TWI_ReadByte( pTwi ) ;

    /* Configure TWI interrupts */
    if( ID_TWI0 == pSource->dev.identify )
    {
          IRQ_ConfigureIT( pSource->dev.identify, TWI_PRIORITY, TWI0_IrqHandler );      
    }
    else if( ID_TWI1 == pSource->dev.identify )
    {
        IRQ_ConfigureIT( pSource->dev.identify, TWI_PRIORITY, TWI1_IrqHandler );        
    }
    else if( ID_TWI2 == pSource->dev.identify )
    {	
        IRQ_ConfigureIT( pSource->dev.identify, TWI_PRIORITY, TWI2_IrqHandler );
    }
    else
    {
	APP_TRACE_INFO(("ALERT:error deive used,please check it!\n\t"));
    }
//    IRQ_ConfigureIT( pSource->dev.identify, 0, TWI1_IrqHandler);
    IRQ_EnableIT( pSource->dev.identify );
    TWI_EnableIt( pTwi, TWI_SR_SVACC ) ;
}

/*
*********************************************************************************************************
*                                    twi_uname_write()
*
* Description :  write special data via twi0
*
* Argument(s) :  pInstance  : device  instance;
*		 buf        : buffer point;
*                len        : size of data buffer
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t twi0_uname_write(void *pInstance, const uint8_t *buf,uint32_t len  )
{     
        assert( NULL != pInstance );
        assert( NULL != buf );
               
        DataSource *pSource = ( DataSource * )pInstance;
        TWI_CFG *option = ( TWI_CFG * )pSource ->privateData;

	Twid *pTwid = &twid[ UNAMED ];  //TWI 0
	assert( NULL != pTwid->pTwi );		
        
        
        if( 0 == len ) return -1;
        
        memset(&async[ UNAMED ], 0, sizeof(Async));
        async[ UNAMED ].callback = ( void * ) twiCallback;
	
#ifndef SYNC
	return TWID_Write( pTwid, option->address, 
                                 option->iaddress, 
                                 option->isize, 
                                 ( uint8_t * )buf, 
                                 len, 
                                 0 );	
#else
       	 TWID_Write( pTwid, option->address, 
                                 option->iaddress, 
                                 option->isize, 
                                 ( uint8_t * )buf, 
                                 len, 
                                 &async[ UNAMED ] );  
        while ( !ASYNC_IsFinished( &async[ UNAMED ]  ) ) ;
#endif
}

/*
*********************************************************************************************************
*                                    twi1_write()
*
* Description :  write special data via twi1
*
* Argument(s) :  pInstance  : device instance;
*	         buf        : buffer point;
*                len        :size of data buffer
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t twi1_write(void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        TWI_CFG *option = ( TWI_CFG * )pSource->privateData;         

	Twid *pTwid = &twid[ CODEC1 ];  // TWI 1
	assert( NULL != pTwid->pTwi );		
       
        if( 0 == len ) return -1;
      
       return TWID_Write( pTwid, option->address, 
                                 option->iaddress, 
                                 option->isize, 
                                 ( uint8_t * )buf, 
                                 len, 
                                 0 );

}

 
/*
*********************************************************************************************************
*                                    twi2_write()
*
* Description :  write special data via twi2
*
* Argument(s) :  pInstance  : device instance;
*	         buf        : buffer point;
*                len        :size of data buffer
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t twi2_write(void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        TWI_CFG *option = ( TWI_CFG * )pSource->privateData;

	Twid *pTwid = &twid[ FM36 ]; //TWI 2
	assert( NULL != pTwid->pTwi );			
        
        if( 0 == len ) return -1;
        
	return TWID_Write( pTwid, option->address, 
                                  option->iaddress, 
                                  option->isize, 
                                  ( uint8_t * )buf, 
                                  len, 
                                  0 );
}


/*
*********************************************************************************************************
*                                    twi_uname_read()
*
* Description :  read  data via twi0
*
* Argument(s) :  pInstance  : device instance;
*		 buf        : buffer point;
*                len        :size of data buffer
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t twi0_uname_read( void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        TWI_CFG *option = ( TWI_CFG * )pSource->privateData;        

	Twid *pTwid = &twid[ UNAMED ];
	assert( NULL != pTwid->pTwi );				
        
        if( 0 == len ) return -1;
        
        return TWID_Read( pTwid, option->address, 
                                 option->iaddress,
                                 option->iaddress, 
                                 ( uint8_t * )buf, 
                                 len, 
                                 0 );
}


/*
*********************************************************************************************************
*                                    twi1_read()
*
* Description :  read  data via twi1
*
* Argument(s) :  pInstance  : device address;
*		 buf        : buffer point;
*                len        :size of data buffer
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t twi1_read( void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        TWI_CFG *option = ( TWI_CFG * )pSource->privateData;

	Twid *pTwid = &twid[ CODEC1 ];
	assert( NULL != pTwid->pTwi );		
        
        if( 0 == len ) return -1;
        
	return TWID_Read( pTwid, option->address, 
                                 option->iaddress, 
                                 option->isize, 
                                 ( uint8_t * )buf, 
                                 len, 
                                 0 );
}

/*
*********************************************************************************************************
*                                    twi2_read()
*
* Description :  read  data via twi2
*
* Argument(s) :  pInstance  : device instance;
*		 buf        : buffer point;
*                len        :size of data buffer
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

uint8_t twi2_read( void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        TWI_CFG *option = ( TWI_CFG * )pSource->privateData;        
		
	Twid *pTwid = &twid[ FM36 ];
	assert( NULL != pTwid->pTwi );
        
        if( 0 == len ) return -1;
        
        return TWID_Read( pTwid, option->address, 
                                 option->iaddress, 
                                 option->isize, 
                                 ( uint8_t * )buf, 
                                 len, 
                                 0 );
}


///////////////////////////////////////////////////////////////////////////////////////////////
unsigned char TWID_WriteBuffer_API( 
    uint8_t address,
    uint32_t iaddress,
    uint8_t isize,
    uint8_t *pData,
    uint32_t num,
    Async *pAsync)
{
  
    return TWID_Write(  &twid[0], address, iaddress, isize, pData, num, pAsync);
      
}
   
unsigned char TWID_ReadBuffer_API(
    uint8_t address,
    uint32_t iaddress,
    uint8_t isize,
    uint8_t *pData,
    uint32_t num,
    Async *pAsync)
{
    
    return TWID_Read(  &twid[0], address, iaddress, isize, pData, num, pAsync);
 
}