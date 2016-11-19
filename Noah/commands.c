
#include <includes.h>


SHELL_CMD ShellComms[ MAX_COMMAND_NUM ];


void InitCommands( void )
{
    SHELL_CMD *pShell = ShellComms ;
    unsigned char cmd_index = 1;
    
	pShell->num  = cmd_index++;
	pShell->name = "help";
	pShell->CommandFunc = HelpFunc;
    pShell->help = "Show descrptions on commands .";
    pShell++;
    
	pShell->num  = cmd_index++;
	pShell->name = "hostname";
	pShell->CommandFunc = HostNameFunc;
    pShell->help = "Print the title and argcs followed.";
    pShell++;
  
    pShell->num  = cmd_index++;
	pShell->name = "setgpio";
	pShell->CommandFunc = WriteGPIOFunc;
    pShell->help = "Set Noah defined GPIO.{cmd nGPIO state(0/1/2)} \n\reg: setgpio 1 0";  
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "initi2c";
	pShell->CommandFunc = InitI2CFunc;
    pShell->help = "Re-initialize board I2C port{cmd i2c_speed} \n\reg: initi2c 400";
    pShell++;
       
    pShell->num  = cmd_index++;
	pShell->name = "ls";
	pShell->CommandFunc = LsFunc;
    pShell->help = "List all command supported.";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "task";
	pShell->CommandFunc = TaskFunc;
    pShell->help = "List all task information.";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "reboot";
	pShell->CommandFunc = RebootFunc;
    pShell->help = "Reboot the MCU.";
    pShell++;
    
    
    
    pShell->num  = cmd_index++;
	pShell->name = "readdm";
	pShell->CommandFunc = ReadDMFunc;
    pShell->help = "Read DSP DM. {cmd address}\n\reg: readdm 0x2300";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "writedm";
	pShell->CommandFunc = WriteDMFunc;
    pShell->help = "Write DSP DM.{cmd address data} \n\reg: writedm 0x2300 0x5555";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "readpm";
	pShell->CommandFunc = ReadPMFunc;
    pShell->help = "Read DSP PM.{cmd address} \n\reg: readpm 0x1000";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "writepm";
	pShell->CommandFunc = WritePMFunc;
    pShell->help = "Write DSP PM.{cmd address data} \n\reg: writepm 0x1000 0x555555";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "readcm";
	pShell->CommandFunc = ReadCMFunc;
    pShell->help = "Read DSP CM. {cmd address}\n\reg: readcm 0x37FF";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "writecm";
	pShell->CommandFunc = WriteCMFunc;
    pShell->help = "Write DSP CM.{cmd address data} \n\reg: writecm 0x3800 0x5555";
    pShell++;
   
    pShell->num  = cmd_index++;
	pShell->name = "readdsp";
	pShell->CommandFunc = ReadDSPREGFunc;
    pShell->help = "Read DSP register.{cmd address} \n\reg: readdsp 0x20";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "writedsp";
	pShell->CommandFunc = WriteDSPREGFunc;
    pShell->help = "Write DSP register.{cmd address data} \n\reg: writedsp 0x20 0x55\n\reg: writedsp 0x30 0x55aa";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "readhost";
	pShell->CommandFunc = ReadHostREGFunc;
    pShell->help = "Read Host register.{cmd address} \n\reg: readhost 0x80";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "writehost";
	pShell->CommandFunc = WriteHostREGFunc;
    pShell->help = "Write Host register.{cmd address data} \n\reg: writehost 0x20 0x55\n\reg: writehost 0x30 0x55aa";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "ver";
	pShell->CommandFunc = Get_Ver_Info;
    pShell->help = "List MCU firmware version info.";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "rulerfw";
	pShell->CommandFunc = Write_Ruler_FW;
    pShell->help = "Update ruler firmware. {cmd ruler_id}\n\reg: rulerfw 0";
    pShell++;
    
    pShell->num  = cmd_index++;
	pShell->name = "flash";
	pShell->CommandFunc = Flash_Info;
    pShell->help = "List flash saved data info.";
    pShell++;
}


CPU_INT08U LsFunc( CPU_INT08U argc,CPU_CHAR **argv )
{
   
    CPU_INT08U i;
    
    UART_SHELL_SEND_STR(("\n\rCommand list :"));	    
    for(i=0;i<MAX_COMMAND_NUM;i++){
        UART_SHELL_SEND_STR(("\n\r %2d. %s",ShellComms[i].num,ShellComms[i].name));
            
    } 
    return 0;
    
}


