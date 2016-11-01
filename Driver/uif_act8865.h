#ifndef _ACT8865_H_
#define _ACT8865_H_

#include "defined.h"

#define ACT8865_ADDRESS 0x5B

#define ACT8865_OUT6_CONTROL_REG  0x61
#define ACT8865_OUT6_DATA_REG     0x60

#define ACT8865_OUT6_VSET_3V3     0x06

uint8_t act8865_read(uint8_t reg_addr, uint8_t *data);
uint8_t act8865_write( uint8_t reg,uint8_t *data );

#endif
