/*************************************************************************
	> File Name: parse.c
	> Author: ZHJ
	> Remarks: 命令解析部分
	> Created Time: Wed 05 Jan 2022 05:23:44 PM CST
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
#include<string.h>
#include<inttypes.h>
#include<ctype.h>
#include"parse.h"
#include"cmdline.h"

/*
* 判断是否为行尾 \r\n
* 返回1为行尾
*/
static int
isendofline(char c)
{
    if(c == '\n' || c == '\r')
        return 1;
    return 0;
}

/*
* 判断是否为注释符#
* 返回1为#
*/
static int
iscomment(char c)
{
    if(c == '#')
        return 1;
    return 0;
}

/*
* 判断是否为令牌末尾
* 当一个字符为注释符#/空白/行尾时 则说明此令牌结束
* 返回1说明令牌结束
*/
int
isendoftoken(char c)
{
    if(!c || iscomment(c) || isblank(c) || isendofline(c))
        return 1;
    return 0;
}

/*
* 比较两个字符串
* 返回两个字符串连续相同字符数
*/
static unsigned int
nb_common_chars(const char* s1, const char* s2)
{
    unsigned int i = 0;

    while(*s1 && *s1 == *s2) 
    {
        ++s1;
        ++s2;
        ++i;
    }
    return i;
}

/*
* 尝试对buf内的字符串依照命令inst进行匹配
* 
*           inst: 比对依照的命令
*            buf: 需要比对的字符串
* nb_match_token: 最大匹配令牌数,当其为0时即匹配所有令牌
*     result_buf: 储存匹配结果的地方 不一定为空
*
* 返回 0  说明匹配成功
* 返回 -1 说明匹配失败
* 返回 >0 即为匹配成功的令牌数
*/
static int
match_inst(parse_inst_t* inst, const char* buf, unsigned int nb_match_token, void* result_buf)
{
    unsigned int token_num = 0;//当前匹配令牌下标
    parse_token_hdr_t* token_p;//指向匹配中的令牌
    struct token_hdr token_hdr;//正在匹配中的令牌
    unsigned int i = 0;//匹配成功数
    int n = 0;

    //获取命令inst的第一个token
    token_p = inst->tokens[token_num];
    if(token_p)//把令牌结构体copy过来方便使用回调函数
        memcpy(&token_hdr, token_p, sizeof(token_hdr));
    
    //开始对令牌进行匹配
    while(token_p && (!nb_match_token || i < nb_match_token)) 
    {
        //跳过空白
        while(isblank(*buf)) 
            buf++;
        //当字符串结束或存在注释符#时停止匹配
        if(isendofline(*buf) || iscomment(*buf))
            break;

        //使用令牌回调函数parse进行解析
        n = token_hdr.ops->parse(token_p, buf, (result_buf ? result_buf + token_hdr.offset : NULL));
        if(n < 0)
            break;
        i++;
        buf += n;

        //开始匹配下一个令牌
        ++token_num;
        token_p = inst->tokens[token_num];
        if(token_p)
            memcpy(&token_hdr, token_p, sizeof(token_hdr));
    }

    //匹配数为0
    if(i == 0)
        return -1;

    //如果匹配数有限制且完成要求
    if(nb_match_token)
    {
        if(i == nb_match_token) 
        {
            return 0;
        }
        return i;
    }

    //如果token_p有内容说明没匹配完
    if(token_p) 
    {
        return i;
    }

    //如果字符串还有连续空白就去掉
    while(isblank(*buf)) 
    {
        buf++;
    }
    //如果去掉空白就没有内容了则匹配成功
    if(isendofline(*buf) || iscomment(*buf))
        return 0;

    //返回匹配成功的数量
    return i;
}

int
parse(struct cmdline* cl, const char* buf)
{
    if(!cl || !buf)
        return PARSE_BAD_ARGS;

    parse_ctx_t* ctx = cl->cmd_group;//命令组
    unsigned int inst_num = 0;//正在匹配的命令下标
    parse_inst_t* inst;//指向正在匹配的命令

    const char* curbuf;//遍历buf用的字符指针
    int comment = 0;
    int linelen = 0;//buf的长度
    int parse_it = 0;//buf中是否存在有效命令 1为存在
    
    char result_buf[PARSE_RESULT_MAX];//解析结果缓冲区
    void (*f)(struct cmdline*, void*, void*) = NULL;//匹配成功调用的回调函数
    void* data = NULL;
    int tok;
    int err = PARSE_NOMATCH;

    //遍历buf统计长度 并查看是否仅存在空白或注释
    curbuf = buf;
    while(!isendofline(*curbuf)) 
    {
        if(*curbuf == '\0') 
        {
            //在一行命令中出现了\0,此命令存在问题
            return 0;
        }
        if(iscomment(*curbuf))
        {
            comment = 1;
        }
        if(!isblank(*curbuf) && !comment) 
        {
            //不在注释中且不为空白 说明存在有效命令
            parse_it = 1;
        }
        curbuf++;
        linelen++;
    }

    //长度加上行末尾的换行符
    while(isendofline(buf[linelen])) 
    {
        linelen++;
    }

    //无效命令
    if (parse_it == 0) 
    {
        return linelen;
    }

    /* parse it !! */
    inst = ctx[inst_num];
    while(inst) 
    {
        //match_inst进行完全匹配
        tok = match_inst(inst, buf, 0, result_buf);
        if(tok > 0)//没有完全匹配
        {
            err = PARSE_BAD_ARGS;
        }
        else if(!tok)//成功 
        {
            //设置回调函数
            if(!f) 
            {
                memcpy(&f, &inst->f, sizeof(f));
                memcpy(&data, &inst->data, sizeof(data));
            }
            //匹配多条: 命令冲突
            else 
            {
                err = PARSE_AMBIGUOUS;
                f = NULL;
                break;
            }
        }
        ++inst_num;
        inst = ctx[inst_num];
    }
    //调用回调函数
    if(f) 
    {
        f(cl, result_buf, data);
    }
    //没有完全匹配
    else 
    {
        return err;
    }
    return linelen;
}

int
complete(struct cmdline* cl, const char* buf, int* state, char* dst, unsigned int size)
{
    if(!cl || !buf || !state || !dst)
        return -1;
    
    parse_ctx_t* ctx = cl->cmd_group;//命令组
    unsigned int inst_num = 0;//正在匹配的命令下标
    parse_inst_t* inst;//指向正在匹配的命令
    
    return 0;
}

