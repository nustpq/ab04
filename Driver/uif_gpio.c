/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Unified EVM Interface Board 2.0
*
* Filename      : uif_gpio.c
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include "uif_gpio.h"
#include "uif_object.h"

extern DataSource source_gpio;

const Pin gpio_pins[ ] = { BOARD_REC_GPIO };

/*
*********************************************************************************************************
*                                    _gpio_event_Handler()
*
* Description :  configure interrupt of special pin 
*
* Argument(s) :  pin        : data source instance;
*		 
*                
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void _gpio_event_Handler( const Pin * pin)
{


}

/*
*********************************************************************************************************
*                                    Config_GPIO_Interrupt()
*
* Description :  configure interrupt of special pin 
*
* Argument(s) :  pInstance  : data source instance;
*		 isr_handler: interrupt callback function
*                
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Config_GPIO_Interrupt( void *pInstance, void (*isr_handler)( void ) )
{
    uint8_t const GPIO_PRIORITY = 3;
    uint8_t per_id;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Pin *pPins = ( Pin * )pSource->privateData;
   
    per_id = ( uint8_t )pPins->id;
    IRQ_DisableIT( per_id );
    
    pPins->pio->PIO_ISR;
    pPins->pio->PIO_AIMER =   pPins->mask;
    pPins->pio->PIO_IER =     pPins->mask;       //enable int
    pPins->pio->PIO_ESR =     pPins->mask;       //edge int
    pPins->pio->PIO_REHLSR =  pPins->mask;       //rising edge int
    pPins->pio->PIO_IFER =    pPins->mask;       //enable input glitch filter

    IRQ_ConfigureIT( per_id, GPIO_PRIORITY, isr_handler );
    IRQ_EnableIT( per_id );
    
}

/*
*********************************************************************************************************
*                                    Disable_GPIO_Interrupt()
*
* Description :  disable interrupt of special pin 
*
* Argument(s) :  pInstance  : data source instance;
*		 
*                
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Disable_GPIO_Interrupt( void *pInstance )
{  
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    Pin *pPins = ( Pin * )pSource->privateData;
    
    pPins->pio->PIO_IDR = pPins->mask; //enable int
        
}

/*
*********************************************************************************************************
*                                    _ConfigureRecGpios()
*
* Description :  configure  special pin 
*
* Argument(s) :  none 
*		 
*                
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void _ConfigureRecGpios( void )
{
    uint8_t const GPIO_PRIORITY = 3;
    
    /* Configure pios as inputs. */
    PIO_Configure( gpio_pins, PIO_LISTSIZE( gpio_pins )  ) ;

    /* Adjust pio debounce filter patameters, uses 10 Hz filter. */
//    PIO_SetDebounceFilter( &BOARD_GPIO_REC, 10 ) ;

    /* Enable PIO controller IRQs. */
    PIO_InitializeInterrupts( GPIO_PRIORITY );
    /* Initialize pios interrupt handlers, see PIO definition in board.h. */
    PIO_ConfigureIt( gpio_pins, ( void (*)( const Pin * ) )_gpio_event_Handler );

    /* Enable PIO line interrupts. */
    PIO_EnableIt( gpio_pins ) ;
}


