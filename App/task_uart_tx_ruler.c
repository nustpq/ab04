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
* Filename      : task_uart_tx.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <includes.h>

//Queue
void  *MsgQ_RulerUART2Noah[MsgUARTQueue_SIZE];
void  *MsgQ_Noah2RulerUART[MsgUARTQueue_SIZE];
//Event
OS_EVENT  *EVENT_MsgQ_RulerUART2Noah;
OS_EVENT  *EVENT_MsgQ_Noah2RulerUART;



CPU_INT32U  Tx_ReSend_Happens_Ruler = 0;   // debug use, resend happen times, NOTE: only writable in this task
CPU_INT08U  PcCmdTxID_Ruler[4];   // Frame TXD ID for 4 rulers


/*
*********************************************************************************************************
*                                    App_TaskUART_Tx_Ruler()
*
* Description : Process UART Transmit related process between Audio Bridge and Ruler.
*               Wait for data message from other task that want to send to Ruler.
*
* Argument(s) : p_arg   Argument passed to 'App_TaskUART_Tx_Ruler()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void App_TaskUART_Tx_Ruler( void *p_arg )
{    
    (void)p_arg;
    
    CPU_INT08U       errCode ; 
    CPU_INT08U       sum ; 
    CPU_INT08U      *pTaskMsgIN ;
    NOAH_CMD        *pPcCmd ; 
    CPU_INT08U       resend_index;  

    pTaskMsgIN  = NULL;
    pPcCmd      = NULL;	
    sum         = 0;
    errCode     = UNKOW_ERR_RPT ;        
     
    while (DEF_TRUE) { 
       
        // Noah to Uart transmit
        pTaskMsgIN   = (INT8U *)OSQPend( EVENT_MsgQ_Noah2RulerUART, 0, &errCode );        
 /*   
        if( pTaskMsgIN != NULL && OS_ERR_NONE == errCode )   {      
            
            pPcCmd  = (NOAH_CMD *)pTaskMsgIN ;             
            if( GET_FRAME_TYPE(pPcCmd->head) == FRAM_TYPE_DATA  ) {  //data frame
                
                for( resend_index = 0; resend_index < MAX_RESEND_TIMES; resend_index++ ) {    
                    
                    APP_TRACE_DBG(("\r\n>>Tx R[%d]:[0x%2x][",Global_Ruler_Index,PcCmdTxID_Ruler[Global_Ruler_Index]));
                    for(unsigned int i = 0; i<pPcCmd->DataLen; i++){   
                        APP_TRACE_DBG((" %2X", *(unsigned char*)(pPcCmd->Data+i) )); 
                    }
                    APP_TRACE_DBG((" ]")); 
                    
                    Queue_Write( (void*)pUART_Send_Buf[RULER_UART] , CMD_DATA_SYNC1   ); //Sync1
                    Queue_Write( (void*)pUART_Send_Buf[RULER_UART] , CMD_DATA_SYNC2_1 ); //Sync2                  
                    ////Queue_Write( (void*)pUART_Send_Buf[RULER_UART] , ((pPcCmd->Data[0] == RULER_CMD_SET_RULER)|| (pPcCmd->Data[0] == RULER_CMD_SETUP_SYNC)) ? RULER_ID_DEFAULT : Global_Ruler_Index ); //ruler_id 
                    
                    pPcCmd->head  = SET_FRAME_HEAD( PcCmdTxID_Ruler[Global_Ruler_Index], FRAM_TYPE_DATA ) ; //set frame ID for data transmit                                
                    sum  =  CheckSum( pPcCmd->head, &(pPcCmd->DataLen), pPcCmd->DataLen + 1); //calculate checksum   
                    
                    Queue_WriteBuf( pTaskMsgIN,  (void*)pUART_Send_Buf[RULER_UART], pPcCmd->DataLen + 2 ); //3Bytes = head(1Bytes) + len(1Bytes)
                    Queue_Write( (void*)pUART_Send_Buf[RULER_UART], sum   ); //  check sum(1Bytes)
                    UART_WriteStart( RULER_UART ); //send data  
                    
                    OSSemPend(ACK_Sem_RulerUART, 500, &errCode);//pending 500ms for ACK back                     
                    if( OS_ERR_NONE == errCode )   {               
                        OSMemPut( pMEM_Part_MsgUART, pTaskMsgIN );    //release mem 
                        PcCmdTxID_Ruler[Global_Ruler_Index] += 0x40;// this frame send out ok, frame ++,   //0xC0                   
                        break;                        
                    } 
                    
                }
                
                if(  resend_index >= MAX_RESEND_TIMES ) {   // reach max send times                    
                    OSMemPut( pMEM_Part_MsgUART, pTaskMsgIN );    //release mem a space at least
                    Noah_CMD_Packing( EVENT_MsgQ_Noah2RulerUART, SET_FRAME_HEAD(PcCmdTxID_Ruler[Global_Ruler_Index]+1,FRAM_TYPE_EST), NULL, 0, 1, NULL, 0) ;   //insert EST  package 
                    
                } 
                
                if(  resend_index > 1 ) {   // resend happens, debug use      
                    Tx_ReSend_Happens_Ruler ++ ;
                }
                
            } else { //ACK / NAK  frame, no resend action  
              
                    Queue_Write( (void*)pUART_Send_Buf[RULER_UART], CMD_DATA_SYNC1   ); //Sync1
                    Queue_Write( (void*)pUART_Send_Buf[RULER_UART], CMD_DATA_SYNC2_1 ); //Sync2  
                    //Queue_Write( (void*)pUART_Send_Buf[RULER_UART] , (pPcCmd->head == FRAM_TYPE_EST) ? RULER_ID_DEFAULT : Global_Ruler_Index ); //ruler_id 
                    ////Queue_Write( (void*)pUART_Send_Buf[RULER_UART] ,  RULER_ID_DEFAULT); //ruler_id 
                    Queue_WriteBuf( pTaskMsgIN,(void*)pUART_Send_Buf[RULER_UART], 2 );
                    //OSQAccept( EVENT_MsgQ_Noah2RulerUART, &errCode ); //delete message from queue                    
                    UART_WriteStart( RULER_UART ); //send data   
//                    if( GET_FRAME_TYPE(pPcCmd->head) == FRAM_TYPE_ACK ) {
//                        APP_TRACE_INFO(("\r\n>>ACK"));
//                    } else if ( GET_FRAME_TYPE(pPcCmd->head) == FRAM_TYPE_NAK ) {
//                        APP_TRACE_INFO(("\r\n>>NAK"));
//                    } else if ( GET_FRAME_TYPE(pPcCmd->head) == FRAM_TYPE_EST ) {
//                        APP_TRACE_INFO(("\r\n>>EST"));
//                    } else if ( GET_FRAME_TYPE(pPcCmd->head) == FRAM_TYPE_ESTA ) {
//                        APP_TRACE_INFO(("\r\n>>ESTA"));
//                    } else {
//                        APP_TRACE_INFO(("\r\n>>Err"));
//                    }                     
                    if ( GET_FRAME_TYPE(pPcCmd->head) == FRAM_TYPE_EST ) {
                        OSSemPend(ACK_Sem_RulerUART, 1000, &errCode);//pending 1000ms for ACK back    
                        if( OS_ERR_NONE != errCode ) { 
                            APP_TRACE_INFO_T(("ERROR£ºSend EST to Ruler[%d] failed.\r\n",Global_Ruler_Index)); 
                        }
                    }
                    APP_TRACE_DBG((" [ %2X %2X ]", *pTaskMsgIN, *(pTaskMsgIN+1)));  
                    
                    OSMemPut( pMEM_Part_MsgUART, pTaskMsgIN );    //release mem 
            }                       
         
        }  
       */ 
        ////OSTimeDly(5);		                                     	
	}
    
}




    