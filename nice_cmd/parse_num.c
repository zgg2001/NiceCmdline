/*************************************************************************
	> File Name: parse_num.c
	> Author: ZHJ
	> Remarks: 令牌数据结构特化 数字类型
	> Created Time: Sun 09 Jan 2022 10:37:58 AM CST
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
#include<stdint.h>
#include"parse_num.h"

struct token_ops token_num_ops = {
    .parse = parse_num,
    .complete_get_nb = NULL,
    .complete_get_elt = NULL,
    .get_help = get_help_num,
};

enum num_parse_state_t 
{
    START,//初始状态
    DEC_NEG,//负数
    BIN,//2进制
    HEX,//16进制
    FLOAT_POS,//浮点数
    FLOAT_NEG,//负浮点数
    ERROR,

    /*
    * 上面不带OK的状态是用来确认此数字的类型的
    * 以下若干OK 代表进入对应模式的接收状态
    */
    FIRST_OK, /* not used */
    ZERO_OK,//0开头
    HEX_OK,
    OCTAL_OK,
    BIN_OK,
    DEC_NEG_OK,
    DEC_POS_OK,
    FLOAT_POS_OK,
    FLOAT_NEG_OK,
};

//此处类型与parse_num.h中同步
static const char help1[] = "UINT8";
static const char help2[] = "UINT16";
static const char help3[] = "UINT32";
static const char help4[] = "INT8";
static const char help5[] = "INT16";
static const char help6[] = "INT32";
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
static const char help7[] = "FLOAT";
#endif
static const char *num_help[] = {
        help1, help2, help3, help4,
        help5, help6,
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
        help7,
#endif
};

/*
* 运算 (*res) * base + c
* 即向结果末尾添加新数字
*/
static inline int
add_to_res(unsigned int c, uint32_t* res, unsigned int base)
{
    //溢出
    if((UINT32_MAX - c) / base < *res) 
    {
        return -1;
    }

    *res = *res * base + c ;
    return 0;
}

/*
* 对整数/浮点数进行解析
*
*     tk: 指向解析的令牌 -> 读取numtype
* srcbuf: 要解析成数字的输入
*    res: 储存解析结果
*/
int
parse_num(parse_token_hdr_t* tk, const char* srcbuf, void* res)
{
    if(!tk || !srcbuf)
        return -1;

    struct token_num_data nd;
    enum num_parse_state_t st = START;
    const char* buf = srcbuf;
    char c = *buf;
    uint32_t res1 = 0, res2 = 0, res3 = 1;

    memcpy(&nd, &((struct token_num*)tk)->num_data, sizeof(nd));

    //当状态不为error且未解析完则继续
    while(st != ERROR && c && !isendoftoken(c)) 
    {                
        switch(st) 
        {
            case START:
                if(c == '-')//负数
                {
                    st = DEC_NEG;
                }
                else if(c == '0')//0
                {
                    st = ZERO_OK;
                }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
                else if(c == '.')//小数点 
                {
                    st = FLOAT_POS;
                    res1 = 0;
                }
#endif
                else if(c >= '1' && c <= '9')//常规数字
                {
                    if(add_to_res(c - '0', &res1, 10) < 0)
                        st = ERROR;
                    else
                        st = DEC_POS_OK;
                }
                else  
                {
                    st = ERROR;
                }
            break;
            
            case ZERO_OK:
                if(c == 'x')//十六进制
                {
                    st = HEX;
                }
                else if(c == 'b')//二进制 
                {
                    st = BIN;
                }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
                else if(c == '.')//.
                {
                    st = FLOAT_POS;
                    res1 = 0;
                }
#endif
                else if(c >= '0' && c <= '7') 
                {
                    if(add_to_res(c - '0', &res1, 10) < 0)
                        st = ERROR;
                    else
                        st = OCTAL_OK;
                }
                else  
                {
                    st = ERROR;
                }
            break;
            
            case DEC_NEG:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res1, 10) < 0)
                        st = ERROR;
                    else
                        st = DEC_NEG_OK;
                }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
                else if(c == '.') 
                {
                    res1 = 0;
                    st = FLOAT_NEG;
                }
#endif
                else 
                {
                    st = ERROR;
                }
            break;
            
            case DEC_NEG_OK:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res1, 10) < 0)
                        st = ERROR;
                }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
                else if(c == '.') 
                {
                    st = FLOAT_NEG;
                }
#endif
                else 
                {
                    st = ERROR;
                }
            break;
            
            case DEC_POS_OK:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res1, 10) < 0)
                        st = ERROR;
                }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
                else if(c == '.') 
                {
                    st = FLOAT_POS;
                }
