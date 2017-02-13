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

#include "stdio.h"
#include "stdint.h" 
#include "uif_usb.h"  
#include "defined.h"
#include "audio.h"
#include "uif_i2s.h"


/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                               on the
*                                    Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : audio.c
* Version       : V0.0.1
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :editor from uif board first version that writed by PQ.
*********************************************************************************************************
*/
//because AB04 has two ssc port,so extend to 4 sturct;
AUDIO_CFG  Audio_Configure_Instance0[ 2 ];
AUDIO_CFG  Audio_Configure_Instance1[ 2 ];

extern void Init_Audio_Bulk_FIFO( void );

unsigned char  padding_data_save;


/*
*********************************************************************************************************
*                                    Init_Play_Setting()
*
* Description :  Initialize USB bulk out (play) settings.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static uint8_t Init_Play_Setting( void *pInstance )
{
    uint8_t err = NULL;
    
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    AUDIO_CFG  *reg = ( AUDIO_CFG * )pSource->peripheralParameter;    
    
    if( ( reg[ PLAY ].channel_num == 0 ) 
      || ( reg[ PLAY ].channel_num > 8 ) ) 
    {        
        err = ERR_TDM_FORMAT ; 
    }
    
    if( ( reg[ PLAY ].bit_length != 16 ) 
       && ( reg[ PLAY ].bit_length != 32 )  ) 
    {        
        err = ERR_TDM_FORMAT ; 
    }
    
    if( NULL != err ) {
        printf( "Init Play Setting Error !\r\n" );
        return err;
    }
    
    if( reg[ PLAY ].bit_length == 16 ) 
    {
        pSource->warmWaterLevel = reg[ PLAY ].sample_rate / 1000 * reg[ PLAY ].channel_num * 2 * 2;
    } 
    else 
    { //32
        pSource->warmWaterLevel = reg[ PLAY ].sample_rate / 1000 * reg[ PLAY ].channel_num * 2 * 4;        
    }
    
     pSource->set_peripheral = ssc_txRegister_set;
     
     if( NULL != pSource->set_peripheral )
       pSource->set_peripheral( ( void * )pSource,( void * )&Audio_Configure_Instance1[ PLAY ] );
    
      return err;
}


/*
*********************************************************************************************************
*                                    Init_Rec_Setting()
*
* Description :  Initialize USB bulk in (record) settings.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static uint8_t Init_Rec_Setting( void *pInstance )
{
    uint8_t  err = NULL;
    
    assert( NULL != pInstance );
    
    DataSource *pSource = ( DataSource * )pInstance;
    AUDIO_CFG  *reg = ( AUDIO_CFG * )pSource->peripheralParameter; 

#if 0    
    printf( "\r\nStart [%dth]Rec [%dCH - %dHz - %dBit][%d%s - %dDelay - %d%sLeft]...\r\n",\
             counter_rec++,
             channels_rec,
             sample_rate,
             bit_length,
             cki,(cki==0)?"Fall" :"Rise",
             delay,
             start,
             ( start == 4 )?"Low":"High" );   
#endif
    
    if( ( reg[ REC ].channel_num == 0 ) 
      || ( reg[ REC ].channel_num > 8 ) ) 
    {        
        err = ERR_TDM_FORMAT ; 
    }
    
    if( ( reg[ REC ].bit_length != 16 ) 
       && ( reg[ REC ].bit_length != 32 )  ) 
    {        
        err = ERR_TDM_FORMAT ; 
    }
    
    if( NULL != err ) 
    {
        printf("Init Rec Setting Error !\r\n");
        return err;
    }
    
    if( reg[ REC ].bit_length == 16 ) 
    { 
        pSource->warmWaterLevel  = reg[ REC ].sample_rate / 1000 * reg[ REC ].channel_num  * 2 * 2; // 2ms * 16bit
    } 
    else 
    { //32bit case
        pSource->warmWaterLevel  = reg[ REC ].sample_rate / 1000 * reg[ REC ].channel_num  * 2 * 4; // 2ms * 32bit       
    }
    
     pSource->set_peripheral = ssc_rxRegister_set;
     
     if( NULL != pSource->set_peripheral )
       pSource->set_peripheral( ( void * )pSource,( void * )&Audio_Configure_Instance1[ REC ] );
                 
    return 0;
}


/*
*********************************************************************************************************
*                                    Audio_Start_Rec()
*
* Description :  Start USB data transfer for recording.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static uint8_t Audio_Start_Rec( void )
{  
    uint8_t err;  
    
    err = init_Rec_Setting( &source_ssc0 );
    if( err != 0 ) {
        return err;
    }
//    SSC_Record_Start(); 
   
    return 0;  
}


/*
*********************************************************************************************************
*                                    Audio_Start_Play()
*
* Description :  Start USB data transfer for playing.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static uint8_t Audio_Start_Play( void )
{  
    uint8_t err;  
//    Init_I2S_Buffer();         //--avoid error leo 
    err = Init_Play_Setting( &source_ssc0 ); 
    err = Init_Play_Setting( &source_ssc1 );
    if( err != 0 ) 
    {
        return err;
    }
//    SSC_Play_Start();                 //--avoid error leo 
//    bulkout_enable  = true ;
     
    return 0;
}


/*
*********************************************************************************************************
*                                    Audio_Stop()
*
* Description :  Stop USB data transfer.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
static void Audio_Stop( void )
{     
    printf( "\r\nEnd Audio Transfer..."); 
//    End_Audio_Transfer();      //--avoid error leo
//    delay_ms(10);              //--avoid error leo
    
    printf("\r\nReset USB EP...");

//    delay_ms(50);             //--avoid error leo 
 
//    SSC_Reset(); //I2S_Init();   //--avoid error leo     
    Init_Audio_Bulk_FIFO();    
//    LED_Clear( USBD_LEDDATA ); 
}


/*
*********************************************************************************************************
*                                    Audio_State_Control()
*
* Description : Process command from Host MCU via USB.
*
* Argument(s) : msg: send control command to other task;
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Audio_State_Control( uint8_t *msg )
{    
 /*
  uint8_t err = 0;
    uint32_t  temp ;    

    static uint16_t audio_cmd_index;      //just defined it avoid compile error by leo
    static uint16_t audio_state_check;    //just defined it avoid compile error by leo
    static uint32_t time_start_test;      //just defined it avoid compile error by leo
    static uint32_t second_counter;       //just defined it aboid compile error by leo
    
    if( audio_cmd_index == AUDIO_CMD_IDLE ) {
 //       printf( " program return at %d \n\r",__LINE__);
 //       return;
    }

//    if( USBD_GetState() < USBD_STATE_CONFIGURED && 
//        audio_cmd_index != AUDIO_CMD_VERSION ) {
//        err = ERR_USB_STATE;
        
//    } else {
   
        switch( audio_cmd_index ) 
        {           
            case AUDIO_CMD_START_REC :                
                if( audio_state_check != 0 ) 
                {
                    Audio_Stop(); 
//                    Rec_Voice_Buf_Stop();      //--avoid error leo 
//                    counter_stop_cmd_miss++;
                }                 
//                bulkout_trigger = true; //trigger paly&rec sync
                err = Audio_Start_Rec();  
                time_start_test = second_counter ;
                audio_state_check = 1;
            break;
         
            case AUDIO_CMD_START_PLAY :                
                if( audio_state_check != 0 ) 
                {
                    Audio_Stop(); 
//                    Rec_Voice_Buf_Stop();        //--avoid error leo 
//                    counter_stop_cmd_miss++;
                }                     
                err = Audio_Start_Play();  
                time_start_test = second_counter ;
                audio_state_check = 2; 
            break;
     
            case AUDIO_CMD_START_PALYREC :                
                if( audio_state_check != 0 ) 
                {
                    Audio_Stop();
//                    Rec_Voice_Buf_Stop();      //--avoid error leo 
//                    counter_stop_cmd_miss++;
                }                                         
                err = Audio_Start_Play();
                if( err == 0 ) 
                {                    
//                delay_ms(1);  //make sure play and rec enter interruption in turns 2ms              
                  err = Audio_Start_Rec(); 
                }
                time_start_test = second_counter ;
                audio_state_check = 3; 
            break;
            
            case AUDIO_CMD_STOP : 
                if( audio_state_check != 0 ) 
                {
                  Audio_Stop(); 
//                  Rec_Voice_Buf_Stop(); 
                  printf("\r\nThis cycle test time cost: ");
                  Get_Run_Time(second_counter - time_start_test);   
                  printf("\r\n\r\n");
                  time_start_test = 0 ;
                  audio_state_check = 0; 
                }

            case AUDIO_CMD_CFG: 
                if( Audio_Configure_Instance1[ PLAY ].bit_length == 16 ) 
                {
                    temp = Audio_Configure_Instance1[ PLAY ].sr / 1000 *  Audio_Configure_Instance1[ PLAY ].channels * 2 * 2;
                } 
                else 
                { //32
                    temp = Audio_Configure_Instance1[ PLAY ].sr / 1000 *  Audio_Configure_Instance1[ PLAY ].channels * 2 * 4;        
                }            
                if( ( temp * PLAY_BUF_DLY_CNT ) > USB_RINGOUT_SIZE_16K ) 
                { //play pre-buffer must not exceed whole play buffer
                    err = ERR_AUD_CFG;
                }              
            break;
            
            case AUDIO_CMD_VERSION: 

            break;         
            
            case AUDIO_CMD_RESET:                     
            break;  
            
            case AUDIO_CMD_READ_VOICE_BUF : 
//                    Rec_Voice_Buf_Start();
            break;          
            
            default:         
                err = ERR_CMD_TYPE;
            break;
        
        }   
  */
}

