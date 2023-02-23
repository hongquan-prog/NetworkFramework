#include "tcp_client.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

tcp_client_t *tcp_client_new(void)
{
    return tcp_client_from_fd(-1);
}

tcp_client_t *tcp_client_from_fd(int fd)
{
    tcp_client_t *ret = malloc(sizeof(tcp_client_t));

    if (ret)
    {
        ret->data = NULL;
        ret->fd = fd;
        ret->parser = msg_parser_new();
    }

    return (ret && ret->parser) ? (ret) : (free(ret), NULL);
}

int tcp_client_send_msg(tcp_client_t *client, msg_t *msg)
{
    int ret = -1;
    unsigned int length = msg_size(msg);

    if (client && msg)
    {
        msg_hton(msg);
        ret = send(client->fd, msg, length, 0);
        msg_ntoh(msg);
    }

    return ret;
}

int tcp_client_send_raw(tcp_client_t *client, char *buf, int len)
{
    int ret = -1;

    if (client && buf)
    {
        ret = send(client->fd, buf, len, 0);
    }

    return ret;
}

int tcp_client_available(tcp_client_t *client)
{
    int ret = -1;
    static char temp[2048] = {0};

    if (client)
    {
        ret = recv(client->fd, temp, sizeof(temp), MSG_PEEK | MSG_DONTWAIT);
    }

    return ret;
}

msg_t *tcp_client_recv_msg(tcp_client_t *client)
{
    if (client)
    {
        return msg_parse_from_file(client->parser, client->fd);
    }

    return NULL;
}

int tcp_client_recv_raw(tcp_client_t *client, char *buf, int len)
{
    int ret = -1;

    if (client && buf)
    {
        ret = recv(client->fd, buf, len, 0);
    }

    return ret;
}

bool tcp_client_connect(tcp_client_t *client, char *ip, unsigned short port)
{
    struct sockaddr_in addr = {0};
    bool ret = tcp_client_is_connected(client);

    if (client && !ret && ip)
    {
        client->fd = socket(AF_INET, SOCK_STREAM, 0);

        if (client->fd)
        {
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip);
            addr.sin_family = AF_INET;
            ret = (0 == connect(client->fd, (struct sockaddr *)&addr, sizeof(addr)));
        }
    }

    return ret;
}

bool tcp_client_is_connected(tcp_client_t *client)
{
    bool ret = false;
    struct tcp_info info = {0};
    socklen_t info_len = sizeof(info);

    if (client)
    {
        getsockopt(client->fd, IPPROTO_TCP, TCP_INFO, &info, &info_len);
        ret = (info.tcpi_state == TCP_ESTABLISHED);
    }

    return ret;
}

void tcp_client_close(tcp_client_t *client)
{
    if (client)
    {
        close(client->fd);
        client->fd = -1;
        msg_parser_reset(client->parser);
    }
}

void tcp_client_set_data(tcp_client_t *client, void *data)
{
    if (client)
    {
        client->data = data;
    }
}

void *tcp_client_get_data(tcp_client_t *client)
{
    if (client)
    {
        return client->data;
    }

    return NULL;
}

int tcp_client_set_opt(tcp_client_t *client, int level, int optname, void *optval, unsigned int optlen)
{
    return setsockopt(client->fd, level, optname, optval, optlen);
}

int tcp_client_get_opt(tcp_client_t *client, int level, int optname, void *optval, unsigned int *optlen)
{
    return getsockopt(client->fd, level, optname, optval, optlen);
}

void tcp_client_delete(tcp_client_t *client)
{
    if (client)
    {
        tcp_client_close(client);
        msg_parser_delete(client->parser);
        free(client);
    }
}