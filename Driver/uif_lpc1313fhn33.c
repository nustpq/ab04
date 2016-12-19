#include "uif_lpc1313fhn33.h"
#include "defined.h"
#include "uif_spi.h"

enum _lpc_state
{
  IDLE = 0,
  SEND,
  WAIT_ACK,
  ACKED,
  WAIT_DATA,
  DATA_READY,
  DATA_ERROR,
  TIMEOUT
}LPC_STATE;

extern DataSource source_usart1;
LPC1313 lpc_chip;

/*
*********************************************************************************************************
*                                               calculate_crc()
*
* Description : calculate crc
*
* Arguments   : data       : data buffer point
*               size       : data size
* Returns     : crc
*
* Note(s)     : none
*********************************************************************************************************
*/
static uint16_t calculate_crc( void * data , uint32_t size )
{
  uint16_t ret = 0 ;
  
  char *temp =  ( char * )data;
  
  for( uint16_t i = 0; i < size; i++ )
  {
    ret += *(temp ++);
    
  }
  
  return ret;
}

/*
*********************************************************************************************************
*                                               encode_frame()
*
* Description : encode i2c data
*
* Arguments   : buffer     : databuff
*               frameInfo  : i2c data header point
*               data       : i2c data
*               size       : data size
* Returns     : frame len
*
* Note(s)     : none
*********************************************************************************************************
*/
static uint16_t encode_frame( void *pBuffer ,
                             void *frameInfo , 
                             void ** data, 
                             uint16_t size )
{
  uint16_t len = size + 6;
  uint8_t len_l,len_h;
  
  char *pos = NULL;
  
  len_l = ( len & 0xff );
  len_h = ( len >> 8 );
  
  FRAME *frame = ( FRAME * )frameInfo;
  
  frame->lengthH = len_h;
  frame->lengthL = len_l;
  
  if( NULL == ( pBuffer = ( char * )malloc( len ) ) )
    return NULL;
  
  pos = pBuffer;
  memcpy( pos , ( void * )frame , 6 );
  memcpy( pos + 6 , data , size );
  
  
  return len;
  
}



/*
*********************************************************************************************************
*                                               encode_data()
*
* Description : destroy lpc1313 chip instance
*
* Arguments   : chip     : instance of chip lpc1313
*                                         
* Returns     : encode len
*
* Note(s)     : none
*********************************************************************************************************
*/
static uint16_t encode_data( char *pBuffer,
                            char *data,
                            uint16_t size )
{
  uint16_t len;
  uint8_t len_l,len_h;  
//  uint16_t crc;
//  uint8_t crc_l, crc_h;

  
  char *pos = NULL;
  
  len = size ;
  len_l = ( len & 0xff );
  len_h = ( len >> 8 );	
  
  /*
  crc = calculate_crc( data , size );
  crc_l = ( crc & 0xff );
  crc_h = ( crc >> 8 );
  */
  
  PACKAGE pack = {      
    { 0xeb, 0x90 },
    0,
    0,
    0,
    0,
    NULL
  };
  
//  pack.crc_H = crc_h;
//  pack.crc_L = crc_l;
  
  pack.len_H = len_h;
  pack.len_L = len_l;
  
  //	pBuffer = ( char * )malloc( len + 4 );
  pos = pBuffer;
  
  if( pBuffer == NULL )
    return NULL;
  
  memcpy( pos , ( void * )&pack, 4 );
  pos += 4;
  
  memcpy( pos , ( void * )data, size );
  pos += size;
  
  /*
  *(pos++) = crc_h;
  *(pos++) = crc_l;
  */
  
  return len;
  
}