/*
*********************************************************************************************************
*                                    Get_Run_Time()
*
* Description : Format ms tick to [day:hour:min:sec:0.sec], and print.
*
* Argument(s) : time  -  number of 100ms tick from Timer #2 , using global : second_counter .
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Get_Run_Time( uint32_t time )
{
    
    uint8_t  msec, sec, min, hour;
    uint32_t   day;

    msec = time % 10;
    sec  = time /10 % 60 ;
    min  = time / 600 %60 ;
    hour = time / 36000 %24 ; 
    day  = time / 36000 /24 ;
    printf("[%d:%02d:%02d:%02d.%d]", day, hour, min, sec, msec ); 
    
}


/*
*********************************************************************************************************
*                                  First_Pack_Check_BO()
*
* Description :  Check if first USB bulk out package is same as padding data.
*
* Argument(s) :  None.
*
*Return(s)   :   true  - check ok.
*                false - check failed.
*
* Note(s)     :  None.
*********************************************************************************************************
*/

bool First_Pack_Check_BO( unsigned char *pData, unsigned int size )
{    
    
    unsigned int i;
    
    for( i = 0; i < size ; i++ )   {
        if( padding_data_save != *pData++) {
            return false;
        }
    }
     //printf("\r\nSync\r\n");
    return true; 

}

