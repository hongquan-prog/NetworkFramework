#include "udp_point.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>

#define UDP_TEMP_SIZE 4096

udp_point_t *udp_point_new(unsigned short port)
{
    bool err = false;
    udp_point_t *ret = NULL;
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port)};

    ret = malloc(sizeof(udp_point_t));
    if (!ret)
    {
        printf("no memory to create udp point\r\n");
        goto __exit;
    }

    ret->parser = msg_parser_new();
    if (!ret->parser)
    {
        printf("udp point create parser failed\r\n");
        goto __exit;
    }

    ret->fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (-1 == ret->fd)
    {
        printf("udp point create socket failed\r\n");
        goto __exit;
    }

    if (-1 == bind(ret->fd, (const struct sockaddr *)&addr, sizeof(addr)))
    {
        close(ret->fd);
        printf("udp point bind port failed\r\n");
        goto __exit;
    }

    ret->data = NULL;
    err = true;

__exit:

    if (!err)
    {
        (ret) ? (msg_parser_delete(ret->parser)) : (0);
        free(ret);
        ret = NULL;
    }

    return ret;
}

udp_point_t *udp_point_from_fd(int fd)
{
    bool err = false;
    udp_point_t *ret = malloc(sizeof(udp_point_t));

    if (!ret)
    {
        printf("no memory to create udp point\r\n");
        goto __exit;
    }

    ret->fd = fd;
    ret->data = NULL;
    ret->parser = msg_parser_new();
    if (!ret->parser)
    {
        printf("udp point create parser failed\r\n");
        goto __exit;
    }

    err = true;

__exit:

    if (!err)
    {
        (ret) ? (msg_parser_delete(ret->parser)) : (0);
        free(ret);
        ret = NULL;
    }

    return ret;
}

void udp_point_delete(udp_point_t *point)
{

    (point) ? (msg_parser_delete(point->parser), close(point->fd)) : (0);
    free(point);
}

static void udp_point_parse_addr(struct sockaddr_in *addr, char *ip, unsigned short *port)
{
    if (ip)
    {
        strcpy(ip, inet_ntoa(addr->sin_addr));
    }

    if (port)
    {
        *port = ntohs(addr->sin_port);
    }
}

int udp_point_available(udp_point_t *point)
{
    int ret = -1;
    static char temp[UDP_TEMP_SIZE];

    if (point)
    {
        struct sockaddr_in addr = {0};
        socklen_t len = sizeof(addr);
        ret = recvfrom(point->fd, temp, sizeof(temp), MSG_PEEK | MSG_DONTWAIT, (struct sockaddr *)&addr, &len);
    }

    return ret;
}

int udp_point_send_msg(udp_point_t *point, msg_t *msg, const char *remote, unsigned short port)
{
    int ret = -1;
    unsigned int length = msg_size(msg);

    if (point && msg)
    {
        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = inet_addr(remote),
            .sin_port = htons(port)};

        msg_hton(msg);
        ret = sendto(point->fd, msg, length, 0, (const struct sockaddr *)&addr, sizeof(addr));
        msg_ntoh(msg);
    }

    return ret;
}

int udp_point_send_raw(udp_point_t *point, char *buf, int len, const char *remote, unsigned short port)
{
    int ret = -1;

    if (point && buf && remote)
    {
        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = inet_addr(remote),
            .sin_port = htons(port)};

        ret = sendto(point->fd, buf, len, 0, (const struct sockaddr *)&addr, sizeof(addr));
    }

    return ret;
}

msg_t *udp_point_recv_msg(udp_point_t *point, char *remote, unsigned short *port)
{
    msg_t *ret = NULL;

    if (point)
    {
        struct sockaddr_in addr = {0};
        socklen_t addr_len = sizeof(addr);
        int data_len = udp_point_available(point);
        unsigned char *buf = (data_len > 0) ? (malloc(data_len)) : (NULL);
        int len = recvfrom(point->fd, buf, data_len, 0, (struct sockaddr *)&addr, &addr_len);

        if (0 < len)
        {
            ret = msg_parse_from_ram(point->parser, buf, len);
        }

        if (ret)
        {
            udp_point_parse_addr(&addr, remote, port);
        }

        free(buf);
    }

    return ret;
}

int udp_point_recv_raw(udp_point_t *point, char *buf, int len, char *remote, unsigned short *port)
{
    int ret = -1;

    if (point && buf)
    {
        struct sockaddr_in addr = {0};
        socklen_t addr_len = sizeof(addr);
        ret = recvfrom(point->fd, buf, len, 0, (struct sockaddr *)&addr, &addr_len);

        if (-1 != ret)
        {
            udp_point_parse_addr(&addr, remote, port);
        }
    }

    return ret;
}

void udp_point_set_data(udp_point_t *point, void *data)
{
    if (point)
    {
        point->data = data;
    }
}

void *udp_point_get_data(udp_point_t *point)
{
    if (point)
    {
        return point->data;
    }

    return NULL;
}

int udp_point_set_opt(udp_point_t *point, int level, int optname, void *optval, unsigned int optlen)
{
    return setsockopt(point->fd, level, optname, optval, optlen);
}

int udp_point_get_opt(udp_point_t *point, int level, int optname, void *optval, unsigned int *optlen)
{
    return getsockopt(point->fd, level, optname, optval, optlen);
}