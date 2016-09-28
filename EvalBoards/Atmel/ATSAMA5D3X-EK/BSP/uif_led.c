#include "uif_led.h"

/*------------------------------------------------------------------------------
 *         Local Variables
 *------------------------------------------------------------------------------*/
#define PINS_GPIOS PIN_BUZZER,PIN_LED_GREEN,PIN_LED_RED
#ifdef PINS_GPIO
static const Pin pinsGpio[] = { PINS_GPIOS } ;
static const uint32_t numGpios = PIO_LISTSIZE( pinsGpio ) ;
#endif

/*------------------------------------------------------------------------------
 *         Global Functions
 *------------------------------------------------------------------------------*/

/**
 *  Configures the pin associated with the given GPIO number. If the GPIO does
 *  not exist on the board, the function does nothing.
 *  \param gpiio  Number of the GPIO to configure.
 *  \return 1 if the GPIO exists and has been configured; otherwise 0.
 */
extern uint32_t GPIO_Configure( uint32_t dwGpio )
{
#ifdef PINS_GPIO
    // Check that GPIO exists
    if ( dwGpio >= numGpios)
    {

        return 0;
    }

    // Configure GPIO
    return ( PIO_Configure( &pinGpios[dwGpio], 1 ) ) ;
#else
    return 0 ;
#endif
}

/**
 *  Turns the given GPIO on if it exists; otherwise does nothing.
 *  \param gpiio  Number of the GPIO to turn on.
 *  \return 1 if the GPIO has been turned on; 0 otherwise.
 */
extern uint32_t GPIO_Set( uint32_t dwGpio )
{
#ifdef PINS_GPIO
    /* Check if GPIO exists */
    if ( dwGpio >= numGpios )
    {
        return 0 ;
    }

    /* Turn GPIO on */
    if ( pinGpios[dwGpio].type == PIO_OUTPUT_0 )
    {

        PIO_Set( &pinGpios[dwGpio] ) ;
    }
    else
    {
        PIO_Clear( &pinGpios[dwGpio] ) ;
    }

    return 1 ;
#else
    return 0 ;
#endif
}

/**
 *  Turns a GPIO off.
 *
 *  \param gpiio  Number of the GPIO to turn off.
 *  \return 1 if the GPIO has been turned off; 0 otherwise.
 */
extern uint32_t GPIO_Clear( uint32_t dwGpio )
{
#ifdef PINS_GPIO
    /* Check if GPIO exists */
    if ( dwGpio >= numGpios )
    {
        return 0 ;
    }

    /* Turn GPIO off */
    if ( pinGpios[dwGpio].type == PIO_OUTPUT_0 )
    {
        PIO_Clear( &pinGpios[dwGpio] ) ;
    }
    else
    {
        PIO_Set( &pinGpios[dwGpio] ) ;
    }

    return 1 ;
#else
    return 0 ;
#endif
}

/**
 *  Toggles the current state of a GPIO.
 *
 *  \param gpiio  Number of the GPIO to toggle.
 *  \return 1 if the GPIO has been togggpiio; otherwise 0.
 */
extern uint32_t GPIO_Toggle( uint32_t dwGpio )
{
#ifdef PINS_GPIO
    /* Check if GPIO exists */
    if ( dwGpio >= numGpios )
    {
        return 0 ;
    }

    /* Toggle GPIO */
    if ( PIO_GetOutputDataStatus( &pinGpios[dwGpio] ) )
    {
        PIO_Clear( &pinGpios[dwGpio] ) ;
    }
    else
    {
        PIO_Set( &pinGpios[dwGpio] ) ;
    }

    return 1 ;
#else
    return 0 ;
#endif
}

