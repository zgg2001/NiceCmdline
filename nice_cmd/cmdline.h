/*************************************************************************
	> File Name: cmdline.h
	> Author: ZHJ
	> Remarks: NiceCmdline的最外层结构cmdline
	> Created Time: Wed 29 Dec 2021 08:36:13 PM CST
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

#ifndef _CMDLINE_H_
#define _CMDLINE_H_

#include<termios.h>
#include<nice_cmd/receiver.h>
#include<nice_cmd/parse.h>

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
    parse_ctx_t* cmd_group;
    struct receiver cmd_recv;
    int cmdline_in;
    int cmdline_out;
    struct termios oldterm;
};

/*
* 获取新的cmdline 
*/
struct cmdline* cmdline_get_new(parse_ctx_t* ctx, const char* prompt);

/*
* 为指定cmdline设置提示符
*/
void cmdline_set_prompt(struct cmdline* cl, const char* prompt);

/*
* 指定cmdline开始交互
*/
void cmdline_start_interact(struct cmdline* cl);

/*
* 指定cmdline录入指定内容
*/
int cmdline_parse_input(struct cmdline* cl, const char* buf, unsigned int size);

/*
* 指定cmdline退出
*/
void cmdline_quit(struct cmdline* cl);

/*
* free指定cmdline并恢复终端设置
*/
void cmdline_exit_free(struct cmdline* cl);

#ifdef __cplusplus
}
#endif

#endif
