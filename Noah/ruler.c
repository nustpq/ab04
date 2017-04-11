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
*                                        RULER RELATED OPERATIONS REALIZATION
*
*                                          Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : ruler.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

//#include <includes.h>
#include "noah_cmd.h"
#include "codec.h"


volatile unsigned int   Global_Mic_Mask[4] ;      //MIC sellection status
volatile unsigned char  Global_Ruler_Index = 0 ;  //the ruler index for UART comm NOW
volatile unsigned char  Global_Bridge_POST = 0 ;  //audio bridge POST status
volatile unsigned char  Global_Ruler_State[4];    //ruler status
volatile unsigned char  Global_Ruler_Type[4];     //ruler type
volatile unsigned char  Global_Mic_State[4];      //MIC (8*4=32) status(calib info error or not)

volatile unsigned char  Ruler_Setup_Sync_Data;
unsigned char           Global_Ruler_CMD_Result;

extern EMB_BUF   Emb_Buf_Data;
extern EMB_BUF   Emb_Buf_Cmd;


/*
*********************************************************************************************************
*                                           Init_Global_Var()
*
* Description : Initialize Ruler and MIC related global variables to defalut value.
* Argument(s) : None.
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Init_Global_Var( void )
{
    unsigned char ruler_id;

    for( ruler_id = 0; ruler_id < 4; ruler_id++ ) {
        Global_Ruler_State[ruler_id] = RULER_STATE_DETACHED;
        Global_Ruler_Type[ruler_id]  = 0 ;
        Global_Mic_State[ruler_id]   = 0 ;
        Global_Mic_Mask[ruler_id]    = 0 ;
    }

}


/*
*********************************************************************************************************
*                                           Check_Actived_Mic_Number()
*
* Description : Check MIC mask global variable to get the total actived MICs number.
* Argument(s) : None.
* Return(s)   : mic_counter : the total actived MICs number.
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Check_Actived_Mic_Number( void )
{
    unsigned char mic_counter = 0;
    unsigned char i, j;

    for( i = 0; i < 4 ; i++ ) { //scan 4 slots
        for( j = 0; j < 32; j++ ) { //scan max 32mics per slot
            if( (Global_Mic_Mask[i]>>j)&1) {
                mic_counter++;
            }
        }
    }  
    return mic_counter;
    
}


/*
*********************************************************************************************************
*                                           Get_Mask_Num()
*
* Description : Check mask bit number
* Argument(s) : None.
* Return(s)   : mic_counter : the total actived MICs number.
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Get_Mask_Num( unsigned int mask )
{
   unsigned char i;
   unsigned char num = 0;

   for ( i = 0 ; i<32; i++ ) {
       if( mask & (1<<i) ) {
           num++;
       }
   }
   return num;

}


/*
*********************************************************************************************************
*                                           Check_UART_Mixer_Ready()
*
* Description : Check and wait until all data transmission inbuffer for current channel ruler is done .
*               To make sure ruler channels will not be mix up.
*               HW switch is important for this !
* Argument(s) : None.
* Return(s)   : mic_counter : the total actived MICs number.
*
* Note(s)     : If HW switch fast enough, no need this routine.
*********************************************************************************************************
*/
void Check_UART_Mixer_Ready( void )
{
#if 0  //No need UART switch on AB04
    unsigned char err;
    unsigned int  counter;

    counter = 0;
    while( OSQGet( EVENT_MsgQ_Noah2RulerUART, &err ) ) {
        OSTimeDly(1);
        counter++;
    }
    if( counter) {
        APP_TRACE_INFO(("Check_UART_Mixer_Ready, stage 1 : wait %d ms\r\n",counter));
    }

    counter = 0;
    while( Queue_NData((void*)pUART_Send_Buf[RULER_UART]) ) {
        OSTimeDly(1);
        counter++;
    }
    if( counter) {
        APP_TRACE_INFO(("Check_UART_Mixer_Ready, stage 2 : wait %d ms\r\n",counter));
    }
    OSTimeDly(5);
#endif

}



