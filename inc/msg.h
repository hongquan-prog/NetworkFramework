#pragma once

typedef struct
{
    unsigned short type;
    unsigned short cmd;
    unsigned short index;
    unsigned short total;
    unsigned int length;
    unsigned char payload[];
} msg_t;

msg_t *msg_new(unsigned short type,
                    unsigned short cmd,
                    unsigned short index,
                    unsigned short total,
                    unsigned char *payload,
                    unsigned int length);
void msg_ntoh(msg_t *msg);
void msg_hton(msg_t *msg);
void msg_delete(msg_t *msg);
