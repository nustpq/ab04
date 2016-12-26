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

int usb_init_default( void );
void ssc0_init_default( void );
void ssc1_init_default( void );
void spi0_init_default( void );
void spi1_init_default( void );
void twi0_init_default( void );
void twi1_init_default( void );
void twi2_init_default( void );
void usart0_init_default( void );
void usart1_init_default( void );
void gpio_init_default( void );
void uif_miscPin_init_default( void );
void uif_ports_init_default( void );

unsigned char aic3204_init_default( void );

#endif