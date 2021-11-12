#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <errno.h>


#define NUM 1024
/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT "admin@Ka_Firewall#"
#define MAX_ARGC 10

#define MAX_CMD_LENGTH 256
#define MAX_CMD_NUM 50
#define debug_print() 
/* Buffer size for input from the Console */
#define CONFIG_SYS_CBSIZE		256
#define CHTOKA "dlka@kfyh"

const char *history_file=".test_history";

char History[NUM] = "./history_file"; // history_dir/history_file

int  Exit;

int cmd_num_current = 0;

typedef void (*cmdFun)(char *)  ;

typedef struct{

    char name[MAX_CMD_LENGTH];/* Command Name */ 
    char usage[NUM];/* Usage message */ 
    int8_t hide;
    void (*cmdFun)(char *);/* Command execute function */ 

}CmdTbl;

int test_exit();

int command_common();

CmdTbl command_list[NUM];


//去除尾部空格
char *rtrim(char *str)
{
	if (str == NULL || *str == '\0')
	{
		return str;
	}
 
	int len = strlen(str);
	char *p = str + len - 1;
	while (p >= str  && isspace(*p))
	{
		*p = '\0';
		--p;
	}
 
	return str;
}
 
//去除首部空格
char *ltrim(char *str)
{
	if (str == NULL || *str == '\0')
	{
		return str;
	}
 
	int len = 0;
	char *p = str;
	while (*p != '\0' && isspace(*p))
	{
		++p;
		++len;
	}
 
	memmove(str, p, strlen(str) - len + 1);
 
	return str;
}
 
//去除首尾空格
char *trim(char *str)
{
	str = rtrim(str);
	str = ltrim(str);
	
	return str;
}



int test_exit()

{

    Exit=1;

    return 0;

}

void HelpCmdExeFun(char *cmdstr)
{
    debug_print() ;
 
	int i = 0;
	printf("Ka bash, version 1.3.1(1)\nThese shell commands are defined internally.  Type `help' to see this list.\nType `name --help' to find out more about the function `name'\n\n");
	while (i < cmd_num_current)
	{
        if(!command_list[i].hide)
		    printf("[%-32s]  -- %s\n", command_list[i].name,  command_list[i].usage);
		i++;
	}

	return; 
}

void chtouser_fun(char *cmdstr)
{
    debug_print() ;
    int len = strlen(CHTOKA);
    char *p = cmdstr + len -2;
    *p ='s';
    *(p+1) = 'u';
   // printf("p:'%s'\n",p);
    memcpy(cmdstr, p, strlen(p)+1);
   // printf(" new cmdstr:'%s'\n",cmdstr);
    system(cmdstr);
		
	return; 
}

void system_fun(char *cmdstr)
{
    debug_print() ;

    system(cmdstr);
		
	return; 
}
 
void setenv_fun(char *cmdstr)
{
    debug_print() ;
 
	int i = 0;
	
	while (i < cmd_num_current) 
	{
		if (setenv_fun == command_list[i].cmdFun)
		{
			printf("%s -- %s\n", command_list[i].name,  command_list[i].usage);
			break;
		}
		
		i++;
	}
 
		
	return; 
}

int register_cmd(char *name, char *usage, int8_t hide, cmdFun fun)
{
	int ret;
	
	if (cmd_num_current < MAX_CMD_NUM)
	{
		strcpy(command_list[cmd_num_current].name, name);
		strcpy(command_list[cmd_num_current].usage , usage);
		command_list[cmd_num_current].hide = hide;
		command_list[cmd_num_current].cmdFun = fun;
 
		cmd_num_current++;
	}
	else
	{
		printf(" error\n");
		return 1;
	}
	
	return 0;
}


/*填充该程序的命令表TAB功能*/

static char *

command_generator(const char *text, int state)

{

    const char *name;

    static int list_index,len;

    if(!state)

    {

      list_index=0;

      len = strlen(text);

    }

    while(list_index < cmd_num_current)

    {
        name = command_list[list_index].name;
        list_index++;

        if(!command_list[list_index-1].hide && strncmp(name,text,len) == 0)

        {

            return strdup(name);

        }

    }

    return NULL;

}

