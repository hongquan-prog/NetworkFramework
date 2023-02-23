#include "tcp_server.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

tcp_server_t *tcp_server_new(void)
{
    tcp_server_t *ret = malloc(sizeof(tcp_server_t));

    if (ret)
    {
        ret->fd = -1;
        ret->valid = false;
        ret->cb = NULL;

        for (int i = 0; i < TCP_FD_SIZE; i++)
        {
            ret->client[i] = NULL;
        }
    }

    return ret;
}

bool tcp_server_start(tcp_server_t *server, unsigned short port, int max)
{
    struct sockaddr_in addr = {
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_family = AF_INET};

    if (server && !(server->valid))
    {
        server->fd = socket(AF_INET, SOCK_STREAM, 0);
        server->valid = (-1 != server->fd);
        server->valid = server->valid && (-1 != bind(server->fd, (struct sockaddr *)&addr, sizeof(addr)));
        server->valid = server->valid && (-1 != listen(server->fd, max));
    }

    return server->valid;
}

void tcp_server_stop(tcp_server_t *server)
{
    if (server)
    {
        server->valid = 0;
        close(server->fd);

        for (int i = 0; i < TCP_FD_SIZE; i++)
        {
            tcp_client_delete(server->client[i]);
            server->client[i] = NULL;
        }
    }
}

void tcp_server_reister_callback(tcp_server_t *server, tcp_event_cb_t func)
{
    if (server)
    {
        server->cb = func;
    }
}

bool tcp_server_is_valid(tcp_server_t *server)
{
    return (server) ? (server->valid) : (false);
}

static int tcp_server_handler(tcp_server_t *server, fd_set *set, fd_set *read_set, int num, int max)
{
    int index = 0;
    tcp_event_def event;
    int cnt = 0;

    for (int i = 0; (i <= max) && (cnt < num); i++)
    {
        if (FD_ISSET(i, set))
        {
            cnt++;
            index = i;

            if (index == server->fd)
            {
                struct sockaddr_in client_info = {0};
                socklen_t len = sizeof(client_info);
                index = accept(server->fd, (struct sockaddr *)(&client_info), &len);

                if (0 <= index)
                {
                    FD_SET(index, read_set);
                    max = (index > max) ? (index) : (max);
                    server->client[index] = tcp_client_from_fd(index);
                    event = TCP_EVENT_CONNECT;
                }
            }
            else
            {
                event = TCP_EVENT_RECV_DATA;
            }

            if (server->cb)
            {
                if (tcp_client_is_connected(server->client[index]))
                {
                    server->cb(server->client[index], event);
                }
                else
                {
                    if (server->client[index])
                    {
                        server->cb(server->client[index], TCP_EVENT_CLOSE);
                    }

                    tcp_client_delete(server->client[index]);
                    server->client[index] = NULL;
                    FD_CLR(index, read_set);
                }
            }
        }
    }

    return max;
}

void tcp_server_exec(tcp_server_t *server)
{
    int num = 0;
    int maxfd = 0;
    fd_set reads = {0};
    fd_set temps = {0};
    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = 50000};

    if (server && server->valid)
    {
        FD_ZERO(&reads);
        FD_SET(server->fd, &reads);
        maxfd = server->fd;

        while (1)
        {
            temps = reads;
            num = select(maxfd + 1, &temps, NULL, NULL, &timeout);

            if (num > 0)
            {
                maxfd = tcp_server_handler(server, &temps, &reads, num, maxfd);
            }
        }
    }
}

int tcp_server_set_opt(tcp_server_t *server, int level, int optname, void *optval, unsigned int optlen)
{
    return setsockopt(server->fd, level, optname, optval, optlen);
}

int tcp_server_get_opt(tcp_server_t *server, int level, int optname, void *optval, unsigned int *optlen)
{
    return getsockopt(server->fd, level, optname, optval, optlen);
}

tcp_client_t *tcp_server_accept(tcp_server_t *server)
{
    tcp_client_t *ret = NULL;

    if (server)
    {
        struct sockaddr_in addr = {0};
        socklen_t len = sizeof(addr);
        int fd = accept(server->fd, (struct sockaddr *)&addr, &len);

        if (0 < fd)
        {
            ret = tcp_client_from_fd(fd);
        }

        if (!ret)
        {
            close(fd);
        }
    }

    return ret;
}

void tcp_server_delete(tcp_server_t *server)
{
    tcp_server_stop(server);
    free(server);
}