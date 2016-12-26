/*
*********************************************************************************************************
*
*                                             APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                                   on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
*
* Filename      : uif_xc3s50an.c
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/

#include <string.h>

#include "uif_xc3s50an.h"
#include "uif_list.h"
#include "bsp.h"


#define SENDER 1
#define RECEIVER 0
#define MAX_BI_NODE 3
#define MAXNODE 7
#define MAXDATAPATH 14


//global object declare
FpgaChip xc3s50an;                        

const DataSource *pFpga = &source_spi1;
List fpga_i2s_clk_list;
List fpga_i2s_data_list;
List fpga_i2s_clk_valid_list;

//All of i2s clock path in fpga that supported;
const char fpga_i2s_clk_path[ MAX_I2S_CLK_PATH ][ MAX_PATH_NAME_LENGTH ] = { 
  "codec0_port0_0",
  "codec0_fm36_1",	
  "codec0_ssc0_2",	
  "codec0_port1_3",
  "codec0_codec1_4",
  "codec0_ssc1_5",	
  "codec1_port0_6",
  "codec1_port1_7",
  
  "codec1_ssc0_8",
  "codec1_ssc1_9",
  "codec1_fm36_10",
  "port0_port1_11",
  "port0_ssc0_12",
  "port0_ssc1_13",  
  "port0_fm36_14",
  
  "port1_ssc0_15",
  "port1_ssc1_16",  
  "port1_fm36_17",    
};
//All of data path switch in fpga implemented;
const char fpga_data_path[ MAX_DATA_PATH ][ 4 ] = {
  "T0",
  "T1",
  "T2",
  "T3",
  "T4",
  "T5",
  "T6"
};

  
const char biNode[ MAXNODE ][ 8 ] = { 
    "port0",
    "port1",
    "codec0",
    "codec1",
    "fm36",
    "ssc0",
    "ssc1"
  };

const char dataSwitch[ 14 ][ 32 ]= { 
  "codec0_tx->fm36_i2s_rx",
  "uif_i2s0_rx->fm36_i2s_rx",
  "ssc0_tx->codec0_rx",
  "uif_i2s0_tx->codec0_rx",
  "codec1_tx->ssc1_rx",
  "uif_i2s1_rx->ssc1_rx",
  "ssc1_tx->codec1_rx",
  "uif_i2s1_rx->codec1_rx",
  "fm36_pdmo_data->uif_pdmo_data",
  "hdmi_pdmi_data->uif_pdmo_data",
  "uif_pdmi_data->fm36_pdmi_data",
  "hdmi_pdmi_data->fm36_pdmi_data",
  "fm36_pdmi_clk->hdmi_pdm_clk",
  "uif_pdmo_clk->hdmi_pdm_clk"
};
	                          
	                          
/*
*********************************************************************************************************
*                                               parase_index()
*
* Description : get last char of string as index in path name;
*
* Arguments   : path   : path name
* Returns       : index number
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t parase_index( char *pPath )
{
  assert( NULL != pPath );
  
  uint8_t len = strlen( pPath );
  
  char *t = &pPath[ len - 1 ];
  
  return ( *t - 48 );
  
}


/*
*********************************************************************************************************
*                                               get_i2s_clk_index()
*
* Description : get path index in i2s clock path name;
*
* Arguments   : path   : path name
* Returns       : index number
*
* Note(s)     : none.
*********************************************************************************************************
*/
int8_t get_i2s_clk_index( char *pPath )
{
  assert( NULL != pPath );
#if 0  
  int8_t ret = -1;
  uint8_t len = strlen( pPath );
  
  for( uint8_t i = 0; i < MAX_I2S_CLK_PATH; i++ )
  {
    if( 0 == strncmp( pPath, fpga_i2s_clk_path[ i ] ,len ) )
    {      
      ret = i;
    }
  }
  
  return ret;
#else
  int8_t ret = -1;  
  char pathName[ 36 ];
  
  for( uint8_t i = 0; i< MAX_I2S_CLK_PATH; i++ )
  {   
    uint8_t len = strlen( ( char * )&fpga_i2s_clk_path[ i ] );
    memset( pathName , 0 ,sizeof( pathName ) );
    strncpy( pathName,(char *)fpga_i2s_clk_path[ i ],len-2 );
    
    if( !strncmp( pPath,pathName,len-2 ) )
    {
      ret = i;
      break;
    }
    
  }
  
  return ret;
#endif
}

