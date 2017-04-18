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
*                                        FM1388 Communication Related
*
*                                          Atmel AT91SAM3U4C
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : fm1388_comm.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <stdbool.h>
#include <intrinsics.h>

#include "USBD.h"
#include "CDCDSerialDriver.h"

#include "kfifo.h"
#include "defined.h"
#include "uif_usb.h"
#include "timetick.h"

#include "fm1388_comm.h"
#include "fm1388_spi.h"

VOICE_BUF  voice_buf_data;
#define spi_ch_num 10
#define max_timeout   96000

unsigned char SPI_FIFO_Buffer[ SPI_FIFO_SIZE ] ;
unsigned char SPI_PLAY_FIFO_Buffer[ SPI_FIFO_SIZE ];
unsigned char SPI_PLAY_FIFO_Buffer2[ SPI_FIFO_SIZE ];

unsigned char SPI_Data_Buffer[ SPI_BUF_SIZE +1 ];  //+1 fix spi bug
unsigned char SPI_Data_Buffer2[ SPI_BUF_SIZE +1 ]; 
unsigned char im501_irq_counter;
unsigned int  global_rec_spi_en = 0 ;
unsigned int  global_play_spi_en = 0 ;
unsigned int  global_rec_spi_mask = 0 ;
unsigned int  global_play_spi_mask = 0 ;
unsigned int  FM1388_Rec_Data_Addr[5];
unsigned char SPI_DATA[SPI_FIFO_SIZE];


unsigned char ch_num = 0;
unsigned char ch_index[spi_ch_num];
unsigned char play_ch_num = 10;              //set a temp value
unsigned char play_ch_index[spi_ch_num];
unsigned char dead_beef_flag = 0;
unsigned char timedelay_flag = 0;
unsigned char SPI_Rec_flag = 0;
unsigned int timeout = 0;

volatile bool spi_bulkin_enable = true;
volatile bool spi_bulkin_start = true;

extern uint8_t usbCacheBulkOut2[USB_DATAEP_SIZE_64B * 16 * 3 ] ;//==>usbBufferBulkOut
extern uint8_t usbCacheBulkIn2[USB_DATAEP_SIZE_64B * 16 * 3 ] ; //==>usbBufferBulkIN     

extern kfifo_t  spi0_bulkOut_fifo;
extern kfifo_t  spi0_bulkIn_fifo;           //==>spi_rec_fifo
extern kfifo_t  ep2BulkOut_fifo;            //==>bulkout_fifo
extern kfifo_t  ep2BulkIn_fifo;             //==>bulkin_fifo 

