
#ifndef FM1388D_H
#define FM1388D_H

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "fm1388_spi.h"

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
 
extern uint32_t  FM1388D_ReadWrite( Fm1388 *pFm1388,
                                    uint8_t *pData,
                                    uint8_t *cmd, 
                                    uint32_t rSize, 
                                    uint32_t wSize );
 
extern uint8_t SPI_WriteBuffer_API( Fm1388 *pFm1388 ,
                                    uint8_t *pdata, 
                                    uint32_t size );
extern uint8_t SPI_WriteReadBuffer_API( Fm1388 *pFm1388 ,  
                                        void *buffer_r,  
                                        void *buffer_w, 
                                        uint32_t length_r,  
                                        uint32_t length_w );

#endif // #ifndef FM1388D_H

