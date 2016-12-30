/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : object.h
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :abstract data struct for communicating port
*********************************************************************************************************
*/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define PINGPONG

#include "board.h"
#include "kfifo.h"

//define stream direct
typedef enum _port_direct
{
	IN = 0,             
	OUT,
	BI,
	INVALIDDIR,
}DIRECT;

//define application layer direct
typedef enum _audio_direct
{
	REC = 0,             
	PLAY,
	INVALIDAUDIODIR,
}AUDIODIRECT;

//trace port status;
typedef enum _status
{
	FREE = 0,
	CONFIGURED,
	START,
        BUFFERED,
        RUNNING,
        BUFFEREMPTY,
        BUFFERFULL,
	STOP
}STATUS;

//define port that all of uif board used;
typedef enum _uif_port
{
	UIDSSC0 = 1,
	UIFSSC1 = 2,
	SPI     = 4,
	GPIO	= 8,           //treat it as a port now,if has better idea,change it;
	I2C	= 0x10,
	INVALIDPORT
}UIFPORT;

typedef enum _uif_port_mask
{      
      SSC0_IN  =  1,
      SSC0_OUT =  2,
      SSC1_IN  =  4,
      SSC1_OUT =  8,
      SPI0_IN  =  16,
      SPI0_OUT =  32,     
      GPIO_IN  =  64,
      INVALIDMASK       
}UIFPORTMASK;

typedef struct _abstractDevice
{
	uint32_t identify;		//peripheral id that defined by datasheet
        uint32_t instanceHandle;        //store port handle 
	uint32_t rxDMAChannel;		//dma rx channel with this port
	uint32_t txDMAChannel;		//dma tx channel with this port
	uint32_t direct;                //data stream direct
        uint32_t reserved;              //for extend in the furture
}Device;



typedef struct _DataSource
{
//public interface to reveal ucosII task,that I want to hide peripheral info in this struct;
	uint32_t ( *register_source )( void *type );
    void ( *init_source )( void *pParameter,void *dParameter );    //initialize the hardware according result that parse protocol
	void ( *set_peripheral )( void *instance,void *parameter );    //set register;
	void ( *peripheral_start )( void *instance );                  //starting peripheral
	void ( *peripheral_stop )( void *instance );		       //stoping peripheral
  
        uint8_t ( *buffer_read )( void *pInstance,
                                  const uint8_t *buf,
                                  uint32_t len);
        uint8_t ( *buffer_write )( void *pInstance,
                                   const uint8_t *buf,
                                   uint32_t len);
	void ( *sync_task ) ( void *semphore );			      //sync this port with others use ucosII sync machinsim
	uint8_t ( *get_sync_unmask )( void *unused );                 //get system sync script;
	void ( *set_sync_unmask )( void * unused );                   //set system sync script  after transfer;
	uint32_t ( * get_direct )( void *unused );                    //stream direct about this peripheral
	void ( * set_direct )( void *unused );
	void ( * read_callback ) ( void *parameter );
	void ( * write_callback ) ( void *parameter );

//private info about peripheral used;	
	Device dev;
	void *peripheralParameter;  //peripheral register parameter;
	void *dmaParameter;         //dma parameter for this; 
	void *privateData;          //user config parameter;
    void *mutex;                //lock port mutex;
	void *buffer;               //maybe should define a private buf for this?
        uint32_t warmWaterLevel;    //corresponding to i2s_play_buffer_size,maybe
                                    //should move it to another struct?
//these points followed pointed public buffers        
        kfifo_t *pRingBulkOut;      //the out ring buffer that this port relevant
        kfifo_t *pRingBulkIn;       //the in  ring buffer that this port relevant
        uint16_t *pBufferIn;        //the pointer pointed PingPong buffer in
        uint16_t *pBufferOut;       //the pointer pointed PingPong buffer out
	    uint8_t status[BI];         //state machine of port
        uint16_t unused;            //keep alive
        
#ifdef PINGPONG
        uint8_t tx_index;           //indicate current buffer is Ping or Pong;
        uint8_t rx_index;
        uint32_t txSize;            //indicate actul data size of 2ms
        uint32_t rxSize;
#endif
}DataSource;
	
#endif