/*填充选项命令，该处暂忽略*/

static char *

option_generator(const char *text, int state)

{

    static int list_index = 0,len = 2;
    return NULL;

   

}

/*与readline库的命令填充接口*/

char **

readline_command_completion(const char *text, int start, int end)

{

    char **matches = (char **)NULL;


    if(start == 0)

        matches = rl_completion_matches(text,command_generator);

    else

        matches = rl_completion_matches(text,option_generator);
    //以上的补全函数返回NULL也不会再调用lib自带的文件补全函数
    rl_attempted_completion_over = 1;

    return matches;

}

/*初始化*/

static int readline_init()

{

    char *userdir;

    rl_readline_name="KAShell";
    //设置tab键的自定义补全函数
    rl_attempted_completion_function = readline_command_completion;

   // rl_bind_key('\', rl_insert);
    //使得ctrl+c 不关闭
    signal(SIGINT,SIG_IGN);
    //ctrl+d 收到的信号。
    signal(SIGTSTP,SIG_IGN);
    //ctrl+| 会使readline函数引发QUIT信号，该函数屏蔽掉QUIT信号
    signal(SIGQUIT,SIG_IGN);
 //   signal(SIGCHLD, SIG_IGN);
    //rl_bind_key函数设置对第一个参数中的字符原本对应的操作清空
    rl_bind_key(18, rl_insert);
    rl_bind_key(28,rl_insert);
    stifle_history(500);

    read_history(History);

    return 0;  

}

 

/*store the commands into History file*/

static int readline_deinit()

{

    write_history(History);

    return 0;

}

 

static int command_parse(char *cmdstr)

{
    char *name;

    char cmd[MAX_CMD_LENGTH];
    char cmd_bak[MAX_CMD_LENGTH];
    char *cmdname;

    int list_index=0;

    strcpy(cmd,trim(cmdstr));
    strcpy(cmd_bak,cmd);

    cmdname=strtok(cmd," ");

    if(cmdname==NULL)

        return -1;

    while(list_index < cmd_num_current)

    {
       name=command_list[list_index].name;
       if(strcmp(name,cmdname) == 0)

        {
          //  printf("Find a command defined within program!\n");
            command_list[list_index].cmdFun(cmd_bak);

            return 1;

        }

        list_index++;

    }
    //如果允许其他系统指令，则可以让该处返回0
    return -1;

   

}

int main()

{

    char *r=NULL;

    Exit=0;
    memset(command_list, 0, sizeof(CmdTbl)*MAX_CMD_NUM);
	register_cmd("help", "list all cmd\n\r",0,HelpCmdExeFun);
	register_cmd("ifconfig", "Read the net config\n\r",0,system_fun);
	register_cmd("ls","Get the files in the current directory\n\r",0,system_fun);
	register_cmd("ls -a","Get all the files in the current directory\n\r",0,system_fun);
	register_cmd(CHTOKA, "equal to su\n\r",  1, chtouser_fun);
//	register_cmd("list", "List information about the FILEs\n\r",    0,    system_fun);
    readline_init();


    while(!Exit)

    {

        if(r!=NULL)

        {

            free(r);

            r=NULL;

        }

        r=readline(CONFIG_SYS_PROMPT);

        //NULL或者EOF 
        if(r == NULL || *r == -1)
        {
            printf("\n");
            continue;
        }
        add_history(r);

        int result;
        if(strlen(r)>MAX_CMD_LENGTH - sizeof(CONFIG_SYS_PROMPT))
        {
            printf("[%s ]:  command not found!\n", r);
            HelpCmdExeFun(r);
            continue;
        }
        result=command_parse(r);

        if(result < 0)
        {
            printf("[%s ]:  command not found!\n", r);
            HelpCmdExeFun(r);
            continue;
        }else if(!result){
            system(r);
        }
    }
    readline_deinit();

    return 0;

}
