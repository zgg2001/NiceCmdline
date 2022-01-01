/*************************************************************************
	> File Name: inputbuf.c
	> Author: ZHJ
	> Remarks: NiceCmdline中接收器的输入缓冲区，用于储存用户输入 
	> Created Time: Sat 01 Jan 2022 10:24:16 AM CST
 ************************************************************************/

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

