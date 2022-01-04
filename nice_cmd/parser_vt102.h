/*************************************************************************
	> File Name: parser_vt102.h
	> Author: ZHJ
	> Remarks: 依据vt102控制码对用户输入进行解析
	> Created Time: Sat 01 Jan 2022 07:33:32 PM CST
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
extern const char *parser_vt102_commands[];
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
