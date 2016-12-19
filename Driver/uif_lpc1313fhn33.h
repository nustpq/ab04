#ifndef _LPC1313FHN33_H_
#define _LPC1313FHN33_H_

#include "uif_object.h"



#define NO_UART_ERR       0u
#define HEAD_ERR          1u
#define CRC_ERR           2u
#define CMD_ERR           2u
#define LENGTH_ERR        3u
#define TIMEOUT_ERR       5u
#define MISSSTATE_ERR     6u

typedef struct _data_frame_info
{
	uint8_t id;
	uint8_t speed1;
	uint8_t speed2;
	uint8_t cmd;
	uint8_t lengthH;
	uint8_t lengthL;

}FRAME;

#pragma pack ( 1 )
typedef struct _package
{
	const uint8_t header[2];
	uint8_t len_H;
	uint8_t len_L;
	uint8_t crc_H;
	uint8_t crc_L;
	uint8_t *data;
}PACKAGE;
#pragma pack (  )


typedef struct _lpc_chip
{
//public interface;
	void  ( * init )( void );
	void ( * destroy )( void * );
	uint8_t ( *write )( DataSource *pControl,
		             void * buffer,
		             uint32_t size );
	uint8_t ( *read )( DataSource *pControl,
		            void *buffer,
		            uint32_t size );
//private interface;	
	void ( * sendCommand )( DataSource *pControl,
	                     void * buffer ,
	                     uint32_t size );
	void ( * receiveData )( DataSource *pControl,
	                     void * buffer ,
	                     uint32_t size );
	
// private members;
	DataSource *pController;
    void *parameter;                       //usart configure parameters

    char *sendBuffer;
    uint8_t receiverBuff[ 4 ];            //temp buff

	uint8_t state;                        //protocal state
	uint8_t errCode;                      //protocal error code
}LPC1313;

extern void destroy ( void * chip );
extern uint8_t lpc_read( DataSource *pControl,
				void *buffer,
				uint32_t size );

extern void sendCommand ( DataSource *pControl,
	                     void * buffer ,
	                     uint32_t size );

extern void receiveData ( DataSource *pControl,
	                 void *buffer , 
	                 uint32_t size );

extern void init_lpc( void );
#endif
