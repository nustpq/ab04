#ifndef _TWI_H_
#define _TWI_H_

#include "object.h"

#define UNAME_WR_ADDR 0x60
#define UNAME_RX_ADDR 0x61

#define CODEC_WR_ADDR 0x30
#define CODEC_RX_ADDR 0x31

#define FM36_RX_ADDR ( 0x60 << 1 )
#define FM36_WR_ADDR ( FM36_RX_ADDR | 0x01 )

typedef enum _twi_port
{
    UNAMED = 0,
    CODEC ,
    FM36,
    UNKNOWN
}TWIPORT;

typedef struct twi_option
{
  uint32_t address;
  uint32_t iaddress;
  uint8_t  isize;
  uint8_t  revers;
}OPTIONPARAMETER;



void twi_init_master( void *pInstance, void* pFreq );
void twi_init_slave( void *pInstance, void* pSlave );

uint8_t twi_uname_write(void *pInstance, const uint8_t *buf,uint32_t len );
uint8_t twi_codec_write(void *pInstance, const uint8_t *buf,uint32_t len );
uint8_t twi_fm36_write(void *pInstance, const uint8_t *buf,uint32_t len  );

uint8_t twi_uname_read(void *pInstance, const uint8_t *buf,uint32_t len  );
uint8_t twi_codec_read(void *pInstance, const uint8_t *buf,uint32_t len  );
uint8_t twi_fm36_read(void *pInstance, const uint8_t *buf,uint32_t len   );




#endif