#endif
                else 
                {
                    st = ERROR;
                }
            break;
            
            case HEX:
                st = HEX_OK;//此处HEX状态直接进入HEX_OK状态开始进行输入接收
            case HEX_OK:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res1, 16) < 0)
                        st = ERROR;
                }
                else if(c >= 'a' && c <= 'f') 
                {
                    if(add_to_res(c - 'a' + 10, &res1, 16) < 0)
                    st = ERROR;
                }
                else if(c >= 'A' && c <= 'F') 
                {
                    if(add_to_res(c - 'A' + 10, &res1, 16) < 0)
                        st = ERROR;
                }
                else 
                {
                    st = ERROR;
                }
            break;
            
            case OCTAL_OK:
                if(c >= '0' && c <= '7') 
                {                    
                    if(add_to_res(c - '0', &res1, 8) < 0)
                        st = ERROR;
                }
                else 
                {
                    st = ERROR;
                }
            break;
            
            case BIN:
                st = BIN_OK;//no break直接进入二进制读写模式
            case BIN_OK:
                if(c >= '0' && c <= '1') 
                {
                    if(add_to_res(c - '0', &res1, 2) < 0)
                        st = ERROR;
                }
                else 
                {
                    st = ERROR;
                }
            break;
            
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
            case FLOAT_POS:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res2, 10) < 0)
                        st = ERROR;
                    else
                        st = FLOAT_POS_OK;
                    res3 = 10;
                }
                else 
                {
                    st = ERROR;
                }
            break;
            
            case FLOAT_NEG:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res2, 10) < 0)
                        st = ERROR;
                    else
                        st = FLOAT_NEG_OK;
                    res3 = 10;
                }
                else 
                {
                    st = ERROR;
                }
            break;
            
            case FLOAT_POS_OK:
                if(c >= '0' && c <= '9') 
                {
                    if(add_to_res(c - '0', &res2, 10) < 0)
                        st = ERROR;
                    if(add_to_res(0, &res3, 10) < 0)
                        st = ERROR;
                }
                else 
                {
                    st = ERROR;
                }
            break;
            
            case FLOAT_NEG_OK:
                if(c >= '0' && c <= '9')
                {
                    if(add_to_res(c - '0', &res2, 10) < 0)
                        st = ERROR;
                    if(add_to_res(0, &res3, 10) < 0)
                        st = ERROR;
                }
                else 
                {
                    st = ERROR;
                }
            break;
#endif
            default:
                //printf("error -- not impl\n");
            break;
        }
        
        //下一个
        buf++;
        c = *buf;
        
        //令牌过长
        if(buf - srcbuf > 127)
            return -1;
    }
    
    switch(st) 
    {
        //常规非负整数
        case ZERO_OK:
        case DEC_POS_OK:
        case HEX_OK:
        case OCTAL_OK:
        case BIN_OK:
            if(nd.type == INT8 && res1 <= INT8_MAX) 
            {
                if(res)
                    *(int*)res = (int8_t)res1;
                return (buf - srcbuf);
            }
            else if(nd.type == INT16 && res1 <= INT16_MAX) 
            {
                if(res)
                    *(int16_t*)res = (int16_t)res1;
                return (buf - srcbuf);
            }
            else if(nd.type == INT32 && res1 <= INT32_MAX) 
            {
                if(res)
                    *(int32_t*)res = (int32_t)res1;
                return (buf - srcbuf);
            }
            else if(nd.type == UINT8 && res1 <= UINT8_MAX) 
            {
                if(res)
                    *(unsigned int*)res = (unsigned int)res1;
                return (buf - srcbuf);
            }
            else if(nd.type == UINT16  && res1 <= UINT16_MAX) 
            {
                if(res)
                    *(uint16_t*)res = (uint16_t)res1;
                return (buf - srcbuf);
            }
            else if(nd.type == UINT32) 
            {
                if(res)
                    *(uint32_t*)res = (uint32_t)res1;
                return (buf - srcbuf);
            }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
            else if(nd.type == FLOAT) 
            {
                if(res)
                    *(float*)res = (float)res1;
                return (buf - srcbuf);
            }
#endif
            else 
            {
                return -1;
            }
        break;
        
        //负整数
        case DEC_NEG_OK:
            if(nd.type == INT8 && res1 <= INT8_MAX + 1) 
            {
                if(res)
                    *(int*)res = - (int8_t)res1;
                return (buf-srcbuf);
            }
            else if(nd.type == INT16 && res1 <= (uint16_t)INT16_MAX + 1) 
            {
                if(res)
                    *(int16_t*)res = - (int16_t)res1;
                return (buf-srcbuf);
            }
            else if(nd.type == INT32 && res1 <= (uint32_t)INT32_MAX + 1) 
            {
                if(res)
                    *(int32_t*)res = - (int32_t)res1;
                return (buf-srcbuf);
            }
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
            else if(nd.type == FLOAT) 
            {
                if(res)
                    *(float*)res = - (float)res1;
                return (buf-srcbuf);
            }
#endif
            else 
            {
                return -1;
            }
        break;
        
#ifndef CONFIG_MODULE_PARSE_NO_FLOAT
        //正浮点数
        case FLOAT_POS:
        case FLOAT_POS_OK:
            if(nd.type == FLOAT) 
            {
                if(res)
                    *(float*)res = (float)res1 + ((float)res2 / (float)res3);
                return (buf - srcbuf);
            }
            else 
            {
                return -1;
            }
        break;
        
        //负浮点数
        case FLOAT_NEG:
        case FLOAT_NEG_OK:
            if(nd.type == FLOAT) 
            {
                if(res)
                    *(float*)res = - ((float)res1 + ((float)res2 / (float)res3));
                return (buf - srcbuf);
            }
            else 
            {
                return -1;
            }
        break;
#endif
        default:
            //printf("error\n");
            return -1;
        break;
    }
    return -1;
}


//help
int
get_help_num(parse_token_hdr_t* tk, char* dstbuf, unsigned int size)
{
    if(!tk || !dstbuf)
        return -1;

    struct token_num_data nd;

    //读取此数字的类型 并将其类型(字符串化)copy至dstbuf中
    memcpy(&nd, &((struct token_num*)tk)->num_data, sizeof(nd));
    strncpy(dstbuf, num_help[nd.type], size);//num_help[]
    dstbuf[size - 1] = '\0';

    return 0;
}






