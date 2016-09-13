#ifndef _DEFINED_H_
#define _DEFINED_H_

#include <ucos_ii.h>

#include "uif_object.h"


#define SUCCESS                          0u
#define NO_ERR                           0u

//constant for audio
#define  AUDIO_CMD_IDLE                 0x00
#define  AUDIO_CMD_START_REC            0x01
#define  AUDIO_CMD_START_PLAY           0x02
#define  AUDIO_CMD_START_PALYREC        0x03
#define  AUDIO_CMD_STOP                 0x04
#define  AUDIO_CMD_CFG                  0x05
#define  AUDIO_CMD_VERSION              0x06
#define  AUDIO_CMD_RESET                0x07
#define  AUDIO_CMD_READ_VOICE_BUF       0x08


#define  AUDIO_STATE_STOP               0x00
#define  AUDIO_STATE_PLAY               0x01
#define  AUDIO_STATE_REC                0x02
#define  AUDIO_STATE_PLAYREC            0x03

//Error
#define  ERR_USB_STATE                  250u
#define  ERR_AUD_CFG                    251u
#define  ERR_CMD_TYPE                   252u
#define  ERR_TDM_FORMAT                 253u


#define I2S_IN_BUFFER_SIZE              ( 192 * 16 )            //audio data transfered per frame, Max 48 kHz:   48k*8Slot*2ms*4B=3072
#define I2S_OUT_BUFFER_SIZE             ( 192 * 16 )            // 
#define USBDATAEPSIZE                   64                      // force use 64Bytes
//#define PINGPONG_SIZE                   USBDATAEPSIZE
#define PINGPONG_SIZE                   ( 192 * 16 )
#define USBCMDDATAEPSIZE                USBDATAEPSIZE
#define USB_OUT_BUFFER_SIZE             16384           //USB audio data, size MUST be 2^n .2^14=16384
#define USB_IN_BUFFER_SIZE              (8192)          //USB audio data, size MUST be 2^n .2^14=16384
#define USB_CMD_OUT_BUFFER_SIZE         1024            //USB cmd data, size MUST be 2^n .
#define USB_CMD_IN_BUFFER_SIZE          1024            //USB cmd data, size MUST be 2^n .

#define PLAY_BUF_DLY_CNT                  5


/*
*********************************************************************************************************
*                                        define contant for fm36
*Note: copy this from uif-1.0 mem_basic.h
*********************************************************************************************************
*/
#define  FM_CMD_SYN_0     0xFC
#define  FM_CMD_SYN_1     0xF3

#define  FM_CMD_DM_WR     0x3B
#define  FM_CMD_DM_RD     0x37

#define  FM_CMD_PM_WR     0x0D
#define  FM_CMD_PM_RD     0x07

#define  FM_CMD_CM_WR     0x2B
#define  FM_CMD_CM_RD     0x27

#define  FM_CMD_HOST_WR_1 0x68
#define  FM_CMD_HOST_WR_2 0x6A
#define  FM_CMD_HOST_RD   0x60

#define  FM_CMD_DSP_WR_1  0x58
#define  FM_CMD_DSP_WR_2  0x5A
#define  FM_CMD_DSP_RD    0x56

//Never try to change this define, due to they are array index !
#define MEM_TYPE_DM  0x00 
#define MEM_TYPE_PM  0x01
#define MEM_TYPE_CM  0x02

#define EMB_BUF_SIZE (4200-8)

#define DEF_VERSION_STR_LEN             ( 11 + 1 )
#define DEF_MODEL_STR_LEN               ( 7 + 1 )

/*
*********************************************************************************************************
*                                        define contant for ruler
*Note: copy this from uif-1.0 ruler.h
*********************************************************************************************************
*/

#define TIMEOUT_AUDIO_COM    2000    //Max 2s timeout
#define TIMEOUT_RULER_COM    5000    //Max 5s timeout

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

#define SAMPLE_LENGTH                     16
#define SAMPLE_RATE_DEF                   16000
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
 
