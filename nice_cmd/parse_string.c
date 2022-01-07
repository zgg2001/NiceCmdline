/*************************************************************************
	> File Name: parse_string.c
	> Author: ZHJ
	> Remarks: 令牌数据结构特化 字符串类型 
	> Created Time: Thu 06 Jan 2022 06:13:10 PM CST
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
#include<ctype.h>
#include<string.h>
#include"parse_string.h"

struct token_ops token_string_ops = {
    .parse = parse_string,
    .complete_get_nb = complete_get_nb_string,
    .complete_get_elt = complete_get_elt_string,
    .get_help = get_help_string,
};

#define MULTISTRING_HELP "Mul-choice STRING"
#define ANYSTRING_HELP   "Any STRING"
#define FIXEDSTRING_HELP "Fixed STRING"

/*
* static 获取(令牌)data长度
*/
static unsigned int
get_token_len(const char* s)
{
    char c;
    unsigned int i = 0;

    c = s[i];
    while (c != '#' && c != '\0') 
    {
        ++i;
        c = s[i];
    }
    return i;
}

/*
* static 获取(令牌)data的下一个内容(由#分隔)
*/
static const char*
get_next_token(const char* s)
{
    unsigned int i;
    
    i = get_token_len(s);
    if(s[i] == '#')
        return s + i + 1;
    return NULL;
}

int
parse_string(parse_token_hdr_t* tk, const char* buf, void* res)
{
    if(!tk || !buf || !(*buf) || !res)
        return -1;
    
    struct token_string* tk2;
    struct token_string_data* sd;
    unsigned int token_len;
    const char* str;

    tk2 = (struct token_string*)tk;
    sd = &tk2->string_data;

    //开始匹配
    if(sd->str) 
    {
        str = (const char*)sd->str;
        do 
        {
            token_len = get_token_len(str);
            //长度超限
            if(token_len >= STR_TOKEN_SIZE - 1) 
            {
                continue;
            }
            //无法匹配
            if(strncmp(buf, str, token_len)) 
            {
                continue;
            }
            //传入字符串与令牌str长度不符
            if(!isendoftoken(*(buf + token_len))) 
            {
                continue;
            }
            break;
        }while((str = get_next_token(str)) != NULL);
        
        //没有匹配
        if(!str)
            return -1;
    }
    //令牌中str为空 -> 直接匹配
    else 
    {
        token_len = 0;
        while(!isendoftoken(buf[token_len]) && token_len < (STR_TOKEN_SIZE - 1))
            ++token_len;
        //长度超限
        if(token_len >= STR_TOKEN_SIZE - 1) 
            return -1;
    }
    
    if(res) 
    {
        //需确保token_len < STR_TOKEN_SIZE - 1
        strncpy(res, buf, token_len);
        *((char*)res + token_len) = '\0';
    }

    return token_len;
}

int complete_get_nb_string(parse_token_hdr_t* tk)
{
    if(!tk)
        return -1;

    struct token_string* tk2;
    struct token_string_data* sd;
    const char* str;
    int ret = 1;

    tk2 = (struct token_string*)tk;
    sd = &tk2->string_data;

    if(!sd->str)
        return 0;

    str = (const char *)sd->str;
    while((str = get_next_token(str)) != NULL) 
        ++ret;
    
    return ret;
}

int
complete_get_elt_string(parse_token_hdr_t* tk, int idx, char* dstbuf, unsigned int size)
{
    if(!tk || idx < 0 || !dstbuf)
        return -1;

    struct token_string* tk2;
    struct token_string_data* sd;
    const char* str;
    unsigned int len;

    tk2 = (struct token_string*)tk;
    sd = &tk2->string_data;

    str = (const char*)sd->str;

    //寻找索引为idx的选择
    while(idx-- && str)
        str = get_next_token(str);

    if(!str)
        return -1;

    len = get_token_len(str);
    if (len > size - 1)
        return -1;

    memcpy(dstbuf, str, len);
    dstbuf[len] = '\0';

    return 0;
}


int get_help_string(parse_token_hdr_t* tk, char* dstbuf, unsigned int size)
{
    if(!tk || !dstbuf)
        return -1;

    struct token_string* tk2;
    struct token_string_data* sd;
    const char* str;

    tk2 = (struct token_string*)tk;
    sd = &tk2->string_data;
    
    str = (const char*)sd->str;

    if(str) 
    {
        //多项可能匹配
        if(get_next_token(str)) 
        {
            strncpy(dstbuf, MULTISTRING_HELP, size);
        }
        //单项匹配
        else 
        {
            strncpy(dstbuf, FIXEDSTRING_HELP, size);
        }
    }
    else 
    {
        strncpy(dstbuf, ANYSTRING_HELP, size);
    }

    dstbuf[size - 1] = '\0';

    return 0;
}

