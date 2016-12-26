/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
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

Pin gpio_pins[ ] = {
                        /*-----------------------------------------------------*/
                            BOARD_REC_GPIO,
                        /*-----------------------------------------------------*/
                            PIN_FPGA_OE,
                            PIN_FPGA_RST,
                            PIN_CODEC1_RST,
                            PIN_CODEC0_RST,
                            PIN_FM36_RST,
                            PIN_PA_SHUTDOWN,
                            PIN_HDMI_PORT_DET0,
                            PIN_FAST_PLUS_RST,
                            PIN_BUZZER,
                            PIN_5V_UIF_EN,
                            PIN_HDMI_UIF_PWR_EN,
                            PIN_LEVEL_SHIFT_OE,
                            PIN_LED_GREEN,
                            PIN_LED_RED,
                            PIN_FPGA_GPO0,
                            PIN_FPGA_GPO1,
                            PIN_LED_PORT0,
                        /*-----------------------------------------------------*/

                            PINS_PUSHBUTTONS,

                          };


static const Pin pinsSwitches[] = {
                                    PIN_MODE_0,
                                    PIN_MODE_1,
                                    PIN_MODE_2,
                                    PIN_MODE_3,
                                  };

static const unsigned int numGpios = PIO_LISTSIZE( gpio_pins );

void GPIO_Init(void)
{

    PIO_Configure( pinsSwitches,  PIO_LISTSIZE(pinsSwitches) );
    PIO_Configure( gpio_pins,     PIO_LISTSIZE(gpio_pins)     );

}


void GPIODIR_FLOAT( unsigned int pin  ) //
{
     unsigned int i;

     for(i=0; i<numGpios; i++)
     {
       if( pin>>i & 0x01 )
       {
           gpio_pins[i].attribute  = PIO_DEFAULT ;
           gpio_pins[i].type       = PIO_INPUT   ;
           PIO_Configure(&gpio_pins[i], 1);
       }

     }
}

unsigned char  GPIOPIN_Set(unsigned int pin , unsigned int dat)
{


    if( pin >= PIO_LISTSIZE( gpio_pins ) ) {
        return SET_GPIO_ERR;

    }

    switch ( dat ) {

        case 0:
           gpio_pins[ pin ].attribute  = PIO_PULLUP ;
           gpio_pins[ pin ].type       = PIO_OUTPUT_0   ;
           PIO_Configure(&gpio_pins[ pin ], 1);
        break;

        case 1:
           gpio_pins[ pin ].attribute  = PIO_PULLUP ;
           gpio_pins[ pin ].type       = PIO_OUTPUT_1   ;
           PIO_Configure(&gpio_pins[ pin ], 1);
        break;

        case 2:
            gpio_pins[ pin ].attribute  = PIO_DEFAULT ;
            gpio_pins[ pin ].type       = PIO_INPUT   ;
            PIO_Configure(&gpio_pins[ pin ], 1);
        break;

        default:
            return SET_GPIO_ERR;
        break;

    }
    return 0;
}

void  __ramfunc GPIOPIN_Set_Fast( unsigned char pin , unsigned char data )
{

    switch( data ) {

        case 0 : //output 0
            gpio_pins[ pin ].pio->PIO_CODR = gpio_pins[ pin ].mask;
            gpio_pins[ pin ].pio->PIO_OER  = gpio_pins[ pin ].mask;
        break;
        case 1 : //output 1
            gpio_pins[ pin ].pio->PIO_SODR = gpio_pins[ pin ].mask;
            gpio_pins[ pin ].pio->PIO_OER  = gpio_pins[ pin ].mask;
        break;
        case 2 : //input
            gpio_pins[ pin ].pio->PIO_ODR  = gpio_pins[ pin ].mask;
        break;
        default:
        break;
   }

}

unsigned char  GPIOPIN_Get(unsigned int pin , unsigned char *pdat)
{
    if( pin >= PIO_LISTSIZE( gpio_pins ) ) {
        return SET_GPIO_ERR;

    }
    *pdat = PIO_Get( &gpio_pins[ pin ] );
    return 0;
}

void  __ramfunc GPIOPIN_Get_Fast( unsigned char pin, unsigned char * pdata )
{
    unsigned int reg ;

    reg = gpio_pins[ pin ].pio->PIO_PDSR;

    if ((reg & gpio_pins[ pin ].mask) == 0) {

        *pdata = 0 ;
    }
    else {

        *pdata = 1 ;
    }
}


