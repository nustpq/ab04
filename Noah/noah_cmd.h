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

#ifndef     __NOAH_CMD_H__
#define     __NOAH_CMD_H__

#include   <includes.h>
/*
*********************************************************************************************************
*                                             ERROR CODES
*  Because uCOS used 0~143 for error code, here define error code from : 150
*********************************************************************************************************
*/
#define SUCCESS                  0u
#define NO_ERR                   0u
#define CMD_ERR                  150u
#define ADC_ARFF_NOT_SURPPOERT   151u
#define RELAY_NOT_EXIST          152u
#define ADC_BUF_OVER             153u
#define PIN_BUF_OVER             154u
#define DM_BUF_OVER              155u
#define ADC_CHANNEL_NOT_EXIT     156u
#define BRT_NOT_SURPORT          157u
#define ADDR_NOT_EXIT            158u
#define DVC_ADDR_NOT_EXIT        159u
#define ADC_CURRENT_ERR          160u
#define MULT_NOT_SUPPORT         161u
#define MONIT_CYC_ERR            162u
#define FRQ_MODULE_ERR           163u
#define FRQ_TYPE_ERR             164u
#define BUST_DM_IN_SESS          165u
#define REPEAD_CMD_ERR           166u
#define TIME_OUT                 167u
#define CODEC_ERR                168u 
#define CMD_NOT_SUPPORT          169u
#define GPIO_HIGHT_WTG           170u
#define CHIP_BUFF_ERR            171u
#define MMX_ERR                  172u
#define PROTOCOL_ERR             173u
#define MCU_FLASH_OP_ERR         174u

#define PARA_SET_ERR             176u
#define SET_GPIO_ERR             177u
#define SET_SPI_ERR              178u
#define SPI_BUS_ERR              179u
#define I2C_BUS_ERR              180u
#define CHIP_UART_ERR            181u
#define POWER_MODULE_ERR         182u
#define POWER_MODULE_NOT_EXIST   183u
#define FRQ_MODULE_NOT_EXIST     184u
#define CHIP_UART_NOT_CONNECT    185u
#define MODE_NOT_SUPPORT         186u
#define SET_I2C_ERR              187u
#define ADDR_PARA_ERR            188u
#define UART_RD_TIMEOUT_ERR      189u     

#define EMB_FORMAT_ERR                 190u
#define EMB_CMD_ERR                    191u
#define EMB_LEN_OERFLOW_ERR            192u
#define RULER_STATE_ERR                193u
#define SEND_DATA_LEN_ERR              194u
#define POST_ERR_CODEC                 195u
#define POST_ERR_FM36                  196u
#define POST_ERR_AUDIO                 197u
#define POST_ERR_RULER                 198u
#define AUD_CFG_MIC_NUM_MAX_ERR        199u
#define AUD_CFG_PLAY_CH_ERR            200u
#define AUD_CFG_MIC_NUM_DISMATCH_ERR   201u
#define AUD_CFG_PLAY_CH_ZERO_ERR       202u
#define AUD_CFG_ERR                    203u
#define UIF_TYPE_NOT_SUPPORT_ERR       204u
#define AUD_CFG_SPI_REC_CONFLICT_ERR   205u
#define FPGA_CFG_ERR                   206u
#define AUD_CFG_AUDIOPATH_ERR          207u


#define FW_BIN_STATE_ERR         211u
#define FW_BIN_STATE_0_ERR       212u
#define FW_BIN_STATE_1_ERR       213u
#define FW_BIN_SAVE_CMD_ERR      214u
#define FW_BIN_SAVE_ADDR_ERR     215u
#define FW_VEC_SAVE_STATE_ERR    216u
#define FW_VEC_SET_CFG_ERR       217u

#define FM36_WR_DM_ERR           220u
#define FM36_WR_PM_ERR           221u
#define FM36_WR_HOST_ERR         222u
#define FM36_RD_DM_ERR           223u
#define FM36_RD_CM_ERR           224u
#define FM36_CHECK_ROMID_ERR     225u
#define FM36_CHECK_COUNTER_ERR   226u
#define FM36_CHECK_FLAG_ERR      227u
#define FM36_DMIC_PGA_GAIN_ERR   228u

