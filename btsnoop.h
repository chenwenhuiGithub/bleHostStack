#ifndef BTSNOOP_H
#define BTSNOOP_H

#include <stdint.h>

// bit0: 0 - Host -> Controller, 1 - Host <- Controller
// bit1: 0 - ACL/SCO, 1 - CMD/EVT
typedef enum {
    BTSNOOP_PACKET_FLAG_ACL_SCO_SEND,
    BTSNOOP_PACKET_FLAG_ACL_SCO_RECV,
    BTSNOOP_PACKET_FLAG_CMD_SEND,
    BTSNOOP_PACKET_FLAG_EVT_RECV,
} btsnoop_packet_flag_t;


void btsnoop_open();
void btsnoop_wirte(uint8_t *data, uint32_t length, btsnoop_packet_flag_t flag);
void btsnoop_close();

#endif // BTSNOOP_H