/*
*********************************************************************************************************
*                                       Init_Ruler()
*
* Description : Communicate with ruler to check connected or not
*
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   ruler connected
*               others :   =error code . ruler connection error,
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Init_Ruler( unsigned char ruler_slot_id ) //0 ~ 3
{
    unsigned char err;
  
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {
        return RULER_STATE_ERR ;
    }
    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_EST, NULL, 0, 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Init ruler[%d] timeout!\r\n",ruler_slot_id));
        } else {
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Init_Ruler[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
   
    return err ;
}


/*
*********************************************************************************************************
*                                       Setup_Ruler()
*
* Description : Send ruler slot id to ruler for identification.
*
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code . ruler connection error,
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Setup_Ruler( unsigned char ruler_slot_id ) //0 ~ 3
{
    unsigned char err;
    EMB_BUF      *pEBuf_Data;
    unsigned char buf[] = { RULER_CMD_SET_RULER, ruler_slot_id };
  
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    pEBuf_Data  = &Emb_Buf_Data; //Golbal var
    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {
        return RULER_STATE_ERR ;
    }

    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Setup_Ruler[%d] timeout\r\n",ruler_slot_id));
        } else {
            Ruler_Setup_Sync_Data = pEBuf_Data->data[0] ;
            APP_TRACE_INFO(("Get Ruler_Setup_Sync_Data : 0x%X\r\n",Ruler_Setup_Sync_Data));
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Setup_Ruler[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Get_Ruler_Type()
*
* Description : Get the specified ruler's type, and stored in a global variable, in which
*               bit7: 0-ruler, 1- handset. Other bits reserved.
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   refer to error code defines.
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Get_Ruler_Type(  unsigned char ruler_slot_id )
{
    unsigned char err;
    EMB_BUF      *pEBuf_Data;
    unsigned char buf[] = { RULER_CMD_GET_RULER_TYPE };
   
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    pEBuf_Data  = &Emb_Buf_Data; //Golbal var
    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {
        return RULER_STATE_ERR ;
    }

    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }
    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Ruler_Type[%d] timeout\r\n",ruler_slot_id));
        } else {
            Global_Ruler_Type[ruler_slot_id] =  pEBuf_Data->data[0] ;
            if( Global_Ruler_Type[ruler_slot_id]==0x30) {Global_Ruler_Type[ruler_slot_id] = 0x90;} //bugfix
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Get_Ruler_Type[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Read_Ruler_Status()
*
* Description : Get back specified ruler's POST status.
*
* Argument(s) : ruler_slot_id: 0~ 3.
*               status_data:   pointer to the address that store the read status data
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Read_Ruler_Status( unsigned char ruler_slot_id, unsigned short *status_data )
{
    unsigned char err ;
    EMB_BUF      *pEBuf_Data;
    unsigned char buf[] = { RULER_CMD_RAED_RULER_STATUS };
  
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    pEBuf_Data  = &Emb_Buf_Data; //Golbal var
    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {
        return RULER_STATE_ERR ;
    }
    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Ruler_Status[%d] timeout\r\n",ruler_slot_id));
        } else {
            *status_data = (pEBuf_Data->data[1] << 8) + pEBuf_Data->data[0] ;
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Read_Ruler_Status[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Read_Ruler_Info()
*
* Description : Get back specified ruler's infomation data.
*               And the read back data is stored in global varies : Emb_Buf_Data
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code . ruler connection error,
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Read_Ruler_Info( unsigned char ruler_slot_id )
{
    unsigned char  err;
    unsigned char  buf[] = { RULER_CMD_RAED_RULER_INFO };

    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {
        return RULER_STATE_ERR ;
    }
    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Ruler_Info[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Global_Ruler_CMD_Result;
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Read_Ruler_Info[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Write_Ruler_Info()
*
* Description : Write infomation data to specified ruler.
*               And before this function is called, the data to be written need have been stored in global varies : Emb_Buf_Cmd
* Argument(s) : ruler_slot_id: 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Write_Ruler_Info( unsigned char ruler_slot_id )
{
    unsigned char   err;
    unsigned short  data_length;
    unsigned char   temp;
    unsigned char  *pdata;
    unsigned char   buf[4];
    EMB_BUF        *pEBuf_Cmd;
   
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    pEBuf_Cmd  = &Emb_Buf_Cmd; //Golbal var
    buf[0] =  RULER_CMD_WRITE_RULER_INFO;
    buf[1] =  EMB_DATA_FRAME;
    buf[2] = (pEBuf_Cmd->length) & 0xFF;
    buf[3] = ((pEBuf_Cmd->length)>>8) & 0xFF;

    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {
        return RULER_STATE_ERR ;
    }
    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE != err ) { return err ; }
    pdata = pEBuf_Cmd->data;
    data_length = pEBuf_Cmd->length;
    while( data_length > 0 ){
        temp = data_length > (NOAH_CMD_DATA_MLEN-1) ? (NOAH_CMD_DATA_MLEN-1) : data_length ;
        err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, pdata, temp, 0, buf, 1 ) ;
        if( OS_ERR_NONE != err ) { break;}
        OSTimeDly(50); //wait for ruler operation
        data_length -= temp;
        pdata += temp;
    }
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Write_Ruler_Info[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Write_Ruler_Info[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;

}


/*
*********************************************************************************************************
*                                       Read_Mic_Cali_Data()
*
* Description : Get back specified ruler specified mic's calibration data.
*               And the read back data is stored in global varies : Emb_Buf_Data
* Argument(s) : ruler_slot_id : 0~ 3.
*               mic_id        : 0~ 7
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Read_Mic_Cali_Data(unsigned char ruler_slot_id, unsigned char mic_id)
{
    unsigned char  err ;
    unsigned char  buf[] = { RULER_CMD_READ_MIC_CALI_DATA, mic_id };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {
        return RULER_STATE_ERR ;
    }

    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Read_Mic_Cali_Data[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Global_Ruler_CMD_Result;
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Read_Mic_Cali_Data[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Write_Mic_Cali_Data()
*
* Description : Write calibration data to specified ruler specified mic.
*               And before this function is called, the data to be written need have been stored in global varies : Emb_Buf_Cmd
* Argument(s) : ruler_slot_id : 0~ 3.
*               mic_id        : 0~ 7
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Write_Mic_Cali_Data(unsigned char ruler_slot_id, unsigned char mic_id)
{
    unsigned char   err;
    unsigned short  data_length;
    unsigned char   temp;
    unsigned char  *pdata;
    unsigned char   buf[5];
    EMB_BUF        *pEBuf_Cmd;
  
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    pEBuf_Cmd  = &Emb_Buf_Cmd; //Golbal var
    buf[0] =  RULER_CMD_WRITE_MIC_CALI_DATA;
    buf[1] =  mic_id;
    buf[2] =  EMB_DATA_FRAME;
    buf[3] = (pEBuf_Cmd->length) & 0xFF;
    buf[4] = ((pEBuf_Cmd->length)>>8) & 0xFF;

    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {
        return RULER_STATE_ERR ;
    }
    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE != err ) { return err ; }
    pdata = pEBuf_Cmd->data;
    data_length = pEBuf_Cmd->length;
    while( data_length > 0 ){
        temp = data_length > (NOAH_CMD_DATA_MLEN-2) ? (NOAH_CMD_DATA_MLEN-2) : data_length ;
        err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, pdata, temp, 0, buf, 2 ) ;
        if( OS_ERR_NONE != err ) { break;}
        OSTimeDly(50); //wait for ruler operation
        data_length -= temp;
        pdata += temp;
    }
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Write_Mic_Cali_Data[%d][%d] timeout\r\n",ruler_slot_id, mic_id));
        } else {
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Write_Mic_Cali_Data[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;

}


/*
*********************************************************************************************************
*                                       Update_Mic_Mask()
*
* Description : Update specified ruler's all mic's active state.
* Argument(s) : ruler_slot_id : 0~ 3.
*               mic_mask      : bit[0..31]. 0 - deactive, 1 - active.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : Support: Ruler(8Mic) for Ruler
*                        Handset(16Mic) for H01/H02/H02A
*                        Handset(18Mic) for H03
*********************************************************************************************************
*/
unsigned char Update_Mic_Mask( unsigned char ruler_slot_id, unsigned int mic_mask )
{
    unsigned char err ;
    unsigned char buf_size_send ;
    unsigned char buf[] = { RULER_CMD_TOGGLE_MIC, mic_mask&0xFF, (mic_mask>>8)&0xFF,
                            (mic_mask>>16)&0xFF,  (mic_mask>>24)&0xFF };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif 
    //check ruler connection state 
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_CONFIGURED ) {      
        return RULER_STATE_ERR ;         
    }  
    
    //OSSemPend( UART_MUX_Sem_lock, 0, &err );  
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL(); 
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah 
        //OS_EXIT_CRITICAL();  
        UART1_Mixer( ruler_slot_id );
    }
    if( Global_Ruler_Type[ruler_slot_id] == RULER_TYPE_H03  ||
        Global_Ruler_Type[ruler_slot_id] == RULER_TYPE_C01  ||
        Global_Ruler_Type[ruler_slot_id] == RULER_TYPE_ECHO    ) {
        buf_size_send = 5; //H03 cmd data size = 1+4 for 16> mic
    } else {
        buf_size_send = 3; //Default cmd data size = 1+2 for <16 mic
    }
    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, buf_size_send, 0, NULL, 0 ) ; 
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );  
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO_T(("Update_Mic_Mask for Ruler[%d] timeout",ruler_slot_id));
        }
    } else {
        APP_TRACE_INFO_T(("Ruler[%d] pcSendDateToBuf failed: %d",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );    
    return err ;
}

/*
*********************************************************************************************************
*                                       Ruler_Active_Control()
*
* Description : Active/Deactive ruler(LED)when play and record start/stop.
* Argument(s) : active_state : 0 - deactive ruler (LED)
*                              1 - active ruler (LED).
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : Support Ruler(8Mic) and Handset(16Mic)
*********************************************************************************************************
*/
unsigned char Ruler_Active_Control( unsigned char active_state )
{
    unsigned char err = 0;
    unsigned char ruler_id;
    unsigned char buf[] = { RULER_CMD_ACTIVE_CTR, active_state };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    for( ruler_id = 0 ; ruler_id < 4 ; ruler_id++ ) {
        //check ruler connection state
        if( //RULER_TYPE_MASK(Global_Ruler_Type[ruler_id]) == RULER_TYPE_HANDSET ||
            Global_Ruler_State[ruler_id] < RULER_STATE_CONFIGURED ||
            Global_Mic_Mask[ruler_id] == 0 ) {
            continue;
        }
        APP_TRACE_INFO(("Ruler[%d]_Active_Control : [%d]\r\n",ruler_id,active_state));
        //OSSemPend( UART_MUX_Sem_lock, 0, &err );
        if( Global_Ruler_Index != ruler_id ) {
            Check_UART_Mixer_Ready();
            //OS_ENTER_CRITICAL();
            Global_Ruler_Index = ruler_id ; //for ruler status switch in TX/RX/Noah
            //OS_EXIT_CRITICAL();
            UART1_Mixer( ruler_id );
        }
        err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
        if( OS_ERR_NONE == err ) {
            OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
            if( OS_ERR_TIMEOUT == err ) {
                APP_TRACE_INFO(("Ruler[%d]_Active_Control timeout\r\n",ruler_id));
            } else {
                err = Global_Ruler_CMD_Result; //exe result from GACK
                if(OS_ERR_NONE != err ){
                    APP_TRACE_INFO(("Ruler[%d]_Active_Control err = %d\r\n",ruler_id,err));
                }
            }

        } else {
            APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_id,err));
        }
        //OSSemPost( UART_MUX_Sem_lock );
        if( err != NO_ERR ) {
            break;
        }
    }
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Get_Ruler_Version()
*
* Description : Get back specified ruler's version info.
*               And the version data is stored in global varies : Emb_Buf_Data
* Argument(s) : ruler_slot_id : 0~ 3.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Get_Ruler_Version( unsigned char ruler_slot_id )
{
    unsigned char err = 0 ;
    unsigned char buf[] = { RULER_CMD_GET_RULER_VERSION };
    EMB_BUF      *pEBuf_Data;
    
    pEBuf_Data  = &Emb_Buf_Data;  //Global var
#ifdef DEBUG_GUI_RULER_OFFLINE
    return 0;
#endif 
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif
    
    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {
        return RULER_STATE_ERR ;
    }

    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Get_Ruler_Version[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Global_Ruler_CMD_Result;
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Get_Ruler_Version[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }
        if(err == OS_ERR_NONE ) {
            APP_TRACE_INFO(("Ruler[%d] FW Version: %s\r\n",ruler_slot_id, pEBuf_Data->data));
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );
    
    return err ;
}


/*
*********************************************************************************************************
*                                       Reset_Mic_Mask()
*
* Description : Reset all mics to deactived state on the specified rulers and update FPGA mic signal switch array.
* Argument(s) : pInt : pointer to a int data, the 4 bytes of wihch control 4 ruler's all mic need be
*               reset to deactive state or not.
*                      1 - deactive all mics on this ruler
*                      0 - do nothing. ignore the reset operation
* Return(s)   : NO_ERR :   execute successfully
*               others :   = error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Reset_Mic_Mask(  unsigned int *pInt )
{
    unsigned char  err ;
    unsigned char  id;
    unsigned char  *pChar;
    unsigned int   fpga_mask;

    fpga_mask = 0;
    pChar     = (unsigned char *)pInt;
    err       = 0;

    
    for( id = 0; id < 4; id++ ) {
        if( *(pChar+id) == 0 ) {
            continue;
        }
        if( Global_Ruler_State[id] < RULER_STATE_CONFIGURED ) { //why not RULER_STATE_SELECTED  ? Because UI need reset mic in any case
            continue;
        }
        Global_Ruler_State[id] = RULER_STATE_CONFIGURED ;
        err = Update_Mic_Mask( id, 0 );
        if( OS_ERR_NONE != err ) {
            return err;
        }
        Global_Mic_Mask[id] = 0;
        if( RULER_TYPE_MASK( Global_Ruler_Type[id] ) == RULER_TYPE_RULER ) {//ruler
            fpga_mask += (Global_Mic_Mask[id]&0xFF) << (id<<3);

        } else {
            fpga_mask += 0x3F << (id<<3); //handset choose the lowest slot H01
        }
    }
    
    
    //Init_FPGA(fpga_mask);
    return err;
}

/*
*********************************************************************************************************
*                                       Toggle_Mic()
*
* Description : Toggle specified mic's active state by sending command to related ruler and updating
*               FPGA mic signal switch array.
*               One mic One time.
* Argument(s) : pdata : pointer to TOGGLE_MIC structure data
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Toggle_Mic(  TOGGLE_MIC *pdata )
{
    unsigned char  err ;
    unsigned char  id;
    unsigned int   mic_mask;
    unsigned int   fpga_mask;
#ifdef DEBUG_GUI_RULER_OFFLINE
    return 0;
#endif 
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;                                 /* Storage for CPU status register         */
#endif
    fpga_mask = 0;
    //check ruler connection state
    if( Global_Ruler_State[pdata->ruler_id] < RULER_STATE_CONFIGURED ) {
        return RULER_STATE_ERR ;
    }
    APP_TRACE_INFO(("Toggle Ruler[%d]-Mic[%d] : %d  : ", pdata->ruler_id, pdata->mic_id, pdata->on_off ));
    OS_ENTER_CRITICAL();
    mic_mask = Global_Mic_Mask[pdata->ruler_id];
    OS_EXIT_CRITICAL();
    mic_mask &= ~( 1<<(pdata->mic_id));
    mic_mask |=  (pdata->on_off&0x01)<<( pdata->mic_id);
    err = Update_Mic_Mask( pdata->ruler_id, mic_mask );
    APP_TRACE_INFO((" %s [0x%X]\r\n", err == OS_ERR_NONE ? "OK" : "FAIL" , err ));
    if( OS_ERR_NONE != err ) {
        return err;
    }
    OS_ENTER_CRITICAL();
    Global_Mic_Mask[pdata->ruler_id] = mic_mask;
    //APP_TRACE_INFO(("Update Ruler[%d] Mic_Mask:  %d\r\n",pdata->ruler_id,Global_Mic_Mask[pdata->ruler_id]));
    if( mic_mask == 0 ) {
        Global_Ruler_State[pdata->ruler_id] = RULER_STATE_CONFIGURED;
    } else {
        Global_Ruler_State[pdata->ruler_id] = RULER_STATE_SELECTED;
    }
    OS_EXIT_CRITICAL();
    if( RULER_TYPE_MASK( Global_Ruler_Type[pdata->ruler_id] ) == RULER_TYPE_RULER ) { //ruler
        for( id = 0; id < 4; id++ ) {
            fpga_mask += (Global_Mic_Mask[id]&0xFF) << (id<<3);
        }
    } else { //handset
       fpga_mask = 0x3F << ((pdata->ruler_id)<<3);
    }
    //Init_FPGA(fpga_mask);
    return err;

}











