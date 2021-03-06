#ifndef L2CAP_H
#define L2CAP_H

#include <stdint.h>

#define L2CAP_LENGTH_HEADER                     4

typedef enum {
    L2CAP_CID_BT_SIGNAL     = 1,
    L2CAP_CID_CONNECTLESS   = 2,
    L2CAP_CID_ATT           = 4,
    L2CAP_CID_BLE_SIGNAL    = 5,
    L2CAP_CID_SM            = 6
} l2cap_cid_t;

void l2cap_recv(uint16_t connect_handle, uint8_t *data, uint32_t length);
void l2cap_send(uint16_t connect_handle, l2cap_cid_t cid, uint8_t *data, uint32_t length);

#endif // L2CAP_H
