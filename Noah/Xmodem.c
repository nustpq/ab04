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
*                                            APP PACKAGE
*
*                                          Atmel AT91SAM3U4C
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : Xmodem.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <includes.h>
 
/*
*********************************************************************************************************
*                                       calc_crc()
*
* Description :  calculate CRC16 data
*               
* Argument(s) :  *ptr :  pointer to data need to calculate CRC   
*                count:  data size 
*
* Return(s)   : CRC data
*
* Note(s)     : None.
*********************************************************************************************************
*/
static unsigned short calc_crc(unsigned char *ptr, signed short count)
{
    
    unsigned short crc = 0;
    unsigned char  i;
    while (--count >= 0)
    {
        crc = crc ^ (int) *ptr++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while(--i);
    }
    return (crc);
    
}


static void flushinput(void)
{
	//while (_inbyte(((DLY_1S)*3)>>1) >= 0);
    //unsigned char c;
    //while( USART_Read_Timeout( RULER_UART, &c, 1, 100 ) ==0 );
    
}

/*
*********************************************************************************************************
*                                       Xmodem_Transmit()
*
* Description :  Xmodem data transfer function.
*               
* Argument(s) :  *src :  pointer to data need to transmit 
*                srcsz:  data szie to be transmit 
*
* Return(s)   : NO_ERR :   execute successfully
*               others :   =error code .  
*
* Note(s)     : None.
*********************************************************************************************************
*/
unsigned char Xmodem_Transmit( unsigned char *src, int srcsz )
{
  /*
	unsigned char  xbuff[XMODEM_PACK_SIZE+5]; //xModem  1SOH + 2PACK_NO + 128 DATA + 2CRC = 133
    unsigned short crc;
	unsigned char  packetno = 1;
	int c, len = 0;
    unsigned char  temp;
	unsigned char  i;
    unsigned char  err;
    
    err = 0; //no err

	for(;;) {   
        
		for( i = 0; i < MAX_RETRANS; ++i) {
            temp = 0 ;
            if( USART_Read_Timeout( RULER_UART, &temp, 1, 1000 ) == 0 ) {	
				switch (temp) {
				case 'C':				
					goto start_trans;
				case NAK:			
					goto start_trans;
				case CAN:
                    temp  = 0 ;
                    USART_Read_Timeout( RULER_UART, &temp, 1, 1000 );
					if (temp == CAN) {
						USART_SendByte( RULER_UART, ACK);
						flushinput();
						return XMODEM_CANCEL_ERR; // canceled by remote 
					}
					break;
				default:
					break;
				}
			}
		}
		USART_SendByte( RULER_UART, CAN);
		USART_SendByte( RULER_UART, CAN);
		USART_SendByte( RULER_UART, CAN);
		flushinput();
		return XMODEM_SYNC_ERR; // no sync 

		while(1) {
            
		start_trans:
            Buzzer_Toggle();          
			xbuff[0] = SOH; 
			xbuff[1] = packetno;
			xbuff[2] = ~packetno;
            
			if ( len < srcsz ) {
                
                c = srcsz - len;
                c = c > XMODEM_PACK_SIZE ? XMODEM_PACK_SIZE : c ;               
				memset (&xbuff[3], 0, XMODEM_PACK_SIZE); //padding 0			
				memcpy (&xbuff[3], src+len, c);
			    crc = calc_crc(&xbuff[3], XMODEM_PACK_SIZE);
				xbuff[XMODEM_PACK_SIZE+3] = (crc>>8) & 0xFF;
				xbuff[XMODEM_PACK_SIZE+4] = crc & 0xFF;				
	
				for (i = 0; i < MAX_RETRANS; ++i) {                   
                    USART_SendBuf( RULER_UART, xbuff,  sizeof(xbuff));  
                    temp  = 0 ;
                    if( USART_Read_Timeout( RULER_UART, &temp, 1, 5000 ) == 0 ) {
						switch (temp) {
						case ACK:
							packetno++;
							len += c;
                            APP_TRACE_INFO(("> "));                             
							goto start_trans;
                            
						case CAN:
                            APP_TRACE_INFO(("CAN "));                  
                            temp  = 0 ;
                            USART_Read_Timeout( RULER_UART, &temp, 1, 1000 );
                            if (temp == CAN) {
                                USART_SendByte( RULER_UART, ACK);
								flushinput();
                                Buzzer_OnOff(0);                                
								return XMODEM_CANCEL_ERR; //canceled by remote  
							}
							break;
						case NAK:
                            APP_TRACE_INFO(("NAK ")); 
						default:
                            APP_TRACE_INFO(("DEFAULT ")); 
							break;
						}
					}
                    
				}
	            USART_SendByte( RULER_UART, CAN);
		        USART_SendByte( RULER_UART, CAN);
		        USART_SendByte( RULER_UART, CAN);
				flushinput();
                Buzzer_OnOff(0);
				return XMODEM_TRANS_ERR; //xmit error 
                
			} else {
                err = XMODEM_TRANS_END_ERR;
                temp  = 0 ;
				for (i = 0; i < MAX_RETRANS; ++i) {
					USART_SendByte( RULER_UART,EOT);                   
                    USART_Read_Timeout( RULER_UART, &temp, 1, 1000 );
                    if (temp == ACK) {
                        err = 0;
                        break;
                    } 
				}           
                flushinput();
                Buzzer_OnOff(0);                
				return err;
			}
            
		}
        
        
	}
    
    */
}