extern Fm1388 fm1388;
/*
*********************************************************************************************************
*                                           Init_SPI_FIFO()
*
* Description :  Init kfifo for SPI recording.
*
* Argument(s) :  None.
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/

void Init_SPI_FIFO( void )
{   
    kfifo_t *pfifo;
    
    pfifo = &spi0_bulkIn_fifo;;
    kfifo_init_static(pfifo, SPI_FIFO_Buffer, SPI_FIFO_SIZE);
    
}

/*
void Init_SPI_PLAY_FIFO(void)
{
    kfifo_t *pfifo;
    pfifo = &spi_play_fifo;;
    kfifo_init_static(pfifo, SPI_PLAY_FIFO_Buffer, SPI_FIFO_SIZE);
}
*/
/*
*********************************************************************************************************
*                                           fm1388_single_write_dram_spi()
*
* Description :  FM1388 write DRAM via SPI, just limited to 2 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 4 bytes length and need 2 bytes alignment
*
*                *pdata         is point to where data to be written is stored 
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
unsigned char fm1388_single_write_dram_spi( unsigned int mem_addr, unsigned char *pdata )
{
    unsigned char err, state;
    unsigned char buf[8];
    unsigned char *pbuf;
    
    err   =  NO_ERR;
    pbuf  = (unsigned char *)SPI_Data_Buffer; //global usage
      
    buf[0] =  FM1388_SPI_S_16BIT_WR; //0x01
    buf[4] =  mem_addr & 0xFF;
    buf[3] =  (mem_addr>>8) & 0xFF;
    buf[2] =  (mem_addr>>16) & 0xFF;
    buf[1] =  (mem_addr>>24) & 0xFF;
    buf[6] =  *pdata++;
    buf[5] =  *pdata;    
    buf[7] =  0; //dummy data byte
  
    state =  SPI_WriteBuffer_API( &fm1388 ,buf , sizeof(buf) );

    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        //APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d",state));
    }   
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           fm1388_single_read_dram_spi()
*
* Description :  FM1388 read DRAM via SPI, just limited to 2 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 4 bytes length and need 2 bytes alignment
*
*                *pdata         is point to where read back data will be stored 
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
unsigned char fm1388_single_read_dram_spi( unsigned int mem_addr, unsigned char *pdata )
{
    unsigned char err, state;
    unsigned char buf[9];
    unsigned char *pbuf;
    
    err   =  NO_ERR;
    pbuf  = (unsigned char *)SPI_Data_Buffer; //global usage
    
//    memset( ( void * )&SPI_Data_Buffer, 0xff , sizeof( SPI_Data_Buffer ) );
    
    buf[0] =  FM1388_SPI_S_16BIT_RD; //0x00
    buf[4] =  mem_addr & 0xFF;
    buf[3] =  (mem_addr>>8) & 0xFF;
    buf[2] =  (mem_addr>>16) & 0xFF;
    buf[1] =  (mem_addr>>24) & 0xFF;
    buf[5] =  0;
    buf[6] =  0;    
    buf[7] =  0; 
    buf[8] =  0; 

    state =  SPI_WriteReadBuffer_API(  &fm1388,
                                       pbuf, 
                                       buf, 
                                       2 , 
                                       sizeof(buf) );

    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        //APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d",state));
        return err;
    } 
    
#if 0        
    pbuf = pbuf + 1; //fix bug  --comment by leo for ab04
#endif
    
    *pdata++ = *(pbuf+1); //LSB
    *pdata++ = *(pbuf);   //MSB
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           fm1388_single_read_dram_spi_32bit()
*
* Description :  FM1388 read DRAM via SPI, just limited to 4 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 4 bytes length and need 4 bytes alignment
*
*                *pdata         is point to where read back data will be stored 
*
* Return(s)   :  error number.           
*
* Note(s)     :  None.
*********************************************************************************************************
*/
unsigned char fm1388_single_read_dram_spi_32bit( unsigned int mem_addr, unsigned char *pdata )
{
    unsigned char err, state;
    unsigned char buf[9];
    unsigned char *pbuf;
    
    err   =  NO_ERR;
    pbuf  = (unsigned char *)SPI_Data_Buffer; //global usage
  
    
    buf[0] =  FM1388_SPI_S_32BIT_RD; //0x00
    buf[4] =  mem_addr & 0xFF;
    buf[3] =  (mem_addr>>8) & 0xFF;
    buf[2] =  (mem_addr>>16) & 0xFF;
    buf[1] =  (mem_addr>>24) & 0xFF;
    buf[5] =  0;
    buf[6] =  0;    
    buf[7] =  0; 
    buf[8] =  0; 

    state =  SPI_WriteReadBuffer_API(  &fm1388,
                                       pbuf, 
                                       buf, 
                                       4 , 
                                       sizeof(buf) );

    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d in func:%s at line %d\r\r",state,__FUNCTION__,__LINE__));
        assert( 0 );
        return err;
    }   
#if 0        
    pbuf = pbuf + 1; //fix bug--comment by leo for ab04
#endif    
    *pdata++ = *(pbuf+3); //LSB
    *pdata++ = *(pbuf+2);  
    *pdata++ = *(pbuf+1);
    *pdata++ = *(pbuf);   //MSB
    
    return err;
    
}


