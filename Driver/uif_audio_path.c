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


#include <includes.h>

AUDIOPATH g_audio_path;

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
*                                               getPathName()
*
* Description : trans index to path name 
*
* Arguments   : name   :path index
*            
*             
* Returns     : name
*
* Note(s)     : none.
*********************************************************************************************************
*/
char * getPathName( unsigned char index )
{
  
  unsigned char *p_path_name;
  if( index >= 8 ){
        APP_TRACE_INFO( ("invalid path index\r\n") );
        return NULL;
  }
  p_path_name = (unsigned char *)halfPath[index];
  
  return p_path_name;
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
      
      if( strcmp( pPath->name , name ) == 0 )
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
      
      p = strstr( pPath->name , pName );
      
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
* Note(s)     : Error.
*********************************************************************************************************
*/
extern void *malloc( uint32_t num_bytes );

unsigned char  createPath( void *source,
                  void *parameter
                          )
{
    unsigned char err;
    AUDIOPATH *path;
    int8_t     index = 0; 
    uint32_t   len   = strlen( halfPath[ 0 ] ) + 1;
    err = 0;
    
    assert( NULL != source );
    
    path = ( AUDIOPATH * )malloc( sizeof( AUDIOPATH ) );
    
    memset( path->name, 0 ,sizeof( path->name ) );     
    memcpy( path->name , source , len );
    
    index = getPathIndex( path->name );
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
   

    switch( index )
    {
      //up link usb<--ssc0/spi0/gpio          : Recording
        case 0:                            //ep1<-ssc0
          {
              //step1:install port;
              path->pSource = &source_ssc0;
              //step2:set initialize function;
              path->pSource->set_peripheral = ssc_rxRegister_set;
              //step3:install buffer;              
              path->pfifoIn  = &ssc0_bulkin_fifo;
              path->pfifoOut = &ep0BulkIn_fifo;
              //step4: install ep;
              path->ep = CDCDSerialDriverDescriptors_AUDIO_0_DATAIN;
          }   
          break;
        case 2:                          //ep5<-ssc1
          {
              path->pSource = &source_ssc1; 
              path->pSource->set_peripheral = ssc_rxRegister_set;
              path->pfifoIn = &ssc1_bulkin_fifo; 
              path->pfifoOut = &ep1BulkIn_fifo;             
              path->ep = CDCDSerialDriverDescriptors_AUDIO_1_DATAIN;              
          }
          break;
        case 4:                           //ep7<-spi0
          {
              path->pSource = &source_spi0;
//              path->pSource->set_peripheral = spi_register_set;
              path->pfifoIn = &spi0_bulkIn_fifo;
              path->pfifoOut = &ep2BulkIn_fifo;
              path->ep = CDCDSerialDriverDescriptors_SPI_DATAIN;              
          }
          break;
        case 6:                           //ep7<-gpio
          {
//              path->pInSource = &source_gpio; 
//              path->pInSource->set_peripheral = gpio_Init;
//              path->pUpfifoIn = &ssc0_bulkin_fifo; 
//              path->pUpfifoOut = &ep0BulkIn_fifo;
//              path->epIn = CDCDSerialDriverDescriptors_AUDIO_0_DATAIN;              
          }
          break;

    
          
          
          //down link usb<--ssc0/spi0/gpio             : Playing
        case 1:                            //ep2->ssc0
          {
              path->pSource = &source_ssc0;
              path->pSource->set_peripheral = ssc_txRegister_set;
              path->pfifoIn = &ep0BulkOut_fifo;          
              path->pfifoOut = &ssc0_bulkout_fifo;
              path->ep = CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT;
          }
          break;
        case 3:                          //ep6->ssc1
          {
              path->pSource = &source_ssc1;
              path->pSource->set_peripheral = ssc_txRegister_set;
              path->pfifoIn = &ep1BulkOut_fifo;           
              path->pfifoOut = &ssc1_bulkout_fifo;
              path->ep = CDCDSerialDriverDescriptors_AUDIO_1_DATAOUT;  
          }
          break;  
        case 5:                           //ep8->spi0
          {
              path->pSource = &source_spi0;
//              path->pSource->set_peripheral = spi_register_set;
              path->pfifoIn = &ep2BulkOut_fifo;           
              path->pfifoOut = &spi0_bulkOut_fifo; 
              path->ep = CDCDSerialDriverDescriptors_SPI_DATAOUT; 
          }
          break;
        case 7:                          //ep8->gpio
          {
//              path->pOutTarget = &source_gpio;
//              path->pOutTarget->set_peripheral = gpio_Init; 
//              path->pDownfifoIn  = &ep0BulkOut_fifo;          
//              path->pDownfifoOut = &ssc0_bulkout_fifo;  
//              path->epOut = CDCDSerialDriverDescriptors_AUDIO_0_DATAOUT;
          }          
          break;


        default:
          return AUD_CFG_AUDIOPATH_ERR ;         
          break;      
    }
    
    //step5: configure port sent registers ;
    if( path->pSource->set_peripheral != NULL ) { 
        err = path->pSource->set_peripheral( path->pSource, parameter );
    } else {
        err = AUD_CFG_AUDIOPATH_ERR ;
        APP_TRACE_INFO(("\nCan't configure port !\r\n"));
    }
    if( err != NO_ERR ) {
        return err;
    }
    
    //step6: enquen
    if( portsList.match( &portsList,path->name ) ) {
        list_ins_next( &portsList,portsList.tail,path );
    }else{
        APP_TRACE_INFO(("\nPath already in use !\r\n"));
        err = AUD_CFG_AUDIOPATH_ERR;
    } 
    
    return err;
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
    
      if( path->pSource->peripheral_stop != NULL ){
          path->pSource->peripheral_stop( path->pSource );
      }
      
      kfifo_reset( path->pfifoIn );
      kfifo_reset( path->pfifoOut );
      
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


/*
*********************************************************************************************************
*                                               xxx_Audio_Path()
*
* Description : audio path related functions;
*
* Arguments   :  
*             
*            
* Returns     : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void Init_Audio_Path( void )
{
    g_audio_path.createAudioPath  = createPath;
    g_audio_path.destroyAudioPath = destroyAllPath;
    g_audio_path.findPort         = findPort;   
    
    g_audio_path.destroyAudioPath( "any" );
  
}

void Destroy_Audio_Path( void )
{
   g_audio_path.destroyAudioPath( "any" );

}


unsigned char Add_Audio_Path( void *path_name, AUDIO_CFG *pAudioCfg )
{
   unsigned char err; 
   err = g_audio_path.createAudioPath(  path_name,  pAudioCfg );                        
   return err;   
}