/*
*********************************************************************************************************
*                                               get_data_path_index()
*
* Description : get path index in data path name;
*
* Arguments   : path   : path name
* Returns       : index number, if failed ,return -1;
*
* Note(s)     : none.
*********************************************************************************************************
*/
int8_t get_data_path_index( char *pPath )
{
  assert( NULL != pPath );
  
  int8_t ret = -1;
  
    for( uint8_t i = 0; i < MAXDATAPATH; i++ )
  	{
		if( 0 == strcmp( pPath, dataSwitch[ i ] ) )
			{

				ret =  i;
			}
  	}

  return ret;
}


/*
*********************************************************************************************************
*                                               map_name_to_path()
*
* Description : transmit path name to data path node;
*
* Arguments   : path   : path name
* Returns       : path node ptr or NULL;
*
* Note(s)     : none.
*********************************************************************************************************
*/
FPGA_DATA_SWITCH * map_name_to_path( const char *pPath )
{
  assert( NULL != pPath );

  FPGA_DATA_SWITCH *dNode;

  dNode = ( FPGA_DATA_SWITCH * )malloc( sizeof( FPGA_DATA_SWITCH ) );

    if( NULL == dNode ){
        APP_TRACE_INFO(("ERROR : memory : malloc failed @ map_name_to_path\r\n"));   
        return NULL;
    }

    for( uint8_t i = 0; i < MAXDATAPATH; i++ )
  	{
		if( 0 == strcmp( pPath, dataSwitch[ i ] ) )
			{
				dNode->ord = i / 2;
  				dNode->sel = i % 2;
				return dNode;
			}
  	}
    APP_TRACE_INFO(("ERROR :  Data Path is invalid : %s\r\n", pPath)); 
    
    return NULL;
}



/*
*********************************************************************************************************
*                                               set_i2s_clk_path()
*
* Description : set spi command word  bit field;
*
* Arguments   : index   : path index
*                     cmd:    :command word
*                     dir :    data direct
*                     oe :    path enable bit
* Returns       : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void set_i2s_clk_path( uint8_t index,FPGA_COMMAND* cmd,uint8_t dir, uint8_t oe )
{
  
  assert( NULL != cmd ) ;	
  
  assert( index < MAX_I2S_CLK_PATH );
  
  switch( index )
  {
  case 0:
    cmd->dir_codec0_port0 = dir;
    cmd->oe_codec0_port0 = oe;
    break;
  case 1:
    cmd->dir_codec0_fm36 = dir;
    cmd->oe_codec0_fm36 = oe;				
    break;
  case 2:
    cmd->dir_codec0_ssc0 = dir;
    cmd->oe_codec0_ssc0 = oe;				
    break;
  case 3:
    cmd->dir_codec0_port1 = dir;
    cmd->oe_codec0_port1 = oe;				
    break;
  case 4:
    cmd->dir_codec0_codec1 = dir;
    cmd->oe_codec0_codec1 = oe;				
    break;
  case 5:
    cmd->dir_codec0_ssc1 = dir;
    cmd->oe_codec0_ssc1 = oe;				
    break;
  case 6:
    cmd->dir_codec1_port0 = dir;
    cmd->oe_codec1_port0 = oe;				
    break;
  case 7:
    cmd->dir_codec1_port1 = dir;
    cmd->oe_codec1_port1 = oe;				
    break;    
  case 8:
    cmd->dir_codec1_ssc0 = dir;
    cmd->oe_codec1_ssc0 = oe;				
    break;
  case 9:
    cmd->dir_codec1_ssc1 = dir;
    cmd->oe_codec1_ssc1 = oe;				
    break;  
  case 10:
    cmd->dir_codec1_fm36 = dir;
    cmd->oe_codec1_fm36 = oe;				
    break; 
  case 11:
    cmd->dir_port0_port1 = dir;
    cmd->oe_port0_port1 = oe;				
    break; 
  case 12:
    cmd->dir_port0_ssc0 = dir;
    cmd->oe_port0_ssc0 = oe;				
    break; 
  case 13:
    cmd->dir_port0_ssc1 = dir;
    cmd->oe_port0_ssc1 = oe;				
    break; 
  case 14:
    cmd->dir_port0_fm36 = dir;
    cmd->oe_port0_fm36 = oe;				
    break;    
  case 15:
    cmd->dir_port1_ssc0 = dir;
    cmd->oe_port1_ssc0 = oe;				
    break; 
  case 16:
    cmd->dir_port1_ssc1 = dir;
    cmd->oe_port1_ssc1 = oe;				
    break; 
  case 17:
    cmd->dir_port1_fm36 = dir;
    cmd->oe_port1_fm36 = oe;				
    break;     
  default:
    break;
  }
  
}


/*
*********************************************************************************************************
*                                               set_data_path()
*
* Description : get path index in data path name;
*
* Arguments   : cmd   : spi command word
*                     index : switch index
*                     value : state
* Returns       : none
*
* Note(s)     : none.
*********************************************************************************************************
*/

