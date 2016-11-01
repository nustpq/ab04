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
#ifndef __MEM_BASIC_H__
#define __MEM_BASIC_H__

#include "defined.h"

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



extern uint8_t DSP_PM_Type ; //FM36 unlock_mmreg is different from previous dsp


extern uint8_t HOST_SingleWrite_1(uint8_t dev_addr,uint8_t host_addr,uint8_t host_val) ; // 1bytes reg
extern uint8_t HOST_SingleWrite_2(uint8_t dev_addr,uint8_t host_addr,uint16_t host_val); // 2bytes reg
extern uint8_t HOST_LegacyRead(uint8_t dev_addr, uint8_t host_addr,uint8_t *pVal);
extern uint8_t HOST_SingleWrite_1_uart(uint8_t dev_addr,uint8_t host_addr,uint8_t host_val) ; // 1bytes reg
extern uint8_t HOST_SingleWrite_2_uart(uint8_t dev_addr,uint8_t host_addr,uint16_t host_val); // 2bytes reg
extern uint8_t HOST_LegacyRead_uart(uint8_t dev_addr, uint8_t host_addr,uint8_t *pVal);


extern uint8_t DSP_SingleWrite_1(uint8_t dev_addr,uint8_t dsp_addr,uint8_t dsp_val) ;
extern uint8_t DSP_SingleWrite_2(uint8_t dev_addr,uint8_t host_addr,uint16_t dsp_val);
extern uint8_t DSP_LegacyRead(uint8_t dev_addr, uint8_t host_addr,uint8_t *pVal);
extern uint8_t DSP_SingleWrite_1_uart(uint8_t dev_addr,uint8_t dsp_addr,uint8_t dsp_val) ;
extern uint8_t DSP_SingleWrite_2_uart(uint8_t dev_addr,uint8_t host_addr,uint16_t dsp_val);
extern uint8_t DSP_LegacyRead_uart(uint8_t dev_addr, uint8_t host_addr,uint8_t *pVal);

extern uint8_t DM_LegacyRead(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);
extern uint8_t DM_SingleWrite(uint8_t dev_addr,uint16_t dm_addr,uint16_t dm_val);
extern uint8_t DM_LegacyRead_uart(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);
extern uint8_t DM_SingleWrite_uart(uint8_t dev_addr,uint16_t dm_addr,uint16_t dm_val);

extern uint8_t PM_SingleWrite(uint8_t dev_addr,uint16_t dm_addr,uint8_t *pdata, uint32_t xor_key);
extern uint8_t PM_LegacyRead(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);
extern uint8_t PM_SingleWrite_uart(uint8_t dev_addr,uint16_t dm_addr,uint8_t *pdata, uint32_t xor_key);
extern uint8_t PM_LegacyRead_uart( uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);

extern uint8_t PM_BurstWrite_s(uint8_t dev_addr,uint16_t StAddr,uint8_t DatNum,void *pDat); //fake burst

extern uint8_t CM_SingleWrite(uint8_t dev_addr,uint16_t dm_addr,uint8_t *pdata);
extern uint8_t CM_LegacyRead(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);
extern uint8_t CM_SingleWrite_uart(uint8_t dev_addr,uint16_t dm_addr,uint8_t *pdata);
extern uint8_t CM_LegacyRead_uart(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);

extern uint8_t DM_FastRead(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);
extern uint8_t DM_FastReadReStart(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal) ;
extern uint8_t DM_LegacyReadReStart(uint8_t dev_addr, uint16_t dm_addr,uint8_t *pVal);
extern uint8_t DM_BurstWrite_s(uint8_t dev_addr,uint16_t StAddr,uint8_t DatNum,void *pDat);

extern uint8_t MEM_Block_LegacyRead(    uint8_t dev_addr, 
                                        uint8_t mem_type, 
                                        uint16_t start_addr,
                                        uint8_t num, 
                                        uint8_t *pVal );
extern uint8_t MEM_Block_LegacyRead_uart( uint8_t  dev_addr, 
                                          uint8_t  mem_type, 
                                          uint16_t start_addr,
                                          uint8_t  num, 
                                          uint8_t *pVal );
extern uint8_t MEM_Block_SingleWrite( uint8_t dev_addr,                                        
                                      uint8_t mem_type, 
                                      uint16_t start_addr,
                                      uint8_t num, 
                                      uint8_t *pVal );
extern uint8_t MEM_Block_SingleWrite_uart(  uint8_t dev_addr,                                             
                                            uint8_t mem_type, 
                                            uint16_t start_addr,
                                            uint8_t num, 
                                            uint8_t *pVal );

extern uint8_t Check_IDMA( uint8_t dev_addr ) ;




#endif
