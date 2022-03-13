#ifndef L2CAP_H
#define L2CAP_H

#include <stdint.h>

#define L2CAP_LENGTH_HEADER		4

#define L2CAP_CID_BT_SIGNAL		0x0001
#define L2CAP_CID_CONNECTLESS	0x0002
#define L2CAP_CID_ATT			0x0004
#define L2CAP_CID_BLE_SIGNAL	0x0005
#define L2CAP_CID_SM			0x0006

void l2cap_recv(uint8_t *data, uint16_t length);
void l2cap_recv_bt_signal(uint8_t *data, uint16_t length);
void l2cap_recv_connectless(uint8_t *data, uint16_t length);
void l2cap_recv_ble_signal(uint8_t *data, uint16_t length);
void l2cap_send(uint16_t cid, uint8_t *data, uint16_t length);
void l2cap_set_max_mtu(uint16_t mtu);

#endif // L2CAP_H
