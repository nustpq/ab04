#include "uif_hardware_init.h"

/*
*********************************************************************************************************
*                                    usb_init_default()
*
* Description :  initialize usb device to default state;
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
int usb_init_default( void )
{

    //initialize usb object and it's operation 
    source_usb.init_source = init_usb;
    source_usb.buffer_read = NULL;
    source_usb.buffer_write = NULL;
    source_usb.get_direct = NULL;	

    if(NULL != source_usb.init_source )
        source_usb.init_source(NULL,NULL);
    else
    {
        APP_TRACE_INFO(("this version isn't a release version \r\n"));
	return -1;
    }
    return 0;
}


/*
*********************************************************************************************************
*                                    ssc0_init_default()
*
* Description :  initialize ssc0 port to default state;
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
void ssc0_init_default( void )
{
    //initialize ssc0 object and it's operation 
    memset( ( void * )&source_ssc0, 0 , sizeof( DataSource ) );
    memset( ( void * )ssc0_PingPongOut, 0 , sizeof( ssc0_PingPongOut ) ); 
    memset( ( void * )ssc0_PingPongIn, 0 , sizeof( ssc0_PingPongIn ) );     
    source_ssc0.dev.direct = ( uint8_t )BI;
    source_ssc0.dev.identify = ID_SSC0;
    source_ssc0.dev.instanceHandle = (uint32_t)SSC0;
    source_ssc0.status[ IN ] = ( uint8_t )FREE;
    source_ssc0.status[ OUT ] = ( uint8_t )FREE;    
    source_ssc0.tx_index = 0;
    source_ssc0.rx_index = 0;
    source_ssc0.peripheralParameter = ( void * )Audio_Configure_Instance0;
    source_ssc0.warmWaterLevel = 3072;
    source_ssc0.txSize = 3072;
    source_ssc0.rxSize = 3072;     
    
    source_ssc0.init_source = init_I2S;
    source_ssc0.buffer_write = ssc0_buffer_write;
    source_ssc0.buffer_read  = ssc0_buffer_read;
    source_ssc0.peripheral_stop = stop_ssc;
    
    source_ssc0.pRingBulkOut = &ssc0_bulkout_fifo;
    source_ssc0.pRingBulkIn = &ssc0_bulkin_fifo;
    source_ssc0.pBufferOut = ( uint16_t * )ssc0_PingPongOut;
    source_ssc0.pBufferIn = ( uint16_t * )ssc0_PingPongIn;
    
    if( NULL != source_ssc0.init_source )
        source_ssc0.init_source( &source_ssc0,NULL );       
}

/*
*********************************************************************************************************
*                                    ssc1_init_default()
*
* Description :  initialize ssc1 port to default state;
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
void ssc1_init_default( void )
{
      //initialize ssc1 object and it's operation 
    memset( ( void * )&source_ssc1, 0 , sizeof( DataSource ) );
    memset( ( void * )ssc1_PingPongOut, 0 , sizeof( ssc1_PingPongOut ) ); 
    memset( ( void * )ssc1_PingPongIn, 0 , sizeof( ssc1_PingPongIn ) );      
    source_ssc1.dev.direct = ( uint8_t )BI;
    source_ssc1.dev.identify = ID_SSC1;
    source_ssc1.dev.instanceHandle = (uint32_t)SSC1;
    source_ssc1.status[ IN ] = ( uint8_t )FREE;
    source_ssc1.status[ OUT ] = ( uint8_t )FREE; 
    source_ssc1.tx_index = 0;
    source_ssc1.rx_index = 0;
    source_ssc1.peripheralParameter = ( void * )Audio_Configure_Instance1;
    source_ssc1.warmWaterLevel = 3072; 
    source_ssc1.txSize = 3072;
    source_ssc1.rxSize = 3072;    
    
    source_ssc1.init_source = init_I2S;
    source_ssc1.buffer_write = ssc1_buffer_write;
    source_ssc1.buffer_read  = ssc1_buffer_read;
    source_ssc0.peripheral_stop = stop_ssc;    
    
    source_ssc1.pRingBulkOut = &ssc1_bulkout_fifo;
    source_ssc1.pRingBulkIn = &ssc1_bulkin_fifo;
    source_ssc1.pBufferOut = ( uint16_t * )ssc1_PingPongOut;
    source_ssc1.pBufferIn = ( uint16_t * )ssc1_PingPongIn;    
    
    if( NULL != source_ssc1.init_source )
       source_ssc1.init_source( &source_ssc1,NULL );
}

/*
*********************************************************************************************************
*                                    spi0_init_default()
*
* Description :  initialize spi0 port to default state;
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
void spi0_init_default( void )
{
      //initialize spi0 object and it's operation
    SPI_PLAY_REC_CFG spi0_cfg;
    memset( ( void * )&source_spi0, 0 , sizeof( DataSource ) );
    memset( ( void * )&spi0_cfg, 0 , sizeof( SPI_PLAY_REC_CFG ) );
    source_spi0.dev.direct = ( uint8_t )BI;
    source_spi0.dev.identify = ID_SPI0;
    source_spi0.dev.instanceHandle = (uint32_t)SPI0;    
    source_spi0.status[ IN ] = ( uint8_t )FREE;
    source_spi0.status[ OUT ] = ( uint8_t )FREE; 
    source_spi0.tx_index = 0;
    source_spi0.rx_index = 0;
    source_spi0.privateData = spi0_RingBulkIn;
    spi0_cfg.spi_speed = 10 * 1000 * 1000;
    spi0_cfg.spi_mode = 1;
    source_spi0.warmWaterLevel = 3072; 
    source_spi0.txSize = 3072;
    source_spi0.rxSize = 3072;     
    
    source_spi0.init_source = init_spi;
    source_spi0.peripheral_stop = stop_spi;
    source_spi0.buffer_write = _spiDmaTx;
    source_spi0.buffer_read = _spiDmaRx;
    source_spi0.set_peripheral = spi_register_set;
    
    source_spi0.pRingBulkOut = &spi0_bulkOut_fifo;
    source_spi0.pRingBulkIn = &spi0_bulkIn_fifo;
    source_spi0.pBufferOut = ( uint16_t * )spi0_2MSOut;
    source_spi0.pBufferIn = ( uint16_t * )spi0_2MSIn;  
    
    if( NULL != source_spi0.init_source )
       source_spi0.init_source( &source_spi0,&spi0_cfg );
}

/*
*********************************************************************************************************
*                                    spi1_init_default()
*
* Description :  initialize spi1 port to default state;
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
void spi1_init_default( void )
{
      //initialize spi1 object and it's operation
    SPI_PLAY_REC_CFG spi1_cfg;
    memset( ( void * )&source_spi1, 0 , sizeof( DataSource ) );
    memset( ( void * )&spi1_cfg, 0 , sizeof( SPI_PLAY_REC_CFG ) );
    source_spi1.dev.direct = ( uint8_t )BI;
    source_spi1.dev.identify = ID_SPI1;
    source_spi1.dev.instanceHandle = (uint32_t)SPI1;    
    source_spi1.status[ IN ] = ( uint8_t )FREE;
    source_spi1.status[ OUT ] = ( uint8_t )FREE; 
    source_spi1.tx_index = 0;
    source_spi1.rx_index = 0;
    source_spi1.privateData = spi1_RingBulkIn;
    source_spi1.buffer = ( uint8_t * )spi1_2MSOut;
    spi1_cfg.spi_speed = 10 * 1000 * 1000;
    spi1_cfg.spi_mode  = 1;
    source_spi1.warmWaterLevel = 3072;
    source_spi1.txSize = 3072;
    source_spi1.rxSize = 3072;     
    
    source_spi1.init_source = init_spi;
    source_spi1.peripheral_stop = stop_spi;
    source_spi1.buffer_write = _spiDmaTx;
    source_spi1.buffer_read = _spiDmaRx;
    source_spi1.set_peripheral = spi_register_set;
    
    source_spi1.pRingBulkOut = &spi1_bulkOut_fifo;
    source_spi1.pRingBulkIn = &spi1_bulkIn_fifo;
    source_spi1.pBufferOut = ( uint16_t * )spi1_2MSOut;
    source_spi1.pBufferIn = ( uint16_t * )spi1_2MSIn;      
    
    if( NULL != source_spi1.init_source )
       source_spi1.init_source( &source_spi1,&spi1_cfg );
}


/*
*********************************************************************************************************
*                                    twi0_init_default()
*
* Description :  initialize twi0 port to default state;
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
void twi0_init_default( void )
{
    uint32_t twi_hz = 100000;
      //initialize twi0 object and it's operation     
    TWI_CFG twi0ChipConf[ 2 ];
    memset( ( void * )&twi0ChipConf[ 0 ], 0 ,sizeof( TWI_CFG ) << 1 );
    twi0ChipConf[ 0 ].address = 0xc0 >> 1;
    twi0ChipConf[ 0 ].iaddress = 0;
    twi0ChipConf[ 0 ].isize = 0;
    twi0ChipConf[ 0 ].revers = 0;
    
    memset( ( void * )&source_twi0, 0 , sizeof( DataSource ) );
    source_twi0.dev.direct = ( uint8_t )BI;
    source_twi0.dev.identify = ID_TWI0;
    source_twi0.dev.instanceHandle = (uint32_t)TWI0;	
    source_twi0.status[ IN ] = ( uint8_t )FREE;
    source_twi0.status[ OUT ] = ( uint8_t )FREE; 
    source_twi0.tx_index = 0;
    source_twi0.rx_index = 0;
    source_twi0.privateData = &twi0ChipConf[ 0 ];

    source_twi0.init_source = twi_init_master;
    source_twi0.buffer_write = twi0_uname_write;
    source_twi0.buffer_read = twi0_uname_read;
    
    if( NULL != source_twi0.init_source )
        source_twi0.init_source( &source_twi0,&twi_hz );
}


/*
*********************************************************************************************************
*                                    twi1_init_default()
*
* Description :  initialize twi1 port to default state;
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
void twi1_init_default( void )
{
    uint32_t twi_hz = 100000;
    
    //initialize twi0 object and it's operation  
    TWI_CFG twi1_chipConf[ 2 ];
    memset( ( void * )&twi1_chipConf[ 0 ], 0 ,sizeof( TWI_CFG ) << 1 );
    twi1_chipConf[ 0 ].address = 0x18;
    twi1_chipConf[ 0 ].iaddress = 0;
    twi1_chipConf[ 0 ].isize = 0;
    twi1_chipConf[ 0 ].revers = 0;
    
    twi1_chipConf[ 1 ].address = 0x18;
    twi1_chipConf[ 1 ].iaddress = 0;
    twi1_chipConf[ 1 ].isize = 0;
    twi1_chipConf[ 1 ].revers = 0;
    
    memset( ( void * )&source_twi1, 0 , sizeof( DataSource ) );
    source_twi1.dev.direct = ( uint8_t )BI;
    source_twi1.dev.identify = ID_TWI1;
    source_twi1.dev.instanceHandle = (uint32_t)TWI1;	
    source_twi1.status[ IN ] = ( uint8_t )FREE;
    source_twi1.status[ OUT ] = ( uint8_t )FREE; 
    source_twi1.tx_index = 0;
    source_twi1.rx_index = 0;
    source_twi1.privateData = &twi1_chipConf[ 0 ];

    source_twi1.init_source = twi_init_master;
    source_twi1.buffer_write = twi1_write;
    source_twi1.buffer_read = twi1_read;
    
    if( NULL != source_twi1.init_source )
        source_twi1.init_source( &source_twi1,&twi_hz );
}

/*
*********************************************************************************************************
*                                    twi2_init_default()
*
* Description :  initialize twi2 port to default state;
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
void twi2_init_default( void )
{
    uint32_t twi_hz = 100000;
    
    //initialize twi2 object and it's operation      
    TWI_CFG twi2_chipConf[ 2 ];
    memset( ( void * )&twi2_chipConf[ 0 ], 0 ,sizeof( TWI_CFG ) << 1 );
    twi2_chipConf[ 0 ].address = 0x18;
    twi2_chipConf[ 0 ].iaddress = 0;
    twi2_chipConf[ 0 ].isize = 0;
    twi2_chipConf[ 0 ].revers = 0;
    
    twi2_chipConf[ 1 ].address = 0x18;
    twi2_chipConf[ 1 ].iaddress = 0;
    twi2_chipConf[ 1 ].isize = 0;
    twi2_chipConf[ 1 ].revers = 0;    
    
    memset( ( void * )&source_twi2, 0 , sizeof( DataSource ) );
    source_twi2.dev.direct = ( uint8_t )BI;
    source_twi2.dev.identify = ID_TWI2;
    source_twi2.dev.instanceHandle = (uint32_t)TWI2;	
    source_twi2.status[ IN ] = ( uint8_t )FREE;
    source_twi2.status[ OUT ] = ( uint8_t )FREE; 
    source_twi2.tx_index = 0;
    source_twi2.rx_index = 0;
    source_twi2.privateData = &twi2_chipConf[ 0 ];
    
    source_twi2.init_source = twi_init_master;
    source_twi2.buffer_write = twi2_write;
    source_twi2.buffer_read = twi2_read;
    
    if( NULL != source_twi2.init_source )
        source_twi2.init_source( &source_twi2,&twi_hz );
}

/*
*********************************************************************************************************
*                                    usart1_init_default()
*
* Description :  initialize usart1 port to default state;
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
void usart1_init_default( void )
{
    //initialize usart1 object and it's operation 
    memset( ( void * )&source_usart1, 0 , sizeof( DataSource ) );
    source_usart1.dev.direct = ( uint8_t )BI;
    source_usart1.dev.identify = ID_USART1;
    source_usart1.dev.instanceHandle = (uint32_t)USART1;
    source_usart1.status[ IN ] = ( uint8_t )FREE;
    source_usart1.status[ OUT ] = ( uint8_t )FREE; 
    source_usart1.tx_index = 0;
    source_usart1.rx_index = 0;
    
    source_usart1.init_source = usart_init;
    source_usart1.buffer_write = usart1_DmaTx;
    source_usart1.buffer_read = usart1_DmaRx;
    
    if( NULL != source_usart1.init_source )
       source_usart1.init_source( &source_usart1,NULL );
}




/*
*********************************************************************************************************
*                                    gpio_init_default()
*
* Description :  initialize gpio device to default state;
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
void gpio_init_default( void )
{
  extern const Pin gpio_pins[ ];
    //initialize usart1 object and it's operation 
    memset( ( void * )&source_gpio, 0 , sizeof( DataSource ) );
    
    memset( ( void * )&gpio_PingPong_bufferOut, 
              0 , 
              sizeof( gpio_PingPong_bufferOut ) );    
    memset( ( void * )&gpio_PingPong_bufferIn, 
              0 , 
              sizeof( gpio_PingPong_bufferIn ) ); 
    
        ///todo: this struct should be initialzed after receieved command from usb;
    // here,just for test Cpu performence;
    GPIO_REC_CFG gpio_cfg;
    gpio_cfg.mask = 0xf;
    gpio_cfg.sampleCnt = 4;
    gpio_cfg.gpioscnt = 8;
    gpio_cfg.tdmChannelCnt = 4;
    gpio_cfg.index = 0;

    source_gpio.dev.direct = ( uint8_t )BI;
    source_gpio.dev.identify = ID_PIOD;
    source_gpio.dev.instanceHandle = (uint32_t)PIOD;
    source_gpio.status[ IN ] = ( uint8_t )FREE;
    source_gpio.status[ OUT ] = ( uint8_t )FREE;
    source_gpio.privateData = ( void * )gpio_pins;
    source_gpio.peripheralParameter = ( void * )&gpio_cfg;
    source_gpio.tx_index = 0;
    source_gpio.rx_index = 0;
    source_gpio.txSize = 3072;
    source_gpio.rxSize = 3072;
    
    source_gpio.init_source = gpio_Init;
    source_gpio.buffer_write = gpio_Pin_Set;
    source_gpio.buffer_read = gpio_Pin_Get;
    
    source_gpio.pRingBulkOut = &ep0BulkOut_fifo;
    source_gpio.pRingBulkIn = &ep0BulkIn_fifo;
    source_gpio.pBufferOut = ( uint16_t * )gpio_PingPong_bufferOut;
    source_gpio.pBufferIn = ( uint16_t * )gpio_PingPong_bufferIn;    
    
    if( NULL != source_gpio.init_source )
       source_gpio.init_source( &source_gpio,NULL );  
}

/*
*********************************************************************************************************
*                                    aic3204_init_default()
*
* Description :  initialize aic3204 codec to default state;
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
void aic3204_init_default( void )
{
    CODEC_SETS codec_set;
    
    codec_set.sr = 48000;
    codec_set.sample_len = 16;
    codec_set.format = 1;
    codec_set.slot_num = 2;
    codec_set.m_s_sel = 0;
    codec_set.bclk_polarity = 1;
    codec_set.flag = 1;
    codec_set.delay = 0;
    Init_CODEC( &source_twi1,codec_set );
    codec_set.sr = 48000;
    codec_set.sample_len = 16;
    codec_set.format = 1;
    codec_set.slot_num = 2;
    codec_set.m_s_sel = 0;
    codec_set.bclk_polarity = 1;
    codec_set.flag = 1;
    codec_set.delay = 0;
    Init_CODEC( &source_twi2,codec_set );
}


/*
*********************************************************************************************************
*                                    uif_ports_init_default()
*
* Description :  initialize uif ports to default state;
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
void uif_ports_init_default( void )
{  
    usb_init_default( ); //init USB 
    ssc0_init_default( );
    ssc1_init_default( );
    spi0_init_default( );
    spi1_init_default( );
    twi0_init_default( );
    twi1_init_default( );
    twi2_init_default( );
    usart1_init_default( );
    gpio_init_default( );
}

/*
*********************************************************************************************************
*                                    uif_miscPin_init_default()
*
* Description :  initialize uif control pin to default state;
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
void uif_miscPin_init_default( void )
{  
    //BSP_LED_Init();
    UIF_LED_Init();
    //BSP_LED_Off( 3 ); 
    BSP_BUZZER_Toggle( BUZZER_OFF );
    UIF_LED_On( LED_D3 );
    UIF_LED_Off( LED_D3 );
    UIF_LED_On( LED_D4 );
    UIF_LED_Off( LED_D4 );  
    
    //Misc switch initialize
    UIF_Misc_Init( );
    //UIF_Misc_On( HDMI_UIF_PWR_EN );
    UIF_Misc_On ( CODEC0_RST );
    UIF_Misc_On ( CODEC1_RST );
    UIF_Misc_On ( FAST_PLUS_RST );  
  
}