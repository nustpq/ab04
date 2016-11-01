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
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : JBL
* Modify	: Leo
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  BSP_PRESENT
#define  BSP_PRESENT


#include  <stdarg.h>
#include  <stdio.h>
#include  <assert.h>

#include  <cpu.h>

#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_mem.h>

#include  <app_cfg.h>

#include  <bsp_int.h>
//#include  <bsp_gpio.h>
//#include  <bsp_pmc.h>
//#include  <bsp_ser.h>
//#include  <bsp_os.h>

#include  <lib_def.h>
#include  <lib_ascii.h>

//#include  <at91sam3u4.h>
#include  <app_cfg.h>
#include  <ucos_ii.h>
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdbool.h>
#include  <string.h>
//#include  <probe_com_cfg.h>

#include  <taskcomm.h>
#include  <kfifo.h>

//#include  <nvic.h>
//#include  <board.h>
//#include  <uart.h>
//#include  <pio.h>
#include  <pio_it.h>
//#include  <gpio.h>
#include  <led.h>
//#include  <timer.h>
//#include  <eefc.h>
//#include  <flashd.h>
#include  <twid.h>
#include  <spi.h>
//#include  <i2c_gpio.h>
//#include  <im501_comm.h>
#include  <ruler.h>
#include  <emb.h>
#include  <mem_basic.h>
//#include  <uif.h>
#include  <uif_dsp.h>
#include  <noah_cmd.h>
//#include  <shell_commands.h>
#include  <codec.h>
#include  <xmodem.h>
//#include  <dma.h>
//#include  <dmad.h>


#include "board.h"
#include "defined.h"
#include "uif_object.h"


#include "codec.h"
//#include "sine_table.h"

#include "uif_cmdparse.h"
#include "uif_i2s.h"
#include "uif_usb.h"
#include "uif_spi.h"
#include "uif_twi.h"
#include "uif_usart.h"
#include "uif_nandflash.h"
#include "uif_gpio.h"
#include "uif_led.h"
#include "uif_act8865.h"
//#include "uif_dsp.h"    
#include "uif_hardware_init.h"

/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/



//#define  BOARD_TYPE_AB01  
//#define  BOARD_TYPE_AB02  
//#define  BOARD_TYPE_AB03  
#define  BOARD_TYPE_UIF

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


