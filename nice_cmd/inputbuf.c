/*************************************************************************
	> File Name: inputbuf.c
	> Author: ZHJ
	> Remarks: NiceCmdline中接收器的输入缓冲区，用于储存用户输入 
	> Created Time: Sat 01 Jan 2022 10:24:16 AM CST
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

#include<errno.h>
#include"inputbuf.h"

int
inputbuf_init(struct inputbuf* ibuf, char* buf, unsigned int maxlen)
{
    if(!ibuf || !buf)
        return -EINVAL;
    ibuf->maxlen = maxlen;
    ibuf->len = 0;
    ibuf->start = 0;
    ibuf->end = ibuf->start;
    ibuf->buf = buf;
    return 0;
}

int 
inputbuf_add_head(struct inputbuf* ibuf, char c)
{
    if(!ibuf || INPUT_BUF_IS_FULL(ibuf))
        return -EINVAL;
    if(!INPUT_BUF_IS_EMPTY(ibuf))
    {
        ibuf->start += ibuf->maxlen;
        ibuf->start -= 1;
        ibuf->start %= ibuf->maxlen;
    }
    (ibuf->len)++;
    ibuf->buf[ibuf->start] = c;
    return 0;
}

int 
inputbuf_add_tail(struct inputbuf* ibuf, char c)
{
    if(!ibuf || INPUT_BUF_IS_FULL(ibuf))
        return -EINVAL;
    if(!INPUT_BUF_IS_EMPTY(ibuf))
    {
        ibuf->end += 1;
        ibuf->end %= ibuf->maxlen;
    }
    (ibuf->len)++;
    ibuf->buf[ibuf->end] = c;
    return 0;
}

int 
inputbuf_del_head(struct inputbuf* ibuf)
{
    if(!ibuf || INPUT_BUF_IS_EMPTY(ibuf))
        return -EINVAL;
    if(ibuf->len == 1)
    {
        ibuf->len = 0;
        ibuf->start = 0;
        ibuf->end = 0;
        return 0;
    }
    (ibuf->len)--;
    ibuf->start += 1;
    ibuf->start %= ibuf->maxlen;
    return 0;
}

int 
inputbuf_del_tail(struct inputbuf* ibuf)
{
    if(!ibuf || INPUT_BUF_IS_EMPTY(ibuf))
        return -EINVAL;
    if(ibuf->len == 1)
    {
        ibuf->len = 0;
        ibuf->start = 0;
        ibuf->end = 0;
        return 0;
    }
    (ibuf->len)--;
    ibuf->end += ibuf->maxlen;
    ibuf->end -= 1;
    ibuf->end %= ibuf->maxlen;
    return 0;
}

char 
inputbuf_get_head(struct inputbuf* ibuf)
{
    if(!ibuf || INPUT_BUF_IS_EMPTY(ibuf))
        return -EINVAL;
    return ibuf->buf[ibuf->start];
}

char 
inputbuf_get_tail(struct inputbuf* ibuf)
{
    if(!ibuf || INPUT_BUF_IS_EMPTY(ibuf))
        return -EINVAL;
    return ibuf->buf[ibuf->end];
}

