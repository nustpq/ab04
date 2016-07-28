/*
*********************************************************************************************************
*
*                                              APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Unified EVM Interface Board
*
* Filename          :  uif_twi.c
* Version           :  V0.0.1
* Programmer(s)     :  Leo
*********************************************************************************************************
* Note(s)           : twi communicate implement
*********************************************************************************************************
*/

#include "uif_twi.h"

#include <ucos_ii.h>

//for compatiblility£¬this structure unused£»
/*
typedef struct i2c_device
{
  DataSource *pI2c;
  OPTIONPARAMETER *first; 
  OPTIONPARAMETER *second;
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
static Twid twid[ MAXTWI ];


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
    TWID_Handler( &twid[ CODEC ] );
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
    Twi *pTwi = ( Twi * )pSource->dev.instanceHandle;
    
    uint32_t hz = *( uint32_t * )pFreq;   
    assert( 0 != hz );
    
    /* Configure TWI pins. no other choice,so hardcode here*/
	if( ID_TWI0 == pSource->dev.identify )
	{
        PIO_Configure( pins_uname, PIO_LISTSIZE( pins_uname ) );
	}
	else if( ID_TWI1 == pSource->dev.identify )
	{
		PIO_Configure( pins_codec, PIO_LISTSIZE( pins_codec ) );
	}
	else if( ID_TWI2 == pSource->dev.identify )
	{
		PIO_Configure( pins_fm36, PIO_LISTSIZE( pins_fm36 ) );	
	}
	else
	{
		APP_TRACE_INFO(("ALERT:error deive used,please check it!\n\t"));
	}
    /* Enable TWI peripheral clock */
    PMC_EnablePeripheral( pSource->dev.identify );

    /* Configure TWI */
    TWI_ConfigureMaster( pTwi, hz , BOARD_MCK );
 
    /* Configure TWI interrupts */
    if( ID_TWI0 == pSource->dev.identify )
    {
          TWID_Initialize(&twid[ UNAMED ], pTwi);
          IRQ_ConfigureIT( pSource->dev.identify, 0, TWI0_IrqHandler );      
    }
    else if( ID_TWI1 == pSource->dev.identify )
    {
	TWID_Initialize(&twid[ CODEC ], pTwi);
        IRQ_ConfigureIT( pSource->dev.identify, 0, TWI1_IrqHandler );        
    }
    else if( ID_TWI2 == pSource->dev.identify )
    {
	TWID_Initialize(&twid[ FM36 ], pTwi);	
        IRQ_ConfigureIT( pSource->dev.identify, 0, TWI2_IrqHandler );
    }
    else
    {
	APP_TRACE_INFO(("ALERT:error deive used,please check it!\n\t"));
    }


//    IRQ_ConfigureIT( pSource->dev.identify, 0, TWI1_IrqHandler );
    IRQ_EnableIT( pSource->dev.identify );
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
          IRQ_ConfigureIT( pSource->dev.identify, 0, TWI0_IrqHandler );      
    }
    else if( ID_TWI1 == pSource->dev.identify )
    {
        IRQ_ConfigureIT( pSource->dev.identify, 0, TWI1_IrqHandler );        
    }
    else if( ID_TWI2 == pSource->dev.identify )
    {	
        IRQ_ConfigureIT( pSource->dev.identify, 0, TWI2_IrqHandler );
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

uint8_t twi_uname_write(void *pInstance, const uint8_t *buf,uint32_t len  )
{     
        assert( NULL != pInstance );
        assert( NULL != buf );
               
        DataSource *pSource = ( DataSource * )pInstance;
        OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource ->privateData;

	Twid *pTwid = &twid[ UNAMED ];
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
*                                    twi_codec_write()
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

uint8_t twi_codec_write(void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;         

	Twid *pTwid = &twid[ CODEC ];
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
*                                    twi_fm36_write()
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

uint8_t twi_fm36_write(void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;

	Twid *pTwid = &twid[ FM36 ];
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

uint8_t twi_uname_read( void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;        

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
*                                    twi_codec_read()
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

uint8_t twi_codec_read( void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;

	Twid *pTwid = &twid[ CODEC ];
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
*                                    twi_fm36_read()
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

uint8_t twi_fm36_read( void *pInstance, const uint8_t *buf,uint32_t len  )
{        
        assert( NULL != pInstance );
        assert( NULL != buf );
        
        DataSource *pSource = ( DataSource * )pInstance;
        OPTIONPARAMETER *option = ( OPTIONPARAMETER * )pSource->privateData;        
		
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


