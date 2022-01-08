/*************************************************************************
	> File Name: history.h
	> Author: ZHJ
	> Remarks: Nice_Cmdline配套历史记录系统
	> Created Time: Sat 08 Jan 2022 11:16:36 AM CST
 ************************************************************************/

#ifndef _HISTORY_H_
#define _HISTORY_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
* 命令结构体 储存着一条历史命令
* 双向链表结构
*
*  cmd: 指向储存命令的内存(malloc/需free)
*  len: 命令长度
* prev: 上一条命令
* next: 下一条命令
*/
struct command
{
    char* cmd;
    int len;
    struct command* prev;
    struct command* next;
};
typedef struct command history_cmd_t;

/*
* 历史记录主体
* head->prev = 尾历史记录
* head->next = 头历史记录
*
*                 head: 头节点 记录历史命令头尾(malloc/需free)
*                  now: 指向当前历史命令
*      history_cmd_num: 当前历史记录数
*  history_cmd_max_num: 最大历史记录数 0为无限制 负数为不记录
* command_buf_max_size: struct command中cmd的最大size
*
* [以下两个变量用于用户查询历史记录时,储存已经输入的内容]
*       user_input_buf: 用户输入缓冲区(malloc/需free)
*   user_input_buf_len: 输入长度
*/
struct history
{
    history_cmd_t* head;
    history_cmd_t* now;
    int history_cmd_num;
    int history_cmd_max_num;
    int command_buf_max_size;
    char* user_input_buf;
    int user_input_buf_len;
}

/*
* 新建struct command并返回
*
*      cmd: 需要储存的命令
*      len: 需要储存命令的长度
* max_size: 最大储存长度/对应command_buf_max_size
*
* 此处使用malloc创建,返回一个struct command指针
*/
history_cmd_t* new_command(char* cmd, int len, int max_size);

/*
* free指定struct command
* 首先free(command->cmd)
* 随后free(command)
*
* command: 指向要free的命令结构体
*/
void delete_command(history_cmd_t* command);

/*
* 历史记录系统初始化
*
*     hist: 指向要初始化的struct history
*  max_num: history中最多记录max_num条命令
* max_size: history中单条命令的最大size
*/
void history_init(struct history* hist, int max_num, int max_size);

/*
* 添加新历史记录
* 
* hist: 指向要要添加命令的struct history
*  cmd: 要添加的命令
*  len: 命令的长度
*
* 返回0为成功 -1为失败
*/
int history_add_new(struct history* hist, char* cmd, int len);

/*
* 删除最久的一条历史记录
* 即head->next
* 
* hist: 指向要要删除命令的struct history
*
* 返回0为成功 -1为失败
*/
int history_del_head(struct history* hist);

/*
* free历史记录系统
*
* hist: 指向要free的struct history
*/
void history_free(struct history* hist);

#ifdef __cplusplus
}
#endif

#endif
