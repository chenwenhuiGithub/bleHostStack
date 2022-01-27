#ifndef BTSNOOP_H
#define BTSNOOP_H

#include <stdint.h>

#define BTSNOOP_HTONL(x)    ((((x) & 0x000000ffUL) << 24) | \
                             (((x) & 0x0000ff00UL) <<  8) | \
                             (((x) & 0x00ff0000UL) >>  8) | \
                             (((x) & 0xff000000UL) >> 24))
#define BTSNOOP_HTONLL(x)   ((((uint64_t)BTSNOOP_HTONL(x)) << 32) + BTSNOOP_HTONL((x) >> 32))

#define BTSNOOP_HCI_PACKET_TYPE_CMD     1
#define BTSNOOP_HCI_PACKET_TYPE_ACL     2
#define BTSNOOP_HCI_PACKET_TYPE_SCO     3
#define BTSNOOP_HCI_PACKET_TYPE_EVT     4

#define BTSNOOP_BASE_TIMESTAMP  0x00dcddb30f2f8000ULL  // 1970-01-01 00:00:00

typedef enum {
    BTSNOOP_DIRECT_HOST_TO_CONTROLLER,
    BTSNOOP_DIRECT_CONTROLLER_TO_HOST
} BTSNOOP_DIRECT;

struct btsnoop_header {
    uint32_t length_original;
    uint32_t length_captured;
    uint32_t flags;
    uint32_t dropped_packets;
    uint64_t timestamp;
};

void btsnoop_open();
void btsnoop_wirte(uint8_t *data, uint32_t length, BTSNOOP_DIRECT direct);
void btsnoop_close();

#endif // BTSNOOP_H
