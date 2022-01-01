/*************************************************************************
	> File Name: parser_vt102.c
	> Author: ZHJ
	> Remarks: 解析器 依据vt102控制码对用户输入进行解析
	> Created Time: Sat 01 Jan 2022 08:44:20 PM CST
 ************************************************************************/

#include<string.h>
#include"parser_vt102.h"

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
