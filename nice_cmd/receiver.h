/*************************************************************************
	> File Name: receiver.h
	> Author: ZHJ
	> Remarks: struct cmdline的配套接收器相关
	> Created Time: Wed 29 Dec 2021 10:19:11 PM CST
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

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include<nice_cmd/inputbuf.h>
#include<nice_cmd/parser_vt102.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
* 配置宏
*/
#define PROMPT_MAX_SIZE 32
#define INPUT_BUF_MAX_SIZE 512

/*
* 接收器配套回调函数
*   func_write_char: 设定字符如何write至输出流
*    func_parse_cmd: 解析字符串
* func_complete_cmd: 补全字符串
*/
struct receiver;
typedef int (func_write_char)(struct receiver*, char);
typedef void (func_parse_cmd)(struct receiver*, const char*);
typedef int (func_complete_cmd)(struct receiver*, const char*, int*, char*, unsigned int);

/*
* 接收器状态
*/
enum receiver_status 
{
    RECEIVER_INIT,
    RECEIVER_RUNNING,
    RECEIVER_EXITED
};

/*
* 接收器结构体
* 负责输入输出内容
*
*       owner: 此接收器所有者，指向一个struct cmdline
*      prompt: 接收器需要输出的提示符
* prompt_size: 接收器提示符的长度
*      status: 接收器状态
*
*       vt102: vt102控制码解析器
*
*    left_buf: 输入缓冲区的左部分
*        left: 左缓冲配套的内存
*   right_buf: 输入缓冲区的右部分
*       right: 右缓冲配套的内存
*     all_cmd: 完整的命令 用于解析
*
*   paste_buf: 粘贴缓冲区 储存ctrl k等删除的内容
*       paste: 粘贴缓冲区配套的内容
*
*  write_char: 输出字符
*   parse_cmd: 解析命令
*complete_cmd: 补全命令
*/
struct receiver
{
    //基础属性
    void* owner;
    char prompt[PROMPT_MAX_SIZE];
    int prompt_size;
    enum receiver_status status;
    //控制码解析器
    struct parser_vt102 vt102; 
    //输入缓冲区
    struct inputbuf left_buf;
    char left[INPUT_BUF_MAX_SIZE];
    struct inputbuf right_buf;
    char right[INPUT_BUF_MAX_SIZE];
    char all_cmd[INPUT_BUF_MAX_SIZE * 2];
    //粘贴缓冲区 储存ctrl k等删除的内容
    struct inputbuf paste_buf;
    char paste[INPUT_BUF_MAX_SIZE * 2];
    //历史记录
    //回调函数
    func_write_char* write_char;
    func_parse_cmd* parse_cmd;
    func_complete_cmd* complete_cmd;
};

/*
* receiver初始化
* 返回0为初始化成功
*/
int receiver_init(struct receiver* recv, func_write_char* write_char, func_parse_cmd* parse_cmd, func_complete_cmd* complete_cmd);

/*
* 创建新的命令行
* 对缓冲区进行重置/输出prompt
* 返回0为执行成功
*/
int receiver_new_cmdline(struct receiver* recv, const char* prompt);

/*
* 接收器退出(RECEIVER_EXITED)
*/
void receiver_quit(struct receiver* recv);

/*
* 对接收器缓冲区的字符串进行组合
* mode = 0时为组合左右缓冲区
* mode = 1时为仅组合左缓冲区
*/
const char* receiver_combi_cmd(struct receiver* recv, int mode);

#define RECEIVER_RES_SUCCESS       0
#define RECEIVER_RES_PARSED        1
#define RECEIVER_RES_COMPLETE      2
#define RECEIVER_RES_NOT_RUNNING  -1
#define RECEIVER_RES_EOF          -2
#define RECEIVER_RES_EXITED       -3

/*
* 对输入的字符进行解析
* 返回值按上面宏定义解释
*/
int receiver_parse_char(struct receiver* recv, char c);

/*
* 重新显示当前命令行
*/
void receiver_redisplay(struct receiver* recv);

#ifdef __cplusplus
}
#endif

#endif
