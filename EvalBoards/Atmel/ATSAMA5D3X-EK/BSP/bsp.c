/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*
*                          (c) Copyright 2003-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               This BSP is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
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
*                                    MICRIUM BOARD SUPPORT PACKAGE
*                                      ATSAMA5D3x Evaluation Kit
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : JBL
* Editor        : Leo
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/


#include <includes.h>


/*
*********************************      Version Declaration       ****************************************
*/
const CPU_CHAR fw_version[]  = "[FW:V0.9982]"; //fixed size string

#ifdef  BOARD_TYPE_AB04
const CPU_CHAR hw_version[]  = "[HW:V1.0]";
const CPU_CHAR hw_model[]    = "[AB04][EVM:FM1388]";
#endif

OS_EVENT *Bsp_Ser_Tx_Sem_lock;
OS_EVENT *Bsp_Ser_Rx_Sem_lock;
OS_EVENT *DBGU_UART_Tx_Sem_lock;
OS_EVENT *DBGU_USB_Tx_Sem_lock;


CPU_INT08U Debug_COM_Sel = 0 ; //debug uart use:    0: DBGUART, 1: UART1, >1: debug muted


/*
*********************************************************************************************************
*                                            BSP_PLL_GetFreq()
*
* Description : Enable  and configure the PLL
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE      If the PLL is enabled and stabilized
*               DEF_FALSE     If the PLL could not be stabilized during a period of time
*                             specified by BSP_PLL_MAX_TIMEOUT
*
* Caller(s)   : Application.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_INT32U  BSP_PLL_GetFreq (void)
{
    CPU_INT08U  pll_div;
    CPU_INT16U  pll_mul;
    CPU_INT32U  freq;


    pll_div = ((SAMA5_REG_PMC_PLLAR >>  0) & 0x0FF);
    pll_mul = ((SAMA5_REG_PMC_PLLAR >> 18) & 0x7F);

    if (pll_div != 0) {
        freq = (BOARD_MAINOSC * (pll_mul + 1)) / pll_div;
    }

    return (freq);
}

/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Get the processor clock frequency.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    CPU_INT32U  mclk_css;
    CPU_INT32U  cpu_freq;
    CPU_INT32U  pclk_div;
    CPU_INT32U  plla_div2;

    mclk_css = (SAMA5_REG_PMC_MCKR   )    & 0x00000003;    /* Get the master clk source                          */
    pclk_div = (SAMA5_REG_PMC_MCKR >> 4)  & 0x00000007;    /* Get the processor clk preescaler                      */
    pclk_div = DEF_BIT(pclk_div);
    plla_div2= (SAMA5_REG_PMC_MCKR >> 12) & 0x00000001;

    switch (mclk_css) {

        case BSP_CLK_SRC_SLOW:                                 /* Slow clock                                         */
             cpu_freq = BOARD_SLOW_XTAL;
             break;

        case BSP_CLK_SRC_MAIN:                                 /* Main clock                                         */
             cpu_freq = BOARD_MAINOSC;
             break;

        case BSP_CLK_SRC_PLLA:                                  /* PLL clock                                          */
             cpu_freq = BSP_PLL_GetFreq();
             cpu_freq = cpu_freq >> plla_div2;
             break;

        //case BSP_CLK_SRC_RESERVED:
        default:
             cpu_freq = 0;
             break;
    }

    cpu_freq = cpu_freq / pclk_div;
    return (cpu_freq);
}


