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

#ifndef __RULER_H__
#define __RULER_H__



//#define FOR_USE_USBTEST_EXE

#define TIMEOUT_AUDIO_COM    2000    //Max 2s timeout
#define TIMEOUT_RULER_COM    5000    //Max 5s timeout

//#define RULER_MASK(mic_mask,ruler_id)    ((mic_mask>>(ruler_id<<3))&0xFF)

#define RULER_TYPE_RULER                  0
#define RULER_TYPE_HANDSET                1
#define RULER_TYPE_MASK( type )           ( (type>>7) & 0x01 )

#define RULER_TYPE_R01                    0x00
#define RULER_TYPE_R02                    0x01
#define RULER_TYPE_R03                    0x02

#define RULER_TYPE_TA01                   0x10
#define RULER_TYPE_TD01                   0x11

#define RULER_TYPE_WT01                   0x20

#define RULER_TYPE_H01                    0x80
#define RULER_TYPE_H02                    0x81
#define RULER_TYPE_H03                    0x82

#define RULER_ID_DEFAULT                  0xFF

#define SAMPLE_LENGTH_DEFAULT             16
#define SAMPLE_RATE_DEFAULT               16000
#define SLOT_NUM_DEFAULT                  8
#define SET_VOLUME_MUTE                   1000
//ruler state defines
#define  RULER_STATE_DETACHED           0x00
#define  RULER_STATE_ATTACHED           0x01
#define  RULER_STATE_CONFIGURED         0x02
#define  RULER_STATE_SELECTED           0x03
#define  RULER_STATE_RUN                0x04

//audio mcu cmd parameters
#define  AUDIO_START_REC                0x01
#define  AUDIO_START_PLAY               0x02
#define  AUDIO_START_PALYREC            0x03
#define  AUDIO_TYPE_REC                 0x00
#define  AUDIO_TYPE_PLAY                0x01

//host mcu cmd defines
#define  RULER_CMD_SET_AUDIO_CFG        0x01
#define  RULER_CMD_START_AUDIO          0x02
#define  RULER_CMD_STOP_AUDIO           0x03
#define  RULER_CMD_SET_RULER            0x04
#define  RULER_CMD_RAED_RULER_STATUS    0x05
#define  RULER_CMD_RAED_RULER_INFO      0x06
#define  RULER_CMD_WRITE_RULER_INFO     0x07
#define  RULER_CMD_READ_MIC_CALI_DATA   0x08
#define  RULER_CMD_WRITE_MIC_CALI_DATA  0x09
#define  RULER_CMD_TOGGLE_MIC           0x0A
#define  RULER_CMD_GET_AUDIO_VERSION    0x0B
#define  RULER_CMD_GET_RULER_TYPE       0x0C
#define  RULER_CMD_ACTIVE_CTR           0x0D
#define  RULER_CMD_GET_RULER_VERSION    0x0E
#define  RULER_CMD_SETUP_SYNC           0x0F
#define  RULER_CMD_RESET_AUDIO          0x10
#define  RULER_CMD_START_RD_VOICE_BUF   0x11

////////////////////////////////////////////////////////////////////////////////

#define DEF_VERSION_STR_LEN             ( 11 + 1 )
#define DEF_MODEL_STR_LEN               ( 7 + 1 )


/////////////////  Flash store Vec file related defines  ///////////////////////

#define FW_DOWNLAD_CMD_START           1
#define FW_DOWNLAD_CMD_DOING           2
#define FW_DOWNLAD_CMD_DONE            3

#define FW_DOWNLAD_STATE_INITIALIZE    0xFF
#define FW_DOWNLAD_STATE_UNFINISHED    0xAA
#define FW_DOWNLAD_STATE_FINISHED      0x55

#define FLASH_RULER_FW_BIN_MAX_SIZE  ( 0x10000 )  //64kB max for ruler fw bin
#define FLASH_ADDR_FW_STATE          ( AT91C_IFLASH1 )  //from 128kB-
#define FLASH_ADDR_FW_BIN            ( FLASH_ADDR_FW_STATE + AT91C_IFLASH_PAGE_SIZE ) //from 128kB+256

#define FLASH_ADDR_FW_VEC_SIZE       ( 0x2000 ) //8kB
#define FLASH_ADDR_FW_VEC_NUM        ( 7 )      //(64kB = 8kB*7
#define FLASH_ADDR_FW_VEC_STATE      ( AT91C_IFLASH1 +  FLASH_RULER_FW_BIN_MAX_SIZE  ) //256*4
#define FLASH_ADDR_FW_VEC            ( FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * FLASH_ADDR_FW_VEC_NUM ) //from 128kB + 64kB + (0.256*4)kB
#define FLASH_HOST_FW_BIN_MAX_SIZE   ( 0x1C000 ) //128kB for host MCU fw bin