CPU_INT08U TaskFunc( CPU_INT08U argc,CPU_CHAR **argv )
{   
    CPU_INT32U temp ;
    
    temp = 1;
    if(argc == 1)  {  
        temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
        temp =  temp % 10 ;        
    }
    
    Head_Info(); 
    
    Buzzer_OnOff(1); //buzzer on   
    OSTimeDly(10 * temp);  
    Buzzer_OnOff(0); //buzzer off
    OSTimeDly(30* temp);
    Buzzer_OnOff(1); //buzzer on   
    OSTimeDly(10* temp);  
    Buzzer_OnOff(0); //buzzer off
    
    return 0;    
}



CPU_INT08U HelpFunc( CPU_INT08U argc,CPU_CHAR **argv )
{
  
    CPU_INT08U i;
    
    if(argc==0){ 
      
        UART_SHELL_SEND_STR(("Genieshell only support the following command now. If you wanna use \n\r more functions, pls contact PQ, thx !"));	
	LsFunc( 0,NULL );
        UART_SHELL_SEND_STR(("\n\r\n\rFor detailed info, pls use command: help [command]\n\r"));	
        UART_SHELL_SEND_STR(("e.g.: help readdm\n\r"));	
        
    } else {
      
        for( i=0; i<MAX_COMMAND_NUM; i++){           
            if(Str_CmpIgnoreCase(argv[0],ShellComms[i].name) == 0 ){               
                break; //find the command number
            }	
        }						
	    if (i == MAX_COMMAND_NUM ) {
            return 2; //not find it
        }
	    UART_SHELL_SEND_STR((ShellComms[i].help));	
        
    }
    
    return 0;
    
}



CPU_INT08U HostNameFunc( CPU_INT08U argc,CPU_CHAR **argv )
{
  
    CPU_INT08U i ;
    UART_SHELL_SEND_STR(("Hostname is Genies ^_<  : "));	
    
    if(argc>0){      
        for(i=0;i<argc;i++){
            UART_SHELL_SEND_STR(("\n\r"));
            UART_SHELL_SEND_STR((argv[i]));	            
        }
    }
    return 0;
    
}




CPU_INT08U WriteGPIOFunc(CPU_INT08U argc,CPU_CHAR **argv)
{
    CPU_INT32U temp1, temp2;    

//    if(argc != 2)  {
//        return 1 ;
//    }    
//    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 10) ;
//    temp2 =  Str_ParseNbr_Int32U (argv[1],argv, 10) ;   
//      
//    if( temp1 <= 32 && temp2 <= 2) {  
//      
//        if(temp2 == 2) {          
//            GPIODIR_FLOAT( temp1  ) ;
//            UART_SHELL_SEND_STR(("\n\rSet GPIO %d to Float",temp1));
//            
//        } else {
//            GPIOPIN_Set(temp1  , temp2 );
//            UART_SHELL_SEND_STR(("\n\rSet GPIO %d = %d",temp1,temp2));
//        }       
//        
//    } else {
//      return 2 ;
//      
//    }  
//    
    return 0;
 
}


CPU_INT08U InitI2CFunc(CPU_INT08U argc,CPU_CHAR **argv)
{
    CPU_INT32U temp1; 

//    if(argc != 1)  {
//        return 1 ;
//    }    
//    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;      
//     
//    if( temp1 <= 400 && temp1 >= 10) { 
//        TWI_Init( temp1 * 1000 );     
//        UART_SHELL_SEND_STR(("\n\rI2C port is re-initialized to %d kHz",temp1));     
//        
//    } else {
//      return 2 ;
//      
//    }     
    return 0;
 
}


CPU_INT08U RebootFunc(CPU_INT08U argc,CPU_CHAR **argv)
{
     
    UART_SHELL_SEND_STR((">>User cmd reset triggered... \r\n"));     
    UART_SHELL_SEND_STR((">>That's all folks !\r\n\r\n>> Rebooting...\r\n"));      
    OSTimeDly(1000);
    Beep(3); 
    //AT91C_BASE_RSTC->RSTC_RCR = MCU_SW_RESET_PATTERN ; // do reset processor and peripherals
    //while(1);
    
    return 0;
 
}         



/////////////////////     DSP debug use     ////////////////////////////////////


CPU_INT08U ReadDMFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT16U data ;
    CPU_INT32U temp ;    
 
    if(argc != 1)  {
        return 1 ;
    }    
    temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    if( temp <= 0xFFFF) {  
        address =  temp ;    
        errcode =  DM_LegacyRead(0xC0, address,(unsigned char *)&data);    
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rRead DM[0x%X] = 0x%X",address,data));	           	
        } else {
            return 3 ;
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
    
}


