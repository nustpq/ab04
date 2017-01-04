/*
*********************************************************************************************************
*                               UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           TASK PACKAGE
*
*                                        Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : task_audio_manager.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#include <includes.h>

OS_EVENT *App_AudioManager_Mbox;


/*
*********************************************************************************************************
*                                         App_AudioManager()
*
* Description : Check DBG_UART_Send_Buffer[] and Send debug data if kFIFO buffer is not empty
*
* Argument(s) : p_arg       Argument passed to 'App_AudioManager()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void  App_AudioManager (void *p_arg)
{

    CPU_INT08U    err ;
    CPU_INT32U   *msg ;    
    CPU_INT32U    cfg_data;
    
    (void)p_arg;

    err   = 0 ;
    msg   = NULL ;

////////////////////////////////////////////////////////////////////////////////
#if 0
    memset( ssc0_PingPongOut, 0x5555, sizeof( ssc0_PingPongOut ) );
    memset( ssc1_PingPongOut, 0x5555, sizeof( ssc1_PingPongOut ) );
    memset( ssc0_PingPongIn, 0 , sizeof( ssc0_PingPongIn ) );
    memset( ssc1_PingPongIn, 0 , sizeof( ssc1_PingPongIn ) );
#endif

#if 0
    Alert_Sound_Gen( ( uint8_t * )ssc0_PingPongOut,
                      sizeof( ssc0_PingPongOut[ 0 ] ),
                      8000 );

    Alert_Sound_Gen( ( uint8_t * )ssc0_PingPongOut[1],
                      sizeof( ssc0_PingPongOut[ 1 ] ),
                      8000 );

    Alert_Sound_Gen1( ( uint8_t * )ssc1_PingPongOut,
                       sizeof( ssc1_PingPongOut[ 0 ] ),
                       8000 );
    Alert_Sound_Gen1( ( uint8_t * )ssc1_PingPongOut,
                       sizeof( ssc1_PingPongOut[ 1 ] ),
                       8000 );
#endif

#if 0
    uint16_t *pInt = NULL;
    uint32_t i ;
    pInt = ( uint16_t * )ssc0_PingPongOut[0] ;
    for( i = 0; i< ( sizeof( ssc1_PingPongOut ) );  )
    {
       *(pInt+i++) = 0x1122 ;
       *(pInt+i++) = 0x3344 ;
       *(pInt+i++) = 0x5566 ;
       *(pInt+i++) = 0x7788 ;
       *(pInt+i++) = 0x99aa ;
       *(pInt+i++) = 0xbbcc ;
       *(pInt+i++) = 0xddee ;
       *(pInt+i++) = 0xff00 ;
    }

    pInt = ( uint16_t * )ssc1_PingPongOut[0] ;
    for( i = 0; i< ( sizeof( ssc1_PingPongOut ) ); )
    {
       *(pInt+i++) = 0x1122 ;
       *(pInt+i++) = 0x3344 ;
       *(pInt+i++) = 0x5566 ;
       *(pInt+i++) = 0x7788 ;
       *(pInt+i++) = 0x99aa ;
       *(pInt+i++) = 0xbbcc ;
       *(pInt+i++) = 0xddee ;
       *(pInt+i++) = 0xff00 ;
    }
#endif
////////////////////////////////////////////////////////////////////////////////

    while ( DEF_TRUE ) {  /* Task body, always written as an infinite loop.   */

        msg = ( uint32_t *)OSMboxPend( App_AudioManager_Mbox,  0,  &err );
        if( msg == NULL ) {
            continue;
        }
        cfg_data = *msg ; 
        APP_TRACE_INFO(( "\r\n[App_AudioManager_Mbox - cfg_data = 0X%0X ]", cfg_data ));

        
        if( (cfg_data & SSC0_IN) && ( source_ssc0.status[IN] >= CONFIGURED ) ) {

            source_ssc0.buffer_read(   &source_ssc0,
                                      ( uint8_t * )ssc0_PingPongIn,                                              
                                      source_ssc0.rxSize );
            source_ssc0.status[ IN ]  = ( uint8_t )START;
        }        
        if ( (cfg_data & SSC0_OUT) && ( source_ssc0.status[OUT] >= CONFIGURED ) ){
            source_ssc0.buffer_write(  &source_ssc0,
                                       ( uint8_t * )ssc0_PingPongOut,                                                
                                       source_ssc0.txSize ); 
            source_ssc0.status[ OUT ] = ( uint8_t )START;
        }
        if( (cfg_data & SSC1_IN) && ( source_ssc1.status[IN] >= CONFIGURED ) ) {

            source_ssc1.buffer_read(   &source_ssc1,
                                      ( uint8_t * )ssc1_PingPongIn,                                              
                                      source_ssc1.rxSize );
            source_ssc1.status[ IN ]  = ( uint8_t )START;
        }        
        if ( (cfg_data & SSC1_OUT) && ( source_ssc1.status[OUT] >= CONFIGURED ) ){
            source_ssc1.buffer_write(  &source_ssc1,
                                       ( uint8_t * )ssc1_PingPongOut,                                                
                                       source_ssc1.txSize ); 
            source_ssc1.status[ OUT ] = ( uint8_t )START;
        }
        
        /*
        switch( *msg )
        {
            case ( SSC0_IN | SSC0_OUT | SSC1_IN | SSC1_OUT ):
                    if( ( ( uint8_t )START != source_ssc0.status[ IN ] )
                        &&( ( uint8_t )BUFFERED != source_ssc0.status[ IN ] )
                        &&( ( uint8_t )RUNNING != source_ssc0.status[ IN ] ) )
                    {
                          memset( ( uint8_t * )&ssc0_PingPongOut[ 0], 0x55, source_ssc0.txSize );  
                          memset( ( uint8_t * )&ssc0_PingPongOut[ 1], 0x55, source_ssc0.txSize );  
                          source_ssc0.buffer_write( &source_ssc0,( uint8_t * )ssc0_PingPongOut,
                                                    //sizeof( ssc0_PingPongOut ) >> 1 );
                                                    source_ssc0.txSize );
                          OSTimeDly(4);
                          source_ssc0.buffer_read( &source_ssc0,( uint8_t * )ssc0_PingPongIn,
                                                   //sizeof( ssc0_PingPongIn ) >>1 );
                                                    source_ssc0.rxSize );
                          source_ssc0.status[ IN ]  = ( uint8_t )START;
                          source_ssc0.status[ OUT ] = ( uint8_t )START;

//                          source_ssc1.buffer_write( &source_ssc1,( uint8_t * )ssc1_PingPongOut,
//                                                   sizeof( ssc1_PingPongOut ) >> 1 );
//                          source_ssc1.buffer_read( &source_ssc1,( uint8_t * )ssc1_PingPongIn,
//                                                   sizeof( ssc1_PingPongIn ) >> 1 );
//                          source_ssc1.status[ IN ]  = ( uint8_t )START;
//                          source_ssc1.status[ OUT ] = ( uint8_t )START;

                    }
          break;

          default:
          break;

        }
        */
        
        
        OSTimeDly(10);

    }


}

