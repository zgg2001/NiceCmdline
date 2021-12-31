/*************************************************************************
	> File Name: inputbuf.h
	> Author: ZHJ
	> Remarks: NiceCmdline中接收器的输入缓冲区，用于储存用户输入
	> Created Time: Fri 31 Dec 2021 09:09:13 PM CST
 ************************************************************************/

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
* 参数一: 缓冲区最大长度
* 参数二: 缓冲区当前长度
* 参数三: 缓冲区起点
* 参数四: 缓冲区终点
* 参数五: 缓冲区指针
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
* 缓冲区初始化函数
*/
int inputbuf_init();

/*
* 缓冲区头部添加字符
*/
int inputbuf_add_head();

/*
* 缓冲区尾部添加字符
*/
int inputbuf_add_tail();

/*
* 缓冲区头部删除字符
*/
int inputbuf_del_head();

/*
* 缓冲区尾部删除字符
*/
int inputbuf_del_tail();

/*
* 获取缓冲区头部字符
*/
int inputbuf_get_head();

/*
* 获取缓冲区尾部字符
*/
int inputbuf_get_tail();

#ifdef __cplusplus
}
#endif

#endif