/*
*********************************************************************************************************
*                                    gpio_Init()
*
* Description :  initial  gpio 
*
* Argument(s) :  pInstance  : data source instance;
*		 pdata      : pins level
*                mask       : which pins to set
*
* Return(s)   :  None.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
void gpio_Init( void *pInstance,void *dParameter )
{
    assert( NULL != pInstance );
    dParameter = dParameter;

    DataSource *pSource = ( DataSource * )pInstance;
    Pin *pPins = ( Pin * )pSource->privateData;
    
    if( pSource->dev.direct == IN )
    {
          _ConfigureRecGpios( );
    }
    else if( pSource->dev.direct == OUT )
    {
      pPins->pio->PIO_IDR = pPins->mask;     //disable interrupt
    

      pPins->pio->PIO_PUER = pPins->mask;    //pull-up enabled
      pPins->pio->PIO_MDDR = pPins->mask;    //multi-driver disabled
    
    // Enable filter(s)
      pPins->pio->PIO_IFER = pPins->mask;    //glitch input filter enable
   
      pPins->pio->PIO_PER = pPins->mask;     //pio  enable  
    }
    else
    {
      printf( "Please set gpio direct IN or OUT \t\n");
    }
    

}


/*
*********************************************************************************************************
*                                    gpio_Pin_Set()
*
* Description :  set gpio level 
*
* Argument(s) :  pInstance  : data source instance;
*		 pdata      : pins level
*                mask       : which pins to set
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t  gpio_Pin_Set( void *pInstance, const uint8_t * pdata,uint32_t mask )
{    
    assert( NULL != pInstance );
    assert( NULL != pdata );
    assert( 0 < mask );

    DataSource *pSource = ( DataSource * )pInstance;
    Pin *pPins = ( Pin * )pSource->privateData;
    
    switch( *pdata ) 
    {
        
        case 0 : //output 0            
            pPins->pio->PIO_CODR = mask; 
            pPins->pio->PIO_OER  = mask;
        break;
        
        case 1 : //output 1
            pPins->pio->PIO_SODR = mask;
            pPins->pio->PIO_OER  = mask;
        break; 
        
        case 2 : //input        
            pPins->pio->PIO_ODR  = mask;
        break;
        
        default:
        break;             
   } 
   
   return 0;
}


/*
*********************************************************************************************************
*                                    gpio_Pin_Get()
*
* Description :  get gpio port level
*
* Argument(s) :  pInstance  : data source instance;
*		 pdata      : pins level passed from uplayer and return the data
*                mask       : which pins to get
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
uint8_t  gpio_Pin_Get( void *pInstance, const uint8_t * pdata,uint32_t mask )
{
    assert( NULL != pInstance );
    assert( NULL != pdata );  
    assert( 0 < mask );
    
    const uint8_t pinCnt = 10;
    
    uint16_t gpio_data[ 10 ] = { 0 };
    uint8_t temp = 0, i = 0,j,n;
    
    DataSource *pSource = ( DataSource * )pInstance;
    Pin *pPins = ( Pin * )pSource->privateData;
    GPIO_REC_CFG *gpio_cfg = ( GPIO_REC_CFG * )pSource->peripheralParameter;
        
    temp = pPins->pio->PIO_PDSR;
    
    for( i = 0, n = 0 ; i < pinCnt ; i++ ) 
    {
     
        if( gpio_cfg->mask & ( 1<< i ) ) 
	{
            if( temp & ( 1 << i ) ) 
            {
                gpio_data[ n++ ] = 0x3FFF;  //high level
            } 
            else 
            {
                gpio_data[ n++ ] = 0;       //low level
            }
        }
    }

/*    
  uint8_t mask;             //bitmap for gpio which used for rec-->global_rec_gpio_mask
  uint8_t gpioscnt;         //count pins in using-->global_rec_gpio_num
  uint8_t index;            //start index-->global_rec_gpio_index
  uint8_t data_mask;        //bitmap for gpio level which used
  
  uint8_t tdmChannelCnt;    //how much channels for tdm --->global_rec_num
  uint8_t sampleCnt;        //samples per package of one interruption-->global_rec_samples     
*/    
    for( i = 0; i < gpio_cfg->sampleCnt ; i++ ) 
    { //2ms buffer        
      for( j = 0; j < gpio_cfg->gpioscnt ; j ++ ) 
      {      

         *( uint8_t * )( pdata + gpio_cfg->index + j ) = gpio_data[ j ];
        }
        
        pdata += gpio_cfg->tdmChannelCnt ;
    }
    
    //copy data to target address.Because it is const,so...
    //memcpy( ( void * )pdata, gpio_data,sizeof( gpio_data ) );
    
    return 0;
}


//// additional time delay :  +10us
//// so, the critical time delay is 11us
void  GPIOPIN_Set_Session( uint32_t pin , uint32_t dat )
{    
     
}







