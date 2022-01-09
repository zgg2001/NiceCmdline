/*************************************************************************
	> File Name: parse_num.h
	> Author: ZHJ
	> Remarks: 令牌数据结构特化 数字类型
	> Created Time: Sun 09 Jan 2022 10:24:56 AM CST
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

#ifndef _PARSE_NUM_H_
#define _PARSE_NUM_H_

#include<nice_cmd/parse.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
* token_num支持的类型
*/
enum numtype 
{
    UINT8 = 0,
    UINT16,
    UINT32,
    INT8,
    INT16,
    INT32,
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
    FLOAT,
#endif
};

/*
* 令牌data区域
*/
struct token_num_data 
{
    enum numtype type;
};

/*
* 令牌结构
*
*         hdr: 令牌前置结构token_hdr
* string_data: 令牌data -> 枚举类型numtype
*/
struct token_num 
{
    struct token_hdr hdr;
    struct token_num_data num_data;
};
typedef struct token_num parse_token_num_t;

/*
* 令牌配套回调函数 定义在parse_num.c
*/
extern struct token_ops token_num_ops;

/*
* token_ops中的令牌回调函数
* 
*    parse_num: srcbuf为待解析字符串 解析结果存至res中
*               返回-1为失败 否则返回解析后数字的长度
*
* get_help_num: 将令牌中的numtype copy至dstbuf中
*               返回-1为失败 0为成功
*/
int parse_num(parse_token_hdr_t* tk, const char* srcbuf, void* res);
int get_help_num(parse_token_hdr_t* tk, char* dstbuf, unsigned int size);

/*
* 令牌初始化宏
*/
#define TOKEN_NUM_INITIALIZER(structure, field, numtype)   \
{                                                          \
        .hdr = {                                           \
                .ops = &token_num_ops,                     \
                .offset = offsetof(structure, field),      \
        },                                                 \
        .num_data = {                                      \
                .type = numtype,                           \
        },                                                 \
}

#ifdef __cplusplus
}
#endif

#endif
