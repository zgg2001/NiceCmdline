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

#include<stdio.h>
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
static void display_right_buffer(struct receiver* recv, int force);

/* a very very basic printf with one arg and one format 'u' */
static void receiver_miniprintf(struct receiver* recv, const char* buf, unsigned int val);

int 
receiver_init(struct receiver* recv, func_write_char* write_char, func_parse_cmd* parse_cmd, func_complete_cmd* complete_cmd)
{
    if(!recv || !write_char)
        return -EINVAL;
    
    memset(recv->left, 0, INPUT_BUF_MAX_SIZE);
    memset(recv->right, 0, INPUT_BUF_MAX_SIZE);
    memset(recv, 0, sizeof(*recv));

    //初始化历史记录系统 储存上限为HISTORY_MAX_NUM条
    history_init(&recv->hist, HISTORY_MAX_NUM, INPUT_BUF_MAX_SIZE);

    recv->status = RECEIVER_INIT;
    recv->write_char = write_char;
    recv->parse_cmd = parse_cmd;
    recv->complete_cmd = complete_cmd;
    
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

void 
receiver_quit(struct receiver* recv)
{
    if(!recv)
        return;
    recv->status = RECEIVER_EXITED;
}

const char*
receiver_combi_cmd(struct receiver* recv, int mode)
{
    if(!recv)
        return NULL;

    struct inputbuf* left = &recv->left_buf;
    struct inputbuf* right = &recv->right_buf;
    char* all = recv->all_cmd;
    
    int l_len, r_len, i, now = 0;
    char c;

    l_len = recv->left_buf.len;
    r_len = recv->right_buf.len;

    //合并左缓冲区
    if(left->start == 0)
    {
        memcpy(all, recv->left, l_len);
    }
    else
    {
        INPUTBUF_FOREACH(left, i, c)
        {
            all[now] = c;
            ++now;
        }
    }

    if(mode == 0)
    {
        //合并右缓冲区
        if(right->start == 0)
        {
            memcpy(all + l_len, recv->right, r_len);
        }
        else
        {
            INPUTBUF_FOREACH(right, i, c)
            {
                all[now] = c;
                ++now;
            } 
        }
        all[l_len + r_len] = '\n';
	    all[l_len + r_len + 1] = '\0';
        return all;
    }

    //收尾
    all[l_len] = '\0';
    return all;
}

int
receiver_parse_char(struct receiver* recv, char c)
{
    if(!recv)
        return -EINVAL;
    if(recv->status == RECEIVER_EXITED)
        return RECEIVER_RES_EXITED;
    if(recv->status != RECEIVER_RUNNING)
        return RECEIVER_RES_NOT_RUNNING;

    int cmd;
    char temp_char;
    unsigned int i;
    char* temp_str;

    cmd = parse_vt102_char(&recv->vt102, c);

    //字符c为控制码的一部分且没有结束
    if(cmd == -2)
        return RECEIVER_RES_SUCCESS;
    
    //字符c组成了完整的控制码
    if(cmd >= 0)
    {
        switch(cmd)
        {
            //ctrl p - 历史记录向上
            case CMDLINE_KEY_UP_ARR:
            case CMDLINE_KEY_CTRL_P:
                if((temp_str = history_get_prev(&recv->hist)) != NULL)
                {
                    //如果当前为用户输入 则需进行记录
                    if(IS_NOT_HISTORY_CMD(&recv->hist))
                    {
                        receiver_combi_cmd(recv, 0);
                        history_save_user_input(&recv->hist, recv->all_cmd);
                    }
                    //清空输入缓冲区
                    parser_vt102_init(&recv->vt102);
                    inputbuf_init(&recv->left_buf, recv->left, INPUT_BUF_MAX_SIZE);
                    inputbuf_init(&recv->right_buf, recv->right, INPUT_BUF_MAX_SIZE);
                    //写入历史记录
                    for(i = 0; temp_str[i] != '\0'; ++i)
                    {
                        inputbuf_add_tail(&recv->left_buf, temp_str[i]);
                    }
                    receiver_redisplay(recv);
                }
            break;
            
            //ctrl n - 历史记录向下
            case CMDLINE_KEY_DOWN_ARR:
            case CMDLINE_KEY_CTRL_N:
                if((temp_str = history_get_next(&recv->hist)) != NULL)
                {
                    //清空输入缓冲区
                    parser_vt102_init(&recv->vt102);
                    inputbuf_init(&recv->left_buf, recv->left, INPUT_BUF_MAX_SIZE);
                    inputbuf_init(&recv->right_buf, recv->right, INPUT_BUF_MAX_SIZE);
                    //写入历史记录
                    for(i = 0; temp_str[i] != '\0'; ++i)
                    {
                        inputbuf_add_tail(&recv->left_buf, temp_str[i]);
                    }
                    receiver_redisplay(recv);
                }
            break;
            
            //ctrl f - 光标向右
            case CMDLINE_KEY_RIGHT_ARR:
            case CMDLINE_KEY_CTRL_F:
                if(INPUT_BUF_IS_EMPTY(&recv->right_buf))
                    break;
                temp_char = inputbuf_get_head(&recv->right_buf);
                inputbuf_del_head(&recv->right_buf);
                inputbuf_add_tail(&recv->left_buf, temp_char);
                receiver_puts(recv, vt102_right_arr);
            break;
            
            //ctrl b - 光标向左
            case CMDLINE_KEY_LEFT_ARR:
            case CMDLINE_KEY_CTRL_B:
                if(INPUT_BUF_IS_EMPTY(&recv->left_buf))
                    break;
                temp_char = inputbuf_get_tail(&recv->left_buf);
                inputbuf_del_tail(&recv->left_buf);
                inputbuf_add_head(&recv->right_buf, temp_char);
                receiver_puts(recv, vt102_left_arr);
            break;
            
            //退格 - 删除光标左侧的第一个字符
            case CMDLINE_KEY_BKSPACE:
                if(INPUT_BUF_IS_EMPTY(&recv->left_buf))
                    break;
                inputbuf_del_tail(&recv->left_buf);
                receiver_puts(recv, vt102_bs);
                display_right_buffer(recv, 1);
            break;
            
            //回车 - 执行命令
            case CMDLINE_KEY_RETURN:
            case CMDLINE_KEY_RETURN2:
                receiver_combi_cmd(recv, 0);
                recv->status = RECEIVER_INIT;
                receiver_puts(recv, "\r\n");
                if(recv->parse_cmd)
                {
                    //新命令长度大于0时进行记录
                    cmd = strnlen(recv->all_cmd, INPUT_BUF_MAX_SIZE * 2) - 1;
                    if(cmd > 0)
                    {
                        history_add_new(&recv->hist, recv->all_cmd, cmd, 0);
                    }
                    //解析
                    recv->parse_cmd(recv, recv->all_cmd);
                }
                if(recv->status == RECEIVER_EXITED)
                    return RECEIVER_RES_EXITED;
                return RECEIVER_RES_PARSED;
            break;
            
            //ctrl a - 移动光标至最左
            case CMDLINE_KEY_CTRL_A:
                if(INPUT_BUF_IS_EMPTY(&recv->left_buf))
                    break;
                receiver_miniprintf(recv, vt102_multi_left, recv->left_buf.len);
                while(!INPUT_BUF_IS_EMPTY(&recv->left_buf)) 
                {
                    temp_char = inputbuf_get_tail(&recv->left_buf);
                    inputbuf_del_tail(&recv->left_buf);
                    inputbuf_add_head(&recv->right_buf, temp_char);
                }
            break;
            
            //ctrl e - 移动光标至最右
            case CMDLINE_KEY_CTRL_E:
                if(INPUT_BUF_IS_EMPTY(&recv->right_buf))
                    break;
                receiver_miniprintf(recv, vt102_multi_right, recv->right_buf.len);
                while(!INPUT_BUF_IS_EMPTY(&recv->right_buf)) 
                {
                    temp_char = inputbuf_get_head(&recv->right_buf);
                    inputbuf_del_head(&recv->right_buf);
                    inputbuf_add_tail(&recv->left_buf, temp_char);
                }
            break;
            
            //ctrl k - 剪切光标右侧的内容
            case CMDLINE_KEY_CTRL_K:
                if(INPUT_BUF_IS_EMPTY(&recv->right_buf))
                    break;
                inputbuf_init(&recv->paste_buf, recv->paste, INPUT_BUF_MAX_SIZE * 2);
                INPUTBUF_FOREACH(&recv->right_buf, i, temp_char)
                {
                    inputbuf_add_tail(&recv->paste_buf, temp_char);
                }
                inputbuf_init(&recv->right_buf, recv->right, INPUT_BUF_MAX_SIZE);
                receiver_puts(recv, vt102_clear_right);
            break;
            
            //ctrl y - 粘贴剪切的内容
            case CMDLINE_KEY_CTRL_Y:
                if(INPUT_BUF_IS_EMPTY(&recv->paste_buf))
                    break;
                INPUTBUF_FOREACH(&recv->paste_buf, i, temp_char)
                {
                    if(recv->left_buf.len >= INPUT_BUF_MAX_SIZE - 1)
                        break;
                    inputbuf_add_tail(&recv->left_buf, temp_char);
                    recv->write_char(recv, temp_char);
                }
                display_right_buffer(recv, 0);
            break;
            
            //ctrl c - 重置命令行
            case CMDLINE_KEY_CTRL_C:
                receiver_puts(recv, "\r\n");
                receiver_new_cmdline(recv, recv->prompt);
            break;
            
            //delete / ctrl d - 删除光标右侧的第一个字符
            case CMDLINE_KEY_SUPPR:
            case CMDLINE_KEY_CTRL_D:
                if(cmd == CMDLINE_KEY_CTRL_D && 
                    INPUT_BUF_IS_EMPTY(&recv->left_buf) &&
                    INPUT_BUF_IS_EMPTY(&recv->right_buf)) 
                {
                    return RECEIVER_RES_EOF;
                }
                
                if(INPUT_BUF_IS_EMPTY(&recv->right_buf))
                    break;
                inputbuf_del_head(&recv->right_buf);    
                display_right_buffer(recv, 1);
            break;
            
            //tab - 尝试补全命令
            case CMDLINE_KEY_TAB:
            case CMDLINE_KEY_HELP:
                if(recv->complete_cmd) 
                {
                    char tmp_buf[BUFSIZ];
                    unsigned int tmp_size;
                    int complete_state;
                    int ret;
                   
                    //将左缓冲区copy至all_cmd 
                    receiver_combi_cmd(recv, 1);
                    
                    //确定是补全还是help
                    if(cmd == CMDLINE_KEY_TAB)
                        complete_state = 0;
                    else
                        complete_state = -1;
                    
                    //初次complete
                    ret = recv->complete_cmd(recv, recv->all_cmd, &complete_state, tmp_buf, sizeof(tmp_buf));
                    
                    //无法补全 or 出错
                    if(ret <= 0) 
                    {
                        return RECEIVER_RES_COMPLETE;
                    }
                    
                    tmp_size = strnlen(tmp_buf, sizeof(tmp_buf));
                    
                    //可补全
                    if(ret == RECEIVER_RES_COMPLETE) 
                    {
                        for(i = 0; i < tmp_size; ++i) 
                        {
                            if(recv->left_buf.len >= INPUT_BUF_MAX_SIZE - 1)
                                break;
                            inputbuf_add_tail(&recv->left_buf, tmp_buf[i]);
                            recv->write_char(recv, tmp_buf[i]);
                        }
                        display_right_buffer(recv, 1);
                        return RECEIVER_RES_COMPLETE;
                    }
                    
                    //存在多种补全可能性
                    receiver_puts(recv, "\r\n");
                    while(ret)
                    {
                        recv->write_char(recv, ' ');
                        for(i = 0; tmp_buf[i]; ++i)
                            recv->write_char(recv, tmp_buf[i]);
                        receiver_puts(recv, "\r\n");
                        //多次complete来取得所有可能性并打印
                        ret = recv->complete_cmd(recv, recv->all_cmd, &complete_state, tmp_buf, sizeof(tmp_buf));
                    }
                    receiver_redisplay(recv);
                }
                return RECEIVER_RES_COMPLETE; 
            break;
            
            //ctrl l - 清屏
            case CMDLINE_KEY_CTRL_L:
                receiver_redisplay(recv);
            break;
            
            //alt 退格 / ctrl w - 删除光标左侧的第一个词
            case CMDLINE_KEY_META_BKSPACE:
            case CMDLINE_KEY_CTRL_W:
                if(INPUT_BUF_IS_EMPTY(&recv->left_buf))
                    break;
                inputbuf_init(&recv->paste_buf, recv->paste, INPUT_BUF_MAX_SIZE * 2);
                while(!INPUT_BUF_IS_EMPTY(&recv->left_buf) &&
                      (temp_char = inputbuf_get_tail(&recv->left_buf)) &&
                      isblank(temp_char))
                {
                    inputbuf_add_head(&recv->paste_buf, temp_char);
                    inputbuf_del_tail(&recv->left_buf);
                    receiver_puts(recv, vt102_bs);
                }
                while(!INPUT_BUF_IS_EMPTY(&recv->left_buf) &&
                      (temp_char = inputbuf_get_tail(&recv->left_buf)) &&
                      !isblank(temp_char))
                {
                    inputbuf_add_head(&recv->paste_buf, temp_char);
                    inputbuf_del_tail(&recv->left_buf);
                    receiver_puts(recv, vt102_bs);
                }
                display_right_buffer(recv, 1);
            break;
            
            //alt d - 删除光标右侧的第一个词
            case CMDLINE_KEY_META_D:
                if(INPUT_BUF_IS_EMPTY(&recv->right_buf))
                    break;
                inputbuf_init(&recv->paste_buf, recv->paste, INPUT_BUF_MAX_SIZE * 2);
                while(!INPUT_BUF_IS_EMPTY(&recv->right_buf) &&
                      (temp_char = inputbuf_get_head(&recv->right_buf)) &&
                      isblank(temp_char))
                {
                    inputbuf_add_tail(&recv->paste_buf, temp_char);
                    inputbuf_del_head(&recv->right_buf);
                }
                while(!INPUT_BUF_IS_EMPTY(&recv->right_buf) &&
                      (temp_char = inputbuf_get_head(&recv->right_buf)) &&
                      !isblank(temp_char))
                {
                    inputbuf_add_tail(&recv->paste_buf, temp_char);
                    inputbuf_del_head(&recv->right_buf);
                }  
                display_right_buffer(recv, 1);
            break;
            
            //alt b - 光标向左移动到当前单词最前端
            case CMDLINE_KEY_WLEFT:
                while(!INPUT_BUF_IS_EMPTY(&recv->left_buf) &&
                      (temp_char = inputbuf_get_tail(&recv->left_buf)) &&
                      isblank(temp_char))
                {
                    inputbuf_del_tail(&recv->left_buf);
                    inputbuf_add_head(&recv->right_buf, temp_char);
                    receiver_puts(recv, vt102_left_arr);
                }
                while(!INPUT_BUF_IS_EMPTY(&recv->left_buf) &&
                      (temp_char = inputbuf_get_tail(&recv->left_buf)) &&
                      !isblank(temp_char))
                {
                    inputbuf_del_tail(&recv->left_buf);
                    inputbuf_add_head(&recv->right_buf, temp_char);
                    receiver_puts(recv, vt102_left_arr);
                }
            break;
            
            //alt f - 光标向右移动到当前单词最后端
            case CMDLINE_KEY_WRIGHT:
                while(!INPUT_BUF_IS_EMPTY(&recv->right_buf) &&
                      (temp_char = inputbuf_get_head(&recv->right_buf)) &&
                      isblank(temp_char))
                {
                    inputbuf_del_head(&recv->right_buf);
                    inputbuf_add_tail(&recv->left_buf, temp_char);
                    receiver_puts(recv, vt102_right_arr);
                }
                while(!INPUT_BUF_IS_EMPTY(&recv->right_buf) &&
                      (temp_char = inputbuf_get_head(&recv->right_buf)) &&
                      !isblank(temp_char))
                {
                    inputbuf_del_head(&recv->right_buf);
                    inputbuf_add_tail(&recv->left_buf, temp_char);
                    receiver_puts(recv, vt102_right_arr);
                } 
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
    display_right_buffer(recv, 0);
    return RECEIVER_RES_SUCCESS;
}

void
receiver_redisplay(struct receiver* recv)
{
    if(!recv)
        return;
    
    unsigned int i;
    char temp_char;

    receiver_puts(recv, vt102_home);
    for(i = 0; i < recv->prompt_size; ++i)
        recv->write_char(recv, recv->prompt[i]);
    INPUTBUF_FOREACH(&recv->left_buf, i, temp_char)
    {
        recv->write_char(recv, temp_char);
    }
    display_right_buffer(recv, 1);
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
display_right_buffer(struct receiver* recv, int force)
{
    if(!recv || (!force && INPUT_BUF_IS_EMPTY(&recv->right_buf)))
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