void set_data_path( FPGA_COMMAND* cmd,uint8_t index,uint8_t value )
{
  assert( NULL != cmd );
  
  assert( index < MAX_DATA_PATH );
  
  switch( index )
  {
  case 0:
    cmd->t0 = value;
    break;
  case 1:
    cmd->t1 = value;				
    break;
  case 2:
    cmd->t2 = value;				
    break;
  case 3:
    cmd->t3 = value;				
    break;				
  case 4:
    cmd->t4 = value;				
    break;
  case 5:
    cmd->t5 = value;				
    break;
  case 6:
    cmd->t6 = value;				
    break;
  default:
    break;
    
  }
}



/*
*********************************************************************************************************
*                                               set_fpga_path()
*
* Description : assemble fpga command word and send to fpga via spi1;
*
* Arguments   : handle   : object handle
*                    pCmd     : command word
*                    clkList    : global link of clock
*                    dataList : global link of data
* Returns       : action successful or failed;
*
* Note(s)     : none.
*********************************************************************************************************
*/
int8_t set_fpga_path( void *handle,FPGA_COMMAND* pCmd,List *clkList,List *dataList )
{
  if( ( NULL == clkList ) || ( NULL == dataList ) ) return -1;

  char pathName[ 36 ];
  int8_t index = -1;

  FpgaChip *fpga = ( FpgaChip * )handle;

  
  //step1:set i2s clock path to command word;
  ListElmt *e = fpga->fpga_i2s_clk_list.head;

  while( e != NULL )
  {
    FPGA_CLK_SWITCH *clk = ( FPGA_CLK_SWITCH * )e->data;

#if 0
    for( uint8_t i = 0; i< MAX_I2S_CLK_PATH; i++ )
    {
      uint8_t len = strlen( ( char * )&fpga_i2s_clk_path[ i ] );
      memset( pathName , 0 ,sizeof( pathName ) );
      strncpy( pathName,(char *)fpga_i2s_clk_path[ i ],len-2 );

      if( !strcmp( clk->switch_name,pathName ) )
      {
        index = i;
        break;
      }
      
    }
#else
    index = get_i2s_clk_index( clk->switch_name );
#endif

    if( index == -1 )
    {
      APP_TRACE_INFO(("ERROR :  Clock Path is invalid : %s\r\n", clk->switch_name));     
      return index;
    }
    
    set_i2s_clk_path( index,pCmd,clk->dir,clk->oe );
    e = e -> next;
  }
  
  
  //step2: set i2s/pdm data to command word
  ListElmt *e1 = fpga->fpga_i2s_data_list.head;

  while( e1 != NULL )
  {
    FPGA_DATA_SWITCH *cfg = ( FPGA_DATA_SWITCH * )e1->data;
    set_data_path( pCmd,cfg->ord,cfg->sel );
    e1 = e1 -> next;
  }
  
  //step3: send command word to fpga and valid it via spi1 port;
  fpga->controller->buffer_write( ( void * )fpga->controller,
                                 ( uint8_t * )pCmd,
                                 sizeof( uint32_t ) << 2 );
//  APP_TRACE_INFO(("%s :  fpga cmd send : %ld\r\n",__func__, *(uint64_t *)pCmd++ ));
//  APP_TRACE_INFO(("%s :  fpga cmd send : %ld\r\n",__func__, *(uint64_t *)pCmd ));  
  return 0;
}