/*
*********************************************************************************************************
*                                First_Pack_Padding_BI()
*
* Description :  Padding the first USB bulk in package.
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     :  Must be called after reset FIFO and before start audio.
*********************************************************************************************************
*/


void First_Pack_Padding_BI( unsigned char usb_data_padding )
{
    unsigned char temp[ USB_DATAEP_SIZE_64B ];
    
    APP_TRACE_INFO(("\r\nUSB data padding = %d\r\n",usb_data_padding));
    padding_data_save = usb_data_padding;
    memset( temp, usb_data_padding, USB_DATAEP_SIZE_64B );
    kfifo_put(&ep0BulkIn_fifo, temp, USB_DATAEP_SIZE_64B) ;
    kfifo_put(&ep0BulkIn_fifo, temp, USB_DATAEP_SIZE_64B) ;//2 package incase of PID error
}




/*
*********************************************************************************************************
*                                    Audio_Manager()
*
* Description : Audio Port Manager .
*
* Argument(s) : confif data
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Audio_Manager( unsigned char cfg_data )
{

    APP_TRACE_INFO(( "\r\nAudio Manager: config data = 0X%0X ]", cfg_data ));

        
        if( (cfg_data & SSC0_IN) && ( source_ssc0.status[IN] >= CONFIGURED ) ) {

            source_ssc0.buffer_read(   &source_ssc0,
                                      ( uint8_t * )ssc0_PingPongIn,                                              
                                      source_ssc0.rxSize );
            source_ssc0.status[ IN ]  = ( uint8_t )START;
        } 
        OSTimeDly(2);      
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
               
      
}