CPU_INT32U  BSP_MCK_ClkFreq (void)
{

    CPU_INT32U  mclk_div;
    CPU_INT32U  MCK;

    mclk_div = (SAMA5_REG_PMC_MCKR >> 8)  & 0x00000003;    /* Get the master clk preescaler                      */


    MCK =  BSP_CPU_ClkFreq() / ( mclk_div + 1 );
    return (MCK);
}
/*
*********************************************************************************************************
*********************************************************************************************************
*                                            GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/
#define LED_ORDER(x) DEF_BIT_#x



/*
*********************************************************************************************************
*                                             BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*********************************************************************************************************
*/
void  BSP_Init (void)
{

    BSP_OS_TmrTickInit( TICK_PER_SECOND );

    Init_Debug_FIFO();
    Init_CMD_Bulk_FIFO();
    Init_Ruler_CMD_FIFO();
    Init_Audio_Bulk_FIFO();

    uif_miscPin_init_default();
    uif_ports_init_default();
    GPIO_Init();
    Init_USB(); //init USB
    usart0_init();
    
    list_init( &portsList , NULL );
    portsList.match = matchPath;

    list_init( &ssc0_data , NULL );
    ssc0_data.match = NULL;  

    init_fpga_instance();

    //config port dma
    Dma_configure( );

    init_spi0( NULL , NULL );

    //initialize Tc1 interval = 1ms
    _ConfigureTc1( 1000u );  

}


