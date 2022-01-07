/*************************************************************************
	> File Name: test.c
	> Author: ZHJ
	> Remarks: example
	> Created Time: Fri 07 Jan 2022 10:24:16 AM CST
 ************************************************************************/

#include<stdio.h>
#include<nice_cmd/cmdline.h>
#include<nice_cmd/parse_string.h>

/*
* 命令 exit 退出
*/
//储存命令结果
struct cmd_exit_result
{
    fixed_string_t exit;
};

//命令匹配后执行的回调函数
static void
cmd_exit_parsed(struct cmdline *cl, void *parsed_result, void *data)
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
* 命令组
*/
parse_ctx_t main_ctx[] = {
    (parse_inst_t*)&cmd_exit,
    NULL,
};

int main()
{
    struct cmdline* cl;
    //new
    cl = cmdline_get_new(main_ctx, "Nice_Cmdline>");
    //开始交互
    cmdline_start_interact(cl);
    //退出
    cmdline_exit_free(cl);
}
