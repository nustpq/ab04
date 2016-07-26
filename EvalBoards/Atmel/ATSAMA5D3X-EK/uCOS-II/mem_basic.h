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



extern unsigned char DSP_PM_Type ; //FM36 unlock_mmreg is different from previous dsp


extern unsigned char HOST_SingleWrite_1(unsigned char dev_addr,unsigned char host_addr,unsigned char host_val) ; // 1bytes reg
extern unsigned char HOST_SingleWrite_2(unsigned char dev_addr,unsigned char host_addr,unsigned short host_val); // 2bytes reg
extern unsigned char HOST_LegacyRead(unsigned char dev_addr, unsigned char host_addr,unsigned char *pVal);
extern unsigned char HOST_SingleWrite_1_uart(unsigned char dev_addr,unsigned char host_addr,unsigned char host_val) ; // 1bytes reg
extern unsigned char HOST_SingleWrite_2_uart(unsigned char dev_addr,unsigned char host_addr,unsigned short host_val); // 2bytes reg
extern unsigned char HOST_LegacyRead_uart(unsigned char dev_addr, unsigned char host_addr,unsigned char *pVal);


extern unsigned char DSP_SingleWrite_1(unsigned char dev_addr,unsigned char dsp_addr,unsigned char dsp_val) ;
extern unsigned char DSP_SingleWrite_2(unsigned char dev_addr,unsigned char host_addr,unsigned short dsp_val);
extern unsigned char DSP_LegacyRead(unsigned char dev_addr, unsigned char host_addr,unsigned char *pVal);
extern unsigned char DSP_SingleWrite_1_uart(unsigned char dev_addr,unsigned char dsp_addr,unsigned char dsp_val) ;
extern unsigned char DSP_SingleWrite_2_uart(unsigned char dev_addr,unsigned char host_addr,unsigned short dsp_val);
extern unsigned char DSP_LegacyRead_uart(unsigned char dev_addr, unsigned char host_addr,unsigned char *pVal);

extern unsigned char DM_LegacyRead(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);
extern unsigned char DM_SingleWrite(unsigned char dev_addr,unsigned short dm_addr,unsigned short dm_val);
extern unsigned char DM_LegacyRead_uart(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);
extern unsigned char DM_SingleWrite_uart(unsigned char dev_addr,unsigned short dm_addr,unsigned short dm_val);

extern unsigned char PM_SingleWrite(unsigned char dev_addr,unsigned short dm_addr,unsigned char *pdata, unsigned int xor_key);
extern unsigned char PM_LegacyRead(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);
extern unsigned char PM_SingleWrite_uart(unsigned char dev_addr,unsigned short dm_addr,unsigned char *pdata, unsigned int xor_key);
extern unsigned char PM_LegacyRead_uart( unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);

extern unsigned char PM_BurstWrite_s(unsigned char dev_addr,unsigned short StAddr,unsigned char DatNum,void *pDat); //fake burst

extern unsigned char CM_SingleWrite(unsigned char dev_addr,unsigned short dm_addr,unsigned char *pdata);
extern unsigned char CM_LegacyRead(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);
extern unsigned char CM_SingleWrite_uart(unsigned char dev_addr,unsigned short dm_addr,unsigned char *pdata);
extern unsigned char CM_LegacyRead_uart(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);

extern unsigned char DM_FastRead(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);
extern unsigned char DM_FastReadReStart(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal) ;
extern unsigned char DM_LegacyReadReStart(unsigned char dev_addr, unsigned short dm_addr,unsigned char *pVal);
extern unsigned char DM_BurstWrite_s(unsigned char dev_addr,unsigned short StAddr,unsigned char DatNum,void *pDat);

extern unsigned char MEM_Block_LegacyRead(     unsigned char dev_addr, 
                                        unsigned char mem_type, 
                                        unsigned short start_addr,
                                        unsigned char num, 
                                        unsigned char *pVal     );
extern unsigned char MEM_Block_LegacyRead_uart( unsigned char  dev_addr, 
                                         unsigned char  mem_type, 
                                         unsigned short start_addr,
                                         unsigned char  num, 
                                         unsigned char *pVal     );
extern unsigned char MEM_Block_SingleWrite(    unsigned char dev_addr,                                        
                                        unsigned char mem_type, 
                                        unsigned short start_addr,
                                        unsigned char num, 
                                        unsigned char *pVal );
extern unsigned char MEM_Block_SingleWrite_uart(   unsigned char dev_addr,                                             
                                            unsigned char mem_type, 
                                            unsigned short start_addr,
                                            unsigned char num, 
                                            unsigned char *pVal );

extern unsigned char Check_IDMA( unsigned char dev_addr ) ;




#endif