/*
*********************************************************************************************************
*                                            BSP_LED_Init()
*
* Description : Initialise user LEDs.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  BSP_LED_Init (void)
{

    SAMA5_REG_PIOE_PER = (DEF_BIT_24 | DEF_BIT_25 | DEF_BIT_10 );
    SAMA5_REG_PIOE_OER = (DEF_BIT_24 | DEF_BIT_25 | DEF_BIT_10 );


    return;
}


/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : Turn ON a led.
*
* Argument(s) : led     led number.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
void BSP_LED_On (CPU_INT32U led)
{
    switch (led) {
        case 1:
            SAMA5_REG_PIOE_SODR = DEF_BIT_24;
            break;

        case 2:
            SAMA5_REG_PIOE_CODR = DEF_BIT_25;
            break;

        case 3:
            SAMA5_REG_PIOE_SODR = DEF_BIT_10;
            break;

        case 0:
        default:
            SAMA5_REG_PIOE_SODR = DEF_BIT_24;
            SAMA5_REG_PIOE_CODR = DEF_BIT_25;
            break;

    }
}


/*
*********************************************************************************************************
*                                             BSP_LED_Off()
*
* Description : Turn OFF a led.
*
* Argument(s) : led     led number.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void BSP_LED_Off (CPU_INT32U led)
{
    switch (led) {
        case 1:
            SAMA5_REG_PIOE_CODR = DEF_BIT_24;
            break;

        case 2:
            SAMA5_REG_PIOE_SODR = DEF_BIT_25;
            break;

        case 3:
            SAMA5_REG_PIOE_CODR = DEF_BIT_10;
            break;

        case 0:
        default:
            SAMA5_REG_PIOE_CODR = DEF_BIT_24;
            SAMA5_REG_PIOE_SODR = DEF_BIT_25;
            break;

    }
}




/*
*********************************************************************************************************
*                                             BSP_LED_Toggle()
*
* Description : Toggle a led state
*
* Argument(s) : led     led number.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void BSP_LED_Toggle( CPU_INT32U led )
{
    CPU_REG32 status = 0;

    status = SAMA5_REG_PIOE_ODSR;

    switch( led ) {
        case 1:
            if( status &  DEF_BIT_24 )
              SAMA5_REG_PIOE_CODR = DEF_BIT_24;
            else
              SAMA5_REG_PIOE_SODR = DEF_BIT_24;
        break;

        case 2:
            if( status &  DEF_BIT_24 )
              SAMA5_REG_PIOE_CODR = DEF_BIT_25;
            else
              SAMA5_REG_PIOE_SODR = DEF_BIT_25;
        break;

        case 3:
            if( status &  DEF_BIT_24 )
              SAMA5_REG_PIOE_CODR = DEF_BIT_10;
            else
              SAMA5_REG_PIOE_SODR = DEF_BIT_10;
        break;

      case 0:
      default:
        break;
    }

}

/*
*********************************************************************************************************
*                                             BSP_BUZZER_Toggle()
*
* Description : Toggle  buzzer state
*
* Argument(s) : state   on/off state.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void BSP_BUZZER_Toggle( CPU_INT32U state )
{

  if( 1 == state )
      SAMA5_REG_PIOA_SODR = DEF_BIT_08;
  else
      SAMA5_REG_PIOA_CODR = DEF_BIT_08;

}

/*
*********************************************************************************************************
*                                             UIF_LED_INIT()
*
* Description : ON/OFF  buzzer state
*
* Argument(s) : state   on/off state.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
void UIF_LED_Init( void )
{

    SAMA5_REG_PIOA_PER  = (DEF_BIT_21 | DEF_BIT_22 | DEF_BIT_24 | DEF_BIT_08 );
    SAMA5_REG_PIOA_OER  = (DEF_BIT_21 | DEF_BIT_22 | DEF_BIT_24 | DEF_BIT_08 );
    SAMA5_REG_PIOA_SODR = (DEF_BIT_21 | DEF_BIT_22 | DEF_BIT_24 | DEF_BIT_08 );  //Clear all LED and Buzzer

}



void UIF_Beep_On ( )
{
    SAMA5_REG_PIOA_CODR = DEF_BIT_08;

}

void UIF_Beep_Off ( )
{
    SAMA5_REG_PIOA_SODR = DEF_BIT_08;

}


volatile unsigned char BUZZER_MUTE = 0 ;

void Buzzer_OnOff( unsigned char onoff )
{

    if( BUZZER_MUTE != 0 ) {
        return;
    }

    if( onoff == 0 ) {
        UIF_Beep_Off ( );

    } else {
        UIF_Beep_On ( );

    }


}

/*
*********************************************************************************************************
*                                             UIF_LED_On()
*
* Description : Turn ON a led.
*
* Argument(s) : led     led number.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
void UIF_LED_On ( CPU_INT32U led )
{
    switch (led) {
        case LED_RUN: //LED_D3
            SAMA5_REG_PIOA_CODR = DEF_BIT_21;
            break;

        case LED_USB: //LED_D4
            SAMA5_REG_PIOA_CODR = DEF_BIT_22;
            break;

        case LED_HDMI: //LED_D5
            SAMA5_REG_PIOA_CODR = DEF_BIT_24;
            break;
        default:
        break;
    }

}

/*
*********************************************************************************************************
*                                             UIF_LED_Off()
*
* Description : Turn OFF a led.
*
* Argument(s) : led     led number.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void UIF_LED_Off ( CPU_INT32U led )
{
    switch (led) {
        case LED_RUN:
            SAMA5_REG_PIOA_SODR = DEF_BIT_21;
            break;

        case LED_USB:
            SAMA5_REG_PIOA_SODR = DEF_BIT_22;
            break;

        case LED_HDMI:  //hdmi interface status indicate;
            SAMA5_REG_PIOA_SODR = DEF_BIT_24;
            break;

        default:
            break;

    }
}


/*
*********************************************************************************************************
*                                             UIF_LED_Toggle()
*
* Description : Toggle a led state
*
* Argument(s) : led     led number.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void UIF_LED_Toggle( CPU_INT32U led )
{
    CPU_REG32 status = 0;

    status = SAMA5_REG_PIOA_ODSR;

    switch( led ) {
        case LED_RUN:
            if( status &  DEF_BIT_21 )
              SAMA5_REG_PIOA_CODR = DEF_BIT_21;
            else
              SAMA5_REG_PIOA_SODR = DEF_BIT_21;
        break;

        case LED_USB:
            if( status &  DEF_BIT_22 )
              SAMA5_REG_PIOA_CODR = DEF_BIT_22;
            else
              SAMA5_REG_PIOA_SODR = DEF_BIT_22;
        break;

        case LED_HDMI:
            if( status &  DEF_BIT_24 )
              SAMA5_REG_PIOA_CODR = DEF_BIT_24;
            else
              SAMA5_REG_PIOA_SODR = DEF_BIT_24;
        break;

      default:
        break;
    }

}

/*
*********************************************************************************************************
*                                         Beep()
*
* Description : Beep Buzzer
*
* Argument(s) : beep times.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskJoy()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void Beep( INT32U beep_cycles)
{

   for( INT32U i = 0; i< beep_cycles; i++)  {

        UIF_Beep_On(); //beep on
        UIF_LED_On(LED_RUN);
        UIF_LED_On(LED_USB);
        UIF_LED_On(LED_HDMI);
        OSTimeDly(250);
        UIF_Beep_Off(); //beep off
        UIF_LED_Off(LED_RUN);
        UIF_LED_Off(LED_USB);
        UIF_LED_Off(LED_HDMI);
        OSTimeDly(250); //delay_ms(250);

    }

}


/*
*********************************************************************************************************
*                                             UIF_Misc_Init()
*
* Description : initialize the uif board misc control pins
*
* Argument(s) : none
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
void UIF_Misc_Init( void )
{

    SAMA5_REG_PIOA_PER = ( DEF_BIT_00 | DEF_BIT_01 | DEF_BIT_02 | DEF_BIT_03    \
                           | DEF_BIT_04 | DEF_BIT_05 | DEF_BIT_07               \
                           | DEF_BIT_09 | DEF_BIT_10 | DEF_BIT_11 );
    SAMA5_REG_PIOA_OER = ( DEF_BIT_00 | DEF_BIT_01 | DEF_BIT_02 | DEF_BIT_03    \
                           | DEF_BIT_04 | DEF_BIT_05 |  DEF_BIT_07              \
                           | DEF_BIT_09 | DEF_BIT_10 | DEF_BIT_11 );

//    SAMA5_REG_PIOA_PER = ( DEF_BIT_00 | DEF_BIT_01 | DEF_BIT_02 );//| DEF_BIT_03 );

//    SAMA5_REG_PIOA_OER = ( DEF_BIT_00 | DEF_BIT_01 | DEF_BIT_02 );//| DEF_BIT_03 );


}

/*
*********************************************************************************************************
*                                             UIF_Misc_On()
*
* Description : Turn ON a switch.
*
* Argument(s) : id switch id.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
void UIF_Misc_On ( CPU_INT32U id )
{
    switch ( id ) {
        case FPGA_OE:
            SAMA5_REG_PIOA_SODR = DEF_BIT_00;
            break;

        case FPGA_RST:
            SAMA5_REG_PIOA_SODR = DEF_BIT_01;
            break;

        case CODEC1_RST:
            SAMA5_REG_PIOA_SODR = DEF_BIT_02;
            break;

        case CODEC0_RST:
            SAMA5_REG_PIOA_SODR = DEF_BIT_03;
            break;

        case FM36_RST:
            SAMA5_REG_PIOA_SODR = DEF_BIT_04;
            break;

        case PA_SHUTDOWN:
            SAMA5_REG_PIOA_SODR = DEF_BIT_05;
            break;

        case FAST_PLUS_RST:
            SAMA5_REG_PIOA_SODR = DEF_BIT_07;
            break;

        case V5_UIF_EN:
            SAMA5_REG_PIOA_SODR = DEF_BIT_09;
            break;

        case HDMI_UIF_PWR_EN:
            SAMA5_REG_PIOA_SODR = DEF_BIT_10;
            break;

        case LEVEL_SHIFT_OE:
            SAMA5_REG_PIOA_SODR = DEF_BIT_11;
            break;

        default:
            break;
    }

}

/*
*********************************************************************************************************
*                                             UIF_Misc_Off()
*
* Description : Turn OFF a switch.
*
* Argument(s) : id switch id.
*
* Return(s)   : none..
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
void UIF_Misc_Off ( CPU_INT32U id )
{
    switch ( id ) {
        case FPGA_OE:
            SAMA5_REG_PIOA_CODR = DEF_BIT_00;
            break;

        case FPGA_RST:
            SAMA5_REG_PIOA_CODR = DEF_BIT_01;
            break;

        case CODEC1_RST:
            SAMA5_REG_PIOA_CODR = DEF_BIT_02;
            break;

        case CODEC0_RST:
            SAMA5_REG_PIOA_CODR = DEF_BIT_03;
            break;

        case FM36_RST:
            SAMA5_REG_PIOA_CODR = DEF_BIT_04;
            break;

        case PA_SHUTDOWN:
            SAMA5_REG_PIOA_CODR = DEF_BIT_05;
            break;

        case FAST_PLUS_RST:
            SAMA5_REG_PIOA_CODR = DEF_BIT_07;
            break;

        case V5_UIF_EN:
            SAMA5_REG_PIOA_CODR = DEF_BIT_09;
            break;

        case HDMI_UIF_PWR_EN:
            SAMA5_REG_PIOA_CODR = DEF_BIT_10;
            break;

        case LEVEL_SHIFT_OE:
            SAMA5_REG_PIOA_CODR = DEF_BIT_11;
            break;

        default:
            break;
    }

}


void UIF_DelayUs( CPU_INT32U us )
{
   const CPU_INT32U limit = 75350303UL;
   const CPU_INT08U unit = 57;
   CPU_INT32U delayUs = 0;

   assert( us < limit );

   delayUs = us * unit;

   while( delayUs -- );

}




/*
*********************************************************************************************************
*                                                BSP_Ser_RdByte()
*
* Description : Read a byte from a serial port and echo byte to port.
*
* Argument(s) : none.
*
* Return(s)   : A byte containing the value of the received charcater.
*
* Note(s)     : (1) This function blocks until a character appears at the port.
*********************************************************************************************************
*/

