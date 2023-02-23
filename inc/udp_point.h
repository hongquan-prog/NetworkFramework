#pragma once

#include "msg_parser.h"

typedef struct
{
    int fd;
    msg_parser_t *parser;
    void *data;
} udp_point_t;

udp_point_t *udp_point_new(unsigned short port);
udp_point_t *udp_point_from_fd(int fd);
void udp_point_delete(udp_point_t *point);

int udp_point_send_msg(udp_point_t *point, msg_t *msg, const char *remote, unsigned short port);
int udp_point_send_raw(udp_point_t *point, char *buf, int len, const char *remote, unsigned short port);
msg_t *udp_point_recv_msg(udp_point_t *point, char *remote, unsigned short *port);
int udp_point_recv_raw(udp_point_t *point, char *buf, int len, char *remote, unsigned short *port);

int udp_point_available(udp_point_t *point);
void udp_point_set_data(udp_point_t *point, void *data);
void *udp_point_get_data(udp_point_t *point);

int udp_point_set_opt(udp_point_t *point, int level, int optname, void *optval, unsigned int optlen);
int udp_point_get_opt(udp_point_t *point, int level, int optname, void *optval, unsigned int *optlen);