/*
*********************************************************************************************************
*                                           fm1388_burst_read_dram_spi()
*
* Description :  burst read iM501 DRAM via SPI, just limited to 4095 bytes.
*
* Argument(s) :  mem_addr       is dram adress, total 3 bytes length and need 4bytes alignment
*
*                **pdata        is point to where the pointer pointing to read back data will be stored 
*
*                data_len       is data length to read in bytes, maxium 4095 bytes  
*
* Return(s)   :  error number.           
*
* Note(s)     :  Non-Reentrant function.   Reg_RW_Data is used.
*                Be care full this function use fixed buffer, and return a **pointer to.
*********************************************************************************************************
*/
unsigned char fm1388_burst_read_dram_spi( unsigned int mem_addr, unsigned char **pdata, unsigned int data_len )
{
    unsigned char  err, state;
    unsigned char  buf[9];
    unsigned char *pbuf;
    
    err   =  NO_ERR;
    pbuf = (unsigned char *)SPI_Data_Buffer; //global usage    
    
    buf[0] =  FM1388_SPI_B_RD; //0x04
    buf[4] =  mem_addr & 0xFF;
    buf[3] =  (mem_addr>>8) & 0xFF;
    buf[2] =  (mem_addr>>16) & 0xFF;
    buf[1] =  (mem_addr>>24) & 0xFF;
    buf[5] =  0;
    buf[6] =  0;    
    buf[7] =  0; 
    buf[8] =  0;    

    state =  SPI_WriteReadBuffer_API(  &fm1388,
                                       pbuf, 
                                       buf, 
                                       data_len , 
                                       sizeof(buf) );
            
    if (state != SUCCESS) {
        err = SPI_BUS_ERR;
        APP_TRACE_INFO(("\r\nSPI_ReadBuffer_API err = %d in function:%s at line:%d",state,
                                                                                    __FUNCTION__,
                                                                                    __LINE__));
        assert( 0 );
        return err;
    } 

#if 0    
    *pdata =  pbuf + 1; //not sure this is an old bug or not,comment by leo for ab04
#else
    *pdata =  pbuf ;    //and instead this line  
#endif    
       
    return err;
    
}


/*
*********************************************************************************************************
*                                           data_revert_burst_mode()
*
* Description :  revert endian-mode from[7..0] to[0..7] of the burst read data .
*
* Argument(s) :  *pDest          is point to where the reverted data will be stored
*
*                *pSour          is point to where the source from  
*
*                data_length     is data length to revert in bytes, must be X 8   
*
* Return(s)   :  error number.           
*
* Note(s)     :  Non-Reentrant function.   Reg_RW_Data is used.
*                Be care full this function use fixed buffer, and return a **pointer to.
*********************************************************************************************************
*/
void data_revert_burst_mode( unsigned char *pDest, unsigned char *pSour,unsigned int data_length )
{
    unsigned int i,j;    
    unsigned int k;

    for( i=0; i<(data_length>>3); i++ ) {
        for( j=0; j<8; j++ ) {
            *(pDest+j) = *(pSour+7-j);
        }
        pDest += 8;
        pSour += 8;
   
    }
    
   
}

/*
*********************************************************************************************************
*                                           fetch_voice_data()
*
* Description :  read voice data from iM501 DRAM via SPI 
*
* Argument(s) :  start_addr     is DRAM data address
*
*                data_length    is data size(bytes) to read
*
* Return(s)   :  error number.           
*
* Note(s)     :  
*********************************************************************************************************
*/

unsigned char init_ch_num_and_ch_index(void)
{
    unsigned char i;
    ch_num=0;
    for(i=0;i<spi_ch_num;i++){
        ch_index[i]=0;
    }   
}


