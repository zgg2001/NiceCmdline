/*************************************************************************
	> File Name: cmdline.h
	> Author: ZHJ
	> Remarks: NiceCmdline的最外层结构cmdline
	> Created Time: Wed 29 Dec 2021 08:36:13 PM CST
 ************************************************************************/

#ifndef _CMDLINE_H_
#define _CMDLINE_H_

#include<termios.h>
#include<lib_nice/receiver.h>

#ifdef __cplusplus
extern "C" 
{
#endif

/*
* 命令行的输入输出流
*/
#define INPUT_STREAM 0
#define OUTPUT_STREAM 1

/*
* struct cmdline为最外层结构体，
* 交互式命令行由此结构体配置
* 其通过函数cmdline_get_new()来新建
*
*      prompt: 命令行提示符
*   cmd_group: 命令行的命令组
*    cmd_recv: 命令行的接收器
*  cmdline_in: 输入流
*       -- 默认设为标准输入流
* cmdline_out: 输出流
*       -- 默认设为标准输出流
*     oldterm: 终端配置备份
*       -- 退出命令行时恢复终端设置
*/
struct cmdline
{
    char prompt[PROMPT_MAX_SIZE];
    //struct command* cmd_group;
    struct receiver cmd_recv;
    int cmdline_in;
    int cmdline_out;
    struct termios oldterm;
};

/*
* 获取新的cmdline 
*/
struct cmdline* cmdline_get_new(const char* prompt);

/*
* 为指定cmdline设置提示符
*/
void cmdline_set_prompt(struct cmdline* cl, const char* prompt);

/*
* 指定cmdline开始交互
*/
void cmdline_start_interact();

/*
* 指定cmdline退出
*/
void cmdline_quit();

/*
* free指定cmdline
*/
void cmdline_free();

/*
* 接收器回调函数 - 字符输出操作
*/
int cmdline_write_char(struct receiver* recv, char c);

#ifdef __cplusplus
}
#endif

#endif
