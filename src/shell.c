#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "host.h"

#define MaxTask 20
static int nTask = 0; 
xTaskHandle xHandles[MaxTask];

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;


void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void log_command(int, char **);
void mmtest_command(int, char **);
void new_command(int, char **);
void kill_command(int, char **);
void test_command(int, char **);
void _command(int, char **);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(new, "create a new task"),
	MKCL(kill, "delete a task create by command:new"),
	MKCL(log, "create a new task to record log"),
	MKCL(test, "test new function"),
	MKCL(, ""),
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    int dir;
    if(n == 0){
        dir = fs_opendir("");
    }else if(n == 1){
        dir = fs_opendir(argv[1]);
        //if(dir == )
    }else{
        fio_printf(1, "Too many argument!\r\n");
        return;
    }
(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}
//--------------------
//new task to do
void vTaskCode(void *pParameter){
	portTickType xLastWakeTime = xTaskGetTickCount();
	const portTickType xFrequency = 200;
	int num = *((int*)pParameter);

	while(1){	
	    vTaskDelayUntil(&xLastWakeTime, xFrequency);
	    fio_printf(1, "Task%d: I'm executed \\OwO/\r\n",num);
	}
	
} 
//new
void new_command(int n,char *argv[]){
	configASSERT(nTask < MaxTask-1);
	static int  ucParameterToPass ;
	ucParameterToPass = nTask; //amount of task
	const signed char str1[4] = "TASK";
	const signed char * const str = str1;
	xTaskCreate(vTaskCode, str, 128, (void*)&ucParameterToPass , tskIDLE_PRIORITY , &xHandles[nTask]);
	configASSERT(xHandles[nTask]);
	nTask++;
}

//kill
void kill_command(int n,char *argv[]){
	configASSERT(nTask>0);
	nTask--;
	vTaskDelete(xHandles[nTask]);	
}
//-----------------------------
void log_rec(void *pParameter) {
    int handle;
    int error;
    portTickType xLastWakeTime = xTaskGetTickCount();
    const portTickType xFrequency = 200;

    fio_printf(1, "\r\n");
    
    handle = host_action(SYS_SYSTEM, "mkdir -p output");
    handle = host_action(SYS_SYSTEM, "touch output/syslog");
    while(1){
	vTaskDelayUntil(&xLastWakeTime, xFrequency); //LastWakeTime refresh
    	handle = host_action(SYS_OPEN, "output/syslog", 8);
    	if(handle == -1) {
        	fio_printf(1, "Open file error!\n\r");
        	return;
    	}
//--------
	signed char buf[512];
	char timeStamp[1024];
	sprintf(timeStamp,"%lu ms\n", xLastWakeTime);
	vTaskList(buf);
	const char* header = "\nName	State   Priority  Stack  Num"
			     "\n*******************************************\n";
	error = host_action(SYS_WRITE, handle, (void *)timeStamp, strlen((const char*)timeStamp));
	error = host_action(SYS_WRITE, handle, (void*)header, strlen(header));
	error = host_action(SYS_WRITE, handle, (void *)buf, strlen((const char*)buf));

    	if(error != 0) {
        	fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
        	host_action(SYS_CLOSE, handle);
        	return;
    	}
        host_action(SYS_CLOSE, handle);
    }
}
//-----------------------------------------------------------------------------
void log_command(int n,char *argv[]){
	xTaskHandle xLogHandle;
	const signed char str1[6] = "logger";
	const signed char * const str = str1;
	xTaskCreate(log_rec, str, 512, (void*)NULL , tskIDLE_PRIORITY , &xLogHandle);
	configASSERT(xLogHandle);
}
//----------------------------
//---------------------

void test_command(int n, char *argv[]) {
    /*int handle;
    int error;

    fio_printf(1, "\r\n");
    
    handle = host_action(SYS_SYSTEM, "mkdir -p output");
    handle = host_action(SYS_SYSTEM, "touch output/syslog");

    handle = host_action(SYS_OPEN, "output/syslog", 8);
    if(handle == -1) {
        fio_printf(1, "Open file error!\n\r");
        return;
    }

    char *buffer = "Test host_write function which can write data to output/syslog\n";
    error = host_action(SYS_WRITE, handle, (void *)buffer, strlen(buffer));
    if(error != 0) {
        fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
        host_action(SYS_CLOSE, handle);
        return;
    }*/
//-----------------------------------------------------------------------------
    if( n > 1){
	if(!strcmp( argv[1], "fib")){
	    //Fibonacci 
	    int i, previousA, previousB, SUM;
	    previousA = -1;
	    previousB = 1;
	    for(i=0;i<42;i++){
		SUM = previousA + previousB;
		previousA = previousB;
		previousB = SUM;
		fio_printf(1,"the %dth fibonacci number is %d\n\r",i,SUM);
	    }
	}
	else if(!strcmp( argv[1], "pri")){
	    //prime
	    int i, j, flag;
	    for(i=1;i<101;i++){
		flag = 0;
		for(j=2;j<i;j++){
		    if((i%j) == 0) flag = 1;
		}
		if(flag == 0) fio_printf(1,">>%d\n\r",i);
	    }
	}
	else{
	    //command list
	    fio_printf(1,"test fib ----show the first 41 fibonacci number\n\r");
	    fio_printf(1,"tese pri ----show the first 100 prime number\n\r");
	}
    }else{
	//command list
	fio_printf(1,"test fib ----show the first 41 fibonacci number\n\r");
	fio_printf(1,"tese pri ----show the first 100 prime number\n\r");
    }
//-----------------------------------------------------------------------------
    //host_action(SYS_CLOSE, handle);
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