unsigned char get_ch_num_and_ch_index(unsigned short mask)
{
    unsigned char i,j=0;
    unsigned short temp;   
    for(i=0;i<spi_ch_num;i++){
        temp=mask>>i;
        if(temp & 0x0001 == 1){
            ch_num++;
            ch_index[j]=i;
            j++;
        }
    }
}

unsigned char init_play_ch_num_and_play_ch_index(void)
{
    unsigned char i;
    play_ch_num=0;
    for(i=0;i<spi_ch_num;i++){
        play_ch_index[i]=0;
    }
    
}

unsigned char get_play_ch_num_and_play_ch_index(unsigned short mask)
{
    unsigned char i,j=0;
    unsigned short temp;
    for(i=0;i<spi_ch_num;i++){
        temp=mask>>i;
        if(temp & 0x0001 == 1){
            play_ch_num++;
            play_ch_index[j]=i;
            j++;
        }
    }
}

unsigned char fetch_voice_data( void )
{
    unsigned char   err;
    unsigned int    i,j,k;
    unsigned char  *pbuf, *pdata;
    unsigned short *pShortDest, *pShortSource;
    unsigned int    data_length;
    unsigned int    start_addr;
    unsigned int    sr_num ;
//  unsigned int    ch_num ; 
    unsigned int    free_size;
    unsigned int    data_size;
    unsigned int    size;

    unsigned char N,M;
    unsigned short data;
    static bool toggle_flag=0;
    unsigned char *pbuf1;
    unsigned char *pChar;
    unsigned char flag=0;
    data_length = FM1388_Rec_Data_Addr[4] ;
    pbuf        = (unsigned char *)&SPI_Data_Buffer2;
    pbuf1       = (unsigned char *)&SPI_Data_Buffer2;
    
  //printf("--  datalen=: %d\r\n",data_length);
 //   get_ch_num_and_ch_index(0x00ff);    
    for(k=0;k<ch_num;k++){
          if(toggle_flag==0){
              start_addr  = FM1388_Rec_Data_Addr[3]+ ch_index[k]*data_length;
         //   start_addr  = FM1388_Rec_Data_Addr[0]+ ch_index[k]*data_length;         
          }
          else{
              start_addr  = FM1388_Rec_Data_Addr[2]+ ch_index[k]*data_length;
         //   start_addr  = FM1388_Rec_Data_Addr[1]+ ch_index[k]*data_length;           
          }
          N=data_length/256;
          M=data_length%256;    
          for(unsigned char i=0;i<N;i++){
              err = fm1388_burst_read_dram_spi( start_addr,  &pdata,  256); //fetch voice data
              if( err != NO_ERR ){ 
                return err;
              }

              if(k ==9){                           //channel 10  for add "deadbeef" 4 bytes offset
                  data_revert_burst_mode( pbuf+4, pdata, 256);
                  pbuf=pbuf+256+4;
                  pdata=pdata+256;
                  start_addr=start_addr+256;
              }
              else{
                  data_revert_burst_mode( pbuf, pdata, 256);
                  pbuf=pbuf+256;
                  pdata=pdata+256;
                  start_addr=start_addr+256;
              }  

          }    

          
          if(M!=0 && N !=0){
              err = fm1388_burst_read_dram_spi( start_addr,  &pdata,  M); //fetch voice data
              if( err != NO_ERR ){ 
                return err;
              }              
              data_revert_burst_mode( pbuf, pdata, M);       
              pbuf=pbuf+M;
              pdata=pdata+M;
          }  
          if(M!=0 && N==0){//sampe rate 8K
              err = fm1388_burst_read_dram_spi( start_addr,  &pdata,  M); //fetch voice data
              if( err != NO_ERR ){ 
                return err;
              }  
              if(k ==9){                           //channel 10  for add "deadbeef" 4 bytes offset
                  data_revert_burst_mode( pbuf+4, pdata, M);
                  pbuf=pbuf+M+4;
                  pdata=pdata+M;
                  start_addr=start_addr+M;
              }
              else{
                  data_revert_burst_mode( pbuf, pdata, M);
                  pbuf=pbuf+M;
                  pdata=pdata+M;
                  start_addr=start_addr+M;
              }  
          }

      }
 
//add "deadbeef" in debug info     
#if 1  
    if(ch_index[ch_num-1]==9){
        SPI_Data_Buffer2[data_length*(ch_num-1)]=0xde;
        SPI_Data_Buffer2[data_length*(ch_num-1)+1]=0xad;
        SPI_Data_Buffer2[data_length*(ch_num-1)+2]=0xbe;
        SPI_Data_Buffer2[data_length*(ch_num-1)+3]=0xef;
    }
#endif  
    
    pShortDest    = (unsigned short *)&SPI_Data_Buffer;
    pShortSource  = (unsigned short *)&SPI_Data_Buffer2;
    sr_num        = data_length>>1 ;//bytes to word
  //ch_num        = 10; 
    for(i = 0 ; i < sr_num ; i++ ) {      
        for( j = 0; j <ch_num ; j++ ){
           *pShortDest++ = *(pShortSource + i + j*(data_length>>1) );     
        }        
    } 

    toggle_flag =! toggle_flag;
    
    while(1)
    {

        i= kfifo_get_free_space( &ep2BulkIn_fifo );
        if( i > data_length*ch_num ) 
        {
            break;
        }
   
        timeout++;
        if( timeout == max_timeout ){//fix bug : noah script spi record closed unusual, fw break the while cycle
            timeout=0;
            break;
        }      
    }
    
    timeout=0;
    kfifo_put(&spi0_bulkIn_fifo, (unsigned char *)&SPI_Data_Buffer, data_length*ch_num); 
    free_size=kfifo_get_free_space( &ep2BulkIn_fifo);
    data_size=kfifo_get_data_size( &spi0_bulkIn_fifo);   
    size = free_size >= data_size ? data_size : free_size ;
   

    kfifo_get(&spi0_bulkIn_fifo, (unsigned char *)&SPI_Data_Buffer2, size);
    kfifo_put(&ep2BulkIn_fifo,  (unsigned char *)&SPI_Data_Buffer2, size); 
    if ( spi_bulkin_enable && spi_bulkin_start &&  ( ( USB_DATAEP_SIZE_64B << 0 ) <= kfifo_get_data_size(&ep2BulkIn_fifo)) ) {     
          spi_bulkin_start = false ; 
          kfifo_get(&ep2BulkIn_fifo, usbCacheBulkIn2,USB_DATAEP_SIZE_64B ); 
          CDCDSerialDriver_WriteSPI(   usbCacheBulkIn2,
                                    USB_DATAEP_SIZE_64B,
                                    (TransferCallback) UsbSPIDataTransmit,
                                    0);    
    }    
    return err;      
}

