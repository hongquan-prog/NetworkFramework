#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "tcp_server.h"
#include "udp_point.h"

int main(void)
{
    int len = 0;
    char buf[128];

    tcp_server_t *server = tcp_server_new();

    tcp_server_start(server, 9000, 100);

    if (tcp_server_is_valid(server))
    {
        printf("server started ...\r\n");

        tcp_client_t *ret = tcp_server_accept(server);

        if (ret)
        {
            len = tcp_client_recv_raw(ret, buf, sizeof(buf) - 1);
            buf[len] = 0;
            printf("recv:%s\r\n", buf);
            tcp_client_send_raw(ret, buf, len);
            tcp_client_delete(ret);
        }
    }
    tcp_server_delete(server);

    return 0;
}