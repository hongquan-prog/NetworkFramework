#include "msg.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

msg_t *msg_new(unsigned short type,
               unsigned short cmd,
               unsigned short index,
               unsigned short total,
               unsigned char *payload,
               unsigned int length)
{
    msg_t *ret = (msg_t *)malloc(sizeof(msg_t) + length);

    if (ret)
    {
        ret->cmd = cmd;
        ret->index = index;
        ret->length = length;
        ret->total = total;
        ret->type = type;

        if (payload)
        {
            memcpy(ret + 1, payload, length);
        }
    }

    return ret;
}

void msg_ntoh(msg_t *msg)
{
    if (msg)
    {
        msg->type = ntohs(msg->type);
        msg->cmd = ntohs(msg->cmd);
        msg->index = ntohs(msg->index);
        msg->total = ntohs(msg->total);
        msg->length = ntohl(msg->length);
    }
}

void msg_hton(msg_t *msg)
{
    if (msg)
    {
        msg->type = htons(msg->type);
        msg->cmd = htons(msg->cmd);
        msg->index = htons(msg->index);
        msg->total = htons(msg->total);
        msg->length = htonl(msg->length);
    }
}

unsigned int msg_size(msg_t *msg)
{
    return (msg) ? (sizeof(msg_t) + msg->length) : (0);
}

void msg_delete(msg_t *msg)
{
    free(msg);
}