/*
*********************************************************************************************************
*                                       FLASHD_Write_Safe()
*
* Description : Add code area protection for FLASHD_Write()
*               Writes a data buffer in the internal flash. This function works in polling
*               mode, and thus only returns when the data has been effectively written.
* Argument(s) :  address  Write address.
*                pBuffer  Data buffer.
*                size     Size of data buffer in bytes.
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char FLASHD_Write_Safe( unsigned int address, const void *pBuffer,  unsigned int size)
{
    unsigned char err;
    /*
    if( size == 0 ) {
        return 0;
    }
    if( address < (AT91C_IFLASH + FLASH_HOST_FW_BIN_MAX_SIZE) ) {
        APP_TRACE_INFO(("ERROR: this operation wanna flush code area!\r\n"));
        return FW_BIN_SAVE_ADDR_ERR;
    }
    err = FLASHD_Write(  address, pBuffer, size );
    */
    return err;

}


/*
*********************************************************************************************************
*                                       Read_Flash_State()
*
* Description : Save ruler FW bin file to flash
*
* Argument(s) : *pFlash_Info : pointer to FLASH_INFO type data where to save read data
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void Read_Flash_State( FLASH_INFO  *pFlash_Info, unsigned int flash_address )
{

    *pFlash_Info = *(FLASH_INFO *)flash_address;

}


/*
*********************************************************************************************************
*                                       Write_Flash_State()
*
* Description : Save ruler FW bin file to flash
*
* Argument(s) : *pFlash_Info : pointer to FLASH_INFO type data need to be saved
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Write_Flash_State( FLASH_INFO   *pFlash_Info, unsigned int flash_address )
{

    unsigned char err;
    //save state to flash
    pFlash_Info->s_w_counter++ ;
    err = FLASHD_Write_Safe( flash_address, pFlash_Info, AT91C_IFLASH_PAGE_SIZE);
    if(err != NO_ERR ) {
        APP_TRACE_INFO(("ERROR: Write flash state failed!\r\n"));
    }

    return err;

}


/*
*********************************************************************************************************
*                                       Save_Ruler_FW()
*
* Description : Save ruler FW bin file to flash
*
* Argument(s) :  cmd  :  1~ 3.
*               *pBin : pointer to bin file data packge to be wriiten to flash
*               *pStr : pointer to file name string
*                size : bin package file size
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Save_Ruler_FW( unsigned int cmd, unsigned char *pBin, unsigned char *pStr, unsigned int size )
{
    unsigned char err;
    static unsigned int flash_addr = FLASH_ADDR_FW_BIN;
      /*
    FLASH_INFO    flash_info;

    err = NO_ERR;
    Read_Flash_State(&flash_info, FLASH_ADDR_FW_STATE);

    switch( cmd ) {
        case FW_DOWNLAD_CMD_START :
            APP_TRACE_INFO(("Start loading ruler bin file to AB01 flash ... \r\n"));
            flash_addr = FLASH_ADDR_FW_BIN;
            flash_info.f_w_state = FW_DOWNLAD_STATE_UNFINISHED ;
            flash_info.bin_size  = 0;
        break;
        case FW_DOWNLAD_CMD_DOING :
            APP_TRACE_INFO(("> "));
            if( flash_info.f_w_state != FW_DOWNLAD_STATE_UNFINISHED ) {
                APP_TRACE_INFO(("ERROR: flash state not match!\r\n"));
                err  =  FW_BIN_STATE_0_ERR;
            }
        break;
        case FW_DOWNLAD_CMD_DONE :
            APP_TRACE_INFO((">\r\n"));
            if( flash_info.f_w_state != FW_DOWNLAD_STATE_UNFINISHED ) {
                APP_TRACE_INFO(("ERROR: flash state not match!\r\n"));
                err  =  FW_BIN_STATE_1_ERR;
                break;
            }
            flash_info.f_w_state = FW_DOWNLAD_STATE_FINISHED ;
            flash_info.f_w_counter++;
         break;

         default:
            APP_TRACE_INFO(("ERROR:  Save ruler FW bad cmd!\r\n"));
            err = FW_BIN_SAVE_CMD_ERR;
         break;

    }
    if( err != NO_ERR ) {
        return err;
    }
    Buzzer_OnOff(1);
    LED_Toggle(LED_DS2);
    err = FLASHD_Write_Safe( flash_addr, pBin, size );
    Buzzer_OnOff(0);
    if(err != NO_ERR ) {
        APP_TRACE_INFO(("ERROR: Write MCU flash failed!\r\n"));
        return err;
    }
    flash_addr += size;
    flash_info.bin_size   = flash_addr - FLASH_ADDR_FW_BIN ;
    strcpy(flash_info.bin_name, (char const*)pStr);
    if( cmd != FW_DOWNLAD_CMD_DOING ) {
        err = Write_Flash_State( &flash_info, FLASH_ADDR_FW_STATE );
        if( err == NO_ERR && cmd == FW_DOWNLAD_CMD_DONE ) {
              APP_TRACE_INFO(("Bin file[%d Btyes] saved successfully!\r\n",flash_info.bin_size));
        }
    }
    */
    return err;

}