#define CODEC_WR_REG_ERR              230u
#define CODEC_SETVOL_RANGE_ERR        231u
#define CODEC_SETFCLK_RANGE_ERR       232u
#define CODEC_SETMODE_RANGE_ERR       233u
#define CODEC_SR_NOT_SUPPORT_ERR      234u
#define CODEC_SR_LEN_NOT_SUPPORT_ERR  235u
#define CODEC_FUNC_NOT_SUPPORT        236u
#define CODEC_BIT_LEN_NOT_SUPPORT_ERR 237u
#define CODEC_FORMAT_NOT_SUPPORT_ERR  238u
#define CODEC_CH_NUM_NOT_SUPPORT_ERR  239u


//ERROR CODE from 245~ 255 reserved for Audio MCU
/*
*********************************************************************************************************
*                                           Noah CMD Defines
*********************************************************************************************************
*/
#define STAT_SYNC1    0x00
#define STAT_SYNC2    0x01
#define STAT_FLAG     0x02
#define STAT_LENGTH   0x04
#define STAT_DATA     0x08
#define STAT_FRAM     0x10
#define STAT_CHECKSUM 0x20
#define STAT_CMD      0x40

#define CMD_DATA_SYNC1    0xeb
#define CMD_DATA_SYNC2    0x90
#define CMD_DATA_SYNC2_1  0x90
#define FRAM_TYPE_ACK     0x00
#define FRAM_TYPE_NAK     0x01
#define FRAM_TYPE_DATA    0x02
#define FRAM_TYPE_GDD_IIC 0x11
#define FRAM_TYPE_EST     0x3e
#define FRAM_TYPE_ESTA    0x3f

#define DATA_TYPE_GPIO    0x03
#define DATA_TYPE_ADC     0x31
#define DATA_TYPE_DM      0x11
#define DATA_TYPE_PM      0x16
#define DATA_TYPE_CM      0x19

#define DATA_TYPE_EM      0x22
#define DATA_TYPE_RAW     0x25
#define DATA_TYPE_MONIT   0x82
#define DATA_TYPE_REGTIME 0x84
#define DATA_TYPE_HR      0xB0
#define DATA_TYPE_DR      0xB1

//PC to MCU
#define GPIO_WRITE_CMD        0x00   // set GPIO H or L
#define GPIO_HIGHTZ_CMD       0x01   // set GPIO Hight Impendance
#define GPIO_READ_CMD         0x02   // read GPIO
#define DM_SINGLE_WRITE_CMD   0x10   // DM single write
#define DM_BURST_WRITE_CMD    0x12   // DM burst write
#define DM_SINGLE_READ_CMD    0x13   // DM single read
#define PM_BURST_WRITE_CMD    0x14   // PM burst write
#define PM_SINGLE_READ_CMD    0x15   // PM single read
#define PM_SINGLE_WRITE_CMD   0x17   // PM single write
#define CM_SINGLE_WRITE_CMD   0x18   // CM single write
#define CM_SINGLE_READ_CMD    0x1b   // CM single read
#define INTERFACE_CTR_CMD     0x20   // Interface ctr
#define AD_READ_CMD           0x30   // AD read
#define RELAY_CTR_CMD         0x40   // relay ctr
#define SESSION_CTR_CMD       0x50   // session ctr
#define REPEAT_CTR_CMD        0x51   // repeat ctr
#define DELAY_CTR_CMD         0x52   // delay ctr
#define SPECIAL_CTR_CMD       0x80   // special ctr
#define POWER_ADJ_CMD         0x60   // adjust voltage
#define POWER_CTR_CMD         0x61   // power turn on-off
#define FRQ_ADJ_CMD           0x70   // adjust frq
#define FRQ_CTR_CMD           0x71   // FRQ turn on-off
#define EM_WRITE_CMD          0x21   // eeprom write
#define EM_READ_CMD           0x23   // eeprom read
#define RAW_WRITE_CMD         0x24   // RAW write
#define RAW_READ_ACK          0x25   // RAW read data
#define RAW_READ_CMD          0x26   // RAW read
#define GPIO_RECORDE_CMD      0x81   // gpio recorde      
#define REG_TIME_CMD          0x83    
#define HR_WRITE_CMD_1        0xA2
#define HR_WRITE_CMD_2        0xA0
#define DR_WRITE_CMD_1        0xA3
#define DR_WRITE_CMD_2        0xA1
#define HR_READ_CMD           0xA4
#define DR_READ_CMD           0xA5

// MCU to PC
#define GPIO_RPT            0x03       // GPIO report
#define DM_SRPT             0x11       // DM single report
#define PM_SRPT             0x16       // PM single report
#define ADC_RPT             0x31       // ADC report
#define REPORT_FINISH       0x53       // report finish
#define EM_RPT              0x22       // eeprom report
#define RAW_RPT             0x26       // RAW RPT
#define CMD_ERR_RPT         0xff
#define CMDDN_ERR_RPT       0xfe
#define UNKOW_ERR_RPT       0x66

