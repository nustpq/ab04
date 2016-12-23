/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// Interface for configuration the Two Wire Interface (TWI) peripheral.
///
/// !Usage
///
/// -# Configures a TWI peripheral to operate in master mode, at the given
/// frequency (in Hz) using TWI_ConfigureMaster().
/// -# or if hardware possible, configures a TWI peripheral to operate in 
/// slave mode, at the given frequency (in Hz) using TWI_ConfigureSlave().
/// -# Sends a STOP condition on the TWI using TWI_Stop().
/// -# Starts a read operation on the TWI bus with the specified slave using
/// TWI_StartRead(). Data must then be read using TWI_ReadByte() whenever 
/// a byte is available (poll using TWI_ByteReceived()).
/// -# Starts a write operation on the TWI to access the selected slave using
/// TWI_StartWrite(). A byte of data must be provided to start the write;
/// other bytes are written next. 
/// -# Sends a byte of data to one of the TWI slaves on the bus using TWI_WriteByte().
/// This function must be called once before TWI_StartWrite() with the first byte of data
/// to send, then it shall be called repeatedly after that to send the remaining bytes.
/// -# Check if a byte has been received and can be read on the given TWI
/// peripheral using TWI_ByteReceived(). 
/// Check if a byte has been sent using TWI_ByteSent().
/// -# Check if the current transmission is complete (the STOP has been sent)
/// using TWI_TransferComplete().
/// -# Enables & disable the selected interrupts sources on a TWI peripheral
/// using TWI_EnableIt() and TWI_DisableIt().
/// -# Get current status register of the given TWI peripheral using
/// TWI_GetStatus(). Get current status register of the given TWI peripheral, but
/// masking interrupt sources which are not currently enabled using 
/// TWI_GetMaskedStatus().
//------------------------------------------------------------------------------

#ifndef I2C_GPIO_H
#define I2C_GPIO_H

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

extern unsigned char I2C_GPIO_Write (unsigned char addr, unsigned char* buf, unsigned int len) ;
extern unsigned char I2C_GPIO_Write_iM205 (unsigned char addr, unsigned char reg, unsigned char val ) ;

extern unsigned char I2C_GPIO_Read(unsigned char addr, unsigned char* buf, unsigned int len) ;
extern unsigned char I2C_GPIO_Read_iM205(unsigned char addr, unsigned char reg, unsigned char *val) ;

extern void I2C_GPIO_Init ( unsigned int speed, unsigned char scl_gpio, unsigned char sda_gpio ) ;
extern void CS_GPIO_Init ( unsigned char cs_gpio );
extern unsigned char CS_GPIO_Write (unsigned char mode_toggle_num, unsigned char gain_toggle_num);

#endif //
