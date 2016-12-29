/*
*********************************************************************************************************
*
*                                          APP PACKAGE
*
*                                         Atmel  AT91SAMA5D3
*                                             on the
*                                      Audio Bridge 04 Board (AB04 V1.0) 2.0
*
* Filename      : uif_audio_path.c
* Version       : V0.0.1
* Programmer(s) : Leo
*********************************************************************************************************
* Note(s)       :Create audio data path using path name according user command;
*********************************************************************************************************
*/


//#include "uif_audio_path.h"
//#include "uif_list.h"

#include <includes.h>


//private interface 
/*
*********************************************************************************************************
*                                               generateFullPathName()
*
* Description : assemble data path  
*
* Arguments   : in   :data in  path
*             : out  :data out path
*             : full :full path 
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void generateFullPathName( char *in, char *out ,char *full )
{
  strcat( full, in );
  strcat( full, ":");
  strcat( full, out );
}

/*
*********************************************************************************************************
*                                               getPathIndex()
*
* Description : trans path name to index 
*
* Arguments   : name   :path name
*            
*             
* Returns     : index
*
* Note(s)     : none.
*********************************************************************************************************
*/
static int8_t getPathIndex( char *name )
{
  assert( NULL != name );
  
  uint8_t i = 0;
  uint8_t cnt = sizeof( halfPath ) / sizeof( halfPath[ 0 ] );
  
  for( i = 0 ; i < cnt ; i ++ )
  {
     if( strcmp( name ,halfPath[ i ] ) == 0 )
     {
       return i ;
     }
  }
  APP_TRACE_INFO( ("%s is invalid path\r\n",name) );
  return -1;
}


/*
*********************************************************************************************************
*                                               match()
*
* Description : compare path object
*
* Arguments   : key1   :path name
*             : key2   :path name
*             
* Returns     : 0:equal 1:!equal
*
* Note(s)     : none.
*********************************************************************************************************
*/
int matchPath( const void *key1,const void *key2 )
{
  assert(( NULL != key1 ) && ( NULL != key2 ) );

  List *l = ( List * )key1;
  
  if( l->size == 0 )
    return 1;
  
  ListElmt *e = l->head;
  
  char *name = ( char * )key2;
  
  while( NULL != e )
  {
      AUDIOPATH * pPath = ( AUDIOPATH * )e->data;
      
      if( strcmp( pPath->fullPathName , name ) == 0 )
        return 0;
      else
        e = e->next;
  }
  return 1;
}

/*
*********************************************************************************************************
*                                               findPort()
*
* Description : compare port object
*
* Arguments   : pPath   : path list
*             : name¡¡  :port name
*             
* Returns     : 0:equal 1:!equal
*
* Note(s)     : none.
*********************************************************************************************************
*/
int findPort( const void *pPath,const void *port )
{
  assert(( NULL != pPath ) && ( NULL != port ) );
  
  List *l = ( List * )pPath;  
  if( l->size == 0 )
    return 1;
  
  ListElmt *e = l->head;
  
  char *pName = ( char * )port;
  char *p = NULL;
  
  while( NULL != e )
  {
      AUDIOPATH * pPath = ( AUDIOPATH * )e->data;
      
      p = strstr( pPath->fullPathName , pName );
      
      if(  p != NULL )
        return 0;
      else
        e = e->next;
  }
  return 1;
}

/*
*********************************************************************************************************
*                                               createPath()
*
* Description : create audio data path
*
* Arguments   : source       :path name
*             : inParameter  :input node config parameter
*             : target       :path name
*             : outParameter :output node config parameter
*
* Returns     : index
*
* Note(s)     : none.
*********************************************************************************************************
*/
extern void *malloc( uint32_t num_bytes );

