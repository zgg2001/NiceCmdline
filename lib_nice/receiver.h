/*************************************************************************
	> File Name: receiver.h
	> Author: ZHJ
	> Remarks: struct cmdline的配套接收器相关
	> Created Time: Wed 29 Dec 2021 10:19:11 PM CST
 ************************************************************************/

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include<lib_nice/inputbuf.h>
#include<lib_nice/parser_vt102.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
* 配置宏
*/
#define PROMPT_MAX_SIZE 32
#define INPUT_BUF_MAX_SIZE 512

/*
* 接收器配套回调函数
* func_write_char: 设定字符如何write至输出流
*/
struct receiver;
typedef int (func_write_char)(struct receiver*, char);

/*
* 接收器状态
*/
enum receiver_status 
{
	RECEIVER_INIT,
	RECEIVER_RUNNING,
	RECEIVER_EXITED
};

/*
* 接收器结构体
* 负责输入输出内容
*
*       owner: 此接收器所有者，指向一个struct cmdline
*      prompt: 接收器需要输出的提示符
* prompt_size: 接收器提示符的长度
*      status: 接收器状态
*
*       vt102: vt102控制码解析器
*
*    left_buf: 输入缓冲区的左部分
*        left: 左缓冲配套的内存
*   right_buf: 输入缓冲区的右部分
*       right: 右缓冲配套的内存
*/
struct receiver
{
    //基础属性
    void* owner;
    char prompt[PROMPT_MAX_SIZE];
    int prompt_size;
    enum receiver_status status;
    //控制码解析器
    struct parser_vt102 vt102; 
    //输入缓冲区
    struct inputbuf left_buf;
    char left[INPUT_BUF_MAX_SIZE];
    struct inputbuf right_buf;
    char right[INPUT_BUF_MAX_SIZE];
    //历史记录
    //回调函数
    func_write_char* write_char; 
};

/*
* receiver初始化
* 返回0为初始化成功
*/
int receiver_init(struct receiver* recv, func_write_char* write_char);

/*
* 创建新的命令行
* 对缓冲区进行重置/输出prompt
* 返回0为执行成功
*/
int receiver_new_cmdline(struct receiver* recv, const char* prompt);

#ifdef __cplusplus
}
#endif

#endif