CPU_INT08U WriteDMFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT16U data ;
    CPU_INT32U temp1, temp2;
    
    errcode  = 0 ;
    if(argc != 2)  {
        return 1 ;
    }    
    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    temp2 =  Str_ParseNbr_Int32U (argv[1],argv, 16) ;
    if( temp1 <= 0xFFFF && temp2 <= 0xFFFF) {  
        address =  temp1 ;   
        data    =  temp2 ;
        errcode =  DM_SingleWrite(0xC0,  address, data);      
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rWrite DM[0x%X] = 0x%X",address,data));	
           	
        }  else {   
            return 3 ;   
            
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
}


 
CPU_INT08U ReadCMFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT16U data ;
    CPU_INT32U temp ;    
 
    if(argc != 1)  {
        return 1 ;
    }    
    temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    if( temp <= 0xFFFF) {  
        address =  temp ;    
        errcode = CM_LegacyRead( 0xC0, address, (unsigned char *)&data);      
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rRead CM[0x%X] = 0x%X",address,data));	           	
        } else {
            return 3 ;
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
    
}


CPU_INT08U WriteCMFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT16U data ;
    CPU_INT32U temp1, temp2;
    
    errcode  = 0 ;
    if(argc != 2)  {
        return 1 ;
    }    
    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    temp2 =  Str_ParseNbr_Int32U (argv[1],argv, 16) ;
    if( temp1 <= 0xFFFF && temp2 <= 0xFFFF) {  
        address =  temp1 ;   
        data    =  temp2 ;
        errcode =  CM_SingleWrite( 0xC0, address, (unsigned char *)&data);      
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rWrite CM[0x%X] = 0x%X",address,data));	
           	
        }  else {   
            return 3 ;   
            
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
}

 
CPU_INT08U ReadPMFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT32U data ;
    CPU_INT32U temp ;
    
    errcode  = 0 ;
    if(argc != 1)  {
        return 1 ;
    }    
    temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    if( temp <= 0xFFFF) {  
        address =  temp ;    
        errcode =   PM_LegacyRead(0xC0, address,  (CPU_INT08U *)&data ) ;    
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rRead PM[0x%X] = 0x%X",address,data));	
           	
        } else {
            return 3 ;
      
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
    
}


CPU_INT08U WritePMFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT32U data ;
    CPU_INT32U temp1, temp2;
    
    errcode  = 0 ;
    if(argc != 2)  {
        return 1 ;
    }    
    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    temp2 =  Str_ParseNbr_Int32U (argv[1],argv, 16) ;
    if( temp1 <= 0xFFFF && temp2 <= 0xFFFFFF) {  
        address =  temp1 ;   
        data    =  temp2 ;          
        errcode = PM_SingleWrite(0xC0, address,  (CPU_INT08U *)&data,0) ;
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rWrite PM[0x%X] = 0x%X",address,data));	
           	
        } else {
            return 3 ;
      
        }
        
    } else {
      return 2 ;
      
    }
    
	return 0;
}



CPU_INT08U ReadDSPREGFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT08U address ;
    CPU_INT16U data ;
    CPU_INT32U temp ;
    
    errcode  = 0 ;
    if(argc != 1)  {
        return 1 ;
    }    
    temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    if( temp <= 0xFF) {  
        address =  temp ;       
        errcode =   DSP_LegacyRead(0xC0, address,(CPU_INT08U *)&data) ;
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rRead DSP REG[0x%X] = 0x%X",address,data));	
           	
        } else {
            return 3 ;
      
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
    
}



CPU_INT08U WriteDSPREGFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT16U data ;
    CPU_INT32U temp1, temp2;
    
    errcode  = 0 ;
    if(argc != 2)  {
        return 1 ;
    }    
    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    temp2 =  Str_ParseNbr_Int32U (argv[1],argv, 16) ;
    
    if( temp1 <= 0xFF && temp2 <= 0xFFFF) {  
        address =  temp1 ;   
        data    =  temp2 ; 
        if( data&0xFF00 ) {
            errcode = DSP_SingleWrite_2(0xC0, address, data) ;
        } else {
            errcode = DSP_SingleWrite_1(0xC0, address, (CPU_INT08U)data) ;
        }
        
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rWrite DSP REG[0x%X] = 0x%X",address,data));	
           	
        } else {
            return 3 ;
      
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
    
}



CPU_INT08U ReadHostREGFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    CPU_INT08U errcode  ;
    CPU_INT08U address  ;
    CPU_INT16U data ;
    CPU_INT32U temp ;
    
    errcode  = 0 ;
    if(argc != 1)  {
        return 1 ;
    }    
    temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    if( temp <= 0xFF) {  
        address =  temp ;       
        errcode =   HOST_LegacyRead(0xC0, address,(CPU_INT08U *)&data) ;
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rRead Host REG[0x%X] = 0x%X",address,data));	
           	
        } else {
            return 3 ;
      
        }
        
    } else {
      return 2 ;
      
    }
    return 0;
    
}



