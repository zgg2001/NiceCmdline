/*************************************************************************
	> File Name: receiver.c
	> Author: ZHJ
	> Remarks: struct cmdline的配套接收器相关
	> Created Time: Sun 02 Jan 2022 11:25:44 AM CST
 ************************************************************************/

#include<errno.h>
#include<string.h>
#include"receiver.h"

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
    //重置缓冲区 控制码解析器/左右缓冲
    parser_vt102_init(&recv->vt102);
    inputbuf_init(&recv->left_buf, recv->left, INPUT_BUF_MAX_SIZE);
    inputbuf_init(&recv->right_buf, recv->right, INPUT_BUF_MAX_SIZE);
    //输出prompt
    recv->prompt_size = strnlen(prompt, INPUT_BUF_MAX_SIZE - 1);
    if(prompt != recv->prompt)
		memcpy(recv->prompt, prompt, recv->prompt_size);
    recv->prompt[INPUT_BUF_MAX_SIZE - 1] = '\0';
    unsigned int i;
    for(i = 0; i < recv->prompt_size; ++i)
		recv->write_char(recv, recv->prompt[i]);
    recv->status = RECEIVER_RUNNING;
    return 0;
}








