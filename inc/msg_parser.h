#pragma once

#include "msg.h"
#include <stdbool.h>

typedef struct 
{
    /* 缓存已经解析的消息头 */
    msg_t headr_cache;
    /* 标识是否已经接收到消息头 */
    bool header_recved;
    /* 解析完成还需要接收的数据长度 */
    unsigned int remain;
    /* 解析中的协议消息 */
    msg_t *msg;
} msg_parser_t;

msg_parser_t *msg_parser_new();
msg_t *msg_parse_from_ram(msg_parser_t *parser, unsigned char *ram, unsigned int len);
msg_t *msg_parse_from_file(msg_parser_t *parser, int fd);
unsigned int msg_size(msg_t *msg);
void msg_parser_reset(msg_parser_t *parser);
void msg_parser_delete(msg_parser_t *parser);