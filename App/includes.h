/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          MASTER INCLUDES
*
*                                          Atmel AT91SAM3U4
*                                                on the
*                                  Atmel AT91SAM3U-EK Development Board.
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  INCLUDES_PRESENT
#define  INCLUDES_PRESENT


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <stdarg.h>                  

#include    <ucos_ii.h>
#include    <cpu.h>
#include    <cpu_core.h>
#include    <lib_def.h>
#include    <lib_ascii.h>
#include    <lib_math.h>
#include    <lib_mem.h>
#include    <lib_str.h>
#include    <libnandflash.h>
#include    <libpmecc.h>

#include    <taskcomm.h>

#include    <bsp.h>
#include    <bsp_int.h>
#include    <bsp_os.h>
#include    <bsp_cache.h>




/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include    <bsp.h>
#include    <app_cfg.h>

#if (APP_CFG_PROBE_COM_EN        == DEF_ENABLED) || \
    (APP_CFG_PROBE_OS_PLUGIN_EN  == DEF_ENABLED)
#include  <app_probe.h>
#endif




/*
*********************************************************************************************************
*                                           ATMEL HEADER FILE
*********************************************************************************************************
*/




/*
*********************************************************************************************************
*                                            INCLUDES END
*********************************************************************************************************
*/
#endif
