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
#ifndef __UIF_H__
#define __UIF_H__

#include "audio.h"
#include "ruler.h"

//Global_UIF_Setting CMD:  num = 7
//note: CMD nums must not exceed boundary

//////////////////  UIF TYPE 
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
#define   ATTRI_I2C_FM1388_LOAD_CODE           30
#define   ATTRI_I2C_FM1388_LOAD_EEPROM         21
#define   ATTRI_I2C_IM205                      11

#define   ATTRI_SPI_IM501_CPHA0_CPOL0          0 //iM501_CPHA_CPOL
#define   ATTRI_SPI_IM501_CPHA0_CPOL1          1
#define   ATTRI_SPI_IM501_CPHA1_CPOL0          2
#define   ATTRI_SPI_IM501_CPHA1_CPOL1          3

#define   FM1388_ALLOWED_DATA_PACK_SIZE        (240+6)
#define   EEPROM_ALLOWED_DATA_PACK_SIZE        (128+2)
#define   FM1388_I2C_DATA_PACK_SIZE            (4) //4bytes

#define   ATTRI_FM36_PATH_NORMAL           0 //normal mode: PDM path 
#define   ATTRI_FM36_PATH_PWD_BP           1 //bypass mode : I2S/TDM/PCM path

//////////////////

#define   GET_BYTE_LOW_4BIT( x )     ((x) & 0x0F) 
#define   GET_BYTE_HIGH_4BIT( x )    (((x)>>4) & 0x0F) 
#define   GET_I2C_GPIO_SCL(x)        GET_BYTE_LOW_4BIT( x ) 
#define   GET_I2C_GPIO_SDA(x)        GET_BYTE_HIGH_4BIT( x ) 



extern unsigned char Reg_RW_Data[];
extern INTERFACE_CFG   Global_UIF_Setting[];

void Reverse_Endian( unsigned char *pdata, unsigned char size ) ;
unsigned char Setup_Interface( INTERFACE_CFG *interface_cfg );
unsigned char Raw_Write( RAW_WRITE *p_raw_write );
unsigned char Raw_Read( RAW_READ *p_raw_read );
unsigned char Write_Burst( BURST_WRITE nurst_write );
void Dump_Data ( unsigned char *pdata, unsigned int size );
unsigned char GPIO_Session( GPIO_SESSION *p_gpio_session );


unsigned char TWID_Write_UIF( 
    uint8_t address,
    uint32_t iaddress,
    uint8_t isize,
    uint8_t *pData,
    uint32_t num,
    Async *pAsync);
unsigned char TWID_Read_UIF(
    uint8_t address,
    uint32_t iaddress,
    uint8_t isize,
    uint8_t *pData,
    uint32_t num,
    Async *pAsync);

#endif