//#define FLASH_RULER_FW_BIN_MAX_SIZE  ( 0x10000 - AT91C_IFLASH_PAGE_SIZE )  //64kB max for ruler fw bin,
//#define FLASH_VOICE_BUFFER_MAX_SIZE  ( 0x20000 - AT91C_IFLASH_PAGE_SIZE )  //128kB max for ruler fw bin
//#define FLASH_ADDR_FW_STATE          ( AT91C_IFLASH1 )  //from 128kB -> (128k+256)B
//#define FLASH_ADDR_FW_BIN            ( FLASH_ADDR_FW_STATE + AT91C_IFLASH_PAGE_SIZE ) //store from 128kB+256B ->end
//
//#define FLASH_HOST_FW_BIN_MAX_SIZE   ( 0x1C000 ) //128kB for host MCU fw bin
//#define FLASH_ADDR_FW_VEC_SIZE       ( 0x800 ) //2kB/vec
//#define FLASH_ADDR_FW_VEC_NUM        ( 8 )     //(16kB = 2kB*8
//#define FLASH_ADDR_FW_VEC_STATE      ( AT91C_IFLASH0 +  FLASH_HOST_FW_BIN_MAX_SIZE  )
//#define FLASH_ADDR_FW_VEC            ( FLASH_ADDR_FW_VEC_STATE + AT91C_IFLASH_PAGE_SIZE * FLASH_ADDR_FW_VEC_NUM ) // 0x80000 + 0x1C000 + (0.256*8)kB = 0x9C800



///////////////////////////////////////////////////////////////////////////////

typedef  unsigned char  VERSION_DATA[ DEF_VERSION_STR_LEN ] ;
typedef  unsigned char  MODEL_DATA[ DEF_MODEL_STR_LEN ] ;

typedef struct {
    float           phase ;
    float           sensitivity ;
    float           noise_floor ;
    unsigned int    inlet_position ;
    unsigned int    data_len ; //add
    unsigned char  *p_data ;
}MIC_CALIB_INFO ;


typedef struct {
    MODEL_DATA     model;
    VERSION_DATA   hw_ver;
    VERSION_DATA   sw_ver;
}BRIDGE_INFO ;

typedef struct {
    unsigned char    POST_status;
    unsigned char    ruler_status;
    unsigned char    mic_status;
}BRIDGE_STATUS ;

typedef struct {
    VERSION_DATA    mic_vendor ; //string
    VERSION_DATA    mic_part_no ;//string
    unsigned char   mic_type ;
    unsigned char   mic_id ;
}MIC_INFO ;

//this is a struct with flexible length
typedef struct {
    MODEL_DATA       model ;//format: 'R''X''X'0
    VERSION_DATA     hw_ver ;
    VERSION_DATA     sw_ver ;
    MIC_INFO        *p_mic_info[16] ; //H01  16?
    unsigned char    mic_num ;
    unsigned char    date[1];
} RULER_INFO ;


////////////////////////////////////////////////////////////////////////////////
typedef struct {
  unsigned char  type;//Rec =0, Play =1
  unsigned char  channel_num; //valid data cahnnel num 1~8
  unsigned short sample_rate; //8000, 16000, 24000, 32000, 48000

  unsigned char  bit_length; // 16, 24, 32
  unsigned char  lin_ch_mask;

  unsigned char  gpio_rec_num;
  unsigned char  gpio_rec_start_index;
  unsigned char  gpio_rec_bit_mask;

  unsigned char  spi_rec_num;
  unsigned char  spi_rec_start_index;
  unsigned char  spi_rec_bit_mask;

  unsigned char  format;  //1:I2S  2:PDM  3:PCM/TDM
  unsigned char  slot_num;  //bus BCLK slot num
  unsigned char  ssc_cki;
  unsigned char  ssc_delay;

  unsigned char  ssc_start;
  unsigned char  master_slave;
  unsigned char  bclk_polarity;  //0 or 1

  unsigned char  id;  //0: SSC0 , 1: SSC1, 2
}AUDIO_CFG;


typedef struct {
  unsigned int    spi_speed;

  unsigned short  rec_ch_mask;
  unsigned short  play_ch_mask;

  unsigned short  chip_id;
  unsigned char   spi_mode;
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
    unsigned char    ruler_id;
}READ_RULER_INFO;

typedef struct {
    RULER_INFO       ruler_info;
    unsigned char    ruler_id;
}WRITE_RULER_INFO ;

typedef struct {
    unsigned char    ruler_id;
    unsigned char    mic_id;
}READ_MIC_CLAIB_INFO ;

typedef struct {
    unsigned char    ruler_id;
    unsigned char    mic_id;
    MIC_CALIB_INFO   mic_calib_info;
}WRITE_MIC_CALIB_INFO ;

typedef struct {
    unsigned char    ruler_id;
    unsigned char    mic_id;
    unsigned char    on_off;
}TOGGLE_MIC ;

typedef struct {
    signed int    mic;
    signed int    lout;
    signed int    spk;
    signed int    lin;
}SET_VOLUME ;

typedef struct {
    unsigned char    port;
    unsigned char   *pdata;
}RAW_DATA_TRANS ;

typedef struct {
    unsigned char    ruler_id;
    unsigned char    cmd;
    unsigned char   *pdata;
}UPDATE_RULER_FW ;

typedef struct {
    unsigned char    cmd;
    unsigned char    *pdata;
}UPDATE_BRIDGE_FW ;

