/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2009-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          APPLICATION CODE
*                                         ATMEL ATSAMA5D3X-EK
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JBL
*********************************************************************************************************
* Note(s)       : none.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/


#include  <app_cfg.h>
#include  <lib_mem.h>

#include  <bsp.h>
#include  <bsp_int.h>
#include  <bsp_os.h>
#include  <bsp_cache.h>

#include  <cpu.h>
#include  <cpu_core.h>

#include  <ucos_ii.h>

#include "object.h"
#include "board.h"
#include "ssc.h"
#include "i2s.h"

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
#define PIN_SSC0      PINS_SSC_CODEC

dataSource source_usb;
dataSource source_ssc0;

sDmad g_dmad;

OS_FLAG_GRP *g_StartUSBTransfer;

static TCMR tcmr ;
static TFMR tfmr ;
static RCMR rcmr ;
static RFMR rfmr ;


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  CPU_STK  AppTaskStartStk[4096u];
static  CPU_STK  AppTaskSSC0Stk[4096u];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart  (void        *p_arg);
static  void  AppTaskSSC0  (void        *p_arg);
/*
*********************************************************************************************************
*                                               _config_pins()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if 1
void ISR_SSC0( void )
{
  uint32_t imr,status,pending;
#if 1
  OSIntEnter();                                             /* Tell uC/OS-II that we are starting an ISR            */

//  printf(" SSC: interrupt handle at %s\r\n",__func__);
  
  imr = SSC0->SSC_IMR;
  status = SSC0->SSC_SR;
  
  pending = status & imr;
  
  if( pending & ( SSC_SR_OVRUN ) )
  {
      printf(" SSC:  OVRUN \n" );
  }

  if( pending & (  SSC_SR_RXRDY ) )
  {
      uint32_t trashedValue;
      trashedValue = SSC0->SSC_RHR;
   }
  
  OSIntExit();
#endif
  
}
#endif
             

#if 1
void start_ssc( uint32_t id )
{
    id = id;
  
    SSC_EnableTransmitter( SSC0 );   
    SSC_EnableReceiver( SSC0 );	
}
#endif

/*
*********************************************************************************************************
*                                               _config_pins()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if 0
static void _config_pins( uint32_t id)
{
  static const Pin pins[] = { PIN_SSC0 };
  
	if( SSC0_ID == id )
          PIO_Configure(pins, PIO_LISTSIZE( pins ) );
}
#endif
/*
*********************************************************************************************************
*                                               _get_ssc_instance()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if 1
static Ssc * _get_ssc_instance( uint32_t id )
{
#define MAXCHIP_ID 50         //datasheet page 38;

	assert( id < MAXCHIP_ID );

	if( ( id != (uint32_t )SSC0_ID ) && ( id != (uint32_t )SSC1_ID ) )
		return NULL;
	
	return ( SSC0_ID == id ) ?  SSC0 :  SSC1;

}
#endif

/*
*********************************************************************************************************
*                                               _SSC_init()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if 1
static void _SSC_Init( uint32_t id,
		unsigned int slave,
                unsigned int mclk, 
                unsigned char slot_num, 
                unsigned char slot_len )
{
	assert( ( (uint32_t )SSC0_ID == id ) || ( (uint32_t )SSC1_ID == id ) );
	
    Ssc *pSSC = _get_ssc_instance( id );

	if( NULL == pSSC )
		return;
    
    SSC_Configure(  pSSC,
                    slave,  //0:slave not gen clk 1:gen clk
                    mclk 
                 );    
      
    tcmr.cks    = 0 ;   // 0:MCK 1:RK 2:TK
    rcmr.cks    = 1 ;   // 0:MCK 1:TK 2:RK
    
    tcmr.cko    = 1 ;   // 0:input only 1:continus 2:only transfer
    rcmr.cko    = 0 ;   // 0:input only 1:continus 2:only transfer
    
    tcmr.cki    = 0;    // 0: falling egde send
    rcmr.cki    = 0;    // 1: rising edge lock  
    
    tcmr.start  = 0;    // 4: falling edge trigger for low left, 5: rising edge trigger for high left,
    rcmr.start  = 0;    //0:continuous 1:transmit 2:RF_LOW 3:RF_HIGH 4:RF_FAILLING
    					//5:RF_RISING 6:RF_LEVEL 7:RF_EDGE 8:CMP_0
    tcmr.sttdly = 0;
    rcmr.sttdly = 0;   
	
    tcmr.period = 0;   // period ;  slave not use 0-->15
    rcmr.period = 0;    // period ;  slave not use
    
    tcmr.ckg    = 0 ;   //slave not use
    rcmr.ckg    = 0 ;   //slave not use
       
    tfmr.fsos   = 0 ;   //input only
    rfmr.fsos   = 0 ;   //input only
    
    tfmr.datnb  = slot_num-1;	//5 ; //6 slot TDM
    rfmr.datnb  = slot_num-1;	//5 ; 
    
    tfmr.datlen = slot_len-1;	//31 ; //32bits
    rfmr.datlen = slot_len-1;	//31 ;
    
    tfmr.fslen  = 0 ; 	//frame sync is not used
    rfmr.fslen  = 0 ; 	//frame sync is not used
       
    tfmr.fsedge = 1 ;
    rfmr.fsedge = 1 ;
          
    tfmr.msbf   = 1 ;
    rfmr.msbf   = 1 ;   

    tfmr.datdef = 0 ;
    tfmr.fsden  = 0 ;
    
    rfmr.loop   = 1 ; //0:normal 1:loop 
    
    SSC_ConfigureTransmitter( pSSC,  tcmr.value,  tfmr.value   );
    SSC_ConfigureReceiver(  pSSC,  rcmr.value , rfmr.value   );
    
   
}
#endif

/*
*********************************************************************************************************
*                                               _init_I2S()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if 1
static void _init_I2S( void  )
{
    printf("\r\nInit I2S ..."); 
    _config_pins( SSC0_ID );

    PMC_EnablePeripheral( SSC0_ID );
    IRQ_DisableIT( SSC0_ID );

    /* initialize ssc port to default state,if other config,invoke set_parameter interface */
    _SSC_Init( SSC0_ID ,1,BOARD_MCK , 8 , 16 ); 

    IRQ_ConfigureIT( SSC0_ID, 0, ISR_SSC0 );
    IRQ_EnableIT( SSC0_ID );
    
 
    SSC_EnableInterrupts( SSC0, ( SSC_SR_OVRUN |SSC_SR_RXRDY ));

}
#endif




