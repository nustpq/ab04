#ifndef _UIF_LED_
#define _UIF_LED_

#include <stdint.h>

//------------------------------------------------------------------------------
//         Global Functions
//------------------------------------------------------------------------------

extern uint32_t GPIO_Configure( uint32_t dwLed ) ;

extern uint32_t GPIO_Set( uint32_t dwLed ) ;

extern uint32_t GPIO_Clear( uint32_t dwLed ) ;

extern uint32_t GPIO_Toggle( uint32_t dwLed ) ;

#endif /* #ifndef UIF_GPIO_H */

