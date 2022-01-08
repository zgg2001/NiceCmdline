/*************************************************************************
	> File Name: history.c
	> Author: ZHJ
	> Remarks: Nice_Cmdline配套历史记录系统
	> Created Time: Sat 08 Jan 2022 04:42:13 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"history.h"

history_cmd_t* 
new_command(char* cmd, int len, int max_size)
{
    if(len > max_size)
        return NULL;

    history_cmd_t* ret;
    char* temp_ptr; 
    
    ret = (history_cmd_t*)malloc(sizeof(history_cmd_t));
    temp_ptr = (char*)malloc(sizeof(char) * (len + 1));
    
    strncpy(temp_ptr, cmd, len);
    temp_ptr[len] = '\0';

    ret->cmd = temp_ptr;
    ret->len = len;
    ret->prev = NULL;
    ret->next = NULL;
    return ret;
}

void 
delete_command(history_cmd_t* command)
{
    if(command == NULL)
        return;
    if(command->cmd != NULL)
        free(command->cmd);
    free(command);
}

void 
history_init(struct history* hist, int max_num, int max_size)
{
    if(!hist)
        return;
    
    hist->head = new_command("head-node", 9, max_size);
    hist->now = 0;
    hist->history_cmd_num = 0;
    hist->history_cmd_max_num = max_num;
    hist->command_buf_max_size = max_size;
    hist->user_input_buf = (char*)malloc(sizeof(char) * (max_size + 1));
    hist->user_input_buf_len = 0;
}

int 
history_add_new(struct history* hist, char* cmd, int len)
{
    if(!hist || !cmd || hist->history_cmd_max_num < 0)
        return -1;
    if(hist->head == NULL)
        return -1;

    history_cmd_t* temp_ptr;
    history_cmd_t* old_ptr;

    //获取新节点
    temp_ptr = new_command(cmd, len, hist->command_buf_max_size);
    if(temp_ptr == NULL)
        return -1;
    
    //插入链表
    if(hist->history_cmd_num == 0)//第一条
    {
        ++hist->history_cmd_num;
        hist->head->next = temp_ptr;
        hist->head->prev = temp_ptr;
        temp_ptr->next = hist->head;
        temp_ptr->prev = hist->head;
    }
    else//非第一条
    {
        ++hist->history_cmd_num;
        old_ptr = hist->head->prev;
        old_ptr->next = temp_ptr;
        temp_ptr->prev = old_ptr;
        temp_ptr->next = hist->head;
        hist->head->prev = temp_ptr;
    }
    
    //如果储存命令数量超限-删除head->next
    if(hist->history_cmd_max_num > 0 && 
       hist->history_cmd_num > hist->history_cmd_max_num)
    {
        history_del_head(hist);
    }

    return 0;
}

int
history_del_head(struct history* hist)
{
    if(!hist)
        return -1;
    if(hist->head == NULL)
        return -1;
    if(hist->history_cmd_num == 0)
        return 0;
    
    history_cmd_t* del_ptr;

    //delete
    if(hist->history_cmd_num == 1)//只有一个节点
    {
        del_ptr = hist->head->next;
        if(del_ptr == NULL)
            return -1;
        hist->head->next = NULL;
        hist->head->prev = NULL;
        --hist->history_cmd_num;
        delete_command(del_ptr);
    }
    else//多节点
    {
        del_ptr = hist->head->next;
        if(del_ptr == NULL)
            return -1;
        hist->head->next = del_ptr->next;
        del_ptr->next->prev = hist->head;
        --hist->history_cmd_num;
        delete_command(del_ptr);
    }
    return 0;
}

void 
history_free(struct history* hist)
{
    if(!hist)
        return;

    history_cmd_t* temp_ptr;
    history_cmd_t* del_ptr;

    //free每一条命令
    temp_ptr = hist->head->next;
    while(hist->history_cmd_num > 0)
    {
        del_ptr = temp_ptr;
        temp_ptr = temp_ptr->next;
        --hist->history_cmd_num;
        delete_command(del_ptr);
    }
    delete_command(hist->head);

    //free输入缓冲区
    free(hist->user_input_buf);
}


