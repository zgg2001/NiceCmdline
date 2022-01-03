/*************************************************************************
	> File Name: receiver.c
	> Author: ZHJ
	> Remarks: struct cmdline的配套接收器相关
	> Created Time: Sun 02 Jan 2022 11:25:44 AM CST
 ************************************************************************/

#include<errno.h>
#include<string.h>
#include<ctype.h>
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
            recv->write_char(recv ,'\r');
            recv->write_char(recv ,'\n');
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
	//display_right_buffer(recv, 0);
    return RECEIVER_RES_SUCCESS;
}