/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

int main()
{
    CPU_INT08U  os_err;
    
    
    BSP_CachesEn();                                           /* Enable L1 I&D caches.                                      */


    APP_TRACE_INFO(("Application start!\r\n"));

    CPU_Init();

    Mem_Init();
    
    BSP_LED_Init();

#ifndef NEW_VERSION    
    _init_I2S(  );
#else
    /*initialize ssc object and it's operation */
    source_ssc0.dev.identify = SSC0_ID;
    source_ssc0.dev.direct = ( uint8_t )BI;
    source_ssc0.status = (uint8_t )FREE;
    source_ssc0.init_source = init_I2S;
    source_ssc0.peripheral_start = start_ssc;
    
    if( NULL != source_ssc0.init_source )
        source_ssc0.init_source( ( void * )&source_ssc0,NULL );
#endif
    

    OSInit();

    os_err = OSTaskCreateExt( AppTaskStart,   /* Create the start task.                               */
                              DEF_NULL,
                             &AppTaskStartStk[4096 - 1],
                              4u,
                              4u,
                             &AppTaskStartStk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }
    
    os_err = OSTaskCreateExt( AppTaskSSC0,   /* Create the start task.                               */
                              DEF_NULL,
                             &AppTaskSSC0Stk[4096 - 1],
                              6u,
                              6u,
                             &AppTaskSSC0Stk[0],
                              4096u,
                              0u,
                             (OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error creating task. OSTaskCreateExt() returned with error %u\r\n", os_err));
    }


    OSStart();


    if(os_err != OS_ERR_NONE) {
        APP_TRACE_INFO(("Error starting. OSStart() returned with error %u\r\n", os_err));
    }


    return 0;
}


/*
*********************************************************************************************************
*                                             AppTaskStart()
*
* Description : Example task.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{

    BSP_OS_TmrTickInit(1000u);

    for(;;) {
        OSTimeDlyHMSM(0, 0, 0, 500);

        BSP_LED_On(0);

        OSTimeDlyHMSM(0, 0, 0, 500);
        
        BSP_LED_Off(0);

    }

}


/*
*********************************************************************************************************
*                                             AppTaskSSC0()
*
* Description : Example task.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskSSC0 (void *p_arg)
{
    for(;;) {
      if( source_ssc0.status != ( uint8_t )START )
      {
        source_ssc0.status = ( uint8_t )START;
        source_ssc0.peripheral_start( &source_ssc0 );
      }
      
      OS_Sched( );

    }

}

