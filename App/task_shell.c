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
*                                           TASK PACKAGE
*
*                                          Atmel ATSAMA5D3X
*                                               on the
*                                      Audio Bridge 04 Board (AB04 V1.0)
*
* Filename      : task_shell.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#include <includes.h>


/*
*********************************************************************************************************
*                                    App_TaskGenieShell()
*
* Description : Realize a command shell interface on Debug_UART for real time debug purpose.
*
* Argument(s) : p_arg   Argument passed to 'App_TaskGenieShell()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Note(s)     : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void App_TaskGenieShell( void *p_arg )
{

    (void)p_arg;

    CPU_INT08U  index ;		/*index is the pointer of data in commandbuf */
    CPU_INT08U  num ;
    CPU_CHAR    ch ;
    CPU_CHAR    CommandBuf[ MaxLenComBuf + 1 ];	/*store '\0'*/
    CPU_INT08U  (*Func)(CPU_INT08U argc, CPU_CHAR **argv);
    CPU_CHAR    *argv[10];
    CPU_INT08U  argc;
    CPU_INT08U  error_code;
    CPU_INT08U  idx;
    
    index  =  0 ;
    cmd_loop_idx = 0;
    CommandBuf[0] = '\0';
    InitCommands();
    Init_CommandLoop();
    OSTimeDly(1000);

    /*To be done: Login & Password*/
    UART_SHELL_SEND_STR(( "\n\rLaunching Genieshell, press any to continue..."));
    UART_SHELL_GET_BYTE(());
    Time_Stamp();
    UART_SHELL_SEND_STR((">"));

    while (DEF_TRUE) {

        do {	    //only accept a-z,0-9,A-Z,.,space,/,-
	      ch = UART_SHELL_GET_BYTE(());

        } while(  !(   (ch>='0'&&ch<='9')||(ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')\
                     ||(ch=='.')||(ch==' ')||(ch=='-')||(ch=='/')\
                     ||(ch=='\r')||(ch=='\b')||(ch==',')  )  );

        switch(ch) {

            case '\r':				//enter
                if ( index == 0 ){     //commandbuf is null,begin a new line
                    Time_Stamp();
                    UART_SHELL_SEND_STR((">"));

                } else {

                    if(CommandBuf[index-1]==' ') {
                        index--;			//get rid of the end space
                    }                 
                    CommandBuf[index] = '\0';                     
                    idx = atoi(CommandBuf);
                    if( idx > 0 && idx <= MAX_COMMAND_NUM) { 
                        memcpy(CommandBuf, ShellComms[idx-1].name, strlen(ShellComms[idx-1].name));                     
                        index = strlen(ShellComms[idx-1].name);
                        CommandBuf[index] = '\0'; 
                        UART_SHELL_SEND_STR((" : %s",CommandBuf));
                        break;
                    }
                    //UART_SHELL_SEND_STR("\n\rThe command is %s",CommandBuf);
                    num = CommandParse( CommandBuf,&argc,argv );	//analys the argv in the commandbuf
                    if( num == ERRORCOMMAND ){             	//error or none exist command
                        index = 0;
                        CommandBuf[index] = '\0';
                        //UART_SHELL_SEND_STR("\n\rError command is %s",CommandBuf);
                        UART_SHELL_SEND_STR(("Error: bad command or filename."));
                        Time_Stamp();
                        UART_SHELL_SEND_STR((">"));

                    } else {
                        if( strcmp(CommandBuf,CommandBufLoop[cmd_loop_idx] ) != 0 ) {  //save cmd history
                            cmd_loop_idx++;
                            cmd_loop_idx %= MaxLenComBufLoop ;
                            memcpy(CommandBufLoop[cmd_loop_idx], CommandBuf, strlen(CommandBuf)+1);
                        }
                        
                        Func = ShellComms[num].CommandFunc;	//call corresponding CommandFunc
                        error_code = Func(argc,argv) ;
                        if( error_code == 1 ) {
                          UART_SHELL_SEND_STR(("Error : number of parameters error..."));

                        } else if(error_code == 2 ) {
                          UART_SHELL_SEND_STR(("Error : parameters content error ..."));

                        } else if(error_code == 3 ) {
                           UART_SHELL_SEND_STR(("Error : Function execution error ..."));

                        } else if(error_code > 3) {
                           UART_SHELL_SEND_STR(("Error : Unknown error ..."));

                        }
                        index = 0;
                        CommandBuf[index] = '\0';
                        UIF_LED_Toggle(LED_RUN);
                        Time_Stamp();
                        UART_SHELL_SEND_STR((">"));

                    }
                }
            break;

            case '\b':				//backspace
                if ( index==0 ){		//has backed to first one
                    //do nothing
                } else {
                    index--;			   //pointer back once
                    UART_SHELL_SEND_BYTE(('\b'));	//cursor back once
                    UART_SHELL_SEND_BYTE((' '));	      //earse last char in screen
                    UART_SHELL_SEND_BYTE(('\b'));		//cursor back again

                }
            break;

            case ' ':               //don't allow continuous or begin space(' ')
                if((CommandBuf[index-1] == ' ')||(index==0)||( index>MaxLenComBuf) ){
                    //do nothing
                } else {
                    CommandBuf[index] = ch;
                    index++;
                    UART_SHELL_SEND_BYTE((ch));  //display and store ch
                }
            break;

            default:				//normal key
                if ( index> MaxLenComBuf ){	//the buf reached MAX
                    index = 0;
                    CommandBuf[index] = '\0';
                    UART_SHELL_SEND_STR(("Error: command length overflow!"));
                    Time_Stamp();
                    UART_SHELL_SEND_STR((">"));
                //do nothing
                } else {
                    CommandBuf[index] = ch;
                    index++;
                    UART_SHELL_SEND_BYTE((ch));  //display and store ch
                }
            break;

	}  //switch end


    OSTimeDly(10);

    }


}




