#ifndef COMMANDDEF
#define COMMANDDEF

#include <cpu.h>



#define MAX_COMMAND_NUM            21
#define MAX_COMMAND_NAME_LENGTH    20
#define ERRORCOMMAND              255

#define MaxLenComBuf	          100
#define MaxLenComBufLoop	      10

typedef struct{
  
	CPU_INT08U  num;
	CPU_CHAR    *name;
	CPU_INT08U (*CommandFunc)(CPU_INT08U argc, CPU_CHAR **argv);
    CPU_CHAR    *help;
    
}SHELL_CMD;

	 

//typedef CPU_CHAR  (*pCommandBuf_t)[ MaxLenComBuf + 1 ];   
extern CPU_CHAR CommandBufLoop[][MaxLenComBuf + 1 ];
extern SHELL_CMD ShellComms[];

void InitCommands(void);
void Init_CommandLoop( void );
CPU_INT08U  CommandParse( CPU_CHAR *Buf, CPU_INT08U *p_argc, CPU_CHAR *argv[] );

CPU_INT08U HelpFunc(CPU_INT08U argc,CPU_CHAR **argv);
CPU_INT08U HostNameFunc(CPU_INT08U argc,CPU_CHAR **argv);
CPU_INT08U WriteGPIOFunc(CPU_INT08U argc,CPU_CHAR **argv);
CPU_INT08U InitI2CFunc(CPU_INT08U argc,CPU_CHAR **argv) ;
CPU_INT08U LsFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U TaskFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U RebootFunc(CPU_INT08U argc,CPU_CHAR **argv);
CPU_INT08U ReadDMFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U WriteDMFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U ReadPMFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U WritePMFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U WriteHostREGFunc( CPU_INT08U argc,CPU_CHAR **argv );	
CPU_INT08U ReadHostREGFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U WriteDSPREGFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U ReadDSPREGFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U WriteCMFunc( CPU_INT08U argc,CPU_CHAR **argv );	
CPU_INT08U ReadCMFunc( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U Get_Ver_Info( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U Write_Ruler_FW( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U Flash_Info( CPU_INT08U argc,CPU_CHAR **argv );
CPU_INT08U Cmd_History( CPU_INT08U argc,CPU_CHAR **argv );
#endif 