void  createPath( void *source,
                  void *inParameter,
                  void *target,
                  void *outParameter
                          )
{
    AUDIOPATH *path;
    int8_t index = 0; 
    uint32_t len = strlen( halfPath[ 0 ] ) + 1;
    
    assert(( NULL != source ) && ( NULL != target ) );
    
    path = ( AUDIOPATH * )malloc( sizeof( AUDIOPATH ) );
    
    memset( path->inHalfName, 0 ,sizeof( path->inHalfName ) );
    memset( path->outHalfName, 0 ,sizeof( path->outHalfName ) ); 
    memset( path->fullPathName , 0 , sizeof( path->fullPathName ) );
    
    memcpy( path->inHalfName , source , len );
    memcpy( path->outHalfName , target , len );    
    generateFullPathName( path->inHalfName, path->outHalfName, path->fullPathName );
    
    index = getPathIndex( path->inHalfName );
    assert( index != -1 );
    
    /*step0 : check the ports state and shut down them;
    if( ( path->pInSource->status[ 0 ] != ( uint8_t )STOP )
          || ( path->pInSource->status[ 1 ] != ( uint8_t )STOP ) )
    {
       if( path->pInSource->peripheral_stop != NULL )
          path->pInSource->peripheral_stop( path->pInSource );                 
    }
    
    if( ( path->pOutTarget->status[ 0 ] != ( uint8_t )STOP )
          || ( path->pOutTarget->status[ 1 ] != ( uint8_t )STOP ) )      
    {
       if( path->pOutTarget->peripheral_stop != NULL )
          path->pOutTarget->peripheral_stop( path->pInSource );                 
    }
    */
   
    //up link usb<--ssc0/spi0/gpio
    switch( index )
    {
        case 0:                            //ep1<-ssc0
          {
              //step1:install port;
              path->pInSource = &source_ssc0;
              //step2:set initialize function;
              path->pInSource->set_peripheral = ssc_rxRegister_set;
              //step3:install buffer;              
              path->pUpfifoIn = &ssc0_bulkin_fifo;
              path->pUpfifoOut = &ep0BulkIn_fifo;
              //step4: install ep;
              path->epIn = CDCDSerialDriverDescriptors_AUDIO_0_DATAIN;
          }   
          break;
        case 2:                           //ep7<-spi0
          {
              path->pInSource = &source_spi0;
              path->pInSource->set_peripheral = spi_register_set;
              path->pUpfifoIn = &spi0_bulkIn_fifo;
              path->pUpfifoOut = &ep2BulkIn_fifo;
              path->epIn = CDCDSerialDriverDescriptors_SPI_DATAIN;              
          }
          break;
        case 4:                           //ep7<-gpio
          {
//              path->pInSource = &source_gpio; 
//              path->pInSource->set_peripheral = gpio_Init;
//              path->pUpfifoIn = &ssc0_bulkin_fifo; 
//              path->pUpfifoOut = &ep0BulkIn_fifo;
//              path->epIn = CDCDSerialDriverDescriptors_AUDIO_0_DATAIN;              
          }
          break;
        case 6:                          //ep5<-ssc1
          {
              path->pInSource = &source_ssc1; 
              path->pInSource->set_peripheral = ssc_rxRegister_set;
              path->pUpfifoIn = &ssc1_bulkin_fifo; 
              path->pUpfifoOut = &ep1BulkIn_fifo;             
              path->epIn = CDCDSerialDriverDescriptors_AUDIO_1_DATAIN;              
          }
          break;

        default:
          break; 
    }  
    
    //step4:configure port received registers ;
    AUDIO_CFG *reg = ( AUDIO_CFG * )inParameter;
    if( path->pInSource->set_peripheral != NULL )  
            path->pInSource->set_peripheral( path->pInSource,inParameter )
      ;
    else
           APP_TRACE_INFO(("\nCann't configure port !\r\n"));

    index = getPathIndex( path->outHalfName );
    assert( index != -1 );
    
    //down link usb<--ssc0/spi0/gpio
    switch( index )
    {
        case 1:                            //ep2->ssc0
          {
              path->pOutTarget = &source_ssc0;
              path->pOutTarget->set_peripheral = ssc_txRegister_set;
              path->pDownfifoIn = &ep0BulkOut_fifo;          
              path->pDownfifoOut = &ssc0_bulkout_fifo;
              path->epOut = CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT;
          }
          break;
        case 3:                           //ep8->spi0
          {
              path->pOutTarget = &source_spi0;
              path->pOutTarget->set_peripheral = spi_register_set;
              path->pDownfifoIn = &ep2BulkOut_fifo;           
              path->pDownfifoOut = &spi0_bulkOut_fifo; 
              path->epOut = CDCDSerialDriverDescriptors_SPI_DATAOUT; 
          }
          break;
        case 5:                          //ep8->gpio
          {
//              path->pOutTarget = &source_gpio;
//              path->pOutTarget->set_peripheral = gpio_Init; 
//              path->pDownfifoIn  = &ep0BulkOut_fifo;          
//              path->pDownfifoOut = &ssc0_bulkout_fifo;  
//              path->epOut = CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT;
          }          
          break;
        case 7:                          //ep6->ssc1
          {
              path->pOutTarget = &source_ssc1;
              path->pOutTarget->set_peripheral = ssc_txRegister_set;
              path->pDownfifoIn = &ep1BulkOut_fifo;           
              path->pDownfifoOut = &ssc1_bulkout_fifo;
              path->epOut = CDCDSerialDriverDescriptors_AUDIO_1_DATAOUT;  
          }
          break;

        default:
          break;      
    }
    
    //step5: configure port sent registers ;
    if( path->pOutTarget->set_peripheral != NULL )  
           path->pOutTarget->set_peripheral( path->pOutTarget,outParameter )
             ;

    else
           APP_TRACE_INFO(("\nCan't configure port !\r\n"));
    
    //step6: enquen
    if( portsList.match( &portsList,path->fullPathName ) )
        list_ins_next( &portsList,portsList.tail,path );
    else
        APP_TRACE_INFO(("\nPath already in use !\r\n"));
            
    return;
}

/*
*********************************************************************************************************
*                                               destroyPath()
*
* Description : destroy audio data path according full path name;
*
* Arguments   : pFullName    : full path name
*             
*            
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void destroyAllPath( char *pFullName )
{
//    assert( NULL != pFullName );
    pFullName  = pFullName;
    
    while( portsList.size > 0 )
    {
      
      AUDIOPATH *path;
    
      list_rem_next( &portsList , NULL ,( void ** )&path );
    
      if( path->pInSource->peripheral_stop != NULL )
          path->pInSource->peripheral_stop( path->pInSource );
    
      if( path->pOutTarget->peripheral_stop != NULL )
          path->pOutTarget->peripheral_stop( path->pOutTarget );
      
      free( path );
    }
}

/*
*********************************************************************************************************
*                                               stratPath()
*
* Description : start audio data path according full path name;
*
* Arguments   : pFullName    : full path name
*             
*            
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void stratPath( void *path )
{
    assert( NULL != path );
}

/*
*********************************************************************************************************
*                                               stopPath()
*
* Description : stop audio data path according full path name;
*
* Arguments   : pFullName    : full path name
*             
*            
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void stopPath( void *path )
{
  assert( NULL != path );
}