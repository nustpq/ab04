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

#ifndef _CMDPARASE_H_
#define _CMDPARASE_H_ 

#include "uif_object.h"


static uint8_t Init_Play_Setting( void *pInstance );
static uint8_t Init_Rec_Setting( void *pInstance );

static uint8_t Audio_Start_Rec( void );
static uint8_t Audio_Start_Play( void );

static void Audio_Stop( void );

void Audio_State_Control( uint8_t *msg );

static void Get_Run_Time( unsigned int time );
bool First_Pack_Check_BO( unsigned char *pData, unsigned int size );
void First_Pack_Padding_BI( unsigned char usb_data_padding );
void Audio_Manager( unsigned char cfg_data );

#endif