/*
*********************************************************************************************************
*                                       Update_Ruler_FW()
*
* Description :  Write firmware to specified ruler's MCU flash
*
* Argument(s) :  ruler_slot_id :  0~ 3.
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
*
* Note(s)     : Do not care if ruler is attached or not.Because host can not detect ruler if FW was crashed.
*********************************************************************************************************
*/
unsigned char Update_Ruler_FW( unsigned char ruler_slot_id )
{
    unsigned char err;
    unsigned int  flash_addr;
    FLASH_INFO   *pFlash_Info;
    unsigned char Buf[9];
    unsigned char i;
     /*
    err = NO_ERR;
    flash_addr  = FLASH_ADDR_FW_BIN;
    pFlash_Info = (FLASH_INFO *)FLASH_ADDR_FW_STATE ;

    if( pFlash_Info->f_w_state != FW_DOWNLAD_STATE_FINISHED ) {
        APP_TRACE_INFO(("ERROR: FW bin file missed!\r\n"));
        return FW_BIN_STATE_ERR;
    }

    APP_TRACE_INFO(("Start updating ruler[%d] firmware to \"%s\" version ...\r\n",ruler_slot_id,pFlash_Info->bin_name));
    memset(Buf,'d',sizeof(Buf)); //send 'd' to start download
    Ruler_Power_Switch(0);   //power off ruler
    OSTimeDly(200);
    for( i = 0; i < 4; i++ ) {
        Global_Ruler_State[i] = RULER_STATE_DETACHED ;
    }
    UART_Init(RULER_UART,  NULL,  115200 );   //Init Ruler to inquire mode
    Port_Detect_Enable(0); //disable ruler detect

    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    Ruler_Power_Switch(1);   //power on ruler
    OSTimeDly(200);
    err = USART_SendBuf( RULER_UART, Buf,  sizeof(Buf));
    if( OS_ERR_NONE == err ) {
        OSTimeDly(800);
        err = USART_Read_Timeout( RULER_UART, Buf, 3, 5000 );
        if( OS_ERR_NONE == err && ( Buf[0] == 'c' || Buf[0] == 'C' )) {
            Global_Ruler_State[ruler_slot_id] = RULER_STATE_RUN ;
            err = Xmodem_Transmit( (unsigned char *)flash_addr, pFlash_Info->bin_size );
            Global_Ruler_State[ruler_slot_id] = RULER_STATE_DETACHED ;
        }
    }
    if( OS_ERR_NONE != err ) {
        APP_TRACE_INFO(("\r\nFailed to init ruler bootloader. Err Code = [0x%X]\r\n", err));
    } else {
        APP_TRACE_INFO(("\r\nUpdate ruler[%d] firmware successfully!\r\n", ruler_slot_id));
    }
    Port_Detect_Enable(1); //enable ruler detect
    UART_Init(RULER_UART,  ISR_Ruler_UART,  115200 );  //Init Ruler back to interuption mode
    Ruler_Power_Switch(0);   //power off ruler
    OSTimeDly(500);
    Ruler_Power_Switch(1);   //power on ruler
    //OSSemPost( UART_MUX_Sem_lock );
    */
    return err ;

}




