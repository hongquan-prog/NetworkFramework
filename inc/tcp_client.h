#pragma once

#include "msg_parser.h"

typedef struct
{
    int fd;
    msg_parser_t *parser;
    void *data;
} tcp_client_t;

tcp_client_t *tcp_client_new(void);
tcp_client_t *tcp_client_from_fd(int fd);

int tcp_client_send_msg(tcp_client_t *client, msg_t *msg);
int tcp_client_send_raw(tcp_client_t *client, char *buf, int len);
msg_t *tcp_client_recv_msg(tcp_client_t *client);
int tcp_client_recv_raw(tcp_client_t *client, char *buf, int len);

bool tcp_client_connect(tcp_client_t *client, char *ip, unsigned short port);
bool tcp_client_is_connected(tcp_client_t *client);
void tcp_client_close(tcp_client_t *client);
void tcp_client_delete(tcp_client_t *client);

int tcp_client_set_opt(tcp_client_t *client, int level, int optname, void *optval, unsigned int optlen);
int tcp_client_get_opt(tcp_client_t *client, int level, int optname, void *optval, unsigned int *optlen);

void tcp_client_set_data(tcp_client_t *client, void *data);
void *tcp_client_get_data(tcp_client_t *client);