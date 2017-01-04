#ifndef _UIF_AUDIO_PATH_H_
#define _UIF_AUDIO_PATH_H_

#include <string.h>


#include "kfifo.h"
#include "defined.h"

#include "uif_i2s.h"
#include "uif_gpio.h"

                               
const static char* halfPath[]={
  
                                "ep1->ssc0",                                //0
                                "ep2<-ssc0",                                //1
                                
                                "ep5->ssc1",                                //2
                                "ep6<-ssc1",                                //3 
                                
                                "ep7->spi0",                                //4
                                "ep8<-spi0",                                //5
                                
                                "ep7->gpio",                                //6
                                "ep8<-gpio",                                //7
                    
                              };  



                                
typedef struct _audio_path
{
  char name[16];                             //input path

  DataSource  *pSource;                      //data from
  kfifo_t     *pfifoIn;                      //input ringbuffer handle
  kfifo_t     *pfifoOut;                     //output ringbuffer handle  

  uint32_t    state;                         //work/idle
  uint32_t    ep;                            //usb end point about this path,
  
  void ( *createAudioPath )( void *source,
                             void *inParameter
                            );
  
  void ( *destroyAudioPath )( char *pFullName );  
  void ( *startPath )( void *fullPath );
  void ( *stopPath  )( void *fullPath );
  int  ( *findPort  )( const void *pPath,const void *port );
  
}AUDIOPATH;

void createPath( void *source,
                 void *inParameter
                );

void destroyAllPath( char *pFullName );
void stratPath( void *path );
void stopPath( void *path );

//private interface
static void audio_path_register( char *name );
static void audio_path_unregister( char *name );
static void generateFullPathName( char *in, char *out ,char *full);
static int8_t getPathIndex( char *name );
static void path_set_buffer( char *pFullName, 
                             kfifo_t * in, 
                             kfifo_t * out );

int matchPath(const void *key1,const void *key2 );
int findPort( const void *pPath,const void *port );

char * getPathName( unsigned char index );
void Init_Audio_Path( void );   
void Destroy_Audio_Path( void ); 
void Add_Audio_Path( void *path_name, AUDIO_CFG *pAudioCfg );

#endif