//EMB
#define EMB_DATA_FRAME      0xE0
#define CMD_D_ACK           0xFF
#define CMD_G_ACK           0xFE
        

/*
*********************************************************************************************************
*                                      iSAM Test Bench CMD Defines
*********************************************************************************************************
*/
#define  PC_CMD_SET_AUDIO_CFG        1
#define  PC_CMD_START_AUDIO          2
#define  PC_CMD_STOP_AUDIO           3
#define  PC_CMD_RAED_RULER_INFO      4
#define  PC_CMD_WRITE_RULER_INFO     5  
#define  PC_CMD_READ_MIC_CALI_DATA   6
#define  PC_CMD_WRITE_MIC_CALI_DATA  7  
#define  PC_CMD_TOGGLE_MIC           8
#define  PC_CMD_READ_AB_STATUS       9
#define  PC_CMD_RAED_AB_INFO         11
#define  PC_CMD_RAW_DATA_TRANS       12
#define  PC_CMD_RESET_MIC            13
#define  PC_CMD_SET_VOLUME           14
#define  PC_CMD_RESET_AUDIO          15
#define  PC_CMD_UPDATE_AUDIO         16
#define  PC_CMD_UPDATE_FPGA_SWITCH   17

#define  PC_CMD_AB_POST              20

#define  PC_CMD_SET_IF_CFG           30
#define  PC_CMD_RAW_WRITE            31
#define  PC_CMD_RAW_READ             32
#define  PC_CMD_BURST_WRITE          33
#define  PC_CMD_BURST_READ           34
#define  PC_CMD_SESSION              35
#define  PC_CMD_DELAY                36
#define  PC_CMD_MCU_FLASH_WRITE      40
#define  PC_CMD_SET_VEC_CFG          41

#define  PC_CMD_REC_VOICE_BUFFER     42 //trigger SPI voice buf rec 
//#define  PC_CMD_FETCH_VOICE_BUFFER   43
#define  PC_CMD_TO_IM501_CMD         44
#define  PC_CMD_ENTER_PSM            45
#define  PC_CMD_GPIO_SESSION         46
#define  PC_CMD_SPI_REC              47 //start SPI record
#define  PC_CMD_IF_ONOFF             48
#define  PC_CMD_SPI_PLAY             49 //start SPI play

#define  PC_CMD_DOWNLOAD_RULER_FW    100
#define  PC_CMD_UPDATE_RULER_FW      101
#define  PC_CMD_UPDATE_AB_FW         102
 

//Data
#define  DATA_RULER_INFO             5
#define  DATA_CALIB_INFO             7
#define  DATA_AB_STATUS              10    
#define  DATA_AB_INFO                50

#define  DATA_UIF_RAW_RD             PC_CMD_RAW_WRITE //PC_CMD_RAW_WRITE type data

/*
*********************************************************************************************************
*                                      Noah De-analyze Macro
*********************************************************************************************************
*/
#define GET_FRAME_ID( head )                      ((head) & 0xC0)
#define GET_FRAME_TYPE( head )                    ((head) & 0x3F)
#define SET_FRAME_HEAD( frame_id, frame_type )    ((frame_type & 0x3F)|(frame_id & 0xC0))

#define NOAH_CMD_DATA_MLEN     255  // max 255 cmd data per package
#define NEW_CMD_DATA_MLEN     (EMB_BUF_SIZE) //(2048-8) //KFIFO size is 1024, EMB = 1024 - Header

/*
*********************************************************************************************************
*                  Internal FLASH used by FW Code size in byte (128 Kbytes)
*********************************************************************************************************
*/
#define AT91C_IFLASH_CODE_SIZE	 (  0x00020000 )   //128kB
 

/*
*********************************************************************************************************
*                          Noah Related Type Defines
*********************************************************************************************************
*/
typedef struct {
    unsigned char head ;
    unsigned char DataLen ;
    unsigned char Data[256];
    unsigned char checkSum ;
}NOAH_CMD ;

typedef struct {
    unsigned char  head_sync_1 ;   //EB                             
    unsigned char  data_len[3] ;   //MSB first
    unsigned char  head_sync_2 ;   //90
    unsigned char  pkt_sn ;        //1 ~31
    unsigned char  cmd[2] ;        //MSB first
    unsigned char  data[NEW_CMD_DATA_MLEN] ; 
}*pNEW_NOAH_CMD, NEW_NOAH_CMD ;