/*
*********************************************************************************************************
*                                       Ruler_Port_LED_Service()
*
* Description : Control the ruler port identify LED state:
*               turn on LED after ruler configured, blink LED during recording
* Argument(s) : None.
* Return(s)   : None.
* Note(s)     : None.
*********************************************************************************************************
*/
void Ruler_Port_LED_Service( void )
{
    static unsigned int counter;
    static unsigned int counter_buz;
    unsigned char ruler_id;
    unsigned char ruler_state;
    unsigned char LED_Freq;
    unsigned char post_err_flag;
    
    LED_Freq      = 0x3F;
    post_err_flag = 0;

    for( ruler_id = 0 ; ruler_id < 1 ; ruler_id++ ) {

        ruler_state = Global_Ruler_State[ruler_id];
        if( Global_Bridge_POST != NO_ERR ) { //if POST err, start all LED
            ruler_state = RULER_STATE_RUN ;
            post_err_flag = 1;
        }
        switch( ruler_state ) {

            case RULER_STATE_DETACHED :
            case RULER_STATE_ATTACHED :
                //LED_Clear( LED_P0 + ruler_id );
                UIF_LED_Off(LED_HDMI);
            break;
            case RULER_STATE_CONFIGURED :
            case RULER_STATE_SELECTED :
                //LED_Set( LED_P0 + ruler_id );
                UIF_LED_On(LED_HDMI);
            break;
            case RULER_STATE_RUN :
                if( (counter & LED_Freq) == 0 ) {
                    //LED_Toggle( LED_P0 + ruler_id );
                    UIF_LED_Toggle(LED_HDMI);
                    if( post_err_flag== 1 && ruler_id == 0 && (counter_buz++ < 6 ) ) {
                        //Buzzer_Toggle(); //buzzer off id POST err
                        Buzzer_OnOff( counter_buz&0x01 );   //fix long buz issue in some case
                    }
                }

            default:
            break;
        }

    }
    counter++;
    
}