void GPIOPIN_Init_Fast( unsigned int pin )
{

    gpio_pins[ pin ].pio->PIO_IDR = gpio_pins[ pin ].mask;

    //pull up
    gpio_pins[ pin ].pio->PIO_PUER = gpio_pins[ pin ].mask;  //enable
    //pinsGpios[pin].pio->PIO_PPUDR = pinsGpios[pin].mask;

    //multi-drive OP
    //pinsGpios[pin].pio->PIO_MDER = pinsGpios[pin].mask;  //enable
    gpio_pins[ pin ].pio->PIO_MDDR = gpio_pins[ pin ].mask;

    // Enable filter(s)
    gpio_pins[ pin ].pio->PIO_IFER = gpio_pins[ pin ].mask;  //enable
    //pinsGpios[pin].pio->PIO_IFDR = pinsGpios[pin].mask;

    gpio_pins[ pin ].pio->PIO_PER = gpio_pins[ pin ].mask;

}


void  __ramfunc GPIOPIN_Set_Session( unsigned int pin , unsigned int dat )
{
    unsigned int i;

    for( i=0; i < numGpios; i++ ) {  //here 28 is used instead of numGpios for speed up !

        if( pin & 0x01<<i ) {

            if( dat  & 0x01<<i ) {
                //PIO_Set( &pinsGpios[i]);
                gpio_pins[i].pio->PIO_SODR = gpio_pins[i].mask;
            } else {
                //PIO_Clear( &pinsGpios[i]);
                gpio_pins[i].pio->PIO_CODR = gpio_pins[i].mask;
            }
        }
     }
}

typedef struct __MONITCTR
{
    unsigned char DataType ;
    unsigned short  DataNum ;

}MONITCTR ;

unsigned int GPIOPIN_Read(void)
{
  /*pq
    GpioPin.portStt.porta = GPIO_ReadPin(PA) ;
    GpioPin.portStt.portc = GPIO_ReadPin(PC) ;
    GpioPin.portStt.portg = GPIO_ReadPin(PG) ;
    GpioPin.portStt.portj = GPIO_ReadPin(PJ) ;

    return(GpioPin.pinStt & PINMASK) ;
  */
  return 0;
}

void  RecordGpio(
               unsigned char pTime,    /*读取数据的时间间隔,单位为uS*/
               unsigned short  dTime,  /*记录数据时间长度,单位为mS*/
               void *p                 /*存放数据位置*/
               )
{

    /*pq
    MONITCTR *pMonitCtr = (MONITCTR *)DataBufCtr.pBufTop ;
    unsigned char *pDataSt ;
    unsigned short tdelay ;
    DataBufCtr.pBufTop += 3 ;
    pMonitCtr->DataType = DATA_TYPE_MONIT ;

    pDataSt  = DataBufCtr.pBufTop ;
    MonitCount = 0;
    pinb = 0x76;

    TIMER_Open_8bit(0,T_INT_EN|T_PRS_8, pTime-2) ;

    while(dTime)
    {
        tdelay = ((dTime>4000)? 4000:dTime) ;
        dTime -= tdelay ;
        CtrFlage.Time3Over = 0 ;

        TIMER_Open_16bit(3,T_INT_EN|T_PRS_1024, tdelay) ;

        while(!CtrFlage.Time3Over) ;
    }

    TCCR0B &= 0xf8 ;      // 关闭定时器0
    OCR0A = 0;
    OCR0B = 0;
    *(unsigned int *)DataBufCtr.pBufTop = MonitCount ;
    DataBufCtr.pBufTop += 4 ;
    *(DataBufCtr.pBufTop++) = PINB ;
    *(DataBufCtr.pBufTop++) = PINC ;
    *(DataBufCtr.pBufTop++) = PIND ;
      DataBufCtr.pBufTop++ ;
    pMonitCtr->DataNum = (DataBufCtr.pBufTop - pDataSt)/8 ;

    */
}

void Ruler_Power_Switch( unsigned char onoff )
{

//    pinsGpios[5].type = (onoff == 0) ? PIO_OUTPUT_0 : PIO_OUTPUT_1 ;
//    PIO_Configure(&pinsGpios[5], 1);

}

//Ruler MCU selector
void UART1_Mixer( unsigned char index )
{
//    unsigned char i;
//    //OSTimeDly(1000);
//    if( index<= 3) {
//        for( i=0; i<=1; i++) {
//            pinsGpios[i].type = (index & (1<<i) ) == 0 ? PIO_OUTPUT_0 : PIO_OUTPUT_1 ;
//            PIO_Configure(&pinsGpios[i], 1);
//
//        }
//
//    }
//    //OSTimeDly(50);

}


//Audio MCU selector
void UART2_Mixer( unsigned char index )
{

#ifdef   BOARD_TYPE_AB01
    unsigned char i;

    if( index<= 3 ) {
        for( i=2; i<=3; i++) {
            pinsGpios[12+i].type = (index & (1<<(i-2)) ) == 0 ? PIO_OUTPUT_0 : PIO_OUTPUT_1 ;
            PIO_Configure(&pinsGpios[i], 1);

        }

    }
#endif
}