/*
*********************************************************************************************************
*                                               match_data_path()
*
* Description : compare i2s data switch path object
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:equal 1:!equal
*
* Note(s)     : none.
*********************************************************************************************************
*/
int match_data_path( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  List *l = ( List * )key1;

  if( l->size == 0 )
    return 1;

  ListElmt *e = l->head;

  FPGA_DATA_SWITCH * node = ( FPGA_DATA_SWITCH * )key2;

  while( NULL != e )
  {
    FPGA_DATA_SWITCH * pPath = ( FPGA_DATA_SWITCH * )e->data;

    if( pPath->ord == node->ord )
      return 0;
    else
      e = e->next;
  }
  return 1;
}


/*
*********************************************************************************************************
*                                               find_clk_path()
*
* Description : lookinf for i2s clock/audio switch path object exist in the array or not
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : index, -1:not found
*
* Note(s)     : none.
*********************************************************************************************************
*/
int find_clk_path( const void *key1,const void *key2 )
{
  assert( NULL != key2  );
  key1 = key1;

  char* s;
  int len = 0;

  FPGA_CLK_SWITCH *cfg = ( FPGA_CLK_SWITCH * )key2;

  for( uint8_t i = 0; i < MAX_I2S_CLK_PATH ; i ++ )
  {
    s = ( char * )fpga_i2s_clk_path[ i ];
    len = strlen( cfg->switch_name );


    if( !strncmp( s , cfg->switch_name, len ) )
       return i;
  }
  return -1;
}


/*
*********************************************************************************************************
*                                               find_clk_node()
*
* Description : lookinf for i2s clock/audio switch node exist in the array or not
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : index, -1:not found
*
* Note(s)     : none.
*********************************************************************************************************
*/
int find_clk_node( const void *key1,const void *key2 )
{
  assert( NULL != key2  );
  key1 = key1;

  char* s;
  int len = 0;

  ROLE *r = ( ROLE * )key2;

  for( uint8_t i = 0; i < MAX_I2S_CLK_PATH ; i ++ )
  {
    s = ( char * )biNode[ i ];
    len = strlen( s );

    if( !strncmp( s , r->name, len ) )
       return i;
  }
  return -1;
}


/*
*********************************************************************************************************
*                                               match_clk_path()
*
* Description : compare i2s clock/audio switch path object
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:equal 1:!equal
*
* Note(s)     : none.
*********************************************************************************************************
*/
int match_clk_path( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  List *l = ( List * )key1;

  if( l->size == 0 )
    return 1;

  ListElmt *e = l->head;

  FPGA_CLK_SWITCH *cfg = ( FPGA_CLK_SWITCH * )key2;

  while( NULL != e )
  {
    FPGA_CLK_SWITCH * pPath = ( FPGA_CLK_SWITCH * )e->data;

    if( strcmp( pPath->switch_name , cfg->switch_name ) == 0 )
      return 0;
    else
      e = e->next;
  }
  return 1;
}


/*
*********************************************************************************************************
*                                               match_clk_role()
*
* Description : compare i2s clock/audio switch path object
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:equal 1:!equal
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint32_t match_clk_role( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  List *l = ( List * )key1;

  if( l->size == 0 )
    return 1;

  ListElmt *e = l->head;

  ROLE *role = ( ROLE * )key2;

  while( NULL != e )
  {
    ROLE * pRole = ( ROLE * )e->data;

    if( strcmp( pRole->name , role->name ) == 0 )
      return 0;
    else
      e = e->next;
  }
  return 1;
}


/*
*********************************************************************************************************
*                                               			clk_rule_1()
*
* Description : implement i2s clock path valid verify rule 1;
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:passed 1:!passed
*
* Note(s)     : none.
*********************************************************************************************************
*/
static uint32_t clk_rule_1( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  List *l = ( List * )key1;

  if( l->size == 0 )
    return 1;

  ListElmt *e = l->head;

  ROLE *role = ( ROLE * )key2;

  while( NULL != e )
  {
    ROLE * pRole = ( ROLE * )e->data;

    if( ( strcmp( pRole->name , role->name ) == 0 )
		&&( pRole->direct != role->direct ) )
      return 0;
    else
      e = e->next;
  }
  return 1;

}

