/*************************************************************************
	> File Name: cmdline.c
	> Author: ZHJ
	> Remarks: cmdline相关定义 
	> Created Time: Sun 02 Jan 2022 10:07:13 AM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"cmdline.h"

struct cmdline* 
cmdline_get_new(const char* prompt)
{
	if (!prompt)
        return NULL;
    
    struct cmdline *cl;
    struct termios term;
    
    //cmdline内存初始化
    cl = malloc(sizeof(struct cmdline));
	if (cl == NULL)
        return NULL;
	memset(cl, 0, sizeof(struct cmdline));
    
    //cmdline成员初始化
	cl->cmdline_in = INPUT_STREAM;
	cl->cmdline_out = OUTPUT_STREAM;
	cmdline_set_prompt(cl, prompt);
	receiver_init(&cl->cmd_recv, cmdline_write_char);
	cl->cmd_recv.owner = cl;
    tcgetattr(0, &term);
    memcpy(&cl->oldterm, &term, sizeof(struct termios));
    
    //终端配置设置
	term.c_lflag &= ~(ICANON | ECHO | ISIG);
	tcsetattr(0, TCSANOW, &term);
	setbuf(stdin, NULL);
    
    //启动命令行接收器
    receiver_new_cmdline(&cl->cmd_recv, prompt);
    
    return cl;
}

void
cmdline_set_prompt(struct cmdline* cl, const char* prompt)
{
    if(!cl || !prompt)
        return;
    
    int len;

    len = strlen(prompt);
    if(len > PROMPT_MAX_SIZE - 1)
    {
        const char str[] = "error: prompt length exceeds limit";
        if(cl->cmdline_out >= 0)
            write(cl->cmdline_out, str, strlen(str));
        exit(1);
    }
    memcpy(cl->prompt, prompt, len);
    cl->prompt[len] = '\0';
}

void
cmdline_start_interact(struct cmdline* cl)
{
    if(!cl)
        return;
    
    char c = -1;
    
    while(1)
    {
        if(read(cl->cmdline_in, &c, 1) <= 0)
            break;
        if(cmdline_parse_input(cl, &c, 1) < 0)
            break;
    }
}

int
cmdline_parse_input(struct cmdline* cl, const char* buf, unsigned int size)
{
    if(!cl || !buf)
        return -1;

    unsigned int i;
    int ret = -1;
    
    //按size对字符挨个处理
    for(i = 0; i < size; ++i)
    {
        ret = receiver_parse_char(&cl->cmd_recv, buf[i]);
    }
    
    return i;
}

void
cmdline_quit(struct cmdline* cl)
{
    if(!cl)
        return;
}

void
cmdline_exit_free(struct cmdline* cl)
{
    if(!cl)
        return;
    
    //关闭输入输出流
	if (cl->cmdline_in > 2)
        close(cl->cmdline_in);
	if (cl->cmdline_out != cl->cmdline_in && cl->cmdline_out > 2)
        close(cl->cmdline_out);
	
    //free并且恢复终端设置
    free(cl);
    tcsetattr(fileno(stdin), TCSANOW, &cl->oldterm);
}

int
cmdline_write_char(struct receiver* recv, char c)
{
    if(!recv)
        return -1;
    
    int ret= -1;
    struct cmdline *cl;
    
    cl = recv->owner;
    if(cl->cmdline_out >= 0)
        ret = write(cl->cmdline_out, &c, 1);
    
    return ret;
}









