/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Unified EVM Interface Board 2.0
*
* Filename      : uif_act8865.c
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include "uif_act8865.h"
#include "uif_object.h"
#include "uif_twi.h"

DataSource *pAct8865Source;

uint8_t act8865_read( uint8_t reg,uint8_t *data )
{
  uint8_t regData[2];
  OPTIONPARAMETER *option;
  
  option = ( OPTIONPARAMETER * )source_twi1.privateData;
  
  option[ 1 ].address = ACT8865_ADDRESS >> 1;
  option[ 1 ].iaddress = 0;
  option[ 1 ].isize = 0;
  option[ 1 ].revers = 0;
  
  regData[ 0 ] = reg;
  
  return source_twi1.buffer_read( &source_twi1,regData,1 ); 
}

uint8_t act8865_write( uint8_t reg,uint8_t *data )
{
//twi_read(bus, ACT8865_ADDR, reg_addr, 1, data, 1);
    uint8_t regData[2];
    OPTIONPARAMETER *option;
     
    option = ( OPTIONPARAMETER * )source_twi1.privateData;
    
    option[ 0 ].address = ACT8865_ADDRESS;
    option[ 0 ].iaddress = reg;
    option[ 0 ].isize = 1;
    option[ 0 ].revers = 0;
    
    regData[ 0 ] = *data;
       
   return source_twi1.buffer_write( &source_twi1,regData,1 ); 
  
}