/*
**************************************************************************************
                                      spi_play_enable()
* Description : 
*
* Argument(s) :  None.
*
* Return(s)   :  error number.           
*
* Note(s)     :  
**************************************************************************************
*/
unsigned char spi_play_enable(void)
{
    unsigned char err;
    unsigned short data=0;
    unsigned short state;

    fm1388_single_read_dram_spi( SYNC_WORD_ADDR, (unsigned char*)&state );
    data = state | P_EN_1 | P_RDY_0  ;
    err = fm1388_single_write_dram_spi( SYNC_WORD_ADDR, (unsigned char *)&data );
    if( err != 0 ){ 
        return err;
    }
    return err;
}

/*
**************************************************************************************
                                      spi_rec_enable()
* Description : 
*
* Argument(s) :  None.
*
* Return(s)   :  error number.           
*
* Note(s)     :  
**************************************************************************************
*/
unsigned char spi_rec_enable(void)
{
    unsigned char err;
    unsigned short data=0;
    unsigned short state;

    fm1388_single_read_dram_spi( SYNC_WORD_ADDR, (unsigned char*)&state );

     data = state | R_EN_1 & 0xfffd;
    err = fm1388_single_write_dram_spi( SYNC_WORD_ADDR, (unsigned char *)&data );
    if( err != 0 )
    { 
        return err;
    }
    return err;
}