CPU_INT08U WriteHostREGFunc( CPU_INT08U argc,CPU_CHAR **argv )	
{
    
    CPU_INT08U errcode  ;
    CPU_INT16U address ;
    CPU_INT16U data ;
    CPU_INT32U temp1, temp2;
    
    errcode  = 0 ;
    if(argc != 2)  {
        return 1 ;
    }    
    temp1 =  Str_ParseNbr_Int32U (argv[0],argv, 16) ;
    temp2 =  Str_ParseNbr_Int32U (argv[1],argv, 16) ;
    
    if( temp1 <= 0xFF && temp2 <= 0xFFFF) {  
        address =  temp1 ;   
        data    =  temp2 ; 
        if( data&0xFF00 ) {
            errcode = HOST_SingleWrite_2(0xC0, address, data) ;
        } else {
            errcode = HOST_SingleWrite_1(0xC0, address, (CPU_INT08U)data) ;
        }
        
        if(errcode == 0) {
            UART_SHELL_SEND_STR(("\n\rWrite Host REG[0x%X] = 0x%X",address,data));	
           	
        } else {
            return 3 ;
      
        }
        
    } else {
      return 2 ;
      
    }
    
    return 0;
}

CPU_INT08U Write_Ruler_FW( CPU_INT08U argc,CPU_CHAR **argv )	
{   
    CPU_INT08U errcode  ;
    CPU_INT32U temp;
    
    errcode  = 0 ;
    if(argc != 1)  {
        return 1 ;
    }    
    
    temp =  Str_ParseNbr_Int32U (argv[0],argv, 16) ; 
    
    if( temp <= 3 ) {      
        errcode = Update_Ruler_FW( temp );        
        if(errcode != 0) {           
            return 3 ;
        }   
        
    } else {
        
      return 2 ;
      
    }
    
	return 0;
}


CPU_INT08U Get_Ver_Info( CPU_INT08U argc,CPU_CHAR **argv )	
{
    
    UART_SHELL_SEND_STR(("\n\rHost  MCU Version : %s",fw_version));
    UART_SHELL_SEND_STR(("\n\rAudio MCU Version : %s",Audio_Version));	 
    
    return 0;
}



CPU_INT08U Flash_Info( CPU_INT08U argc,CPU_CHAR **argv )
{   
 
    Get_Flash_Info();
    Buzzer_OnOff(1); //buzzer on   
    OSTimeDly(10 );  
    Buzzer_OnOff(0); //buzzer off
    OSTimeDly(30);
    Buzzer_OnOff(1); //buzzer on   
    OSTimeDly(10);  
    Buzzer_OnOff(0); //buzzer off
    
    return 0;    
}


CPU_INT08U CommandParse( CPU_CHAR *Buf, CPU_INT08U *p_argc, CPU_CHAR *argv[] )
{
  
	CPU_INT08U i;
	CPU_INT08U pointer;
	CPU_INT08U num;
        CPU_INT08U argc;
	CPU_CHAR   name[MAX_COMMAND_NAME_LENGTH+1];		//SHELL_CMD name length <20

	argc    = 0;            
	pointer = 0;
	num     = 0;
    
	UART_SHELL_SEND_STR(("\n\r"));

	while((Buf[pointer]!=' ') && (Buf[pointer]!='\0') && (pointer < MAX_COMMAND_NAME_LENGTH) ) {
		name[pointer]=Buf[pointer];
		pointer++;
	}    
	name[pointer] = '\0';	//now got the command name, and pointer is to the first space in the Buf
        pointer = 0;
    
	for( i=0; i<MAX_COMMAND_NUM; i++){
		//if(!strcmp(name,ShellComms[i].name)){
        if(Str_CmpIgnoreCase(name,ShellComms[i].name) == 0 ){
			num = i;
			break; //find the command number
		}	
	}						
	if (i == MAX_COMMAND_NUM ) return ERRORCOMMAND; //not find it
					
	while( Buf[pointer]!='\0') {
      
		if(Buf[pointer]==' ') {
			if(argc>0){
				Buf[pointer] = '\0';			//end of last argv
			}
			pointer++;
			argv[argc] = &Buf[pointer];			//add a parameter for every space
			argc++;
            
		} else {          
			pointer++;
            
		}
	}//while
    
        *p_argc = argc ;    
	return ( num );
    
}