/*
*********************************************************************************************************
*                                               clk_rule_2()
*
* Description : implement i2s clock path valid verify rule 2;
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:passed 1:!passed
*
* Note(s)     : none.
*********************************************************************************************************
*/
static uint32_t clk_rule_2( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  int cnt = 0;

  List *l = ( List * )key1;

  if( l->size == 0 )
    return 1;

  ListElmt *e = l->head;

  ROLE *role = ( ROLE * )key2;

  /**/
  while( NULL != e )
  {
    ROLE * pRole = ( ROLE * )e->data;

    if( ( strcmp( pRole->name , role->name ) == 0 )
		&&( pRole->direct == RECEIVER ) )
            cnt++;          //TODO : 

      e = e->next;
  }
 
  return ( cnt >= 1 ) ? 0 : 1;


}


/*
*********************************************************************************************************
*                                               clk_rule_3()
*
* Description : implement i2s clock path valid verify rule 3;
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:passed 1:!passed
*
* Note(s)     : none.
*********************************************************************************************************
*/
static uint32_t clk_rule_3( const void *key1,const void *key2 )
{
  assert( (  NULL != key1 ) && ( NULL != key2 ) );

  int index = 0;

  ROLE *role = ( ROLE * )key2;

  index = find_clk_node( key1, key2 );

  if( -1 == index )
    return 1;

  //TODO: rule is logic error? when index = 4?
//  if( ( index > MAX_BI_NODE ) && ( role->direct == SENDER ) )
//    return  0;

   return 1;

}


/*
*********************************************************************************************************
*                                               clk_rule_4()
*
* Description : implement i2s clock path valid verify rule 4;
*
* Arguments   : key1   :path name list
*                   : key2   :path name
*
* Returns     : 0:passed 1:!passed
*
* Note(s)     : none.
*********************************************************************************************************
*/
static uint32_t clk_rule_4( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  ROLE *role0 = ( ROLE * )key1;
  ROLE *role1 = ( ROLE * )key2;

  if( role0->direct == role1->direct )
  	return 0;

	return 1;
}



