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
* Filename      : task_joy.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <includes.h>




/*
*********************************************************************************************************
*                                    App_TaskJoy()
*
* Description : Monitor the state change of the reset button, switches and ruler port connection. And 
*               passes messages to 'App_TaskUserIF()' if state change happened.
*
* Argument(s) : p_arg       Argument passed to 'App_TaskKbd()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*               (2) Here suppose App_TaskJoy's priority is bellow App_TaskUserIF().
*                   So we can use one memory to pass message.
* 
*********************************************************************************************************
*/
void  App_TaskJoy (void *p_arg)
{  
    (void)p_arg;
    
    CPU_INT32U   switch_value;
    CPU_INT32U   switch_value_prev;
    CPU_INT32U   ruler_port_value;
    CPU_INT32U   ruler_port_value_prev;    
    CPU_INT32U   data ;
    CPU_INT32U   i ;
    
    OSTimeDly(1000); //wait for App_TaskUserIF() be ready, incase of MBox lost to cause buzzer error
    
    //avoid a fake trigger after POR
//    switch_value_prev      =  Get_Switches()  & 0x03; 
//    switch_value_prev      =  switch_value_prev ^ 0x01;  //make sure check Switch 'SW1' Buzzer 1st  
//    ruler_port_value_prev  =  Get_Port_Detect() & 0xFF;  //GPIOs   
   
    //flash_test(); //debug use
    
    while ( DEF_TRUE ) {     /* Task body, always written as an infinite loop.           */   
        
//        if( Flag_Reset_Pin_Trigger != 0 ){ //check reset flag          
//            
//            //APP_TRACE_INFO(("\r\n\r\n>> Flag_Reset_Pin_Trigger = %d\r\n",Flag_Reset_Pin_Trigger ));
//            Flag_Reset_Pin_Trigger-- ;
//            for(i = 0; i<100 ; i++) { //check if a 1000ms low level on reset pin                  
//                if( (AT91C_BASE_RSTC->RSTC_RSR & AT91C_RSTC_NRSTL) != 0) {
//                    break;
//                }
//                OSTimeDly(10);                
//            } 
//            
//            if(i >= 100 ) {// trigger reset             
//                APP_TRACE_INFO((">>User reset pin triggered... \r\n" ));
//                APP_TRACE_INFO((">>That's all folks !\r\n\r\n>> Rebooting...\r\n"));                             
//                Beep(3); 
//                //PDM_Pattern_Gen(1); //Generate idle PDM data for CP2240 
//                Pin_Reset_Audio_MCU(); //reset audio
//                AT91C_BASE_RSTC->RSTC_RCR = MCU_SW_RESET_PATTERN ; // do reset processor and peripherals
//                while(1);
//              
//            } else { // not reset               
//                data = MSG_TYPE_RESET ; //means print inf
//                while( OSMboxPost(App_UserIF_Mbox, &data) == OS_ERR_MBOX_FULL ) {
//                    OSTimeDly(10);  //
//                }
//                //Debug_Audio();
//            }       
//            
//        }
           
//        switch_value = Get_Switches() & 0x03; //mask 0~1     
//        if( switch_value != switch_value_prev ) {  
//            OSTimeDly(100); 
//            if( switch_value == Get_Switches() & 0x03 ) { //jitter immune                         
//                data = switch_value ^ switch_value_prev ;   
//                switch_value_prev = switch_value ;
//                data = ( data << 8 ) | (switch_value & 0xFF) ;
//                data &= ~MSG_TYPE_MASK ;
//                data |= MSG_TYPE_SWITCH; 
//                //APP_TRACE_INFO(("\r\n\r\n>>>>>>>>>>>>>>>>>>>>  mute _ Trigger: %d",switch_value ));
//                while ( OSMboxPost(App_UserIF_Mbox, &data) == OS_ERR_MBOX_FULL ) {
//                    OSTimeDly(10);  //
//                };            
//                
//            }
//        }
//        
//        ruler_port_value = Get_Port_Detect() & 0xFF; //mask  0~7        
//        if( ruler_port_value != ruler_port_value_prev ) {
//            OSTimeDly(1); //for gpio detect, no need delay
//            if( ruler_port_value == Get_Port_Detect() & 0xFF ) {  //jitter immune
//                data = ruler_port_value ^ ruler_port_value_prev ;   
//                ruler_port_value_prev = ruler_port_value ;
//                data = ( data << 8 ) | (ruler_port_value & 0xFF) ; 
//                data &= ~MSG_TYPE_MASK ;
//                data |= MSG_TYPE_PORT_DET;                 
//                while ( OSMboxPost(App_UserIF_Mbox, &data) == OS_ERR_MBOX_FULL ) {
//                    OSTimeDly(10);  //
//                    
//                };  
//                
//                
//            }
//        }
   
        //AB_Status_Change_Report();     
         
        //Service_To_iM501_IRQ();
        
        OSTimeDly(1); 
       
    }
      
    
}
    