/*
*********************************************************************************************************
*                                       Ruler_POST()
*
* Description : Get back specified ruler Power-On-Self-Test status.
*
* Argument(s) : ruler_id :  0~ 3
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Ruler_POST( unsigned char ruler_id )
{
    unsigned char  err = 0;
    unsigned short result;

    APP_TRACE_INFO(("\r\nRuler[%d] POST status check... \r\n",ruler_id));

    err = Read_Ruler_Status( ruler_id, &result);
    if( err == RULER_STATE_ERR ) { //no ruler attached
        return err;;
    }
    if( err != NO_ERR ) {
       return err;
    }
    if( result != 0 ) {
        if( result != 0x8000 ) {
            APP_TRACE_INFO(("\r\n---Error Ruler[%d]: %d-0x%X\r\n",ruler_id,err,result));
            return 1;
        } else {
            APP_TRACE_INFO(("\r\n---WARNING Ruler[%d]: Mic calibration data NOT Initialized!\r\n",ruler_id));
        }
    }
    APP_TRACE_INFO(("\r\n---OK\r\n"));
 
    return err;
}


/*
*********************************************************************************************************
*                                       simple_test_use()
*
* Description : debug use.
*
* Argument(s) : None.
* Return(s)   : None.
* Note(s)     : None.
*********************************************************************************************************
*/
void simple_test_use( void )
{
    APP_TRACE_INFO(("\r\nHi,man. Simple play/rec test triggered...\r\n"));

#if 0

 //R01
    TOGGLE_MIC toggle_mic[6] = {
                                    {0, 6, 1 }, {0, 7, 1 }, {0, 8, 1 },
                                    {0, 12, 1 }, {0, 13, 1 }, {0, 14, 1 }
                                };

    for (unsigned char i = 0; i< 6 ; i++ ) {
        Toggle_Mic(&toggle_mic[i]);
    }

#else

//H01
    Update_Mic_Mask( 0, 0x3f);
    Init_FPGA(0x3F);
    Global_Ruler_State[0] = RULER_STATE_RUN;

    AUDIO_CFG audio_config_play = {SAMPLE_RATE_DEFAULT, AUDIO_TYPE_PLAY, 6 };
    AUDIO_CFG audio_config_rec  = {SAMPLE_RATE_DEFAULT, AUDIO_TYPE_REC,  6 };
    Setup_Audio( &audio_config_play );
    Setup_Audio( &audio_config_rec );
    //Start_Audio( AUDIO_START_PALYREC );

#endif

}