typedef  struct  atsama5_reg_pio {
    CPU_REG32  PER;                                             /* PIO Enable Register.                                 */
    CPU_REG32  PDR;                                             /* PIO Disable Register.                                */
    CPU_REG32  PSR;                                             /* PIO Status Register.                                 */
    CPU_REG32  RESERVED1;                                       /* Reserved.                                            */
    CPU_REG32  OER;                                             /* Output Enable Register.                              */
    CPU_REG32  ODR;                                             /* Output Disable Register.                             */
    CPU_REG32  OSR;                                             /* Output Status Register.                              */
    CPU_REG32  RESERVED2;                                       /* Reserved.                                            */
    CPU_REG32  IFER;                                            /* Glitch Input Filter Enable Register.                 */
    CPU_REG32  IFDR;                                            /* Glitch Input Filter Disable Register.                */
    CPU_REG32  IFSR;                                            /* Glitch Input Filter Status Register.                 */
    CPU_REG32  RESERVED3;                                       /* Reserved.                                            */
    CPU_REG32  SODR;                                            /* Set Output Data Register.                            */
    CPU_REG32  CODR;                                            /* Clear Output Data Register.                          */
    CPU_REG32  ODSR;                                            /* Output Data Status Register.                         */
    CPU_REG32  PDSR;                                            /* Pin Data Status Register.                            */
    CPU_REG32  IER;                                             /* Interrupt Enable Register.                           */
    CPU_REG32  IDR;                                             /* Interrupt Disable Register.                          */
    CPU_REG32  IMR;                                             /* Interrupt Mask Register.                             */
    CPU_REG32  ISR;                                             /* Interrupt Status Register.                           */
    CPU_REG32  MDER;                                            /* Multi-driver Enable Register.                        */
    CPU_REG32  MDDR;                                            /* Multi-driver Disable Register.                       */
    CPU_REG32  MDSR;                                            /* Multi-driver Status Register.                        */
    CPU_REG32  RESERVED4;                                       /* Reserved.                                            */
    CPU_REG32  PUDR;                                            /* Pull-up Disable Register.                            */
    CPU_REG32  PUER;                                            /* Pull-up Enable Register.                             */
    CPU_REG32  PUSR;                                            /* Pull-up Status Register.                             */
    CPU_REG32  RESERVED5;                                       /* Reserved.                                            */
    CPU_REG32  ABCDSR1;                                         /* Peripheral Select Register 1.                        */
    CPU_REG32  ABCDSR2;                                         /* Peripheral Select Register 2.                        */
    CPU_REG32  RESERVED6[2];                                    /* Reserved.                                            */
    CPU_REG32  IFSCDR;                                          /* Input Filter Slow Clock Disable Register.            */
    CPU_REG32  IFSCER;                                          /* Input Filter Slow Clock Enable Register.             */
    CPU_REG32  IFSCSR;                                          /* Input Filter Slow Clock Status Register.             */
    CPU_REG32  SCDR;                                            /* Slow Clock Divider Debouncing Register.              */
    CPU_REG32  PPDDR;                                           /* Pad Pull-down Disable Register.                      */
    CPU_REG32  PPDER;                                           /* Pad Pull-down Enable Register.                       */
    CPU_REG32  PPDSR;                                           /* Pad Pull-down Status Register.                       */
    CPU_REG32  RESERVED7;                                       /* Reserved.                                            */
    CPU_REG32  OWER;                                            /* Output Write Enable Register.                        */
    CPU_REG32  OWDR;                                            /* Output Write Disable Register.                       */
    CPU_REG32  OWSR;                                            /* Output Write Status Register.                        */
    CPU_REG32  RESERVED8;                                       /* Reserved.                                            */
    CPU_REG32  AIMER;                                           /* Additional Interrupt Mode Enable Register.           */
    CPU_REG32  AIMDR;                                           /* Additional Interrupt Mode Disable Register.          */
    CPU_REG32  AIMMR;                                           /* Additional Interrupt Mode Mask Register.             */
    CPU_REG32  RESERVED9;                                       /* Reserved.                                            */
    CPU_REG32  ESR;                                             /* Edge Select Register.                                */
    CPU_REG32  LSR;                                             /* Level Select Register.                               */
    CPU_REG32  RESERVED10;                                      /* Reserved.                                            */
    CPU_REG32  FELLSR;                                          /* Falling Edge/Low Level Select Register.              */
    CPU_REG32  REHLSR;                                          /* Rising Edge/High Level Select Register.              */
    CPU_REG32  FRLHSR;                                          /* Fall/Rise - Low/High Status Register.                */
    CPU_REG32  RESERVED11;                                      /* Reserved.                                            */
    CPU_REG32  LOCKSR;                                          /* Lock Status Register.                                */
    CPU_REG32  WPMR;                                            /* Write Protect Mode Register.                         */
    CPU_REG32  WPSR;                                            /* Write Protect Status Register.                       */
    CPU_REG32  RESERVED12[4];                                   /* Reserved.                                            */
    CPU_REG32  SCHMITT;                                         /* Schmitt Trigger Register.                            */
    CPU_REG32  RESERVED14[4];                                   /* Reserved.                                            */
    CPU_REG32  DRIVER1;                                         /* I/O Drive Register 1.                                */
    CPU_REG32  DRIVER2;                                         /* I/O Drive Register 2.                                */
} ATSAMA5_REG_PIO, *ATSAMA5_REG_PIO_PTR;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#define LED_D3    1
#define LED_D4    2
#define LED_D5    3

#define LED_RUN   1
#define LED_USB   2
#define LED_HDMI  3

#define BUZZER_OFF 1
#define BUZZER_ON 0

#define FPGA_OE         0
#define FPGA_RST        1
#define CODEC1_RST      2
#define CODEC0_RST      3
#define FM36_RST        4
#define PA_SHUTDOWN     5
#define FAST_PLUS_RST   7
#define V5_UIF_EN       9
#define HDMI_UIF_PWR_EN 10
#define LEVEL_SHIFT_OE  11

void  BSP_LED_Init ( void );

void  BSP_LED_On ( CPU_INT32U led );

void  BSP_LED_Off ( CPU_INT32U led );

void BSP_LED_Toggle( CPU_INT32U led );

void BSP_BUZZER_Toggle( CPU_INT32U state );

void BSP_LED_FLIP( CPU_INT32U state, CPU_INT32U order );

//interface for AB-04 board led;
void UIF_LED_Init( void );

void UIF_LED_On ( CPU_INT32U led );

void UIF_LED_Off ( CPU_INT32U led );

void UIF_LED_Toggle( CPU_INT32U led );

//interface for AB-04 board control pins;
void UIF_Misc_Init( void );

void UIF_Misc_On ( CPU_INT32U id );

void UIF_Misc_Off ( CPU_INT32U id );

void UIF_DelayUs( CPU_INT32U us );

void UIF_Beep_On ( ) ;

void UIF_Beep_Off ( ) ;

void dump_buf_debug( unsigned char *pChar, unsigned int size) ;



void Head_Info( void );
void Beep( INT32U beep_cycles);
void Head_Info( void );
void Beep( INT32U beep_cycles);
//void PDM_Pattern_Gen( INT8U type );
void Time_Stamp( void );
void Get_Flash_Info (void);
void BSP_Init (void);

extern const CPU_CHAR fw_version[];
extern const CPU_CHAR hw_version[];
extern const CPU_CHAR hw_model[];


#endif  /* BSP_PRESENT */
