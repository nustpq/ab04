/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename         : Xc3s50an.h
* Version           : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       : fpga  driver
*********************************************************************************************************
*/
#ifndef _UIF_XC3S50AN_H_
#define _UIF_XC3S50AN_H_

#include "uif_spi.h"
#include "uif_object.h"
#include "uif_list.h"

/*----------------------------------------------------------------------------------
 *
 *
 *  |T6~T0|MAX_DIR|DIR|MAX_OE|OE|
 *
 *  OE :bit6~bit0
 *  DIR:bit8~bit14
 *  T   :bit16~bit22
 *----------------------------------------------------------------------------------*/
#define MAX_PATH_NAME_LENGTH 32
#define MAX_I2S_CLK_PATH  18
#define MAX_DATA_PATH       7
#define MAX_PDM_CLK_PATH    1

#define CODEC0_OE 0
#define PORT0_OE  1
#define SSC0_OE   2
#define FM36_OE   3
#define CODEC1_OE 4
#define PORT1_OE  5
#define SSC1_OE	  6
#define MAX_OE    8

#define CODEC0_DIR ( CODEC0_OE + MAX_OE )
#define PORT0_DIR  ( PORT0_OE + MAX_OE )
#define SSC0_DIR   ( SSC0_OE + MAX_OE )
#define FM36_DIR   ( FM36_OE + MAX_OE )
#define CODEC1_DIR ( CODEC1_OE + MAX_OE )
#define PORT1_DIR  ( PORT1_OE + MAX_OE )
#define SSC1_DIR   ( SSC1_OE + MAX_OE )
#define MAX_DIR    MAX_OE


extern const char fpga_i2s_clk_path[ MAX_I2S_CLK_PATH ][ MAX_PATH_NAME_LENGTH ];
extern const char fpga_data_path[ MAX_DATA_PATH ][ 4 ];

//define fpga data switch object;
typedef struct _fpga_data_switch
{
	uint8_t ord;
	uint8_t sel;

	uint8_t reves0;
	uint8_t reves1;
}FPGA_DATA_SWITCH;

#pragma pack ( 1 )
typedef struct _fpga_clk_switch
{	
	uint8_t oe;
	uint8_t dir;

	uint8_t reves0;
	uint8_t reves1;
        
        char switch_name[16];
}FPGA_CLK_SWITCH;
#pragma pack ( )


//define a path include on i2s clock path and two data path switch. it is the max resource the path owned;
typedef struct _fpga_path
{
	FPGA_CLK_SWITCH clk;

	FPGA_DATA_SWITCH T0;
	FPGA_DATA_SWITCH T1;

	uint16_t rev;
	
}FPGA_PATH;

//this struct is used to verify the path of i2s clock valid or collide with others;
//rule 1:Each node must single input signal; 
//rule 2:Each node must be a single role; 
#pragma pack ( 1 )
typedef struct _node_role
{	
  char name[8];
  uint8_t position;                 //node name in path name position;
  uint8_t direct;                   //path dir;
  uint8_t using;                    //this node is used or not
  uint8_t role;                     //receiver or sender
  uint8_t revers[4];
}ROLE;
#pragma pack (  )

