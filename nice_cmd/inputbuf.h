/*************************************************************************
	> File Name: inputbuf.h
	> Author: ZHJ
	> Remarks: NiceCmdline中接收器的输入缓冲区，用于储存用户输入
	> Created Time: Fri 31 Dec 2021 09:09:13 PM CST
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

#ifndef _INPUTBUF_H_
#define _INPUTBUF_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
* 此结构体是接收器的输入缓冲区
* 使用起点与终点来指向缓冲区内的内容
* 进行头尾字符增删操作的复杂度为O(1)
* 
* maxlen: 缓冲区最大长度
*    len: 缓冲区当前长度
*  start: 缓冲区起点
*    end: 缓冲区终点
*    buf: 缓冲区内存指针
*/
struct inputbuf
{
    unsigned int maxlen;
    unsigned int len;
    unsigned int start;
    unsigned int end;
    char* buf;
};

/*
* 返回值为1时 缓冲区为空
*/
#define INPUT_BUF_IS_EMPTY(ibuf) ((ibuf)->len == 0)

/*
* 返回值为1时 缓冲区为满
*/
#define INPUT_BUF_IS_FULL(ibuf) ((ibuf)->len == (ibuf)->maxlen)

/*
* for each遍历宏
* b: struct inputbuf 指针
* i: 用于for遍历的整形变量
* c: 储存每次迭代的字符
*/
#define INPUTBUF_FOREACH(b, i, c) \
    for(i = 0, c = (b)->buf[(b)->start]; \
        i < (b)->len; \
        ++i, c = (b)->buf[((b)->start + i) % (b)->maxlen])

/*
* 缓冲区初始化函数
* 返回0为成功
*/
int inputbuf_init(struct inputbuf* ibuf, char* buf, unsigned int maxlen);

/*
* 缓冲区头部添加指定字符
* 返回0为成功
*/
int inputbuf_add_head(struct inputbuf* ibuf, char c);

/*
* 缓冲区尾部添加指定字符
* 返回0为成功
*/
int inputbuf_add_tail(struct inputbuf* ibuf, char c);

/*
* 缓冲区头部删除一个字符
* 返回0为成功
*/
int inputbuf_del_head(struct inputbuf* ibuf);

/*
* 缓冲区尾部删除一个字符
* 返回0为成功
*/
int inputbuf_del_tail(struct inputbuf* ibuf);

/*
* 获取缓冲区头部字符
* 返回负数为失败
*/
char inputbuf_get_head(struct inputbuf* ibuf);

/*
* 获取缓冲区尾部字符
* 返回负数为失败
*/
char inputbuf_get_tail(struct inputbuf* ibuf);

#ifdef __cplusplus
}
#endif

#endif
