/*
*********************************************************************************************************
*                                          UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

#ifndef _AUDIO_H_
#define _AUDIO_H_ 

#include "uif_object.h"

//#include "defined.h"


////////////////////////////////////////////////////////////////////////////////
#if 1
typedef struct {
  unsigned char  type;//Rec =0, Play =1
  unsigned char  channel_num; //valid data cahnnel num 1~8
  unsigned short sample_rate; //8000, 16000, 24000, 32000, 48000

  unsigned char  bit_length; // 16, 24, 32
  unsigned char  lin_ch_mask;

  unsigned char  gpio_num;
  unsigned char  gpio_start_index;
  unsigned char  gpio_bit_mask;

  unsigned char  spi_num;
  unsigned char  spi_start_index;
  unsigned char  spi_bit_mask;

  unsigned char  format;  //1:I2S  2:PDM  3:PCM/TDM
  unsigned char  slot_num;  //bus BCLK slot num
  unsigned char  ssc_cki;
  unsigned char  ssc_delay;

  unsigned char  ssc_start;
  unsigned char  master_slave;
  unsigned char  bclk_polarity;  //0 or 1

  unsigned char  id;  //0: SSC0 , 1: SSC1, 2
  
}AUDIO_CFG;
#endif

typedef struct {
  unsigned int    spi_speed;

  unsigned short  rec_ch_mask;
  unsigned short  play_ch_mask;

  unsigned short  chip_id;
  unsigned char   spi_format;
  unsigned char   gpio_irq;

  unsigned char   time_dly;
  unsigned char   slave;
  unsigned char   reserved[2];

}SPI_PLAY_REC_CFG;


typedef struct {
  
    unsigned char    type;    //bit[0..5]= [I2S0 rec, I2S0 play, I2S1 rec, I2S1 play, SPI rec, SPI play]       eg. rec = 1,  play = 2, rec&play = 3
    unsigned char    padding; //usb first package padding
    
}START_AUDIO;


typedef struct {
    unsigned char    vec_index_a;
    unsigned char    vec_index_b;
    unsigned char    flag;
    unsigned char    type; //41£º iM401,  51: iM501
    unsigned int     delay;
    unsigned char    gpio; //irq trigger GPIO index
    unsigned char    trigger_en;
    unsigned char    pdm_clk_off; //trun off pdm clk after pwd or not
    unsigned char    if_type;//1: I2C, 2:SPI
}SET_VEC_CFG ;


typedef struct {
    unsigned char    addr_index;
    unsigned int     data_len;
    unsigned char*   pdata;
    unsigned char*   pStr;
}MCU_FLASH ;

////////////////////////////////////////////////////////////////////////////////

static uint8_t Init_Play_Setting( void *pInstance );
static uint8_t Init_Rec_Setting( void *pInstance );

static uint8_t Audio_Start_Rec( void );
static uint8_t Audio_Start_Play( void );

void Play_Voice_Buf_Start( void );

void Audio_Stop( void );
void Audio_Start( void );

void Audio_State_Control( uint8_t *msg );

bool First_Pack_Check_BO( unsigned char *pData, unsigned int size );
bool First_Pack_Check_BO1( unsigned char *pData, unsigned int size, uint32_t *pos );
bool First_Pack_Check_BO2( unsigned char *pData, unsigned int size, uint32_t *pos );
void First_Pack_Padding_BI( kfifo_t *pFifo );
void Audio_Manager( unsigned char cfg_data );

extern unsigned char Update_Audio( unsigned char id );
extern unsigned char Setup_Audio( AUDIO_CFG *pAudioCfg );
extern unsigned char Start_Audio( START_AUDIO start_audio  );
extern unsigned char Stop_Audio( void );
extern unsigned char Reset_Audio( void );
extern unsigned char Get_Audio_Version( void );
extern unsigned char SPI_Rec_Start( SPI_PLAY_REC_CFG *pSpi_rec_cfg );
extern unsigned char Save_DSP_VEC( MCU_FLASH *p_dsp_vec );  
extern void          Debug_Audio( void ) ;
extern unsigned char Set_DSP_VEC( SET_VEC_CFG *p_dsp_vec_cfg );


extern unsigned char  global_audio_padding_byte;
extern volatile unsigned char Global_SPI_Rec_Start;
extern volatile unsigned char Global_SPI_Rec_En;
extern SET_VEC_CFG     Global_VEC_Cfg;

extern bool global_flag_sync_audio_0 ;
extern bool global_flag_sync_audio_1 ;

extern unsigned char pq_0, pq_1;
////////////////////////////////////////////////////////////////////////////////





#endif
