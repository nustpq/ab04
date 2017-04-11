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
* Filename      : task_cmd_parse.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <includes.h>


//Queue
void       * MsgQ_Noah2CMDParse[MsgUARTQueue_SIZE];
//Event
OS_EVENT   * EVENT_MsgQ_Noah2CMDParse;


/*
*********************************************************************************************************
*                                    App_TaskCMDParse()
*
* Description : This task wait message event from App_TaskNoah().
*               Check if the EMB data is valid in the message. And decode EMB data and parse command and data.
*               Execute the command and return result.          
*
* Argument(s) : p_arg   Argument passed to 'App_TaskCMDParse()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void App_TaskCMDParse( void *p_arg )
{ 
    
    (void)p_arg;
    
    CPU_INT08U      err ;
    CPU_INT08U     *pTaskMsgIN ;
    pNEW_NOAH_CMD   pNewCmd ; 
    
    pTaskMsgIN  = NULL;
  

    while( DEF_TRUE ) {     
        
        pTaskMsgIN  = (INT8U *)OSQPend( EVENT_MsgQ_Noah2CMDParse, 0, &err ); 
        
        if( pTaskMsgIN != NULL && OS_ERR_NONE == err )   { 
            
            UIF_LED_On( LED_RUN );
            //UIF_Beep_On();
            
            pNewCmd  = (pNEW_NOAH_CMD)pTaskMsgIN ; //change to NOAH CMD type           
            err = EMB_Data_Parse( pNewCmd );           
            OSMemPut( pMEM_Part_MsgUART, pTaskMsgIN );  //release mem
            Send_Report( pNewCmd->pkt_sn, err );  //GACK
            
            UIF_LED_Off( LED_RUN );
            UIF_Beep_Off();
        }
        
        
    }
    
    
}
                    
        
    