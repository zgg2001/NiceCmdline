/*************************************************************************
	> File Name: parse_string.h
	> Author: ZHJ
	> Remarks: 令牌数据结构特化 字符串类型 
	> Created Time: Thu 06 Jan 2022 06:11:22 PM CST
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

#ifndef _PARSE_STRING_H_
#define _PARSE_STRING_H_

#include <nice_cmd/parse.h>

#ifdef __cplusplus
extern "C" 
{
#endif

/* size of a parsed string */
#define STR_TOKEN_SIZE 64

typedef char fixed_string_t[STR_TOKEN_SIZE];

/*
* 令牌data区域
*/
struct token_string_data 
{
    const char* str;
};

/*
* 令牌结构
*
*         hdr: 令牌前置结构token_hdr
* string_data: 令牌data -> char*
*/
struct token_string 
{
    struct token_hdr hdr;
    struct token_string_data string_data;
};
typedef struct token_string parse_token_string_t;

/*
* 令牌配套回调函数 定义在parse_string.c
*/
extern struct token_ops token_string_ops;

/*
* token_ops中的令牌回调函数
*
*            parse_string: 匹配srcbuf 返回-1为失败 其余为匹配成功的str长度 
*  complete_get_nb_string: 返回令牌中可能匹配str数量
* complete_get_elt_string: 将索引为idx的选择 存入dstbuf
*         get_help_string: 在dstbuf里存入help类型信息
*/
int parse_string(parse_token_hdr_t* tk, const char* srcbuf, void* res);
int complete_get_nb_string(parse_token_hdr_t* tk);
int complete_get_elt_string(parse_token_hdr_t* tk, int idx, char* dstbuf, unsigned int size);
int get_help_string(parse_token_hdr_t* tk, char* dstbuf, unsigned int size);

/*
* 令牌初始化宏
*/
#define TOKEN_STRING_INITIALIZER(structure, field, string)  \
{                                                           \
        .hdr = {                                            \
                .ops = &token_string_ops,                   \
                .offset = offsetof(structure, field),       \
        },                                                  \
        .string_data = {                                    \
                .str = string,                              \
        },                                                  \
}

#ifdef __cplusplus
}
#endif

#endif