typedef struct {
    unsigned int   index;    
    unsigned int   length ;
    unsigned char *pdata; 
    unsigned char  data[EMB_BUF_SIZE] ;
    unsigned char  pkt_sn;
    bool           state;
}EMB_BUF ;

//#pragma pack(1)
typedef union  {
  
    START_AUDIO           start_audio;
    AUDIO_CFG             audio_cfg;
    READ_RULER_INFO       r_ruler_info;
    WRITE_RULER_INFO      w_ruler_info;
    READ_MIC_CLAIB_INFO   r_mic_info;
    WRITE_MIC_CALIB_INFO  w_mic_info;
    TOGGLE_MIC            toggle_mic; 
    SET_VOLUME            set_volume;
    RAW_DATA_TRANS        raw_data_trans;
    UPDATE_RULER_FW       update_ruler_fw;
    UPDATE_BRIDGE_FW      update_bridge_fw;
    
    INTERFACE_CFG         interface_cfg;
    RAW_WRITE             raw_write;
    RAW_READ              raw_read;
    BURST_WRITE           burst_write;
    BURST_READ            burst_read;
    MCU_FLASH             mcu_flash;
    SET_VEC_CFG           set_vec_cfg;
    VOICE_BUF             voice_buf_data;
    SPI_PLAY_REC_CFG       spi_rec_cfg;
    GPIO_SESSION          gpio_session;
    FPGA_CFG              fpga_cfg;
}PCCMDDAT, *pPCCMDDAT ;

//#pragma pack()
typedef struct {
    void        *pRecvPtr;
    OS_EVENT    *pEvent;
    CPU_INT32U   state_mac;    
    CPU_INT32U   PcCmdCounter;
    CPU_INT32U   PcCmdDataLen;   
}CMDREAD ;


/*
*********************************************************************************************************
*                                 Function Declaration
*********************************************************************************************************
*/
extern void Init_EMB_BUF( EMB_BUF* pEBuf ) ;
extern void Init_CMD_Read( CMDREAD* pCMD_Read, OS_EVENT  *pOS_EVENT ) ;
extern void Noah_CMD_Unpack( CMDREAD* pCMD_Read, CPU_INT08U data_byte ) ;
extern void Noah_CMD_Unpack_New( CMDREAD* pCMD_Read, CPU_INT08U data_byte ) ;
//extern CPU_INT08U Noah_CMD_Parse(   CPU_INT08U  *pCmdDat, 
//                                    CPU_INT32U datalen
//                                );
extern CPU_INT08U Noah_CMD_Pack_Ruler(  OS_EVENT   *pOS_EVENT, 
                                    CPU_INT08U  frame_head, 
                                    CPU_INT08U *pdat, 
                                    CPU_INT08U  data_length, 
                                    CPU_INT08U  msg_post_mode,
                                    CPU_INT08U  *pex_dat,
                                    CPU_INT08U   ex_data_length 
                                 ) ;
extern CPU_INT08U  Noah_CMD_Pack_PC ( OS_EVENT    *pOS_EVENT,                               
                                 PCCMDDAT    *pPcCmdData,
                                 CPU_INT08U   pkt_index,
                                 CPU_INT16U   cmd_id                                 
                               );
extern CPU_INT08U  EMB_Data_Build (  CPU_INT16U   cmd_type, 
                              CPU_INT08U  *pChar, 
                              PCCMDDAT    *pPcCmdData,
                              CPU_INT32U  *p_emb_length);

extern CPU_INT08U CheckSum(     CPU_INT08U init_data, 
                                CPU_INT08U *pdata, 
                                CPU_INT16U length
                          );
extern CPU_INT08U  Noah_CMD_Parse_Ruler ( NOAH_CMD    *pNoahCmd,                                 
                                          CPU_INT08U  *pSessionDone); 
//extern CPU_INT08U EMB_Data_Check( pNEW_NOAH_CMD pNoahCmd, EMB_BUF *pEBuf, CPU_INT08U delay);
extern CPU_INT08U  EMB_Data_Check (NOAH_CMD   *pNoahCmd, EMB_BUF *pEBuf, CPU_INT08U  delay);

extern CPU_INT08U EMB_Data_Parse ( pNEW_NOAH_CMD  pNewCmd )  ;
extern CPU_INT08U  AB_Status_Change_Report (void);
extern void  Send_DACK (CPU_INT08U  error_id);
extern void  Send_GACK (CPU_INT08U  error_id);
extern void  Send_Report (CPU_INT08U pkt_sn, CPU_INT08U error_id);

#endif 