/////////////////  Flash store Vec file related defines  ///////////////////////
#define AT91C_IFLASH1	 (0x00100000)                   // this macro is from sam3u4c header file
#define AT91C_IFLASH_PAGE_SIZE  256                    //so,these macro will be redefine accorder hardware


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
#define FLASH_HOST_FW_BIN_MAX_SIZE   ( 0x1C000 ) //128kB for host MCU fw bi

/*
*********************************************************************************************************
*                                        define contant for 
*Note: copy this from uif-1.0 uif.h 
*********************************************************************************************************
*/
#define   UIF_TYPE_CMD_NUM                     9

#define   UIF_TYPE_I2C                         1
#define   UIF_TYPE_SPI                         2
#define   UIF_TYPE_GPIO                        3
#define   UIF_TYPE_FM36_PATH                   4
#define   UIF_TYPE_I2C_GPIO                    5
#define   UIF_TYPE_I2C_Mixer                   6
#define   UIF_TYPE_FM36_PDMCLK                 7
#define   UIF_TYPE_GPIO_CLK                    8
#define   UIF_TYPE_DUT_ID                      9
 
////////////////// UIF ATTRIBUTE
 
#define   ATTRI_DUT_ID_IM501                   501
#define   ATTRI_DUT_ID_FM1388                  1388
#define   ATTRI_I2C_IM501_LOAD_CODE_IRAM       52
#define   ATTRI_I2C_IM501_LOAD_CODE_DRAM       51
#define   ATTRI_I2C_IM401_LOAD_CODE            41
#define   ATTRI_SPI_FM1388_LOAD_CODE           31
#define   ATTRI_I2C_FM1388_LOAD_EEPROM         21
#define   ATTRI_I2C_IM205                      11

#define   ATTRI_SPI_IM501_CPHA0_CPOL0          0 //iM501_CPHA_CPOL
#define   ATTRI_SPI_IM501_CPHA0_CPOL1          1
#define   ATTRI_SPI_IM501_CPHA1_CPOL0          2
#define   ATTRI_SPI_IM501_CPHA1_CPOL1          3

#define   FM1388_ALLOWED_DATA_PACK_SIZE        (240+6)
#define   EEPROM_ALLOWED_DATA_PACK_SIZE        (128+2)

#define   ATTRI_FM36_PATH_NORMAL           0
#define   ATTRI_FM36_PATH_PWD_BP           1

/*----------------------------------------------------------------------------*/

typedef struct _sys_info
{
  uint8_t firmware_version[ 32 ];
  uint8_t hardware_version[ 32 ];
  uint8_t adaptor_soft_version[ 32 ];
  uint8_t date[ 32 ]; 
  uint8_t model[ 32 ];
}SYSINFO;


//copy these struct from uif1.0 for compatiblity and will delete if don't need
typedef  uint8_t VERSION_DATA[ DEF_VERSION_STR_LEN ] ;  
typedef  uint8_t MODEL_DATA[ DEF_MODEL_STR_LEN ] ;

typedef struct {    
    VERSION_DATA    mic_vendor ; //string    
    VERSION_DATA    mic_part_no ;//string 
    uint8_t  mic_type ; 
    uint8_t  mic_id ;   
}MIC_INFO ;

typedef struct {
    float           phase ;  
    float           sensitivity ;
    float           noise_floor ;
    uint32_t        inlet_position ;
    uint32_t        data_len ; //add
    uint8_t         *p_data ; 
}MIC_CALIB_INFO ;

typedef struct {
    uint32_t   length;
    uint32_t   index;
    uint8_t    *pdata;
    uint8_t    done;
}VOICE_BUF ;

typedef struct _spi_cfg{
    uint32_t   spi_speed;
    uint8_t    spi_mode;  
    uint8_t    gpio_irq;
    uint8_t    slave;
    uint8_t    chip_id;
}VOICE_BUF_CFG;

