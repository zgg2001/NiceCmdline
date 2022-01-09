/*************************************************************************
	> File Name: cmdline.c
	> Author: ZHJ
	> Remarks: cmdline相关定义 
	> Created Time: Sun 02 Jan 2022 10:07:13 AM CST
 ************************************************************************/

/*
 * Copyright (c) 2009, Olivier MATZ <zer0@droids-corp.org>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"cmdline.h"

/*
* 内部函数 输出字符串
*/
static void
cmdline_puts(struct cmdline* cl, const char* str)
{
    if(!cl || !str)
        return;

    size_t len = strnlen(str, INPUT_BUF_MAX_SIZE), i;
    for(i = 0; i < len; ++i)
        if(cl->cmdline_out >= 0)
            write(cl->cmdline_out, &str[i], 1);
}

/*
* 内部函数 字符输出
*/
static int
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

/*
* 内部函数 解析命令
*/
static void
cmdline_parse_cmd(struct receiver* recv, const char* cmd)
{
    struct cmdline* cl = recv->owner;
	int ret;
	ret = parse(cl, cmd);
	if(ret == PARSE_AMBIGUOUS)
		cmdline_puts(cl, "Ambiguous command\n");
	else if(ret == PARSE_NOMATCH)
		cmdline_puts(cl, "Command not found\n");
	else if(ret == PARSE_BAD_ARGS)
		cmdline_puts(cl, "Bad arguments\n");
}

/*
* 内部函数 补全命令
*/
static int
cmdline_complete_cmd(struct receiver* recv, const char* buf, int* state, char* dst, unsigned int size)
{
    struct cmdline* cl = recv->owner;
	return complete(cl, buf, state, dst, size);
}

struct cmdline* 
cmdline_get_new(parse_ctx_t* ctx, const char* prompt)
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
    cl->cmd_group = ctx;
	cl->cmdline_in = INPUT_STREAM;
	cl->cmdline_out = OUTPUT_STREAM;
	cmdline_set_prompt(cl, prompt);
	receiver_init(&cl->cmd_recv, cmdline_write_char, cmdline_parse_cmd, cmdline_complete_cmd);
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
        
        if(ret == RECEIVER_RES_PARSED)
        {
            receiver_new_cmdline(&cl->cmd_recv, cl->prompt);
        }
        else if(ret == RECEIVER_RES_EOF)
            return -1;
        else if(ret == RECEIVER_RES_EXITED)
            return -1;
    }
    
    return i;
}

void
cmdline_quit(struct cmdline* cl)
{
    if(!cl)
        return;
    receiver_quit(&cl->cmd_recv);
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

    //free历史记录部分
    history_free(&cl->cmd_recv.hist);

    //free并且恢复终端设置
    free(cl);
    tcsetattr(fileno(stdin), TCSANOW, &cl->oldterm);
}