typedef struct {
    unsigned int    f_w_state ;
    unsigned int    f_w_counter ; //FW bin burn times counter
    unsigned int    s_w_counter ; //state page burn times counter
    unsigned int    bin_size ;
    unsigned char   flag;
    char            bin_name[30] ; //add
}FLASH_INFO ;


typedef struct {
    unsigned char    if_type;
    unsigned char    reserved[3];
    unsigned short   attribute;
    unsigned short   speed;
}INTERFACE_CFG ;


typedef struct {
    unsigned char    if_type;
    unsigned char    dev_addr;
    unsigned int     data_len;
    unsigned char*   pdata;
}RAW_WRITE ;

typedef struct {
    unsigned char    if_type;
    unsigned char    dev_addr;
    unsigned int     data_len_read;
    unsigned int     data_len_write;
    unsigned char*   pdata_read;
    unsigned char*   pdata_write;
}RAW_READ ;

typedef struct {
    unsigned short   mem_addr_l;
    unsigned short   mem_addr_h;
    unsigned int     data_len;
    unsigned char*   pdata;
    unsigned char    if_type;
    unsigned char    dev_addr;
    unsigned char    mem_addr_len;
}BURST_WRITE ;

typedef struct {
    unsigned char    if_type;
    unsigned char    dev_addr;
    unsigned char    data_len;
    unsigned char    read_data_len;
    unsigned short   mem_addr_l;
    unsigned short   mem_addr_h;
    unsigned int     mem_addr_len;
    unsigned char*   pdata;

}BURST_READ ;

typedef struct {
    unsigned char    addr_index;
    unsigned int     data_len;
    unsigned char*   pdata;
    unsigned char*   pStr;
}MCU_FLASH ;

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
    unsigned char    gpio_num;
    unsigned char    gpio_value[7];
    unsigned int     delay_us[7];
}GPIO_SESSION ;



extern unsigned char Audio_Version[];
extern unsigned char Ruler_CMD_Result;

extern volatile unsigned char Global_Ruler_Type[];
extern volatile unsigned char Global_Ruler_State[];
extern volatile unsigned char Global_Ruler_Index; //current ruler index
extern volatile unsigned char Global_Mic_State[];
extern volatile unsigned char Global_Bridge_POST;
extern volatile unsigned int  Global_Mic_Mask[];
extern volatile unsigned char Ruler_Setup_Sync_Data;
extern volatile unsigned char Global_SPI_Rec_Start;
extern volatile unsigned char Global_SPI_Rec_En;

extern SET_VEC_CFG  Global_VEC_Cfg;

extern void          Init_Global_Var( void );
extern unsigned char Update_Audio( unsigned char id );
extern unsigned char Setup_Audio( AUDIO_CFG *pAudioCfg );
extern unsigned char Start_Audio( START_AUDIO start_audio  );
extern unsigned char Stop_Audio( void );
extern unsigned char Reset_Audio( void );
extern unsigned char Get_Audio_Version( void );
extern unsigned char Init_Ruler( unsigned char ruler_slot_id ) ;
extern unsigned char Setup_Ruler( unsigned char ruler_slot_id );
extern unsigned char Read_Ruler_Status( unsigned char ruler_slot_id, unsigned short *status_data );
extern unsigned char Get_Ruler_Type(  unsigned char ruler_slot_id );
extern unsigned char Update_Mic_Mask( unsigned char ruler_slot_id, unsigned int mic_mask );
extern unsigned char Toggle_Mic(  TOGGLE_MIC *pdata );
extern void          Ruler_Port_LED_Service( void );
extern unsigned char Write_Ruler_Info( unsigned char ruler_slot_id );
extern unsigned char Read_Ruler_Info( unsigned char ruler_slot_id );
extern unsigned char Read_Mic_Cali_Data(unsigned char ruler_slot_id, unsigned char mic_id);
extern unsigned char Write_Mic_Cali_Data(unsigned char ruler_slot_id, unsigned char mic_id);
extern unsigned char Ruler_Active_Control( unsigned char active_state );
extern unsigned char Reset_Mic_Mask(  unsigned int *pInt );
extern unsigned char Set_Volume( SET_VOLUME *pdata );
extern unsigned char Get_Ruler_Version( unsigned char ruler_id );
extern unsigned char AB_POST( void );
extern unsigned char Ruler_POST( unsigned char ruler_id );
extern void          simple_test_use( void );

extern unsigned char Update_Ruler_FW( unsigned char ruler_slot_id );
extern unsigned char Save_Ruler_FW( unsigned int cmd, unsigned char *pBin, unsigned char *pStr, unsigned int size );
extern unsigned char Ruler_Setup_Sync(unsigned char ruler_slot_id);
extern unsigned char Save_DSP_VEC( MCU_FLASH *p_dsp_vec );
extern void Read_Flash_State( FLASH_INFO  *pFlash_Info, unsigned int flash_address );
extern void Debug_Audio( void ) ;
extern unsigned char Set_DSP_VEC( SET_VEC_CFG *p_dsp_vec_cfg );

extern unsigned char SPI_Rec_Start( SPI_PLAY_REC_CFG *pSpi_rec_cfg );

#endif