typedef struct { 
    uint8_t     channels ; //mic num 1~6 
    uint8_t     bit_length;//16, 24, 32    
    uint8_t     cki;
    uint8_t     delay;
    uint8_t     start;  
    uint8_t     type ;  //rec = 0,  play = 1
    uint8_t     lin_ch_mask;
    uint8_t     gpio_rec_bit_mask;
    uint8_t     format;
    uint8_t     master_or_slave;
    uint8_t     sr ;    //16000, 48000     
    uint16_t    reserve1;
    uint16_t    reserve2;
}AUDIO_CFG ;

typedef struct {
    uint8_t    type;    //rec = 1,  play = 2, rec&play = 3   
    uint8_t    padding; //usb first package padding
}START_AUDIO;

typedef struct {
    uint8_t    ruler_id; 
}READ_RULER_INFO;

typedef struct {
    MODEL_DATA       model ;//format: 'R''X''X'0
    VERSION_DATA     hw_ver ;
    VERSION_DATA     sw_ver ;      
    MIC_INFO        *p_mic_info[16] ; //H01  16?
    uint8_t          mic_num ;
    uint8_t          date[1];
} RULER_INFO ;

typedef struct {      
    RULER_INFO  ruler_info; 
    uint8_t     ruler_id;     
}WRITE_RULER_INFO ;

typedef struct {
    uint8_t   ruler_id;   
    uint8_t   mic_id;  
}READ_MIC_CLAIB_INFO ;

typedef struct {
    uint8_t   ruler_id;   
    uint8_t   mic_id; 
    MIC_CALIB_INFO   mic_calib_info;
}WRITE_MIC_CALIB_INFO ;
 
typedef struct {
    uint8_t   ruler_id;   
    uint8_t   mic_id; 
    uint8_t   on_off;
}TOGGLE_MIC ;

typedef struct {
    int32_t    mic;   
    int32_t    lout; 
    int32_t    spk;
    int32_t     lin;
}SET_VOLUME ;

typedef struct {
    uint8_t   port;   
    uint8_t  *pdata; 
}RAW_DATA_TRANS ;

typedef struct {
    uint8_t   ruler_id;   
    uint8_t   cmd; 
    uint8_t  *pdata; 
}UPDATE_RULER_FW ;

typedef struct {
    uint8_t   cmd; 
    uint8_t   *pdata; 
}UPDATE_BRIDGE_FW ;

typedef struct {
    uint32_t    f_w_state ;
    uint32_t    f_w_counter ; //FW bin burn times counter
    uint32_t    s_w_counter ; //state page burn times counter
    uint32_t    bin_size ;
    uint8_t     flag;
    int8_t      bin_name[30] ; //add
}FLASH_INFO ;


typedef struct {
    uint8_t   if_type;      
    uint8_t   attribute;
    uint16_t   speed;
}INTERFACE_CFG ;


typedef struct {
    uint8_t   if_type;      
    uint8_t   dev_addr;   
    uint32_t     data_len;
    uint8_t*   pdata;
}RAW_WRITE ;

typedef struct {
    uint8_t     if_type;      
    uint8_t     dev_addr;
    uint32_t    data_len_read;
    uint32_t    data_len_write;
    uint8_t*    pdata_read;
    uint8_t*    pdata_write;
}RAW_READ ;

typedef struct {
    uint16_t   mem_addr_l; 
    uint16_t   mem_addr_h; 
    uint32_t   data_len;    
    uint8_t*   pdata;
    uint8_t    if_type;      
    uint8_t    dev_addr;
    uint8_t    mem_addr_len;
}BURST_WRITE ;

typedef struct {
    uint8_t    if_type;      
    uint8_t    dev_addr;    
    uint8_t    data_len;
    uint8_t    read_data_len;
    uint16_t   mem_addr_l; 
    uint16_t   mem_addr_h; 
    uint32_t   mem_addr_len;
    uint8_t*   pdata;   
}BURST_READ ;

typedef struct {
    uint8_t    addr_index;   
    uint32_t   data_len;
    uint8_t*   pdata;
    uint8_t*   pStr;
}MCU_FLASH ;