CPU_INT08U  BSP_Ser_RdByte (void)
{
    CPU_INT08U      rx_byte;
    unsigned char   err;

    OSSemPend( Bsp_Ser_Rx_Sem_lock, 0, &err );

    while (DBGU_IsRxReady() == 0) {     /*  Wait for a byte to show up.                         */
         OSTimeDly(2);
    }
    rx_byte = (CPU_INT08U)(DBGU_ReadChar() & 0x00FF);     /* Read the character.                      */

    OSSemPost( Bsp_Ser_Rx_Sem_lock );

    return (rx_byte);
}

/*
*********************************************************************************************************
*                                                BSP_Ser_WrByte()
*
* Description : Writes a single byte to a serial port.
*
* Argument(s) : tx_byte     The character to output.
*
* Return(s)   : none.
*
* Note(s)     : (1) This functino blocks until room is available in the UART for the byte to be sent.
*********************************************************************************************************
*/

void BSP_Ser_WrByte(CPU_CHAR tx_byte)
{

    unsigned char   err;

    OSSemPend( Bsp_Ser_Tx_Sem_lock, 0, &err );

    DBGU_PutChar(tx_byte);

    OSSemPost( Bsp_Ser_Tx_Sem_lock );

}

/*
*********************************************************************************************************
*                                                BSP_Ser_WrStr()
*
* Description : Write a character string to a serial port.
*
* Argument(s) : tx_str      A character string.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  BSP_Ser_WrStr (CPU_CHAR *tx_str)
{
    while ((*tx_str) != 0) {
//        if (*tx_str == '\n') {
//            BSP_Ser_WrByte('\n');
//            BSP_Ser_WrByte('\r');
//            tx_str++;
//        } else {
            BSP_Ser_WrByte(*tx_str++);
//        }
    }
}

/*
*********************************************************************************************************
*                                                BSP_Ser_Printf()
*
* Description : Formatted outout to the serial port.
*
* Argument(s) : format      Format string follwing the C format convention.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  BSP_Ser_Printf (CPU_CHAR *format, ...)
{
    static  CPU_CHAR  buffer[200 + 1];
            va_list   vArgs;
    
    va_start(vArgs, format);
    vsprintf((char *)buffer, (char const *)format, vArgs);
    va_end(vArgs);

#ifndef DBG_UART_METHOD_TASK_EN
    BSP_Ser_WrStr((CPU_CHAR*) buffer);
#else
    BSP_Ser_WrStr_To_Buffer((CPU_CHAR*) buffer);
#endif

#ifdef DBG_USB_LOG_EN
    BSP_Ser_WrStr_To_Buffer_USB((CPU_CHAR*) buffer);
#endif
}



/*
*********************************************************************************************************
*                                         get_os_state()
*
* Description : get_os_state
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

const CPU_INT08S os_stat_desp[][6] = {"Sem  ","MBox ","Q    ","Suspd","Mutex","Flag ","  *  ","Multi","Ready"} ;

CPU_INT08S* get_os_state( INT8U os_state )
{

  CPU_INT08U i = 0 ;

  for(i=0;i<8;i++) {
    if(os_state == (1<<i) ) {
      return  (CPU_INT08S*)os_stat_desp[i];
    }
  }

  return  (CPU_INT08S*)os_stat_desp[i];

}


/*
*********************************************************************************************************
*                                         Get_Task_Info()
*
* Description : Print task related information
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  Get_Task_Info (void)
{
    OS_TCB    *ptcb;
    CPU_INT08U index = 1 ;

    if (OSRunning == OS_TRUE) {

        APP_TRACE_INFO(("------------------------------------------------------   DEBUG INFORMATION   -------------------------------------------------------\r\n"));
        APP_TRACE_INFO(("|-------------------------   T  A  S  K   --------------------------------------|----------------   S  T  A  C  K   ---------------|\r\n"));
        APP_TRACE_INFO(("| ID  |    Name    | Priority | CtxSwCtr | State | Delay |      Waitting On     |  Point@ | Cur. | Max. | Size | Starts@ | Ends@   |\r\n"));
        ptcb = OSTCBList;                                  /* Point at first TCB in TCB list               */
        while (ptcb->OSTCBPrio != OS_TASK_IDLE_PRIO) {     /* Go through all TCBs in TCB list              */
            //APP_TRACE_INFO(( "|%4d ",ptcb->OSTCBId ));  //same as ptcb->OSTCBPrio
            APP_TRACE_INFO(( "|%2d ",index++ ));
            APP_TRACE_INFO(( "%13.13s",ptcb->OSTCBTaskName ));
            APP_TRACE_INFO(( "      %2d ",ptcb->OSTCBPrio ));
            APP_TRACE_INFO(( "  %10d ",ptcb->OSTCBCtxSwCtr ));
            APP_TRACE_INFO(( "   %s  ",get_os_state( ptcb->OSTCBStat )  ));
            APP_TRACE_INFO(( " %5d ",ptcb->OSTCBDly ));
            APP_TRACE_INFO(( " %22.22s ", (INT32U)(ptcb->OSTCBEventPtr) == 0 ?  (INT8U *)" " : ptcb->OSTCBEventPtr->OSEventName ));

            APP_TRACE_INFO(( " %08X ",ptcb->OSTCBStkPtr ));
            APP_TRACE_INFO(( " %4d ",(ptcb->OSTCBStkBase - ptcb->OSTCBStkPtr)*4 ));
            APP_TRACE_INFO(( " %5d ",ptcb->OSTCBStkUsed ));
            APP_TRACE_INFO(( " %5d ",ptcb->OSTCBStkSize * 4 ));
            APP_TRACE_INFO(( " %08X ",ptcb->OSTCBStkBase ));
            APP_TRACE_INFO(( " %08X ",ptcb->OSTCBStkBottom ));
            APP_TRACE_INFO(( " |\r\n" ));
            ptcb = ptcb->OSTCBNext;                        /* Point at next TCB in TCB list                */
        }
        APP_TRACE_INFO(("------------------------------------------------------------------------------------------------------------------------------------\r\n"));

    }
}