/*
*********************************************************************************************************
*                                           spi_rec_start_cmd()
*
* Description :  send CMD to FM1388 for starting SPI record 
*
* Argument(s) :  None.
*
* Return(s)   :  error number.           
*
* Note(s)     :  
*********************************************************************************************************
*/
unsigned char spi_rec_start_cmd( void )
{
    unsigned char err;
    unsigned short data;
    
    data = DSP_INIT_CMD ;
    err = fm1388_single_write_dram_spi( DSP_CMD_ADDR, (unsigned char *)&data );
    if( err != 0 ){ 
        return err;
    }
    return err;
    
}


/*
*********************************************************************************************************
*                                           spi_rec_stop_cmd()
*
* Description :  send CMD to FM1388 for stopping SPI record 
*
* Argument(s) :  None.
*
* Return(s)   :  error number.           
*
* Note(s)     :  
*********************************************************************************************************
*/
unsigned char spi_rec_stop_cmd( void )
{
    unsigned char err;
    unsigned short data;
    
    data = DSP_STOP_CMD ;    
    err = fm1388_single_write_dram_spi( DSP_CMD_ADDR, (unsigned char *)&data );
    if( err != 0 ){ 
        return err;
    }
    return err;
    
}


/*
*********************************************************************************************************
*                                    spi_rec_get_addr()
*
* Description :  get voive buffer address from predefined pointer  
*                use global varies 
*
* Argument(s) :  None
* 
* Return(s)   :  Error number.
*
* Note(s)     :  None.
*********************************************************************************************************
*/
unsigned char spi_rec_get_addr( void )
{
    
    unsigned char err;
    unsigned int  i;
    
    unsigned int  addr[] = {
        DSP_BUFFER_ADDR0,
        DSP_BUFFER_ADDR1,
        DSP_BUFFER_ADDR2,
        DSP_BUFFER_ADDR3,
        DSP_SPI_FRAMESIZE_ADDR 
    };
          
    for( i = 0; i<5; i++ ) 
    {      
      err = fm1388_single_read_dram_spi_32bit( addr[i],(unsigned char*)&FM1388_Rec_Data_Addr[i]);

      if( err != 0 ){
          
          return err;
      }
    }
    FM1388_Rec_Data_Addr[4] &= 0x0000FFFF;

    
    return err;
    
}


/*
*********************************************************************************************************
*                                           spi_rec_check_ready()
*
* Description :  send CMD to FM1388 for stopping SPI record 
*
* Argument(s) :  None.
*
* Return(s)   : 0 - data ready
*               100 - not ready
*               others - error humber
*
* Note(s)     :  
*********************************************************************************************************
*/
unsigned char spi_rec_check_ready( void )
{
    unsigned char   err;
    unsigned short  state;
    
    err = fm1388_single_read_dram_spi( SYNC_WORD_ADDR, (unsigned char*)&state );
    if( err != 0 ){ 
        return err;
    } 
    if( R_RDY_1 != (state & R_RDY_1) ) {
        err = 100;
    }
    if( R_ERR_1 == (state & R_ERR_1)) {
    //    err = 101;
    }  
//    assert( err == 0 );
    return err;   
}

/*
***********************************************************
                  SPI_Rec_Over




***********************************************************
*/
unsigned char SPI_Rec_Over( void )
{
    unsigned char err;
    unsigned short data=0;
    unsigned short state=0;
    //data =  R_EN_1 | R_RDY_0 | KEY_PHRASE ;
    fm1388_single_read_dram_spi( SYNC_WORD_ADDR, (unsigned char*)&state );
    data =( state | R_EN_1 )& 0xFFFD; 
//    data = R_EN_1 | R_RDY_0;
    err = fm1388_single_write_dram_spi( SYNC_WORD_ADDR, (unsigned char *)&data );
    if( err != 0 ){ 
        return err;
    }
    return err;
}


