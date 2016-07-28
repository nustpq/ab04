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
#ifndef __DSP_H__
#define __DSP_H__

#include "defined.h"

#define  FM36_I2C_ADDR    0xC0
#define  MAX98504         0x62
#define  FM36_ROM_ID_1    0x36D0
#define  FM36_ROM_ID_2    0x600C

typedef struct chip_fm36
{
    DataSource *pPort;
    uint16_t sr;
    uint8_t  mic_num;
    uint8_t lin_sp_index; 
    uint8_t start_slot_index;
    uint8_t bit_length;
    uint8_t i2s_tdm_sel;
    uint8_t force_reset;  
    uint32_t isInitialed;
    uint32_t isPowerDown;
}CHIPFM36;


extern void Pin_Reset_FM36( void );

extern uint8_t ReInit_FM36( uint16_t sr );


extern uint8_t Init_FM36_AB03( uint16_t sr,                                   
                                     uint8_t mic_num,
                                     uint8_t lin_sp_index, 
                                     uint8_t start_slot_index,
                                     uint8_t bit_length,
                                     uint8_t i2s_tdm_sel,
                                     uint8_t force_reset );

extern uint8_t Init_FM36( uint16_t sr );

extern uint8_t DMIC_PGA_Control( uint16_t gain );

extern uint8_t FM36_PWD_Bypass( void );

extern uint8_t FM36_PDM_CLK_Set( uint8_t pdm_dac_clk, uint8_t pdm_adc_clk, uint8_t type );

extern uint8_t Init_FM36_AB03_Preset( void );

extern uint8_t Init_FM36_AB03_temp(void );

extern uint8_t FM36_PDMADC_CLK_OnOff( uint8_t onoff, uint8_t fast_switch);

extern uint8_t Config_PDM_PA( void );

#endif
