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

/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAM3U4C
*                                             on the
*                                      Unified EVM Interface Board
*
* Filename      : kfifo.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include "kfifo.h"


// Note: 
// The fifo size must be 2^n length.

 __ramfunc inline uint32_t min( uint32_t min_var_a, uint32_t min_var_b )
{ 

  return ( min_var_a < min_var_b ? min_var_a : min_var_b );

}

void kfifo_init(kfifo_t *fifo, int size) {
	fifo->buffer = (uint8_t *) malloc(size);
	fifo->size = size;
	fifo->in = fifo->out = 0;
}

void kfifo_free(kfifo_t *fifo) {
	free(fifo->buffer);
}

void kfifo_init_static(kfifo_t *fifo, uint8_t *pBuf, int size) {
	fifo->buffer = pBuf;
	fifo->size = size;
	fifo->in = fifo->out = 0;
}

void kfifo_reset(kfifo_t *fifo) {
	fifo->in = fifo->out = 0;
}

//#pragma optimize= none
uint32_t kfifo_put(kfifo_t *fifo, uint8_t *buffer, uint32_t len) {
	uint32_t l;
        //__disable_interrupt(); //PQ
	len = min(len, fifo->size - fifo->in + fifo->out);
	/* first put the data starting from fifo->in to buffer end */
	l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
	//l<=len
	memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);
	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(fifo->buffer, buffer + l, len - l);
	fifo->in += len;
        //__enable_interrupt(); //PQ
	return len;
}

//extern kfifo_t bulkout_fifo;
//#pragma optimize= none
uint32_t kfifo_get(kfifo_t *fifo, uint8_t *buffer, uint32_t len) {
	uint32_t l;       
        //__disable_interrupt(); //PQ
	len = min(len, fifo->in - fifo->out);
	/* first get the data from fifo->out until the end of the buffer */
	l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
	memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);
	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buffer + l, fifo->buffer, len - l);
	fifo->out += len; 
//        if(fifo == &bulkout_fifo) {
//            printf("\r\n::%d, %d ",fifo->out,len);
//        }
        //__enable_interrupt(); //PQ
	return len;
}

__ramfunc uint32_t kfifo_release(kfifo_t *fifo, uint32_t len) {
        //__disable_interrupt(); //PQ
	len = min(len, fifo->in - fifo->out);	
	fifo->out += len;
        //__enable_interrupt(); //PQ
	return len;
}


//no interruption for debug printf use
__ramfunc uint32_t kfifo_get_free_space(kfifo_t *fifo) {
    
    return( fifo->size - fifo->in + fifo->out );     

}


__ramfunc uint32_t kfifo_get_data_size(kfifo_t *fifo) {
           
    return( fifo->in - fifo->out );     
	
}