/*
*********************************************************************************************************
*                                         Get_Uptime()
*
* Description : Print run time information
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void Get_Uptime( void )
{

    INT32U time   ;
    INT8U  sec ;
    INT8U  min ;
    INT8U  hour ;
    INT8U  day ;

    time = OSTime / 1000L ;
    sec  = time % 60 ;
    min  = time / 60 %60 ;
    hour = time / 3600 % 24 ;
    day  = time / 3600 / 24 ;

    APP_TRACE_INFO(("OS Uptime  =  %02d days : %02d hours : %02d min : %02d sec\r\n", day,hour,min, sec ));

}


/*
*********************************************************************************************************
*                                    Get_Run_Time()
*
* Description : Format ms tick to [day:hour:min:sec:0.sec], and print.
*
* Argument(s) : time  -  number of 100ms tick from Timer #2 , using global : second_counter .
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void Get_Run_Time( uint32_t time )
{
    
    uint8_t  msec, sec, min, hour;
    uint32_t   day;

    msec = time % 10;
    sec  = time /10 % 60 ;
    min  = time / 600 %60 ;
    hour = time / 36000 %24 ; 
    day  = time / 36000 /24 ;
    printf("[%d:%02d:%02d:%02d.%d]", day, hour, min, sec, msec ); 
    
}



void Time_Stamp( void )
{

    INT32U time   ;
    INT8U  sec ;
    INT8U  min ;
    INT8U  hour ;
    INT16U  msec ;

    msec = OSTime % 1000L ;
    time = OSTime / 1000L ;
    sec  = time % 60 ;
    min  = time / 60 %60 ;
    hour = time / 3600 % 24 ;

    APP_TRACE_INFO(("\r\n\r\n[%02d:%02d:%02d.%03d] ", hour,min, sec, msec ));

}



void  Get_Flash_Info (void)
{
    unsigned char i = 0;
    FLASH_INFO flash_info;

    APP_TRACE_INFO(("\r\n"));
    APP_TRACE_INFO(("------------------------------------------------------   FLASH INFORMATION   -------------------------------------------------------\r\n"));
    for(i=0; i<FLASH_ADDR_FW_VEC_NUM; i++ ) {
        Read_Flash_State(&flash_info, i==0 ? FLASH_ADDR_FW_STATE : (FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * i) );
        if(flash_info.flag == 0x55 ) {
            APP_TRACE_INFO(("------- Flash Seg[%d] >>>\r\n",i));
            APP_TRACE_INFO(("Flash Write Cycle:       State_Page = %d cycles,  Data_Page = %d cycles\r\n", flash_info.s_w_counter,flash_info.f_w_counter ));
            APP_TRACE_INFO(("Bin File:                \"%s\" (%d Bytes), [0x%0X, %s]\r\n", (flash_info.f_w_state == FW_DOWNLAD_STATE_FINISHED ? flash_info.bin_name : " ?? "), flash_info.bin_size, flash_info.f_w_state,(flash_info.f_w_state == FW_DOWNLAD_STATE_FINISHED ? "OK" : "Error")));
        }
    }
    APP_TRACE_INFO(("------------------------------------------------------------------------------------------------------------------------------------\r\n"));

}


/*
*********************************************************************************************************
*                                         Head_Info()
*
* Description : Print Head information
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskUserIF()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void Head_Info ( void )
{
    
    APP_TRACE_INFO(("\r\n\r\n"));
    APP_TRACE_INFO(("-----------------------------------------------------------\r\n"));
    APP_TRACE_INFO(("----                    Fortemedia                    -----\r\n"));
    APP_TRACE_INFO(("----             Audio Bridge V4 (AB04) Board         -----\r\n"));
    APP_TRACE_INFO(("----     %20s-%s               -----\r\n", hw_model, fw_version));
    APP_TRACE_INFO(("----   Compiled  @    %12s, %8s, by PQ   -----\r\n", __DATE__, __TIME__));
    APP_TRACE_INFO(("-----------------------------------------------------------\r\n"));
    APP_TRACE_INFO(("\r\n"));
    APP_TRACE_INFO(("------------------------------------------------------------------------------------------------------------------------------------\r\n"));
    APP_TRACE_INFO(("Micrium uC/OS-II on the Atmel ATSAMA5D36. Version : V%d.%d \r\n",(OSVersion()/ 100),(OSVersion() % 100)  ));
    APP_TRACE_INFO(("CPU Usage = %d%%, CPU Speed = %3d MHz, MCK Speed = %3d MHz, Tick_Per_Second = %6d ticks/sec  \r\n", OSCPUUsage,  (BSP_CPU_ClkFreq() / 1000000L), (BSP_MCK_ClkFreq() / 1000000L),OS_TICKS_PER_SEC ));
    APP_TRACE_INFO(("#Ticks = %8d, #CtxSw = %8d \r\n", OSTime, OSCtxSwCtr ));
    Get_Uptime();
    APP_TRACE_INFO(("\r\n"));
    APP_TRACE_INFO(("-------------------------------------------------   GLOBAL VARIABLES STATUS   ------------------------------------------------------\r\n"));
    APP_TRACE_INFO(("MEM_Part_MsgUART :         %7d(Max%2d) / %2d   of the memory partiation used\r\n", pMEM_Part_MsgUART->OSMemNBlks - pMEM_Part_MsgUART->OSMemNFree, pMEM_Part_MsgUART->OSMemNBlks - pMEM_Part_MsgUART->OSMemNFreeMin,  pMEM_Part_MsgUART->OSMemNBlks));
    //APP_TRACE_INFO(("Tx_ReSend_Happens:         %7d   times happened\r\n", Tx_ReSend_Happens ));
    //APP_TRACE_INFO(("Tx_ReSend_Happens_Ruler:   %7d   times happened\r\n", Tx_ReSend_Happens_Ruler ));
    //APP_TRACE_INFO(("TWI_Sem_lock:              %7d   ( default 1 )\r\n", TWI_Sem_lock->OSEventCnt ));
    //APP_TRACE_INFO(("TWI_Sem_done:              %7d   ( default 0 )\r\n", TWI_Sem_done->OSEventCnt ));
    //APP_TRACE_INFO(("UART_MUX_Sem_lock:         %7d   ( default 1 )\r\n", UART_MUX_Sem_lock->OSEventCnt ));
    APP_TRACE_INFO(("Done_Sem_RulerUART:        %7d   ( default 0 )\r\n", Done_Sem_RulerUART->OSEventCnt ));
    APP_TRACE_INFO(("Global_Ruler_State[3..0]:        [%d - %d - %d - %d]\r\n", Global_Ruler_State[3],Global_Ruler_State[2],Global_Ruler_State[1],Global_Ruler_State[0] ));
    APP_TRACE_INFO(("Global_Ruler_Type[3..0] :        [%X - %X - %X - %X]\r\n", Global_Ruler_Type[3],Global_Ruler_Type[2],Global_Ruler_Type[1],Global_Ruler_Type[0] ));
    APP_TRACE_INFO(("Global_Mic_Mask[3..0][] :        [%X - %X - %X - %X]\r\n", Global_Mic_Mask[3],Global_Mic_Mask[2],Global_Mic_Mask[1],Global_Mic_Mask[0] ));
    //APP_TRACE_INFO(("Test Counter:            test_counter1, 2, 3, 4  =  %4d,%4d,%4d,%4d\r\n",  test_counter1, test_counter2,test_counter3, test_counter4));
    //APP_TRACE_INFO(("Test Counter:  UART_WriteStart Failed :  %4d  times\r\n",   test_counter5));
    //APP_TRACE_INFO(("BUZZER_MUTE:    %d  , %s\r\n",   BUZZER_MUTE,(BUZZER_MUTE == 0 ? "Unmute" : "Muted") ));
    APP_TRACE_INFO(("DBG_UART_FIFO: [Max_Usage: %d/%d ]  [FIFO Overflow Hit: %d times]\r\n", debug_uart_fifo_data_max,DBG_UART_Send_Buf_Size,debug_uart_fifo_oveflow_counter ));
    APP_TRACE_INFO(("DBG_USB_FIFO:  [Max_Usage: %d/%d ]  [FIFO Overflow Hit: %d times]\r\n", debug_usb_fifo_data_max,DBG_USB_Send_Buf_Size,debug_usb_fifo_oveflow_counter ));
    APP_TRACE_INFO(("\r\n"));
    Get_Task_Info ();
    Get_Flash_Info ();
    //APP_TRACE_INFO(("\r\n"));

}

void dump_buf_debug( unsigned char *pChar, unsigned int size)
{
    unsigned int i;
    unsigned short  *pInt;
    pInt = (unsigned short *)pChar;

    for( i = 0; i< (size>>1);  ) {
        printf(" 0x%04X",*(pInt+i++));
        if( i%16 == 0 ) {
            printf("\n\r");
        }

    }

}
