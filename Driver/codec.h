/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
*
* Filename      : uif_i2s.c
* Version       : V0.0.1
* Programmer(s) : PQ
* Modifer       : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#ifndef __CODEC_H__
#define __CODEC_H__

#include "uif_object.h"
#include <stdbool.h>

#define PCA9546A_ADDRESS  0xE0
#define PCA9540B_ADDRESS  0xE0
#define PCA9548A_ADDRESS  0xEE

#define CODEC_ADDRESS     0x30
#define AD1938_ADDRESS    0x04
#define AD1937_ADDRESS    0x08


/****************************************/
#define   ALC5610      10
#define   ALC5620      20
#define   PLL_Control0 0x0
#define   PLL_Control1 0x1
#define   DAC_Control0 0x2
#define   DAC_Control1 0x3
#define   DAC_Control2 0x4
#define   DAC_Mute     0x5
#define   DAC1L_Volume 0x6
#define   DAC1R_Volume 0x7
#define   DAC2L_Volume 0x8
#define   DAC2R_Volume 0x9
#define   DAC3L_Volume 0xa
#define   DAC3R_Volume 0xb
#define   DAC4L_Volume 0xc
#define   DAC4R_Volume 0xd
#define   ADC_Control0 0xE
#define   ADC_Control1 0xF
#define   ADC_Control2 0x10
#define   I2S_MODE     1
#define   TDM_MODE     2
#define   TDM16_MODE   3


typedef struct {
    unsigned short sr; // 8000 ~ 48000
    uint8_t  sample_len ; //16 or 32 only
    uint8_t  format; // 0 : i2s, 1 : pcm
    uint8_t  slot_num ; //2, 4, 8
    uint8_t  m_s_sel; //0 : master, 1 : slave;
    uint8_t  flag;  // flag if received audio_cfg command
    uint8_t  bclk_polarity; //1: frame start rising edge match bclk,  0: frame start rising edge inverted bclk
    uint8_t  delay;
    uint8_t  id;//CODEC ID : 0 or 1
    uint8_t  reserved[2];
}CODEC_SETS ;

/****************************************/
uint8_t Set_Codec(const DataSource *pSource,uint8_t codec_control_type, uint8_t size_para, uint8_t *pdata);
uint8_t Get_Codec(const DataSource *pSource,uint8_t codec_control_type, uint8_t reg, uint8_t *pdata);

uint8_t Codec_Mixer(const DataSource *pSource,uint8_t i2c_channel );

uint8_t Init_CODEC( const DataSource *pSource,CODEC_SETS codec_set ) ;

uint8_t Set_AIC3204_DSP_Offset( uint8_t slot_index ) ;
uint8_t Init_CODEC_AIC3204( uint32_t sample_rate ) ;

uint8_t CODEC_LOUT_Small_Gain_En( bool small_gain );
uint8_t CODEC_Set_Volume( const DataSource *pSource,float vol_spk, float vol_lout, float vol_lin );
uint8_t Check_SR_Support( uint32_t sample_rate );

void enable_PA( bool en );

#endif
