/*************************************************************************
	> File Name: receiver.c
	> Author: ZHJ
	> Remarks: struct cmdline的配套接收器相关
	> Created Time: Sun 02 Jan 2022 11:25:44 AM CST
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
#include<string.h>
#include<ctype.h>
#include"receiver.h"

/*
* 内部函数 向输出流输出指定字符串
*/
static void receiver_puts(struct receiver* recv, const char* str);

/*
* 内部函数 输出接收器右部缓冲区
*/
static void display_right_buffer(struct receiver* recv);

/* a very very basic printf with one arg and one format 'u' */
static void receiver_miniprintf(struct receiver* recv, const char* buf, unsigned int val);

int 
receiver_init(struct receiver* recv, func_write_char* write_char)
{
    if(!recv || !write_char)
        return -EINVAL;
    
    memset(recv->left, 0, INPUT_BUF_MAX_SIZE);
    memset(recv->right, 0, INPUT_BUF_MAX_SIZE);
    memset(recv, 0, sizeof(*recv));
    
    recv->status = RECEIVER_INIT;
    recv->write_char = write_char;
    
    return 0;
}

int 
receiver_new_cmdline(struct receiver* recv, const char* prompt)
{
    if(!recv || !prompt)
        return -EINVAL;
    
    unsigned int i;
    
    //重置缓冲区 控制码解析器/左右缓冲
    parser_vt102_init(&recv->vt102);
    inputbuf_init(&recv->left_buf, recv->left, INPUT_BUF_MAX_SIZE);
    inputbuf_init(&recv->right_buf, recv->right, INPUT_BUF_MAX_SIZE);
    
    //输出prompt
    recv->prompt_size = strnlen(prompt, INPUT_BUF_MAX_SIZE - 1);
    if(prompt != recv->prompt)
		memcpy(recv->prompt, prompt, recv->prompt_size);
    recv->prompt[INPUT_BUF_MAX_SIZE - 1] = '\0';
    for(i = 0; i < recv->prompt_size; ++i)
		recv->write_char(recv, recv->prompt[i]);
    
    recv->status = RECEIVER_RUNNING;
    
    return 0;
}

int
receiver_parse_char(struct receiver* recv, char c)
{
    if(!recv)
        return -EINVAL;
    if(recv->status != RECEIVER_RUNNING)
        return RECEIVER_RES_NOT_RUNNING;
    if(recv->status == RECEIVER_EXITED)
        return RECEIVER_RES_EXITED;

    int cmd;
    
    cmd = parse_vt102_char(&recv->vt102, c);

    //字符c为控制码的一部分且没有结束
    if(cmd == -2)
        return RECEIVER_RES_SUCCESS;
    
    //字符c组成了完整的控制码
    if(cmd >= 0)
    {
        switch(cmd)
        {
        //ctrl c
        case CMDLINE_KEY_CTRL_C:
            receiver_puts(recv, "\r\n");
            receiver_new_cmdline(recv, recv->prompt);
        break;
        }
        return RECEIVER_RES_SUCCESS;
    }
    
    //字符c非控制码
    if(!isprint((int)c))//不可打印
        return RECEIVER_RES_SUCCESS;
    if(inputbuf_add_tail(&recv->left_buf, c) < 0)//左缓冲区溢出
        return RECEIVER_RES_SUCCESS;
    recv->write_char(recv, c);
	display_right_buffer(recv);
    return RECEIVER_RES_SUCCESS;
}

static void
receiver_puts(struct receiver* recv, const char* str)
{
    if(!recv || !str)
        return;

    size_t len = strnlen(str, INPUT_BUF_MAX_SIZE), i;
    for(i = 0; i < len; ++i)
        recv->write_char(recv, str[i]);
}

static void
display_right_buffer(struct receiver* recv)
{
	if(!recv || INPUT_BUF_IS_EMPTY(&recv->right_buf))
        return;

    unsigned int i;
	char c;

    //消除光标右部数据并再次显示
	receiver_puts(recv, vt102_clear_right);
	INPUTBUF_FOREACH(&recv->right_buf, i, c) 
    {
		recv->write_char(recv, c);
	}
	if(!INPUT_BUF_IS_EMPTY(&recv->right_buf))
        receiver_miniprintf(recv, vt102_multi_left, (recv->right_buf).len);
}

/* a very very basic printf with one arg and one format 'u' */
static void
receiver_miniprintf(struct receiver* recv, const char* buf, unsigned int val)
{
    if(!recv || !buf)
        return;

	char c, started=0, div=100;

	while((c = *(buf++))) 
    {
        if(c != '%')
        {
            recv->write_char(recv, c);
            continue;
        }
        c = *(buf++);
        if (c != 'u') 
        {
            recv->write_char(recv, '%');
            recv->write_char(recv, c);
            continue;
        }
        /* val is never more than 255 */
        while (div) 
        {
            c = (char)(val / div);
            if (c || started) 
            {
                recv->write_char(recv, (char)(c+'0'));
                started = 1;
            }
            val %= div;
            div /= 10;
        }
    }
}