/*
*********************************************************************************************************
*                                               i2s_clk_path_check()
*
* Description : check the clock path is valid or not used rule set ;
*
* Arguments   : path   : path name
* Returns       : valid or not
*
* Note(s)     : Path naming must conform to a certain rule
*********************************************************************************************************
*/
bool i2s_clk_path_check( void * pPath , List *validList )
{
  assert( NULL != pPath );
  assert( NULL != validList );

  FPGA_CLK_SWITCH* clk = ( FPGA_CLK_SWITCH* )pPath;

  char *e="_";
  char back[16];

  ROLE *role1,*role2;
  char *node0, *node1;
  volatile uint8_t matched = 0;

  memcpy( back,clk->switch_name,16);

  role1 = ( ROLE * )malloc( sizeof( ROLE ) );
  memset( role1, 0 , sizeof( ROLE ) );
  if( !role1 ) return 0;

  role2 = ( ROLE * )malloc( sizeof( ROLE ) );
  memset( role1, 0 , sizeof( ROLE ) );
  if( !role2 ) return 0;

#if 0
  //step1:get a
  node0 = strtok( clk->switch_name, e );
  //step2 :get b
  node1 = strtok( NULL, e );
#else
  node0 = strtok( back,e );
  node1 = strtok( NULL , e );
#endif

  if( ( NULL == node0 ) || ( NULL == node1 ) )
    return 0;

  //initialize role1 and set it's attibute;
  for( uint8_t i = 0; i < 7 ; i++ )
  {
    if( !strcmp( biNode[ i ] , node0 ) )                              //match this node in the table,if found
    {
      memset( role1->name , 0 , sizeof( role1->name ) );
      strncpy( role1->name,node0 ,strlen( node0 ) );
      role1->position = 'a';			                                  //named this node 'a' endpoint,because it is the first node in path
      role1->using = 1;
      matched = 1;
      if(( 0 == clk->dir ) && ( role1->position == 'a') )              //
      {
        role1->direct = SENDER;
      }
      else if( ( 1 == clk->dir ) && ( role1->position == 'a' ) )
      {
        role1->direct = RECEIVER;
      }

      break;
    }
  }


  if( 0 == matched )
  {
    ///TODO:trace info print
    return matched;
  }


  //initialize role2 and set it's attibute;
  for( uint8_t i = 0; i < 7 ; i++ )
  {
      if( !strcmp( biNode[ i ] , node1 ) )                              //match this node in the table,if found
      {
        memset( role2->name , 0 , sizeof( role2->name ) );
        strncpy( role2->name,node1 ,strlen( node1 ) );
        role2->position = 'b';			                                  //named this node 'a' endpoint,because it is the first node in path
        role2->using = 1;
        if(( 0 == clk->dir ) && ( role2->position == 'b') )             //
        {
          role2->direct = RECEIVER;
        }
        else if( ( 1 == clk->dir ) && ( role2->position == 'b' ) )
        {
          role2->direct = SENDER;
        }

        break;
      }
  }

  if( 0 == matched )
  {
    ///TODO:trace info print
    return matched;
  }

#if 0
  if( validList->size > 4 )                                        //queue is full,return false;
    return 0;
#endif

  if( validList->size == 0 )
    list_ins_next( validList,validList->tail,( void * )role1 );
  else
  {
    if( clk_rule_1( validList,( void * )role1 )
       && clk_rule_2( validList,( void * )role1 )
         && clk_rule_3( validList,( void * )role1 )
            && clk_rule_1( validList,( void * )role2 )
              && clk_rule_2( validList,( void * )role2 )
                && clk_rule_3( validList,( void * )role2 ))
    {
      list_ins_next( validList,validList->tail,( void * )role1 );
	  list_ins_next( validList,validList->tail,( void * )role2 );
    }
    else
    {
      ///TODO:path invalid,exit this function;
      APP_TRACE_INFO(("add path[ %s ]failed!\r\n",clk->switch_name));
      return 0;
    }
  }


   APP_TRACE_INFO(("add path[ %s ]succeed!\r\n",clk->switch_name));

  return 1;
}


/*
*********************************************************************************************************
*                                               add_clk_switch_cfg()
*
* Description : get path index in data path name;
*
* Arguments   : path   : path name
* Returns       : index number
*
* Note(s)     : none.
*********************************************************************************************************
*/
int8_t add_clk_switch_cfg( FPGA_CLK_SWITCH *cfg , List *clkList )
{
  assert( NULL != cfg );
  
  int8_t ret = -1;
  if( 0 > find_clk_path( ( char * )fpga_i2s_clk_path,cfg ) )
    return ret;
  while( !match_clk_path( clkList,cfg ) )
    return ret ;
  if( i2s_clk_path_check( cfg , &fpga_i2s_clk_valid_list ) )
    list_ins_next( clkList,clkList->tail,cfg );
  else
    //TODO:add check result feedback to caller;
    return ret;
  return 0;  
}
/*
*********************************************************************************************************
*                                               add_data_switch_cfg()
*
* Description : get path index in data path name;
*
* Arguments   : path   : path name
* Returns       : index number
*
* Note(s)     : none.
*********************************************************************************************************
*/

void add_data_switch_cfg( FPGA_DATA_SWITCH *cfg,List *dataList )
{
  assert( NULL != cfg );

  while( !match_data_path( dataList,cfg ) )
    return;

  list_ins_next( dataList,dataList->tail,cfg );


}