typedef struct {
    uint8_t    vec_index_a;   
    uint8_t    vec_index_b;
    uint8_t    flag;
    uint8_t    type; //41£º iM401,  51: iM501
    uint32_t   delay; 
    uint8_t    gpio; //irq trigger GPIO index
    uint8_t    trigger_en;
    uint8_t    pdm_clk_off; //trun off pdm clk after pwd or not
    uint8_t    if_type;//1: I2C, 2:SPI
}SET_VEC_CFG ;  
   
typedef struct {
    uint8_t    gpio_num;       
    uint8_t    gpio_value[7];
    uint32_t   delay_us[7];
}GPIO_SESSION ; 


extern SET_VEC_CFG  Global_VEC_Cfg;

extern INTERFACE_CFG   Global_UIF_Setting[ UIF_TYPE_CMD_NUM ];
extern AUDIO_CFG  Audio_Configure_Instance0[ 2 ];
extern AUDIO_CFG  Audio_Configure_Instance1[ 2 ];

/*
*********************************************************************************************************
*                                        buffer copy from uif1.0 define
*Note: Maybe should move all of these defines to a standard-alone file? that read easier;
*********************************************************************************************************
*/

//Buffer Level 1:  USB data stream buffer : 64 B
extern uint8_t usbBufferBulkOut0[USBDATAEPSIZE];
extern uint8_t usbBufferBulkOut1[USBDATAEPSIZE];
extern uint8_t usbBufferBulkIn0[USBDATAEPSIZE];
extern uint8_t usbBufferBulkIn1[USBDATAEPSIZE];

//Buffer Level 2:  FIFO Loop Data Buffer : 16384 B
extern uint8_t ssc0_FIFOBufferBulkOut[USB_OUT_BUFFER_SIZE] ;
extern uint8_t ssc0_FIFOBufferBulkIn[USB_IN_BUFFER_SIZE] ;
extern uint8_t ssc1_FIFOBufferBulkOut[USB_OUT_BUFFER_SIZE] ;
extern uint8_t ssc1_FIFOBufferBulkIn[USB_IN_BUFFER_SIZE] ;

//Buffer Level 3:  Double-buffer for I2S data : MAX 48*2*8*2*2 = 3072 B
extern uint16_t ssc0_I2SBuffersOut[2][I2S_OUT_BUFFER_SIZE];  // Play 
extern uint16_t ssc0_I2SBuffersIn[2][I2S_IN_BUFFER_SIZE] ;   // Record
extern uint16_t ssc1_I2SBuffersOut[2][I2S_OUT_BUFFER_SIZE];  // Play 
extern uint16_t ssc1_I2SBuffersIn[2][I2S_IN_BUFFER_SIZE] ;   // Record

//------------------------usb cmd buffer copy from uif1.0 defined-------------// 
//Buffer Level 1:  USB Cmd data stream buffer : 64 B
extern uint8_t usbCmdBufferBulkOut[ USBCMDDATAEPSIZE ] ;
extern uint8_t usbCmdBufferBulkIn[ USBCMDDATAEPSIZE ]  ;

//Buffer Level 2:  FIFO Loop Data Buffer : 1024 B
extern uint8_t FIFOBufferBulkOutCmd[ USB_CMD_OUT_BUFFER_SIZE ] ;
extern uint8_t FIFOBufferBulkInCmd[ USB_CMD_IN_BUFFER_SIZE ]  ;


//------------------------port list instance export for other-----------------//
extern sDmad g_dmad;
extern DataSource source_usb;
extern DataSource source_ssc0;
extern DataSource source_ssc1;
extern DataSource source_spi0;
extern DataSource source_spi1;
extern DataSource source_twi0;
extern DataSource source_twi1;
extern DataSource source_twi2;
extern DataSource source_gpio;

//-----------------------task syncoronize P/V---------------------------------//
extern OS_FLAG_GRP *g_StartUSBTransfer;
extern OS_EVENT    *pPortManagerMbox;

#endif