/*
*********************************************************************************************************
*                                               destroy()
*
* Description : destroy lpc1313 chip instance
*
* Arguments   : chip     : instance of chip lpc1313
*                                         
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void destroy ( void * chip )
{
  memset( chip, 0 , sizeof( LPC1313 ) );
}

/*
*********************************************************************************************************
*                                               sendCommand()
*
* Description : send data
*
* Arguments   : pControl : controller port
*             : buffer   : container of command
*             : size     : command size
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void sendCommand ( DataSource *pControl,
                  void * buffer ,
                  uint32_t size )
{
  uint16_t len;
  char *data = ( char * )buffer;
  
  len = encode_data(  lpc_chip.sendBuffer,
                    data,
                    size );
  
  
  if( lpc_chip.sendBuffer == NULL )
    return;
  
  pControl->buffer_write( pControl , ( uint8_t * )lpc_chip.sendBuffer , len + 4 );
}

/*
*********************************************************************************************************
*                                               receiveData()
*
* Description : received data
*
* Arguments   : pControl : controller port
*             : buffer   : container of command
*             : size     : command size
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
static void receiveData ( DataSource *pControl,
	                 void *buffer , 
	                 uint32_t size )
{
  pControl->buffer_read( pControl , buffer , size );
}

/*
*********************************************************************************************************
*                                               lpc_read()
*
* Description : read command
*
* Arguments   : pControl : controller port
*             : buffer   : container of command
*             : size     : command size
*               
* Returns     : error code
*
* Note(s)     : none
*********************************************************************************************************
*/
extern uint8_t usart1Receive[16];
uint8_t lpc_read( DataSource *pControl,
                 void *buffer,
                 uint32_t size )
{
  uint32_t tick;
  uint32_t delay;
  tick = GetTc1TickCount();
  
  const uint32_t timeout = 2000;
  lpc_chip.errCode = NO_UART_ERR;
  
  if( lpc_chip.state != IDLE )
    return MISSSTATE_ERR;
  
  if( lpc_chip.state < WAIT_ACK )
  {
    sendCommand( lpc_chip.pController , buffer , size );
    lpc_chip.state = WAIT_ACK;
    
    receiveData( lpc_chip.pController , lpc_chip.receiverBuff, 1 ); 
    TC_Start(TC1, 0);
  }
  
  
  while( lpc_chip.state != DATA_READY ) 
  {
    delay = GetTc1DelayInTicks(tick, GetTc1TickCount( ) );
    if ( delay > timeout )
    {
      lpc_chip.state = IDLE;
      TC_Stop(TC1, 0);
      return TIMEOUT_ERR;
    }
  }
  if( lpc_chip.state == DATA_READY )
  {
    lpc_chip.state = IDLE;
  }
  return lpc_chip.errCode;
}

/*
*********************************************************************************************************
*                                               lpc_write()
*
* Description : write command
*
* Arguments   : pControl : controller port
*             : buffer   : container of command
*             : command  : command size
*               
* Returns     : error code
*
* Note(s)     : none
*********************************************************************************************************
*/
uint8_t lpc_write( DataSource *pControl,
                  void *buffer,
                  uint32_t size )
{
  uint32_t tick;
  uint32_t delay;
  tick = GetTc1TickCount();
  
  const uint32_t timeout = 2000;
  lpc_chip.errCode = NO_UART_ERR;
  
  if( lpc_chip.state != IDLE )
    return MISSSTATE_ERR;
  
  if( lpc_chip.state < WAIT_ACK )
  {
    sendCommand( lpc_chip.pController , buffer , size );
    lpc_chip.state = WAIT_DATA;
    
    receiveData( lpc_chip.pController , lpc_chip.receiverBuff, 1 ); 
    TC_Start(TC1, 0);
  }
  
  
  while( lpc_chip.state != DATA_READY ) 
  {
    delay = GetTc1DelayInTicks(tick, GetTc1TickCount( ) );
    if ( delay > timeout )
    {
      lpc_chip.state = IDLE;
      TC_Stop(TC1, 0);
      return TIMEOUT_ERR;
    }
  }
  if( lpc_chip.state == DATA_READY )
  {
    lpc_chip.state = IDLE;
  }
  return lpc_chip.errCode;
}

/*
*********************************************************************************************************
*                                               init_lpc()
*
* Description : initialize lpc object
*
* Arguments   : none
*               
* Returns     : none
*
* Note(s)     : none
*********************************************************************************************************
*/
void init_lpc( void )
{  
  extern uint8_t usart1Buffer[ 2 ][ USART_BUFFER_SIZE_1K ];  
  lpc_chip.destroy = destroy;
  lpc_chip.init = NULL;
  lpc_chip.read = lpc_read;
  lpc_chip.write = lpc_write;
  lpc_chip.receiveData = receiveData;
  lpc_chip.sendCommand = sendCommand;
  
  lpc_chip.parameter = NULL;
  lpc_chip.sendBuffer = ( void *)&usart1Buffer;
  
  lpc_chip.pController = &source_usart1;
  lpc_chip.state = IDLE;
  
}