/*
*********************************************************************************************************
*                                               destroy_clk_item()
*
* Description : destroy item in clock list;
*
* Arguments   : cfg   : clock item
* Returns       : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void destroy_clk_item( FPGA_CLK_SWITCH *cfg )
{
  assert( NULL != cfg );



}


/*
*********************************************************************************************************
*                                               destroy_data_item()
*
* Description : destroy item in data list;
*
* Arguments   : cfg   : data item
* Returns       : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void destroy_data_item( FPGA_DATA_SWITCH *cfg )
{
  assert( NULL != cfg );

}


/*
*********************************************************************************************************
*                                               destroy_role_item()
*
* Description : destroy item in role list;
*
* Arguments   : role   : role item
* Returns       : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void destroy_role_item( void *key )
{
  assert( NULL != key );

  ROLE *role = ( ROLE * )key;

  memset( role->name, 0 , sizeof( role->name ) );
  free( role );

}


/*
*********************************************************************************************************
*                                               reset_fpga()
*
* Description : reset fpga chip;
*
* Arguments   : none
* Returns       : none
*
* Note(s)       : none.
*********************************************************************************************************
*/

void reset_fpga( void )
{
  APP_TRACE_INFO(("Reset FPGA...\r\n"));
  UIF_Misc_Off ( FPGA_RST );
  UIF_DelayUs( 10 ) ;
  UIF_Misc_On ( FPGA_RST );
}


/*
*********************************************************************************************************
*                                               init_fpga()
*
* Description : reset fpga chip and associate data struct;
*
* Arguments   : none
* Returns       : none
*
* Note(s)       : none.
*********************************************************************************************************
*/
void init_fpga( )
{
  //TODO:what will do here
  reset_fpga( );
  APP_TRACE_INFO(("Init FPGA...\r\n"));
  //list_init( &fpga_i2s_clk_list , NULL );
  //list_init( &fpga_i2s_data_list , NULL);

  list_init( &xc3s50an.fpga_i2s_clk_list, NULL );
  list_init( &xc3s50an.fpga_i2s_data_list, NULL );
  list_init( &xc3s50an.clock_node_role, destroy_role_item );

  memset( ( void * )&xc3s50an.cmdWord, 0xff , sizeof( FPGA_COMMAND ) );
  memset( ( void * )&xc3s50an.data_cfg, 0 , sizeof( FPGA_DATA_SWITCH ) );
  memset( ( void * )&xc3s50an.clock_cfg, 0 , sizeof( FPGA_CLK_SWITCH ) );

}

/*
*********************************************************************************************************
*                                               init_fpga_instance()
*
* Description : initialize fpga chip instance;
*
* Arguments   : none
* Returns       : none
*
* Note(s)       : none.
*********************************************************************************************************
*/
void init_fpga_instance( void )
{

  //initialize public interface;
  xc3s50an.reset_fpga = reset_fpga;
  xc3s50an.init = init_fpga;
  xc3s50an.set_path = set_fpga_path;
  xc3s50an.get_path_cfg = NULL;

  //initialize private interface;
  xc3s50an.add_clk_switch_cfg = add_clk_switch_cfg;
  xc3s50an.add_data_switch_cfg = add_data_switch_cfg;
  xc3s50an.set_data_path = set_data_path;
  xc3s50an.set_clk_path = set_i2s_clk_path;


  xc3s50an.controller = &source_spi1; 

}

/*
*********************************************************************************************************
*                                               destroy_fpga_instance()
*
* Description : destroy fpga chip instance;
*
* Arguments   : none
* Returns       : none
*
* Note(s)       : none.
*********************************************************************************************************
*/
void destroy_fpga_instance( void )
{

  list_destroy( &xc3s50an.fpga_i2s_clk_list );
  list_destroy( &xc3s50an.fpga_i2s_data_list );
  list_destroy( &xc3s50an.clock_node_role );
  list_destroy( &fpga_i2s_clk_valid_list );
  //initialize public interface;
  memset( ( void * )&xc3s50an, 0 ,sizeof( FpgaChip ) );
  memset( ( void * )&xc3s50an.cmdWord, 0xff , sizeof( FPGA_COMMAND ) );

  reset_fpga( );
}




