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
*                                      APPLICATION CONFIGURATION
*
*                                          ATMEL SAMA5D3X-EK
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : JBL
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                INCLUDE
*********************************************************************************************************
*/

#include  <stdio.h>

#include  <cpu.h>
#include  <lib_def.h>


#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__


#define  DBG_UART_METHOD_TASK_EN  //this define enable the DBG UART in task for speed up, PQ
//#define  DBG_USB_LOG_EN     //enable usb log info
/*
*********************************************************************************************************
*                                        TASK PRIORITIES
*********************************************************************************************************
*/

#define TASKLEDPRIORITY                                    8

#define TASKSSC0PRIORITY                                   6
#define FIRMWAREVECUPDATE                                  26

#define  APP_CFG_TASK_USB_PRIO                             1
#define  APP_CFG_TASK_CMD_PARSE_PRIO                       2
#define  APP_CFG_TASK_UART_TX_PRIO                         3
#define  APP_CFG_TASK_NOAH_PRIO                            4
#define  APP_CFG_TASK_UART_RX_PRIO                         5

#define  APP_CFG_TASK_USER_IF_PRIO                         10
#define  APP_CFG_TASK_JOY_PRIO                   (APP_CFG_TASK_USER_IF_PRIO+1)

#define  APP_CFG_TASK_UART_TX_RULER_PRIO                   13
#define  APP_CFG_TASK_NOAH_RULER_PRIO                      16

#define  APP_CFG_TASK_SHELL_PRIO                           30
#define  APP_CFG_TASK_DBG_INFO_PRIO                        31
#define  APP_CFG_TASK_START_PRIO                           35
#define  APP_CFG_TASK_PROBE_STR_PRIO                       37
#define  PROBE_DEMO_INTRO_CFG_TASK_LED_PRIO                38
#define  OS_PROBE_TASK_PRIO                                40  
#define  OS_PROBE_TASK_ID                                  40 
#define  OS_TASK_TMR_PRIO                         (OS_LOWEST_PRIO - 2)


/*
*********************************************************************************************************
*                                        TASK STACK SIZES
*    task stack type is  OS_STK , a 32-bit width type
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE                     128
#define  APP_CFG_TASK_USER_IF_STK_SIZE                   256
#define  APP_CFG_TASK_JOY_STK_SIZE                       128
#define  APP_CFG_TASK_SHELL_STK_SIZE                     256
#define  APP_CFG_TASK_DBG_INFO_STK_SIZE                  128

#define  APP_CFG_TASK_UART_TX_STK_SIZE                   128
#define  APP_CFG_TASK_UART_TX_RULER_STK_SIZE             128
#define  APP_CFG_TASK_UART_RX_STK_SIZE                   128
#define  APP_CFG_TASK_NOAH_STK_SIZE                      128
#define  APP_CFG_TASK_NOAH_RULER_STK_SIZE                128
#define  APP_CFG_TASK_CMD_PARSE_STK_SIZE                 256

#define  PROBE_DEMO_INTRO_CFG_TASK_LED_STK_SIZE          256

#define  OS_PROBE_TASK_STK_SIZE                          512

#define  APP_CFG_TASK_LED_STK_SIZE                      4096
#define  APP_CFG_TASK_USB_STK_SIZE                      4096
#define  APP_CFG_TASK_SSC0_STK_SIZE                     4096



/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/
                                                                            /* Choose the level of debug messages                   */
#define  BSP_CFG_TRACE_LEVEL                       TRACE_LEVEL_OFF
#define  APP_CFG_TRACE_LEVEL                       TRACE_LEVEL_INFO


void  BSP_Ser_Printf (CPU_CHAR  *format, ...);

//#define  APP_CFG_TRACE                              printf
#define  APP_CFG_TRACE                              BSP_Ser_Printf

#define  APP_TRACE_INFO(x)               ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_CFG_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_CFG_TRACE x) : (void)0)


// for shell uart
#define  UART_SHELL_SEND_STR(x)                (void)( BSP_Ser_Printf x )      
#define  UART_SHELL_GET_BYTE(x)                      ( BSP_Ser_RdByte x )
#define  UART_SHELL_SEND_BYTE(x)               (void)( BSP_Ser_WrByte x )  


#endif /* __APP_CFG_H__ */
