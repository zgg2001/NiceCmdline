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
* 返回 >0 为匹配成功的令牌数(未完全匹配)
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
    parse_token_hdr_t* token_p;
    struct token_hdr token_hdr;
    
    int nb_token = 0;//buf中的完整令牌数
    const char* incomplete_token = buf;
    unsigned int incomplete_token_len;
    
    char completion_buf[COMPLETION_BUF_SIZE];
    int completion_len = -1;
    unsigned int nb_completable;
    unsigned int nb_non_completable;
    
    char tmpbuf[COMPLETION_BUF_SIZE];
    int tmp_len;
    unsigned int i, n;
    int l;
    int local_state = 0;
    const char* help_str;

    //统计buf中的完整令牌数 以及确定需要补全的令牌
    for(i = 0; buf[i]; ++i)
    {
        if(!isblank(buf[i]) && isblank(buf[i + 1]))
            ++nb_token;
        if(isblank(buf[i]) && !isblank(buf[i + 1]))
            incomplete_token = buf + i + 1;
    }
    incomplete_token_len = strnlen(incomplete_token, INPUT_BUF_MAX_SIZE);

    //初筛
    if(*state <= 0)
    {
        nb_completable = 0;
        nb_non_completable = 0;
        
        inst = ctx[inst_num];
        while(inst)
        {
            //对完整令牌进行匹配
            if(nb_token && match_inst(inst, buf, nb_token, NULL))
                goto next;
            
            //匹配成功 -> 获取要补全的令牌
            token_p = inst->tokens[nb_token];
            if(token_p)
                memcpy(&token_hdr, token_p, sizeof(token_hdr));
            
            //无法补全
            if(!token_p || !token_hdr.ops->complete_get_nb || 
               !token_hdr.ops->complete_get_elt || 
               (n = token_hdr.ops->complete_get_nb(token_p)) == 0)
            {
                nb_non_completable++;
                goto next;
            }
            
            //对想要补全的令牌进行匹配
            for(i = 0; i < n; ++i)
            {
                if(token_hdr.ops->complete_get_elt(token_p, i, tmpbuf, sizeof(tmpbuf)) < 0)
                    continue;
                tmp_len = strnlen(tmpbuf, sizeof(tmpbuf));
                if(tmp_len < COMPLETION_BUF_SIZE - 1)
                {
                    tmpbuf[tmp_len] = ' ';
                    tmpbuf[tmp_len + 1] = '\0';
                }
                //匹配补全令牌
                if(!strncmp(incomplete_token, tmpbuf, incomplete_token_len))
                {
                    if(completion_len == -1)//起始
                    {
                        snprintf(completion_buf, sizeof(completion_buf), "%s", tmpbuf + incomplete_token_len);
                        completion_len = strnlen(tmpbuf + incomplete_token_len, sizeof(tmpbuf) - incomplete_token_len);
                    }
                    else
                    {
                        completion_len = nb_common_chars(completion_buf, tmpbuf + incomplete_token_len);
                        completion_buf[completion_len] = '\0';
                    }
                    nb_completable++;
                }
            }
            
        next:
            ++inst_num;
            inst = ctx[inst_num];
        }
       
        //无法补全
        if(nb_completable == 0 && nb_non_completable == 0)
            return 0;
        
        //不需多项选择
        if(*state == 0 && incomplete_token_len > 0)
        {
            //存在可补全内容
            if(completion_len > 0)
            {
                if((unsigned int)completion_len + 1 > size)
                    return 0;
                strcpy(dst, completion_buf);
                return 2;
            }
        }
    }

    /*
    * 进入输出help模式 首先更改state值 
    * 此处的state值意义变为已处理数量
    * local_state: 本次遍历处理数量
    * 
    * 当local_state < *state时，说明此次已处理
    * 开始处理下一个
    */
    if(*state == -1)
        *state = 0;

    inst_num = 0;
    inst = ctx[inst_num];
    while(inst)
    {
        /* we need to redo it */
        inst = ctx[inst_num];
        
        //匹配已完成令牌
        if(nb_token && match_inst(inst, buf, nb_token, NULL))
            goto next2;
       
        //匹配成功 -> 获取要补全的令牌
        token_p = inst->tokens[nb_token];
        if(token_p)
            memcpy(&token_hdr, token_p, sizeof(token_hdr));
        
        //无法补全
        if(!token_p || !token_hdr.ops->complete_get_nb ||
           !token_hdr.ops->complete_get_elt ||
           (n = token_hdr.ops->complete_get_nb(token_p)) == 0)
        {
            if(local_state < *state)
            {
                ++local_state;
                goto next2;
            }
            (*state)++;
            //help str
            if (token_p && token_hdr.ops->get_help)
            {
                token_hdr.ops->get_help(token_p, tmpbuf, sizeof(tmpbuf));
                help_str = inst->help_str;
                if (help_str)
                    snprintf(dst, size, "[%s]: %s", tmpbuf, help_str);
                else
                    snprintf(dst, size, "[%s]: No help", tmpbuf);
            }
            else
            {
                snprintf(dst, size, "[RETURN]");
            }
            return 1;
        }
        
        //可以补全 有多种选择
        for(i = 0; i < n; ++i)
        {
            if(token_hdr.ops->complete_get_elt(token_p, i, tmpbuf, sizeof(tmpbuf)) < 0)
                continue;
            tmp_len = strnlen(tmpbuf, sizeof(tmpbuf));
            if (tmp_len < COMPLETION_BUF_SIZE - 1) 
            {	
                tmpbuf[tmp_len] = ' ';
                tmpbuf[tmp_len + 1] = 0;
            }
            //匹配补全令牌
            if(!strncmp(incomplete_token, tmpbuf, incomplete_token_len))
            {
                if(local_state < *state)
                {
                    ++local_state;
                    continue;
                }
                (*state)++;
                l = snprintf(dst, size, "%s", tmpbuf);
                if(l >= 0 && token_hdr.ops->get_help)
                {
                    token_hdr.ops->get_help(token_p, tmpbuf, sizeof(tmpbuf));
                    help_str = inst->help_str;
                    if(help_str)
                        snprintf(dst + l, size - l, "[%s]: %s", tmpbuf, help_str);
                    else
                        snprintf(dst + l, size - l, "[%s]: No help", tmpbuf);
                }
                return 1;
            }
        }
        
    next2:
        ++inst_num;
        inst = ctx[inst_num];
    }

    return 0;
}