/*
*********************************************************************************************************
*                                               Init_fpga_clock_path()
*
* Description : initialize a new clock path into link ;
*
* Arguments   : clock_dir       : signal direct
*               clock_output_en : enable signal output
*               s_clock_path    £ºpath name
* Returns       : success/fail: 0/-1
*
* Note(s)       : none.
*********************************************************************************************************
*/
static int8_t Init_fpga_clock_path( unsigned char clock_dir, unsigned char clock_output_en, const char *s_clock_path )
{
    int32_t ret = -1;
    
    FPGA_CLK_SWITCH  *clock_path_cfg = ( FPGA_CLK_SWITCH * )malloc( sizeof( FPGA_CLK_SWITCH ) );
    if( NULL == clock_path_cfg )
      return ret;
    
    APP_TRACE_INFO(("set clock path: %s \r\n",s_clock_path));
    

    clock_path_cfg->dir = clock_dir;       //0: uif_portx -> others, 1: others -> uif_portx,
    clock_path_cfg->oe  = clock_output_en; //0: enable port output,  1: high-z port output
    memset( clock_path_cfg->switch_name , 0 , sizeof( clock_path_cfg->switch_name ) );
    memcpy( clock_path_cfg->switch_name, s_clock_path , 16 );
    
    ret = xc3s50an.add_clk_switch_cfg( clock_path_cfg, &xc3s50an.fpga_i2s_clk_list );
    
    if( ret != 0 )
      free( clock_path_cfg );
    
    return ret;

}


/*
*********************************************************************************************************
*                                               Init_fpga_data_path()
*
* Description : initialize a new clock path into link ;
*
* Arguments   : s_data_path : path name
*               
*               
* Returns       : none
*
* Note(s)       : none.
*********************************************************************************************************
*/
static void Init_fpga_data_path( const char *s_data_path )
{

    FPGA_DATA_SWITCH *p_data_path_cfg;
    APP_TRACE_INFO(("set data path: %s \r\n",s_data_path));

    p_data_path_cfg =  map_name_to_path( s_data_path );
    xc3s50an.add_data_switch_cfg( p_data_path_cfg, &xc3s50an.fpga_i2s_data_list );

}

/*******************************************************
  CLOCK Path:
  "port0_codec0_0",
  "port0_fm36_1",
  "port0_ssc0_2",
  "port0_port1_3",
  "port0_codec1_4",
  "port0_ssc1_5",
  "port1_codec1_6",
  "port1_ssc1_7",


  Data Path: 2 to 1 selector:
  "codec0_tx->fm36_i2s_rx",
  "uif_i2s0_rx->fm36_i2s_rx",

  "ssc0_tx->codec0_rx",
  "uif_i2s0_tx->codec0_rx",

  "codec1_tx->ssc1_rx",
  "uif_i2s1_rx->ssc1_rx",

  "ssc1_tx->codec1_rx",
  "uif_i2s1_rx->codec1_rx",

  "fm36_pdmo_data->uif_pdmo_data",
  "hdmi_pdmi_data->uif_pdmo_data",

  "uif_pdmi_data->fm36_pdmi_data",
  "hdmi_pdmi_data->fm36_pdmi_data",

  "fm36_pdmi_clk->hdmi_pdm_clk",
  "uif_pdmo_clk->hdmi_pdm_clk"
************************************************************/
unsigned char FPGA_Setup( void )   //?????
{
    unsigned char err;
  
    FPGA_COMMAND cmd;

    init_fpga();
    
    //test codec0 as master
    Init_fpga_clock_path( 0,0,"codec0_port0_0" );
    Init_fpga_clock_path( 0,0,"codec0_fm36_1" );
    Init_fpga_clock_path( 0,0,"codec0_ssc0_2" );
    Init_fpga_clock_path( 0,0,"codec0_port1_3" );
    Init_fpga_clock_path( 0,0,"codec0_codec1_4" );
    Init_fpga_clock_path( 0,0,"codec0_ssc1_5" );
   
    
    
    Init_fpga_data_path( "uif_i2s0_rx->fm36_i2s_rx" );
    Init_fpga_data_path( "ssc0_tx->codec0_rx" );
    Init_fpga_data_path( "ssc1_tx->codec1_rx" );
    Init_fpga_data_path( "fm36_pdmo_data->uif_pdmo_data" );
    Init_fpga_data_path( "fm36_pdmi_clk->hdmi_pdm_clk" );
    
    err = xc3s50an.set_path( &xc3s50an, 
                       &xc3s50an.cmdWord , 
                       &xc3s50an.fpga_i2s_clk_list, 
                       &xc3s50an.fpga_i2s_data_list );
    return err;

}
                  
                  