/*************************************************************************
	> File Name: parser_vt102.h
	> Author: ZHJ
	> Remarks: 依据vt102控制码对用户输入进行解析 参考部分dpdk-cmdline代码以及如下网址
    [http://www.ibiblio.org/pub/historic-linux/ftp-archives/tsx-11.mit.edu/Oct-07-1996/info/vt102.codes]
	> Created Time: Sat 01 Jan 2022 07:33:32 PM CST
 ************************************************************************/

#ifndef _PARSER_VT102_H_
#define _PARSER_VT102_H_

#ifdef __cplusplus
extern "C" 
{
#endif

/*
* vt102中对应的控制码
*/
#define vt102_bell         "\007"
#define vt102_bs           "\010"
#define vt102_bs_clear     "\010 \010"
#define vt102_tab          "\011"
#define vt102_crnl         "\012\015"
#define vt102_clear_right  "\033[0K"
#define vt102_clear_left   "\033[1K"
#define vt102_clear_down   "\033[0J"
#define vt102_clear_up     "\033[1J"
#define vt102_clear_line   "\033[2K"
#define vt102_clear_screen "\033[2J"
#define vt102_up_arr       "\033\133\101"
#define vt102_down_arr     "\033\133\102"
#define vt102_right_arr    "\033\133\103"
#define vt102_left_arr     "\033\133\104"
#define vt102_multi_right  "\033\133%uC"
#define vt102_multi_left   "\033\133%uD"
#define vt102_suppr        "\033\133\063\176"
#define vt102_home         "\033M\033E"
#define vt102_word_left    "\033\142"
#define vt102_word_right   "\033\146"

/*
* 可以输入识别的控制码
*/
const char *parser_vt102_commands[] = {
	vt102_up_arr,
	vt102_down_arr,
	vt102_right_arr,
	vt102_left_arr,
	"\177",
	"\n",
	"\001",
	"\005",
	"\013",
	"\031",
	"\003",
	"\006",
	"\002",
	vt102_suppr,
	vt102_tab,
	"\004",
	"\014",
	"\r",
	"\033\177",
	vt102_word_left,
	vt102_word_right,
	"?",
	"\027",
	"\020",
	"\016",
	"\033\144",
};

/*
* parser_vt102_commands[]中控制码对应的意义
*/
#define CMDLINE_KEY_UP_ARR 0
#define CMDLINE_KEY_DOWN_ARR 1
#define CMDLINE_KEY_RIGHT_ARR 2
#define CMDLINE_KEY_LEFT_ARR 3
#define CMDLINE_KEY_BKSPACE 4
#define CMDLINE_KEY_RETURN 5
#define CMDLINE_KEY_CTRL_A 6
#define CMDLINE_KEY_CTRL_E 7
#define CMDLINE_KEY_CTRL_K 8
#define CMDLINE_KEY_CTRL_Y 9
#define CMDLINE_KEY_CTRL_C 10
#define CMDLINE_KEY_CTRL_F 11
#define CMDLINE_KEY_CTRL_B 12
#define CMDLINE_KEY_SUPPR 13
#define CMDLINE_KEY_TAB 14
#define CMDLINE_KEY_CTRL_D 15
#define CMDLINE_KEY_CTRL_L 16
#define CMDLINE_KEY_RETURN2 17
#define CMDLINE_KEY_META_BKSPACE 18
#define CMDLINE_KEY_WLEFT 19
#define CMDLINE_KEY_WRIGHT 20
#define CMDLINE_KEY_HELP 21
#define CMDLINE_KEY_CTRL_W 22
#define CMDLINE_KEY_CTRL_P 23
#define CMDLINE_KEY_CTRL_N 24
#define CMDLINE_KEY_META_D 25

/*
* 控制码解析器状态
*/
enum parser_vt102_status
{
    PARSER_VT102_INIT,
    PARSER_VT102_ESCAPE,
    PARSER_VT102_ESCAPE_CSI
};

#define PARSER_VT102_BUF_SIZE 8

/*
* 控制码解析器结构体
* 负责对用户输入的控制码进行解析
*
* buf_pos: 缓冲区当前位置
*     buf: 缓冲区内存
*  status: 解析器状态
*/
struct parser_vt102
{
    unsigned int buf_pos;
	char buf[PARSER_VT102_BUF_SIZE];
	enum parser_vt102_status status;
};

/*
* 解析器初始化
*/
void parser_vt102_init(struct parser_vt102* p);

/*
* 对解析器缓冲区内储存的控制码进行比对解析
* 返回值为-1为比对失败(非控制码)
* 其余返回值为parser_vt102_commands[]下标
*/
int parser_match_command(char* buf, unsigned int size);

/*
* 对用户输入的字符进行解析 
* 匹配是否为控制码
*
* 返回值为-1说明此字符不是控制码，可以直接对照ASCII处理
* 返回值为-2说明此字符是控制码的一部分，且没有结束，不进行处理
* 其余返回值对应parser_vt102_commands[]的下标，参考宏定义即可
*/
int parse_vt102_char(struct parser_vt102* p, char c);

#ifdef __cplusplus
}
#endif

#endif
