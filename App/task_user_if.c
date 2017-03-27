/*
*********************************************************************************************************
*                               UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           TASK PACKAGE
*
*                                          Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : task_user_if.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#include <includes.h>
#include "uif_xc3s50an.h"


OS_EVENT  *App_UserIF_Mbox;


/*
*********************************************************************************************************
*                                         App_TaskUserIF()
*
* Description : Do related process after state change of the reset button, switches and ruler port connection.
*               Wait for messages from 'App_TaskJoy()' if state change happened.
*
* Argument(s) : p_arg       Argument passed to 'App_TaskUserIF()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static bool port_enable = true;

void  App_TaskUserIF (void *p_arg)
{

    CPU_INT32U  *msg;
    CPU_INT08U   err;
    CPU_INT32U   key_state;
    CPU_INT08U   ruler_id;

    //CPU_INT08U   iM401_Ctrl_Enable;

    (void)p_arg;
    Buzzer_OnOff(1);
    OSTimeDly(100); //wait for other tasks be ready , and time for power stable for ruler    
    Head_Info(); //Send header 
    Buzzer_OnOff(0);  
//    
//    //Ruler_Power_Switch(1);
    Init_Global_Var();
    Init_Audio_Path();
    //iM401_Ctrl_Enable = 1;
    OSTimeDly(100);
    if( AB_POST() )  {
        Buzzer_Error();
    }
    ruler_id = 0;    

//#ifndef BOARD_TYPE_AB01
//    APP_TRACE_INFO(( "\r\nWARNING: NOT AB01, NO MCU Ctrl UART SWITCH\r\n"));
//#endif

    while ( DEF_TRUE ) {                                          /* Task body, always written as an infinite loop.           */

        msg = (CPU_INT32U *)(OSMboxPend(App_UserIF_Mbox, 0, &err)); //pending, no timeout    
        if (msg != NULL) {
            key_state = *msg ;
            //APP_TRACE_INFO(("\r\n\r\n"));
            switch( key_state & MSG_TYPE_MASK ) {
                case MSG_TYPE_RESET : //reset send msg
                    //PDM_Pattern_Gen(0); //gen cp2240 pattern
                    Head_Info();

                break;

                case MSG_TYPE_SWITCH ://Switch
                  //  APP_TRACE_INFO(("Switch status updated: \r\n  SW1,  SW0 \r\n"));
                  //  APP_TRACE_INFO((" %4d, %4d\r\n", (key_state>>0)&(0x01),(key_state>>1)&(0x01) ));

                    //To do something to do with Switch selection...

                    // Switch 'SW1' used to control Buzzer mute:
                    //         0: ON :  Buzzer muted
                    //         1: OFF:  Buzzer unmuted
                    if( (key_state>>(8 + 0)) & 0x01) {  //check if SW1 switch status changed
                        if( ((key_state>>0)& 0x01 ) == 0 ) {
                            //BUZZER_MUTE =  1;   //mute buzzer
                        } else {
                            //BUZZER_MUTE =  0;   //unmute buzzer
                        }
                    }

//                    // Switch 'M2' used to control iM401:
//                    //         0: ON :  No CMD to iM401
//                    //         1: OFF:  send iM401 by pass/standby CMD on IRQ
//                    if( (key_state>>(8 + 1)) & 0x01) {  //check if SW1 switch status changed
//                        if( ((key_state>>1)& 0x01 ) == 0 ) {
//                            iM401_Ctrl_Enable =  0;   //no CMD
//                        } else {
//                            iM401_Ctrl_Enable =  1;   //send CMD
//                        }
//                    }
//                    // Switch 'SW1' used to control CODEC LOUT PGA Gain:
//                    //         0: ON :  24dB attenuated signal for Phone MIC input
//                    //         1: OFF:  Normal signal for ACQUA
//                    if( (key_state>>(8 + 4)) & 0x01) {  //check if SW1 switch status changed
//                          if( ((key_state>>4)& 0x01 ) == 0 ) {
//                              err = CODEC_LOUT_Small_Gain_En( true ) ;  //enable 24dB attenuated signal for Phone Mic
//                          } else {
//                              err = CODEC_LOUT_Small_Gain_En( false ) ; //normal signal, no attenuation
//                          }
//                          if( OS_ERR_NONE != err ) {
//                              APP_TRACE_INFO(("ERR: Set CODEC_LOUT_Small_Gain_En err! [%d]\r\n",err));
//                          }
//                    }

                    // Switch 'SW0' used to control DEBUG port:
                    //         0: ON :  UART1 used as debug port
                    //         1: OFF:  DBG UART used as debug port
                    if( (key_state>>(8 + 1)) & 0x01) {  //check if SW0 switch status changed
//                        OSTaskDel( APP_CFG_TASK_SHELL_PRIO );
//                        OSSemSet (Bsp_Ser_Tx_Sem_lock, 1,  &err) ;
//                        OSSemSet (Bsp_Ser_Rx_Sem_lock, 1,  &err) ;
//                        Task_ReCreate_Shell();
//                        if( ((key_state>>1)& 0x01 ) == 0 ) { //debug to UART1
//                            Debug_COM_Sel = 1 ;
//                            BSP_Ser_Init(115200);
//                        } else {                             //debug to DBG_UART
//                            Debug_COM_Sel = 0 ;
//                            UART_Init(PC_UART, ISR_PC_UART, 115200 );    //To PC  ? Sem recreat issue
//                        }
                          //test_send_cmd_to_im501();
                          //Read_iM501_Voice_Buffer(2,15000);
                    }
                break;

                case MSG_TYPE_PORT_DET :
                    if( port_enable == false ) {
                        APP_TRACE_INFO(("Ruler port disabled !\r\n"));
                        break;
                    }
                    APP_TRACE_INFO_T(("GPIO[0x%x].\r\n", key_state ));
//                    APP_TRACE_INFO(("GPIO port status changed:  Port[7..0] = [%1d%1d%1d%1d%1d%1d%1d%1d]\r\n",\
//                                    (key_state>>0)&(0x01),(key_state>>1)&(0x01),(key_state>>2)&(0x01),(key_state>>3)&(0x01),\
//                                    (key_state>>4)&(0x01),(key_state>>5)&(0x01),(key_state>>6)&(0x01),(key_state>>7)&(0x01) ));

                    if( (key_state>>( 16 + 0) & 0x01) )   {  //check if Ruler Port[0] switch status changed
                        if( ( (key_state>>0) & 0x01 ) == 0 ) { // ruler attached, setup ruler
                            UIF_LED_On( LED_HDMI );
                            APP_TRACE_INFO_T(("Ruler[%d] Attached.\r\n", ruler_id ));                            
                            Global_Ruler_State[ruler_id] = RULER_STATE_ATTACHED; 
                            err = Init_Ruler( ruler_id ); 
                            if( OS_ERR_NONE != err ) {
                                    UIF_LED_Off( LED_HDMI );
                                    continue;
                            }                         
                            err = Setup_Ruler( ruler_id ); 
                            if( OS_ERR_NONE != err ) {
                                    UIF_LED_Off( LED_HDMI );
                                    continue;
                            }
////                                err = Ruler_Setup_Sync(  ruler_id );
////                                if( OS_ERR_NONE != err ) {
////                                    //LED_Clear( LED_P0 + ruler_id );
////                                    continue;
////                                }                                
                            err = Get_Ruler_Type( ruler_id ); 
                            if( OS_ERR_NONE != err ) {
                                    UIF_LED_Off( LED_HDMI );
                                    continue;
                            }
                            err = Get_Ruler_Version( ruler_id ); 
                            if( OS_ERR_NONE != err ) {
                                    UIF_LED_Off( LED_HDMI );
                                    continue;
                            }                                  
                            err = Ruler_POST( ruler_id ); 
                            if( OS_ERR_NONE != err ) {
                                    UIF_LED_Off( LED_HDMI );
                                    continue;
                            }                     
                            Global_Ruler_State[ruler_id] = RULER_STATE_CONFIGURED ;                              
//                                mic_mask = Global_Mic_Mask[ruler_id];
//                                err = Update_Mic_Mask( ruler_id, mic_mask );
//                                if( OS_ERR_NONE != err ) {                              
//                                    continue;
//                                }   
//                                if( mic_mask ) {
//                                    Global_Ruler_State[ruler_id]= RULER_STATE_SELECTED;                                   
//                                } 
                                //OSTimeDly(500);
                                //simple_test_use();//test for Dr.Yang and use USBApp0815.exe
                                                                           
                        } else { // ruler detached
                            UIF_LED_Off( LED_HDMI );
                            APP_TRACE_INFO_T(("Ruler[%d] Detached.\r\n", ruler_id )); 
                            Global_Ruler_State[ruler_id] = RULER_STATE_DETACHED ;
                            Global_Ruler_Type[ruler_id]  = 0 ;
                            Global_Mic_Mask[ruler_id]    = 0 ; 
                        } 

                    }
                break;
                
                }
               
            }
            Buzzer_OnOff(1); //buzzer on
            OSTimeDly(5);
            Buzzer_OnOff(0); //buzzer off
            OSTimeDly(20);
            Buzzer_OnOff(1); //buzzer on
            OSTimeDly(5);
            Buzzer_OnOff(0); //buzzer off
            
        }


}


/*******************************************************************************
Control ruler port detection response
in case of update FW requirement
*******************************************************************************/
void Port_Detect_Enable( unsigned char on_off )
{

    if( on_off == 0 ) {
        port_enable = false;

    } else {
        port_enable = true;

    }

}

void Buzzer_Error( void )
{
   Buzzer_OnOff(1); //buzzer on
   OSTimeDly(100);
   Buzzer_OnOff(0); //buzzer off
   OSTimeDly(50);
   Buzzer_OnOff(1); //buzzer on
   OSTimeDly(100);
   Buzzer_OnOff(0); //buzzer off
}