////////////////////////////////////////////////////////////////////////////////

//Not used anymore!
unsigned char Ruler_Setup_Sync( unsigned char ruler_slot_id )
{
    unsigned char err ;
    unsigned char buf[] = { RULER_CMD_SETUP_SYNC, Ruler_Setup_Sync_Data, ruler_slot_id };
    
#if OS_CRITICAL_METHOD == 3u
    //OS_CPU_SR  cpu_sr = 0u;
#endif

    //check ruler connection state
    if( Global_Ruler_State[ruler_slot_id] < RULER_STATE_ATTACHED ) {
        return RULER_STATE_ERR ;
    }

    //OSSemPend( UART_MUX_Sem_lock, 0, &err );
    if( Global_Ruler_Index != ruler_slot_id ) {
        Check_UART_Mixer_Ready();
        //OS_ENTER_CRITICAL();
        Global_Ruler_Index = ruler_slot_id ; //for ruler status switch in TX/RX/Noah
        //OS_EXIT_CRITICAL();
        UART1_Mixer( ruler_slot_id );
    }

    err = Noah_CMD_Pack_Ruler( EVENT_MsgQ_Noah2RulerUART, FRAM_TYPE_DATA, buf, sizeof(buf), 0, NULL, 0 ) ;
    if( OS_ERR_NONE == err ) {
        OSSemPend( Done_Sem_RulerUART, TIMEOUT_RULER_COM, &err );
        if( OS_ERR_TIMEOUT == err ) {
            APP_TRACE_INFO(("Ruler_Setup_Sync[%d] timeout\r\n",ruler_slot_id));
        } else {
            err = Global_Ruler_CMD_Result; //exe result from GACK
            if(OS_ERR_NONE != err ){
                APP_TRACE_INFO(("Ruler_Setup_Sync[%d] err = %d\r\n",ruler_slot_id,err));
            }
        }

    } else {
        APP_TRACE_INFO(("Ruler[%d] Noah_CMD_Pack_Ruler failed: %d\r\n",ruler_slot_id,err));
    }
    //OSSemPost( UART_MUX_Sem_lock );

    return err ;

}



