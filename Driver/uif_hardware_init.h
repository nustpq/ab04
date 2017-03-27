#ifndef _UIF_HARDWARE_INIT_H_
#define _UIF_HARDWARE_INIT_H_

#include "defined.h"
#include "uif_usb.h"
#include "uif_i2s.h"
#include "uif_twi.h"
#include "uif_usart.h"
#include "uif_gpio.h"
#include "bsp.h"
#include "codec.h"

int usb_init( void );
void ssc0_init( void );
void ssc1_init( void );
void spi0_init( unsigned int speed_hz, unsigned int format );
void spi1_init( unsigned int speed_hz, unsigned int format );
void twi0_init( unsigned int speed_hz );
void twi1_init( unsigned int speed_hz );
void twi2_init( unsigned int speed_hz );
void usart0_init( void );
void usart1_init( void );
void gpio_init( void );
void uif_miscPin_init_default( void );
void uif_ports_init_default( void );

unsigned char aic3204_init_default( void );
extern SPI_PLAY_REC_CFG spi0_cfg;
extern SPI_PLAY_REC_CFG spi1_cfg;


#endif