//note: if need to extend new path,
#pragma pack ( 1 )
typedef struct _fpga_command
{
/*------------------------------------------*/
    uint32_t pdm_revers;                 //(left)
/*------------------------------------------*/
  	uint8_t t_revs0;      
        uint8_t t_revs1; 
	uint8_t t_revs2; 
	
	uint8_t t0 : 1;    
	uint8_t t1 : 1;
	uint8_t t2 : 1;    
	uint8_t t3 : 1;
	uint8_t t4 : 1;
	uint8_t t5 : 1;
	uint8_t t6 : 1;
	uint8_t t7 : 1;   
  
/*------------------------------------------*/
	uint8_t dir_revers6;
        uint8_t dir_port1_ssc1     : 1;  
        uint8_t dir_port1_fm36     : 1;
	uint8_t dir_revers0        : 1;
	uint8_t dir_revers1        : 1;
	uint8_t dir_revers2        : 1;
	uint8_t dir_revers3        : 1;	
	uint8_t dir_revers4        : 1;
	uint8_t dir_revers5	   : 1;

	uint8_t dir_codec1_ssc0    : 1;  
	uint8_t dir_codec1_ssc1    : 1;
	uint8_t dir_codec1_fm36    : 1;
	uint8_t dir_port0_port1    : 1;
	uint8_t dir_port0_ssc0     : 1;
	uint8_t dir_port0_ssc1     : 1;
	uint8_t dir_port0_fm36     : 1;
	uint8_t dir_port1_ssc0     : 1;  
        
        uint8_t dir_codec0_port0   : 1;  
	uint8_t dir_codec0_fm36    : 1;
	uint8_t dir_codec0_ssc0    : 1;
	uint8_t dir_codec0_port1   : 1;
	uint8_t dir_codec0_codec1  : 1;
	uint8_t dir_codec0_ssc1    : 1;	
	uint8_t dir_codec1_port0   : 1;
	uint8_t dir_codec1_port1   : 1; 

	
/*--------------------------------------*/
	uint8_t oe_revers6;

        uint8_t oe_port1_ssc1      : 1;  
        uint8_t oe_port1_fm36      : 1;
	uint8_t oe_revers0         : 1;
	uint8_t oe_revers1         : 1;
	uint8_t oe_revers2         : 1;
	uint8_t oe_revers3         : 1;	
	uint8_t oe_revers4	   : 1;
	uint8_t oe_revers5	   : 1;

	uint8_t oe_codec1_ssc0     : 1; 
	uint8_t oe_codec1_ssc1     : 1;
	uint8_t oe_codec1_fm36     : 1;
	uint8_t oe_port0_port1     : 1;
	uint8_t oe_port0_ssc0      : 1;
	uint8_t oe_port0_ssc1      : 1;
	uint8_t oe_port0_fm36      : 1;
	uint8_t oe_port1_ssc0      : 1;  
        
        uint8_t oe_codec0_port0    : 1;  // LSB
	uint8_t oe_codec0_fm36     : 1;
	uint8_t oe_codec0_ssc0     : 1;
	uint8_t oe_codec0_port1    : 1;
	uint8_t oe_codec0_codec1   : 1;
	uint8_t oe_codec0_ssc1     : 1;	
	uint8_t oe_codec1_port0    : 1;
	uint8_t oe_codec1_port1    : 1;  

}FPGA_COMMAND;
#pragma pack (  )


typedef struct _fpga_chip
{
//public interface for xc3s50an chip
	void ( *init )( void );                                                    //initialize fpga and it's data struct
    void ( *reset_fpga )( void );                                                   //reset fpga hardware
    int8_t ( *set_path )( void *handle,
						  FPGA_COMMAND* pCmd,
					      List *clkList,
						  List *dataList);                                          //assemble command word and sent to fpga;
    FPGA_COMMAND * ( *get_path_cfg )( char *pathName );                             //get current command word 

//private interface of xc3s50an chip
   int8_t ( *add_clk_switch_cfg)( FPGA_CLK_SWITCH * cfg,List * clkList );
   void ( *add_data_switch_cfg)( FPGA_DATA_SWITCH *cfg ,List *clkList );
   void ( *set_data_path )( FPGA_COMMAND *pCmd,uint8_t index,uint8_t value );
   void ( *set_clk_path )( uint8_t index,FPGA_COMMAND* cmd,uint8_t dir, uint8_t oe );
//private member of this obj;
	List fpga_i2s_clk_list;                                                         //i2s clock config list
	List fpga_i2s_data_list;                                                        //audio data config list
	List clock_node_role;                                                           //The list records the roles of each clock node in the FPGA 		
	FPGA_DATA_SWITCH data_cfg;                                                      //audio data config
	FPGA_CLK_SWITCH clock_cfg;                                                      //clock config list
	
	FPGA_COMMAND cmdWord;
	DataSource *controller;                                                         //which port to send fpga command;


}FpgaChip;


extern uint8_t parase_index( char *path );
extern int8_t get_i2s_clk_index( char *path );
extern int8_t get_data_path_index( char *path );
extern FPGA_DATA_SWITCH * map_name_to_path( const char *pPath );
extern void set_i2s_clk_path( uint8_t index,FPGA_COMMAND* cmd,uint8_t dir, uint8_t oe );
extern void set_data_path(  FPGA_COMMAND* cmd,uint8_t index,uint8_t value );
extern int8_t set_fpga_path( void *handle,FPGA_COMMAND* pCmd,List *clkList,List *dataList);
extern bool i2s_clk_path_check( void * pPath , List *validList );
extern int8_t add_clk_switch_cfg( FPGA_CLK_SWITCH *cfg ,List *clkList );
extern void add_data_switch_cfg( FPGA_DATA_SWITCH *cfg ,List *dataList );
extern void resset_fpga( void );
extern void init_fpga( void );
extern void init_fpga_instance( void );
extern unsigned char FPGA_Setup( void ) ;
#endif
