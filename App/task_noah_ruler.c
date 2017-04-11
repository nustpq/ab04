/*
*********************************************************************************************************
*                               iSAM TEST BENCH AUDIO BRIDGE BOARD APP PACKAGE
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
*                                          Atmel AT91SAM7A3
*                                               on the
*                                      iSAM Audio Bridge Board
*
* Filename      : task_noah_ruler.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <includes.h>


//OS_EVENT  *App_Noah_Ruler_Mbox ; //???
//OS_EVENT  *UART_MUX_Sem_lock ;

OS_EVENT  *ACK_Sem_RulerUART ;
OS_EVENT  *Done_Sem_RulerUART ;

#if 1 
unsigned int test_counter1 = 0 ;
unsigned int test_counter2 = 0 ;
unsigned int test_counter3 = 0 ;
unsigned int test_counter4 = 0 ;
unsigned int test_counter5 = 0 ;
#endif

/*
*********************************************************************************************************
*                                    App_TaskNoah_Ruler()
*
* Description : Process Comminucation between Ruler module and Audio Bridge.
*               Noah protocol layer related data parsing and processing.
*               This task wait for message event from App_TaskUART_Rx(). Check if the Noah layer data is 
*               valid in the message.  
*
* Argument(s) : p_arg   Argument passed to 'App_TaskNoah_Ruler()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void App_TaskNoah_Ruler( void *p_arg )
{ 
    (void)p_arg;
    
    NOAH_CMD   *pPcCmd ;    
    CPU_INT08U *pCmdBuf ;
    CPU_INT08U  rxID ; 
    CPU_INT08U  PcCmdRxID_Ruler[4];
    CPU_INT08U  sum ;  
    CPU_INT08U  SessionDone;
    CPU_INT08U  err ;
    CPU_INT08U *pTaskMsgIN ;  
    CPU_INT08U *pMsg ;  
    
    pTaskMsgIN  = NULL;
    pMsg        = NULL;
    SessionDone = 0 ;    
   
      
    while( DEF_TRUE ) {          
             
        pTaskMsgIN  = (INT8U *)OSQPend( EVENT_MsgQ_RulerUART2Noah, 0, &err ); 
        
        if( pTaskMsgIN != NULL && OS_ERR_NONE == err )   {
           
            pCmdBuf = pTaskMsgIN; // char point to the data buffer
            pPcCmd  = (NOAH_CMD *)pCmdBuf ; //change to NOAH_CMD type
            rxID    = GET_FRAME_ID( pPcCmd->head ) ; //get frame ID, index       
            sum     = pPcCmd->checkSum; //get check sum data  
            
            UIF_LED_Toggle( LED_RUN );           
            APP_TRACE_DBG(("\r\n<<"));
            switch( GET_FRAME_TYPE( pPcCmd->head ) ) {// GET_FRAME_TYPE(pPcCmd->head)  get frame type 
                
                case FRAM_TYPE_DATA :  
                    
                    if( (sum == 0) || ( sum == CheckSum(0,pCmdBuf, pPcCmd->DataLen + 2)) ) {   
//                        APP_TRACE_INFO(("R[%d]:[0x%2x>0x%2x][",Global_Ruler_Index,PcCmdRxID_Ruler[Global_Ruler_Index],rxID));
//                        for(unsigned int i = 0; i<pPcCmd->DataLen; i++){   
//                            APP_TRACE_INFO((" %2X", *(unsigned char*)(pPcCmd->Data+i) )); 
//                        }
//                        APP_TRACE_INFO((" ]\r\n")); 
                        Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, SET_FRAME_HEAD(rxID,FRAM_TYPE_ACK), NULL, 0, 0, NULL, 0 ) ;  // ACK                                               
                        if( (CPU_INT08U)(PcCmdRxID_Ruler[Global_Ruler_Index]+0x40) == rxID ) { // chweck if frameID repeat  
                            PcCmdRxID_Ruler[Global_Ruler_Index] = rxID ; //save this frameID 
                         
                            err = Noah_CMD_Parse_Ruler( pPcCmd, &SessionDone); // jump to CMD parse 
                            if( (err != OS_ERR_NONE) || (SessionDone == 1) ) {
                                SessionDone = 0;
                                Global_Ruler_CMD_Result = err ;
                                OSTimeDly(2); //wait enough time for ACK to be sent
                                APP_TRACE_DBG((" Res[0x%2x] ", Global_Ruler_CMD_Result));
                                OSSemPost( Done_Sem_RulerUART );                                     
                            }
          
                        } else {                      
                            APP_TRACE_DBG(("PcCmdRxID_Ruler[%d] Err: expect 0x%X, get 0x%X",Global_Ruler_Index,PcCmdRxID_Ruler[Global_Ruler_Index]+0x40,rxID)); 
                        }
                        
                    } else {                
                        Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, SET_FRAME_HEAD(rxID,FRAM_TYPE_NAK), NULL, 0, 0, NULL, 0 ) ;  // NAK   
                    
                    }                 
                break ;                
          
                    
                case FRAM_TYPE_ESTA :
                  
                    PcCmdRxID_Ruler[Global_Ruler_Index] = 0xC0 ; // ? why 0x40: make sure there can be many same setup frame
                    PcCmdTxID_Ruler[Global_Ruler_Index] = 0x00 ; //  
                    
                    OSSemPost( ACK_Sem_RulerUART );                 
                    OSSemPost( Done_Sem_RulerUART ); //end the resend pending--                     
                    OSTimeDly(1); //wait for the TX buffer is empty 
                    
                    //Reset all UART CMD related buffer and release mem 
                    do{  //reset mem used by  EVENT_MsgQ_RulerUART2Noah                    
                        pMsg   = (INT8U *)OSQAccept( EVENT_MsgQ_Noah2RulerUART, &err );
                        OSMemPut( pMEM_Part_MsgUART, pMsg ); 
                    } while ( pMsg != NULL && OS_ERR_NONE == err ) ; 
                    
                    do{  //reset mem used by  EVENT_MsgQ_RulerUART2Noah                  
                        pMsg   = (INT8U *)OSQAccept( EVENT_MsgQ_RulerUART2Noah, &err );
                        OSMemPut( pMEM_Part_MsgUART, pMsg ); 
                    } while ( pMsg != NULL && OS_ERR_NONE == err ) ; 
                       
//                    Queue_Flush( pUART_Send_Buf[RULER_UART] ); //clear uart send&rec data queue
//                    Queue_Flush( pUART_Rece_Buf[RULER_UART] );                      
         
                    SessionDone = 0;  //init  
                    OSSemSet( ACK_Sem_RulerUART, 0, &err ); // clear the sem
                    OSSemSet( Done_Sem_RulerUART, 0, &err );
                    test_counter4++;
                    APP_TRACE_DBG(("EST/ESTA"));   
                break ;         
                        
                case FRAM_TYPE_ACK :
                  
                    if( rxID == PcCmdTxID_Ruler[Global_Ruler_Index] ) {                       
                        OSSemPost( ACK_Sem_RulerUART );                        
                        test_counter2++;
                    } else {
                        APP_TRACE_INFO_T(("\r\nACK: Got %X, Expect %X\r\n",rxID,PcCmdTxID_Ruler[Global_Ruler_Index] )); 
                    }
                    APP_TRACE_DBG(("ACK")); 
                    test_counter3++;       
                break ;
                
                case FRAM_TYPE_NAK :
                    // dismiss NAK, there will be a resend if no ACK got
                    // OSSemPost( Done_Sem_RulerUART );
                    test_counter1++;
                    APP_TRACE_DBG(("NAK")); 
                break;
                
                         
                default :    
                break ;              
            } 
            
            //release mem
            OSMemPut( pMEM_Part_MsgUART, pTaskMsgIN );  
           
        }         
        
        ////OSTimeDly(2);   //use OSQPend(), no delay needed!!! 
        
    }        
    
    
}



    