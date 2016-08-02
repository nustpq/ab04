#ifndef _UIF_GPIO_H_
#define _UIF_GPIO_H_

#include "defined.h"

//gpio rec configure
typedef struct _gpio_rec_parameter
{
  uint8_t mask;                     //bitmap for gpio which used for rec
  uint8_t cnt;                      //count pins in using
  uint8_t index;                    //start index
  uint8_t data_mask;                //bitmap for gpio level which used
}GPIO_REC_CFG;


void      disable_GPIO_Interrupt( void *pInstance );
void      _ConfigureRecGpios( void );
void      gpio_Init( void *pParameter,void *dParameter );
uint8_t   gpio_Pin_Set( void *pinInstance, const uint8_t * pdata,uint32_t mask );
uint8_t   gpio_Pin_Get( void *pinInstance, const uint8_t * pdata,uint32_t mask );
void      GPIOPIN_Set_Session( uint32_t pin , uint32_t dat );

#endif
