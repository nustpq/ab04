/*
*********************************************************************************************************
*                                          UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/


#include "alert_table.h"

#define  ALERT_TABLE_SR     (48)  //48k source table
#define  ALERT_TABLE_POINT  (48000)  //48k source table: 1s length
/*
*********************************************************************************************************
*                                    Alert_Sound_Gen()
*
* Description : Generate a alert predefined sound for USB audio for error reminder.
* Argument(s) :  *pdata      : pointer to the buffer adress where the generate data is stored.
*                 size       : buffer size in bytes
*                 REC_SR_Set : sample rate : 
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Alert_Sound_Gen( uint8_t *pdata, uint32_t size, uint32_t REC_SR_Set )
{
    
    uint32_t     sample_per_ms;
    uint32_t     table_lookup_step;     
    uint16_t  *pDest;
    uint32_t     sample_index = 0;
    uint16_t   temp;
    
    static uint32_t    index=0;    
    const uint16_t  *pVal;
    
    sample_per_ms     = REC_SR_Set / 1000 ;
    table_lookup_step = ALERT_TABLE_SR / sample_per_ms;    
    pVal              = (const uint16_t  *)alert_table; 
    pDest             = ( uint16_t  *)pdata; 
    
        
    while( sample_index < (size>>2) ) {  //3--2 ==> 4ch-->2ch      
       temp =   *(pVal+index) ;
       *( pDest )   =  temp;
       *( pDest+1 ) =  temp;
       //*( pDest+2 ) =  temp;            //4channel时打开
       //*( pDest+3 ) =  temp;            //4channel时打开
       sample_index++;
       pDest+=2;                          //4channel时，步长为4
       index = ( index + table_lookup_step ) % ALERT_TABLE_POINT ;
    
    }  
           
} 

/*
*********************************************************************************************************
*                                    Alert_Sound_Gen1()
*
* Description : Generate a alert predefined sound for USB audio for error reminder.
* Argument(s) :  *pdata      : pointer to the buffer adress where the generate data is stored.
*                 size       : buffer size in bytes
*                 REC_SR_Set : sample rate : 
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/       
void Alert_Sound_Gen1( uint8_t *pdata, uint32_t size, uint32_t REC_SR_Set )
{
    
    uint32_t     sample_per_ms;
    uint32_t     table_lookup_step;     
    uint16_t  *pDest;
    uint32_t     sample_index = 0;
    uint16_t   temp;
    
    static uint32_t    index=0;    
    const uint16_t  *pVal;
    
    sample_per_ms     = REC_SR_Set / 1000 ;
    table_lookup_step = ALERT_TABLE_SR / sample_per_ms;    
    pVal              = (const uint16_t  *)alert_table; 
    pDest             = ( uint16_t  *)pdata; 
    
        
    while( sample_index < (size>>2) ) {  //3--2 ==> 4ch-->2ch      
       temp =   *(pVal+index) ;
       *( pDest )   =  temp;
       *( pDest+1 ) =  temp;
       //*( pDest+2 ) =  temp;            //4channel时打开
       //*( pDest+3 ) =  temp;            //4channel时打开
       sample_index++;
       pDest+=2;                          //4channel时，步长为4
       index = ( index + table_lookup_step ) % ALERT_TABLE_POINT ;
    
    }  
           
}       
       