static unsigned char I2C_Mix_Index_Save =  0;
//I2C bus selector
unsigned char I2C_Mixer( unsigned char index )
{
#ifdef BOARD_TYPE_UIF

    unsigned char err = 0;

    if( index > 3 || index < 1 ) {
        return 1;
    }

    OSSemPend( GPIO_Sem_I2C_Mixer, 1000, &err );
    if( OS_ERR_NONE != err ) {
        APP_TRACE_INFO(( "I2C_Mixer OSSemPend err [0x%X]\r\n", err ));
        return err;
    }

    if( I2C_Mix_Index_Save == index ) { //no need re-set
        OSSemPost( GPIO_Sem_I2C_Mixer );
        return err;
    }
    I2C_Mix_Index_Save = index ;

//    if( index <= 3) {
//        GPIOPIN_Set_Fast(13, 1);//disable all I2C channels
//        GPIOPIN_Set_Fast(14, 1);//disable all I2C channels
//        GPIOPIN_Set_Fast(15, 1);//disable all I2C channels
//        GPIOPIN_Set_Fast(12+index, 0);//enable index I2C channels
//    }
    for( unsigned char i = 1; i <= 3; i++ ) {
        pinsGpios[12+i].type = (index == i) ? PIO_OUTPUT_0 : PIO_OUTPUT_1 ; ////lowe level truned to high after NPN to control switch OE pin
    }
    PIO_Configure(&pinsGpios[13], 3);
    //APP_TRACE_INFO(("\r\nI2C_Mixer switch to: %d ", index ));

    OSSemPost( GPIO_Sem_I2C_Mixer );

    //OSTimeDly(1);
#endif


}



unsigned int Get_Switches( void )
{

    unsigned char i     =   0 ;
    unsigned int  value =   0 ;

    for( i=0; i<PIO_LISTSIZE( pinsSwitches ); i++ ) {
        value <<= 1;
        value +=PIO_Get( &pinsSwitches[i] );
    }

    return value;

}

//GPIO 0~9 detection  ?/PQ  ??
unsigned int Get_Port_Detect( void )
{
    unsigned char i;
    unsigned int  value =   0 ;

    for( i=0; i<8; i++ ) {
        value <<= 1;
        value +=PIO_Get( &gpio_pins[ i ] );
    }

    return value;

}



//note: turn off and on to update a ruler firmware, hwo about other ruler connections , issue ???
void Ruler_PowerOnOff( unsigned char switches )
{

//    if( switches == 0 ) { //power off
//        PIO_Clear(&pinsGpios[5]);
//    } else { //power on
//        PIO_Set(&pinsGpios[5]);
//    }

}

void Config_GPIO_Interrupt1( unsigned char gpio_index, CPU_FNCT_VOID isr_handler )
{
    unsigned char per_id;

    per_id = (CPU_INT08U )gpio_pins[gpio_index].id;
    IRQ_DisableIT( per_id );
    gpio_pins[gpio_index].pio->PIO_ISR;
    gpio_pins[gpio_index].pio->PIO_AIMER = gpio_pins[gpio_index].mask;
    gpio_pins[gpio_index].pio->PIO_IER = gpio_pins[gpio_index].mask; //enable int
    gpio_pins[gpio_index].pio->PIO_ESR = gpio_pins[gpio_index].mask; //edge int
    gpio_pins[gpio_index].pio->PIO_REHLSR = gpio_pins[gpio_index].mask;//rising edge int
    gpio_pins[gpio_index].pio->PIO_IFER = gpio_pins[gpio_index].mask;//enable input glitch filter
//    BSP_IntVectSet( per_id,(CPU_FNCT_VOID)isr_handler);
    IRQ_ConfigureIT( per_id, 3, (CPU_FNCT_VOID)isr_handler );
    IRQ_EnableIT( per_id );

}

unsigned char Check_GPIO_Intrrupt( unsigned char gpio_index )
{

    if( (gpio_pins[gpio_index].pio->PIO_ISR) & gpio_pins[gpio_index].mask ) {
        return 1;
    } else {
        return 0;
    }
}

void Disable_GPIO_Interrupt1( unsigned char gpio_index )
{
    gpio_pins[gpio_index].pio->PIO_IDR = gpio_pins[gpio_index].mask; //enable int

}

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



/*
*********************************************************************************************************
*                                    stop_gpio()
*
* Description :  stop gpio device
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
void stop_gpio( void * pInstance )
{
   pInstance = pInstance;
    //just reset buffers about this device,nothing to do with hardware port;
   memset( gpio_PingPong_bufferOut, 0 , sizeof( gpio_PingPong_bufferOut ) );
   memset( gpio_PingPong_bufferIn, 0 , sizeof( gpio_PingPong_bufferIn ) );

}





