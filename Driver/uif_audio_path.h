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
                                "ep1->spi0",                                //2
                                "ep2<-spi0",                                //3
                                "ep1->gpio",                                //4
                                "ep2<-gpio",                                //5
                                "ep5->ssc1",                                //6
                                "ep6<-ssc1",                                //7
                                "ep3->ssc1",                                //6
                                "ep4<-ssc1",                                //7                                
               //                 "ssc0->ssc1",                               //8
               //                 "ssc1<-ssc0",                               //9
                                "ssc1->ssc0",                               //10
                                "ssc0<-ssc1",                               //11
                                "spi0->ssc0",                               //12
                                "ssc0<-spi0",                               //13
                                "ssc0->ssc1",                               //14
                                "ssc1<-ssc0",                               //15
                              };


typedef struct _audio_path
{
  char fullPathName[32];                     //path name
  char inHalfName[16];                       //input path
  char outHalfName[16];                      //output path
  DataSource  *pInSource;                    //data from
  DataSource  *pOutTarget;                   //data to
  kfifo_t     *pfifoIn;                      //input ringbuffer handle
  kfifo_t     *pfifoOut;                     //output ringbuffer handle
  uint32_t    state;                         //work/idle
  uint32_t    epIn;                          //usb end point about this path,
                                             //if not use, ep = 0xff;
  uint32_t    epOut;                         //usb end point about this path,  
                                             //if not use, ep = 0xff;
      
//  List *data_path;                        //path list that in used;

  
  void ( *createAudioPath )( void *source,
                             void *inParameter,
                             void *target,
                             void *outParameter
                            );
  
  void ( *destroyAudioPath )( char *pFullName );  
  void ( *startPath )( void *fullPath );
  void ( *stopPath  )( void *fullPath );
  int  ( *findPort  )( const void *pPath,const void *port );
  
}AUDIOPATH;
//

void createPath( void *source,
                 void *inParameter,
                 void *target,
                 void *outParameter
                );

void destroyPath( char *pFullName );
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
#endif