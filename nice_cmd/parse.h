/*************************************************************************
	> File Name: parse.h
	> Author: ZHJ
	> Remarks: 命令解析部分
	> Created Time: Wed 05 Jan 2022 11:11:07 AM CST
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

#ifndef _PARSE_H_
#define _PARSE_H_

#ifdef __cplusplus
extern "C" 
{
#endif

struct cmdline;

#ifndef offsetof
/*
* 计算成员field在type类型中的地址偏移量
*/
#define offsetof(type, field)  ((size_t) &( ((type*)0)->field) )
#endif

#define PARSE_RESULT_MAX 10240
#define COMPLETION_BUF_SIZE 64

/*
* 令牌前置结构
* 
* token_ops: 储存令牌相关的配置
*    offset: 储存偏移量
*/
struct token_hdr 
{
    struct token_ops* ops;
    unsigned int offset;
};
typedef struct token_hdr parse_token_hdr_t;

/*
* 令牌回调函数配置
*
*            parse:
*  complete_get_nb:
* complete_get_elt:
*         get_help:
*/
struct token_ops 
{
    int (*parse)(parse_token_hdr_t*, const char*, void*);
    int (*complete_get_nb)(parse_token_hdr_t*);
    int (*complete_get_elt)(parse_token_hdr_t*, int, char*, unsigned int);
    int (*get_help)(parse_token_hdr_t*, char*, unsigned int);
};

/*
* 命令结构体
*/
struct inst 
{
    /* f(parsed_struct, data) */
    void (*f)(struct cmdline*, void*, void*);
    void* data;
    char* help_str;
    parse_token_hdr_t* tokens[];
};
typedef struct inst parse_inst_t;

/*
* struct inst* - 命令指针
*/
typedef parse_inst_t* parse_ctx_t;

/*
* 解析结果对应返回值
*/
#define PARSE_SUCCESS        0
#define PARSE_AMBIGUOUS     -1
#define PARSE_NOMATCH       -2
#define PARSE_BAD_ARGS      -3

/*
* 对buf中的命令进行解析
*
*  cl: cmdline结构体 读取其命令组cmd_group
* buf: 命令字符串
*
* 返回值参照上面的宏
*/
int parse(struct cmdline* cl, const char* buf);

/*
* 补全结果对应返回值
*/
#define COMPLETE_FINISHED   0
#define COMPLETE_AGAIN      1
#define COMPLETE_BUFFER     2

/*
* 对buf中的命令进行补全
*
*    cl: cmdline结构体 读取其命令组cmd_group
*   buf: 命令字符串
* state: 解析模式 (*state)为0时尝试完成 为-1时仅显示选择
*   dst: 返回的字符串
*  size: dst最大长度
*
* 出错时返回值为负数
*/
int complete(struct cmdline* cl, const char* buf, int* state, char* dst, unsigned int size);

/* true if(!c || iscomment(c) || isblank(c) || isendofline(c)) */
int isendoftoken(char c);

#ifdef __cplusplus
}
#endif

#endif
