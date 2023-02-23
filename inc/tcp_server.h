#pragma once

#include "tcp_client.h"
#include <stdbool.h>

#define TCP_FD_SIZE 1024

typedef enum
{
    TCP_EVENT_CONNECT,
    TCP_EVENT_RECV_DATA,
    TCP_EVENT_CLOSE
} tcp_event_def;

typedef void (*tcp_event_cb_t)(tcp_client_t *client, tcp_event_def event);

typedef struct
{
    int fd;
    bool valid;
    tcp_event_cb_t cb;
    tcp_client_t *client[TCP_FD_SIZE];
} tcp_server_t;

tcp_server_t *tcp_server_new(void);
bool tcp_server_start(tcp_server_t *server, unsigned short port, int max);
void tcp_server_stop(tcp_server_t *server);
void tcp_server_reister_callback(tcp_server_t *server, tcp_event_cb_t func);
bool tcp_server_is_valid(tcp_server_t *server);
void tcp_server_exec(tcp_server_t *server);
tcp_client_t *tcp_server_accept(tcp_server_t *server);
int tcp_server_set_opt(tcp_server_t *server, int level, int optname, void *optval, unsigned int optlen);
int tcp_server_get_opt(tcp_server_t *server, int level, int optname, void *optval, unsigned int *optlen);
void tcp_server_delete(tcp_server_t *server);