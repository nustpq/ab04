/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
*
* Filename      : uif_twi.h
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#ifndef _UIF_TWI_H_
#define _UIF_TWI_H_

#include "uif_object.h"

#define UNAME_WR_ADDR 0x60
#define UNAME_RX_ADDR 0x61

#define CODEC_WR_ADDR 0x30
#define CODEC_RX_ADDR 0x31

#define FM36_RX_ADDR ( 0x60 << 1 )
#define FM36_WR_ADDR ( FM36_RX_ADDR | 0x01 )

typedef enum _twi_port
{
    UNAMED = 0,
    CODEC1 ,
    FM36,
    PMIC,    
    CODEC0,
    MAXTWI
}TWIPORT;





void twi_init_master( void *pInstance, void* pFreq );
void twi_init_slave( void *pInstance, void* pSlave );

uint8_t twi0_uname_write(void *pInstance, const uint8_t *buf,uint32_t len );
uint8_t twi1_write(void *pInstance, const uint8_t *buf,uint32_t len );
uint8_t twi2_write(void *pInstance, const uint8_t *buf,uint32_t len  );

uint8_t twi0_uname_read(void *pInstance, const uint8_t *buf,uint32_t len  );
uint8_t twi1_read(void *pInstance, const uint8_t *buf,uint32_t len  );
uint8_t twi2_read(void *pInstance, const uint8_t *buf,uint32_t len   );




#endif
