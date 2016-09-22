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


#define  SAMA5_REG_PIOA_PER (*((CPU_REG32 *)0xFFFFF200))
#define  SAMA5_REG_PIOA_OER (*((CPU_REG32 *)0xFFFFF210))

#define  SAMA5_REG_PIOA_SODR (*((CPU_REG32 *)0xFFFFF230))
#define  SAMA5_REG_PIOA_CODR (*((CPU_REG32 *)0xFFFFF234))
#define  SAMA5_REG_PIOA_ODSR (*((CPU_REG32 *)0xFFFFF238))


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
    
    //initialize buzzer--pin:pa8;
    SAMA5_REG_PIOA_PER = (DEF_BIT_21 | DEF_BIT_22 | DEF_BIT_24 | DEF_BIT_08 );
    SAMA5_REG_PIOA_OER = (DEF_BIT_21 | DEF_BIT_22 | DEF_BIT_24 | DEF_BIT_08 );
       
    return;
  
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
        case LED_D3:
            SAMA5_REG_PIOA_CODR = DEF_BIT_21;
            break;

        case LED_D4:
            SAMA5_REG_PIOA_CODR = DEF_BIT_22;
            break;
            
        case LED_D5:
            SAMA5_REG_PIOA_CODR = DEF_BIT_24;
            break;            
            
        case 0:
        default:
            SAMA5_REG_PIOA_CODR = DEF_BIT_21;
            SAMA5_REG_PIOA_CODR = DEF_BIT_22;
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
        case LED_D3:
            SAMA5_REG_PIOA_SODR = DEF_BIT_21;
            break;

        case LED_D4:
            SAMA5_REG_PIOA_SODR = DEF_BIT_22;
            break;
            
        case LED_D5:  //hdmi interface status indicate;
            SAMA5_REG_PIOA_SODR = DEF_BIT_24;
            break;                    

        case 0:
        default:
            SAMA5_REG_PIOA_SODR = DEF_BIT_21;
            SAMA5_REG_PIOA_SODR = DEF_BIT_22;
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
        case LED_D3:
            if( status &  DEF_BIT_21 )
              SAMA5_REG_PIOA_CODR = DEF_BIT_21;
            else
              SAMA5_REG_PIOA_SODR = DEF_BIT_21;
        break;
        
        case LED_D4:
            if( status &  DEF_BIT_22 )
              SAMA5_REG_PIOA_CODR = DEF_BIT_22;
            else
              SAMA5_REG_PIOA_SODR = DEF_BIT_22;
        break;
        
        case LED_D5:
            if( status &  DEF_BIT_24 )
              SAMA5_REG_PIOA_CODR = DEF_BIT_24;
            else
              SAMA5_REG_PIOA_SODR = DEF_BIT_24;
        break;

      case 0:
      default:
        break;
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


