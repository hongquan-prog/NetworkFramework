#include "msg_parser.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void msg_parser_initial(msg_parser_t *parser)
{
    if (parser)
    {
        parser->header_recved = false;
        parser->remain = sizeof(msg_t);

        if (parser->msg)
        {
            free(parser->msg);
        }

        parser->msg = NULL;
    }
}

static bool msg_parser_recving(msg_parser_t *parser)
{
    parser->header_recved = true;
    parser->remain = parser->headr_cache.length;

    parser->msg = malloc(sizeof(msg_t) + parser->remain);

    if (parser->msg)
    {
        *parser->msg = parser->headr_cache;
    }

    return (NULL != parser->msg);
}

static msg_t *msg_parser_recv_finish(msg_parser_t *parser)
{
    msg_t *ret = NULL;

    if (parser->header_recved && !parser->remain)
    {
        ret = parser->msg;
        parser->msg = NULL;
    }

    return ret;
}

void msg_parser_reset(msg_parser_t *parser)
{
    msg_parser_initial(parser);
}

msg_parser_t *msg_parser_new()
{
    msg_parser_t *ret = (msg_parser_t *)malloc(sizeof(msg_parser_t));
    ret->msg = NULL;

    msg_parser_initial(ret);

    return ret;
}

msg_t *msg_parse_from_ram(msg_parser_t *parser, unsigned char *ram, unsigned int len)
{
    msg_t *ret = NULL;

    if (parser && ram)
    {
        if (parser->header_recved)
        {
            if (parser->msg)
            {
                unsigned int min = (parser->remain < len) ? (parser->remain) : (len);
                unsigned int offset = parser->msg->length - parser->remain;

                memcpy(parser->msg->payload + offset, ram, min);
                parser->remain -= min;
            }

            ret = msg_parser_recv_finish(parser);

            if (ret)
            {
                msg_parser_initial(parser);
            }
        }
        else
        {
            unsigned int min = (parser->remain < len) ? (parser->remain) : (len);
            unsigned int offset = sizeof(msg_t) - parser->remain;

            memcpy((char *)&parser->headr_cache + offset, ram, min);

            if (min >= parser->remain)
            {
                msg_ntoh(&parser->headr_cache);

                ram += parser->remain;
                len -= parser->remain;

                if (msg_parser_recving(parser))
                {
                    ret = msg_parse_from_ram(parser, ram, len);
                }
                else
                {
                    msg_parser_initial(parser);
                }
            }
            else
            {
                parser->remain -= min;
            }
        }
    }

    return ret;
}

static int msg_parse_read(int fd, char *buf, int size)
{
    int retry = 0;
    int i = 0;

    while (i < size)
    {
        int len = read(fd, buf + i, size - i);

        if (len > 0)
        {
            i += len;
        }
        else if (len < 0)
        {
            break;
        }
        else
        {
            if (++retry > 5)
            {
                break;
            }

            usleep(1000 * 200);
        }
    }

    return i;
}

msg_t *msg_parse_from_file(msg_parser_t *parser, int fd)
{
    msg_t *ret = NULL;

    if (fd > 0 && parser)
    {
        if (!parser->header_recved)
        {
            unsigned int offset = sizeof(parser->headr_cache) - parser->remain;
            int len = msg_parse_read(fd, (char *)&parser->headr_cache + offset, parser->remain);

            if (parser->remain == len)
            {
                msg_ntoh(&parser->headr_cache);

                if (msg_parser_recving(parser))
                {
                    ret = msg_parse_from_file(parser, fd);
                }
                else
                {
                    msg_parser_initial(parser);
                }
            }
            else
            {
                parser->remain -= len;
            }
        }
        else
        {
            if (parser->msg)
            {
                unsigned int offset = parser->msg->length - parser->remain;
                int len = msg_parse_read(fd, (char *)parser->msg->payload + offset, parser->remain);
                parser->remain -= len;
            }

            ret = msg_parser_recv_finish(parser);

            if (ret)
            {

                msg_parser_initial(parser);
            }
        }
    }

    return ret;
}

void msg_parser_delete(msg_parser_t *parser)
{
    if (parser)
    {
        free(parser->msg);
        free(parser);
    }
}