/*
*********************************************************************************************************
*                                    SPI_Rec_Service()
*
* Description :  Service to iM501 IRQ interruption 
*                Should be in Main Loop, inquiring the data ready flag
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
void SPI_Rec_Service( void )
{  
    unsigned char err;
    unsigned short state;
    if( global_rec_spi_en == 0 ) {
        return;
    }
    
    if( spi_rec_check_ready() == 0 ) { 
        
        fetch_voice_data();  
         SPI_Rec_Over(); 
         SPI_Rec_flag=0;
    }    
}


/*
********************************************************************
                            spi_play_check_ready()
********************************************************************
*/
unsigned char spi_play_check_ready( void )
{
    unsigned char   err;
    unsigned short  state=0;
    
    err = fm1388_single_read_dram_spi( SYNC_WORD_ADDR, (unsigned char*)&state );
    if( err != 0 ){ 
        return err;
    }
    if(P_RDY_1 == (state &P_RDY_1) ) {
        err = 100;   
    }
    if( P_ERR_1 == (state & P_ERR_1)) {
    //    err = 101;
    }
    if( 0x0002 ==(state & 0x0002) )
    {
  //      err=102;
    }

    printf("\r\nCheckReady  : %04X",  state);    
    return err;   
}

/*
************************************************************************
            fm1388_spi_burst_write(start_addr);

************************************************************************
*/
void fm1388_spi_burst_write(unsigned int addr,unsigned char *pChar ,unsigned char num)
{
    //unsigned char temp[5];
    unsigned char data_array[246]={0};
    unsigned char i;
    data_array[0]=0x05;
    data_array[1]=(addr & 0xFF000000)/0x1000000;
    data_array[2]=(addr & 0xFF0000)/0x10000;
    data_array[3]=(addr & 0xFF00)/0x100;
    data_array[4]=(addr & 0xFF)/0x1;
    //SPI_PLAY_FIFO_Buffer2
    for(i=5;i<num+5;i++){
        data_array[i]=*pChar++;
    }
    data_array[num+5]=0;
    SPI_WriteBuffer_API( &fm1388,( unsigned char * )&data_array, num + 6 ); 
}

/*
************************************************************************
data_convert()
************************************************************************
*/
void data_convert(unsigned char *pChar,unsigned int length)
{
    unsigned int i,j;
    unsigned char temp;
    for(i=0;i<length/8;i++){
        for(j=0;j<4;j++){
            temp=*(pChar+j);
            *(pChar+j)=*(pChar+7-j);
            *(pChar+7-j)=temp;
        }
        pChar+=8;
    }
}

/*************************************************************************

*************************************************************************/
unsigned char spi_play_set_P_RDY(void)
{
    unsigned char err;
    unsigned short data=0;
    unsigned short state;
    //data =  R_EN_1 | R_RDY_0 | KEY_PHRASE ;
    //data =  P_EN_1 | P_RDY_1;
    fm1388_single_read_dram_spi( SYNC_WORD_ADDR, (unsigned char*)&state );
   // data = state | P_EN_1 | P_RDY_1  ;
     data = state | P_EN_1 | P_RDY_1 | R_EN_1;
    err = fm1388_single_write_dram_spi( SYNC_WORD_ADDR, (unsigned char *)&data );
    if( err != 0 ){ 
        return err;
    }
    return err;
}



