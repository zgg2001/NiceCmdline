/*************************************************************************
	> File Name: test.c
	> Author: ZHJ
	> Remarks: example
	> Created Time: Fri 07 Jan 2022 10:24:16 AM CST
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<nice_cmd/cmdline.h>
#include<nice_cmd/parse_string.h>
#include<nice_cmd/parse_num.h>

//全局变量mode和max
unsigned int mode = 0;
unsigned int max = 0;

/*
* 命令 exit 退出
* 此命令中仅有一个字符串令牌
*/
//储存命令结果
struct cmd_exit_result
{
    fixed_string_t exit;
};
//命令匹配后执行的回调函数
static void
cmd_exit_parsed(struct cmdline* cl, void* parsed_result, void* data)
{
    cmdline_quit(cl);
}
//若干令牌 对应result结构体
parse_token_string_t cmd_exit_tok =
    TOKEN_STRING_INITIALIZER(struct cmd_exit_result, exit, "exit");
//命令结构体
parse_inst_t cmd_exit = {
    .f = cmd_exit_parsed,
    .data = NULL,
    .help_str = "exit",
    .tokens = {
        (void*)&cmd_exit_tok,
        NULL,
    },
};

/*
* 命令 设置
* 此命令中存有两个字符串令牌和一个数字令牌
* 第二个字符串令牌可以匹配两条字符串
* 数字令牌类型为UINT32
*/
//储存命令结果
struct cmd_set_result
{
    fixed_string_t str;
    fixed_string_t content;
    int num;
};
//命令匹配后执行的回调函数
static void
cmd_set_parsed(struct cmdline* cl, void* parsed_result, void* data)
{
    struct cmd_set_result* result = parsed_result;
    if(!strcmp(result->content, "mode"))
    {
        if(result->num <= max)
        {
            printf("mode已变更->%d\n", result->num);
            mode = result->num;
        }
        else
            printf("mode设定超限\n");
    }
    else if(!strcmp(result->content, "max"))
    {
        printf("max已更新->%d\n", result->num);
        max = result->num;
    }
}
//若干令牌 对应result结构体
parse_token_string_t cmd_set_tok1 =
    TOKEN_STRING_INITIALIZER(struct cmd_set_result, str, "set");
parse_token_string_t cmd_set_tok2 =
    TOKEN_STRING_INITIALIZER(struct cmd_set_result, content, "mode#max");
parse_token_num_t cmd_set_tok3 = 
    TOKEN_NUM_INITIALIZER(struct cmd_set_result, num, UINT32);
//命令结构体
parse_inst_t cmd_set = {
    .f = cmd_set_parsed,
    .data = NULL,
    .help_str = "set mode|max X",
    .tokens = {
        (void*)&cmd_set_tok1,
        (void*)&cmd_set_tok2,
        (void*)&cmd_set_tok3,
        NULL,
    },
};

/*
* 命令 获取
* 此命令中存有两个字符串令牌
* 第二个字符串令牌可以匹配两条字符串
*/
//储存命令结果
struct cmd_get_result
{
    fixed_string_t str;
    fixed_string_t content;
};
//命令匹配后执行的回调函数
static void
cmd_get_parsed(struct cmdline* cl, void* parsed_result, void* data)
{
    struct cmd_get_result* result = parsed_result;
    if(!strcmp(result->content, "mode"))
    {
        printf("当前mode = %d\n", mode);
    }
    else if(!strcmp(result->content, "max"))
    {
        printf("当前max = %d\n", max);
    }
}
//若干令牌 对应result结构体
parse_token_string_t cmd_get_tok1 =
    TOKEN_STRING_INITIALIZER(struct cmd_get_result, str, "get");
parse_token_string_t cmd_get_tok2 =
    TOKEN_STRING_INITIALIZER(struct cmd_get_result, content, "mode#max");
//命令结构体
parse_inst_t cmd_get = {
    .f = cmd_get_parsed,
    .data = NULL,
    .help_str = "get mode|max",
    .tokens = {
        (void*)&cmd_get_tok1,
        (void*)&cmd_get_tok2,
        NULL,
    },
};

/* 
* 命令组
*/
parse_ctx_t main_ctx[] = {
    (parse_inst_t*)&cmd_exit,
    (parse_inst_t*)&cmd_set,
    (parse_inst_t*)&cmd_get,
    NULL,
};

int main()
{
    struct cmdline* cl;
    //printf("sizeof(struct cmdline) = %d\n", sizeof(struct cmdline));
    //new
    cl = cmdline_get_new(main_ctx, "Nice_Cmdline>");
    //开始交互
    cmdline_start_interact(cl);
    //退出
    cmdline_exit_free(cl);
}
