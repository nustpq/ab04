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
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/


#include  <bsp.h>

#include  <cpu.h>
#include  <lib_def.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  SAMA5_REG_PIOE_PER (*((CPU_REG32 *)0xFFFFFA00))
#define  SAMA5_REG_PIOE_OER (*((CPU_REG32 *)0xFFFFFA10))

#define  SAMA5_REG_PIOE_SODR (*((CPU_REG32 *)0xFFFFFA30))
#define  SAMA5_REG_PIOE_CODR (*((CPU_REG32 *)0xFFFFFA34))
#define  SAMA5_REG_PIOE_ODSR (*((CPU_REG32 *)0xFFFFFA38))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


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
