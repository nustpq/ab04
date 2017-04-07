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
*                                      TASKS HEADERS CONFIGURATION
*
*                                          Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : taskcomm.h
* Version       : V1.0.0
* IDE           : IAR for ARM 5.40
* Programmer(s) : PQ
*
*********************************************************************************************************
* Note(s)       :  defines used for communication between tasks
*********************************************************************************************************
*/

#ifndef _TASKCOMM_H_
#define _TASKCOMM_H_


#define  MSG_TYPE_MASK          0xF0000000
#define  MSG_TYPE_RESET         0x10000000
#define  MSG_TYPE_SWITCH        0x20000000
#define  MSG_TYPE_PORT_DET      0x30000000

#define  MCU_SW_RESET_PATTERN   0xA500000D


/////////  UART Message Storage Area  //////////////////
#define MsgUARTQueue_SIZE       10   //memory partition block numbers
#define MsgUARTBody_SIZE        4200//4096 // package header(8B) + data(4096) + EMB Ext data
#define EMB_BUF_SIZE            (MsgUARTBody_SIZE-8)// 8 bytes are for package header reserved
#define MAX_DATA_SIZE           4096 //suggest maximum data size

//
#define MAX_RESEND_TIMES        3
#define DBG_UART_Send_Buf_Size  8192 //kFIFO
#define DBG_USB_Send_Buf_Size   8192 //kFIFO


extern OS_MEM       *pMEM_Part_MsgUART;
extern CPU_INT08U    MemPartition_MsgUART[MsgUARTQueue_SIZE][MsgUARTBody_SIZE];

//Msg from PC Uart to Noah
extern void     * MsgQ_PCUART2Noah[MsgUARTQueue_SIZE];
extern OS_EVENT * EVENT_MsgQ_PCUART2Noah;
//Msg from Noah to PC Uart
extern void     * MsgQ_Noah2PCUART[MsgUARTQueue_SIZE];
extern OS_EVENT * EVENT_MsgQ_Noah2PCUART;


//Msg from Ruler Uart to Noah
extern void     * MsgQ_RulerUART2Noah[MsgUARTQueue_SIZE];
extern OS_EVENT * EVENT_MsgQ_RulerUART2Noah;
//Msg from Noah to Ruler Uart
extern void     * MsgQ_Noah2RulerUART[MsgUARTQueue_SIZE];
extern OS_EVENT * EVENT_MsgQ_Noah2RulerUART;

//Msg from Noah to CMD Parse
extern void     * MsgQ_Noah2CMDParse[MsgUARTQueue_SIZE];
extern OS_EVENT * EVENT_MsgQ_Noah2CMDParse;

extern OS_EVENT *App_UserIF_Mbox;
extern OS_EVENT *ACK_Sem_PCUART;
extern OS_EVENT *ACK_Sem_RulerUART;
extern OS_EVENT *Done_Sem_RulerUART;
//extern OS_EVENT *UART_MUX_Sem_lock;
extern OS_EVENT *Load_Vec_Sem_lock;

extern OS_EVENT *App_AudioManager_Mbox;

extern CPU_INT08U DBG_UART_Send_Buffer[];

extern CPU_INT08U       PcCmdTxID;
extern CPU_INT08U       PcCmdTxID_Ruler[];
extern CPU_INT32U       Tx_ReSend_Happens ;
extern CPU_INT32U       Tx_ReSend_Happens_Ruler ;

extern volatile CPU_INT08U  Global_Conn_Ready;
extern volatile CPU_INT08U  Global_Idle_Ready;
extern unsigned int         test_counter1, test_counter2,test_counter3, test_counter4, test_counter5 ;
extern CPU_INT16U debug_uart_fifo_data_max ;
extern CPU_INT16U debug_usb_fifo_data_max ;
extern CPU_INT16U debug_uart_fifo_oveflow_counter ;
extern CPU_INT16U debug_usb_fifo_oveflow_counter ;

void App_TaskUART_Tx      ( void *pdata ) ;
void App_TaskUART_Tx_Ruler( void *pdata ) ;
void App_TaskUART_Rx      ( void *pdata ) ;
void App_TaskNoah         ( void *p_arg ) ;
void App_TaskNoah_Ruler   ( void *p_arg ) ;
void App_TaskGenieShell   ( void *p_arg ) ;
void App_TaskUserIF       ( void *p_arg ) ;
void App_TaskJoy          ( void *p_arg ) ;
void App_TaskCMDParse     ( void *p_arg ) ;
void App_TaskDebugInfo    ( void *p_arg ) ;
void App_TaskUSBService   ( void *p_arg ) ;
void App_TaskGenieShell   ( void *p_arg ) ;
void App_AudioManager     ( void *p_arg ) ;
 

void Task_ReCreate_Shell( void );
void Port_Detect_Enable( unsigned char on_off );
void BSP_Ser_WrStr_To_Buffer( char *p_str );
void BSP_Ser_WrStr_To_Buffer_USB( char *p_str );

void Init_Debug_FIFO( void );
void Init_Bulk_FIFO( void );
void Buzzer_Error( void );

void Release_Task_for_Audio( void ) ;
void Hold_Task_for_Audio( void ) ;

#endif