/************************************************************************
                              transfer_voice_data();  
************************************************************************/
void transfer_voice_data(void)
{
    unsigned int   data_length;
    unsigned short *pShortDest, *pShortSource;
    unsigned char *pSend;
    unsigned int    sr_num ;
    unsigned int i,j,k;
    static bool toggle_flag=0;
    unsigned int    start_addr;
    unsigned char M,N;
    unsigned char *pChar;
    unsigned char flag=0;
    unsigned int timeout=0;
    static unsigned char fir_time_flag=0;
    
    uint32_t tick;
    uint32_t delay;
    const uint32_t timeOutTick = 10; //delay time unit is 1ms,so 1388 is 10
    
    data_length = FM1388_Rec_Data_Addr[4] ;

    
    if( kfifo_get_data_size( &ep2BulkOut_fifo ) >= data_length*play_ch_num )
    {
        assert( 1 );
        kfifo_get( &ep2BulkOut_fifo, ( unsigned char * )&SPI_Data_Buffer, data_length*play_ch_num );
        pShortSource = (unsigned short *)&SPI_Data_Buffer;
        pShortDest = (unsigned short *)&SPI_Data_Buffer2;     
         
#if 1 
        sr_num  = ( data_length >> 1 );                          //bytes to word
        for(i = 0 ; i < play_ch_num ; i++ ) 
        {      
            for( j = 0; j < sr_num ; j++ )
            {
                *pShortDest++ = *( pShortSource + i + j * ( play_ch_num ) );                
            }        
        }             

        data_convert((unsigned char *)&SPI_Data_Buffer2,data_length*play_ch_num);
        
        pSend=(unsigned char *)&SPI_Data_Buffer2;
        
        for( k = 0;k < play_ch_num ; k++ )
        {
            if(toggle_flag==0)
            {
                start_addr  = FM1388_Rec_Data_Addr[0]+ play_ch_index[k]*data_length;
            }
            else
            {
                start_addr  = FM1388_Rec_Data_Addr[1]+ play_ch_index[k]*data_length;
            }
            
            N=data_length/240;
            M=data_length%240; 
            
            for( i = 0;i < N; i++ )
            {
                fm1388_spi_burst_write(start_addr,(unsigned char*)pSend,240);
                start_addr+=240;
                pSend += 240;
            }            
            if(M != 0)
            {
                fm1388_spi_burst_write( start_addr,(unsigned char*)pSend,M );
                pSend += M;
            }           
        }       
#endif
        
        tick = GetTickCount();

        while( ( timedelay_flag != 1 ) && ( fir_time_flag != 0 ) )
        {
              timeout++;
              if( timeout == max_timeout )
              {
                  //fix bug : noah script spi record closed unusual, fw break the while cycle
                  timeout=0;
                  break;
              } 
              delay = GetDelayInTicks(tick, GetTickCount());
              if ( delay > timeOutTick )
              {
                timedelay_flag = 1;
                assert( 0 );
              }
        }

        fir_time_flag = 1;
        spi_play_set_P_RDY();         
       
        timedelay_flag = 0;
        toggle_flag =! toggle_flag;
        
        CDCDSerialDriver_ReadSPI( usbCacheBulkOut2,
                                  USB_DATAEP_SIZE_64B,
                                  (TransferCallback) UsbSPIDataReceived,
                                  0);     
    } 
    else
    {
              CDCDSerialDriver_ReadSPI( usbCacheBulkOut2,
                                  USB_DATAEP_SIZE_64B,
                                  (TransferCallback) UsbSPIDataReceived,
                                  0);
      
    }
}

/*
*********************************************************************************************************
*                                    SPI_Play_Service()
*
* Description :  
*                
*
* Argument(s) :  None.
*
* Return(s)   :  None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
                          
void SPI_Play_Service( void )
{  
//    unsigned char err;
//    unsigned short state;
    
    if( global_play_spi_en == 0 ) 
    {
        return;
    }

    if( spi_play_check_ready() == 0 ) 
    {     
        if(SPI_Rec_flag==0)
        {
           transfer_voice_data();
           SPI_Rec_flag=1;
        }
    }    
}




