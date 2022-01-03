/*************************************************************************
	> File Name: parser_vt102.c
	> Author: ZHJ
	> Remarks: 解析器 依据vt102控制码对用户输入进行解析
	> Created Time: Sat 01 Jan 2022 08:44:20 PM CST
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

#include<string.h>
#include"parser_vt102.h"

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

void
parser_vt102_init(struct parser_vt102* p)
{
    if(!p)
        return;
    p->buf_pos = 0;
    p->status = PARSER_VT102_INIT;
}

int
parser_match_command(char* buf, unsigned int size)
{
    const char *cmd;
	unsigned int cmdlen;
	unsigned int i = 0;
    //遍历parser_vt102_commands[]比对
	for(i = 0; i < sizeof(parser_vt102_commands) / sizeof(const char*); ++i) 
    {
        cmd = *(parser_vt102_commands + i);
        cmdlen = strnlen(cmd, PARSER_VT102_BUF_SIZE);
        if(size == cmdlen && !strncmp(buf, cmd, cmdlen)) 
        {
            return i;
        }
    }
    return -1;
}

int
parse_vt102_char(struct parser_vt102* p, char c)
{
	if(!p)
        return -1;
	if(p->buf_pos >= PARSER_VT102_BUF_SIZE) 
    {
        p->status = PARSER_VT102_INIT;
        p->buf_pos = 0;
	}
    //新字符加入解析器缓冲区
    unsigned int size;
	p->buf[p->buf_pos++] = c;
	size = p->buf_pos;
    //根据解析器状态进行控制码比对
	switch(p->status) 
    {
	case PARSER_VT102_INIT:
        if(c == 033) 
        {
            p->status = PARSER_VT102_ESCAPE;
        }
        else 
        {
            p->buf_pos = 0;
            return parser_match_command(p->buf, size);
        }
    break;
	case PARSER_VT102_ESCAPE:
        if(c == 0133) 
        {
            p->status = PARSER_VT102_ESCAPE_CSI;
        }
        else if(c >= 060 && c <= 0177) 
        {
            p->buf_pos = 0;
            p->status = PARSER_VT102_INIT;
            return parser_match_command(p->buf, size);
        }
    break;
	case PARSER_VT102_ESCAPE_CSI:
        if(c >= 0100 && c <= 0176) 
        {
            p->buf_pos = 0;
            p->status = PARSER_VT102_INIT;
            return parser_match_command(p->buf, size);
        }
	break;
	default:
        p->buf_pos = 0;
        break;
	}
	return -2;
}
