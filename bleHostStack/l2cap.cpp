#include "l2cap.h"
#include "hci.h"
#include "att.h"
#include "sm.h"
#include "log.h"

uint16_t l2cap_max_mtu = 0;

void l2cap_recv(uint8_t *data, uint16_t length) {
    uint16_t data_length = data[0] | (data[1] << 8);
    uint16_t cid = data[2] | (data[3] << 8);

    if (data_length != length - L2CAP_LENGTH_HEADER) {
        LOG_ERROR("l2cap_recv invalid, data_length:%u, length:%u", data_length, length);
        return;
    }

    switch (cid) {
    case L2CAP_CID_BT_SIGNAL: l2cap_recv_bt_signal(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER); break;
    case L2CAP_CID_CONNECTLESS: l2cap_recv_connectless(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER); break;
    case L2CAP_CID_ATT: att_recv(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER); break;
    case L2CAP_CID_BLE_SIGNAL: l2cap_recv_ble_signal(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER); break;
    case L2CAP_CID_SM: sm_recv(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER); break;
    default: LOG_WARNING("l2cap_recv invalid, cid:%u", cid); break;
    }
}

void l2cap_recv_bt_signal(uint8_t *data, uint16_t length) {
    (void)data;
    (void)length;
}

void l2cap_recv_connectless(uint8_t *data, uint16_t length) {
    (void)data;
    (void)length;
}

void l2cap_recv_ble_signal(uint8_t *data, uint16_t length) {
    (void)data;
    (void)length;
}

void l2cap_send(uint16_t cid, uint8_t *data, uint16_t length) {
    QByteArray byteArray;

    byteArray.resize(L2CAP_LENGTH_HEADER + length);
    byteArray[0] = length;
    byteArray[1] = length >> 8;
    byteArray[2] = cid;
    byteArray[3] = cid >> 8;
    memcpy_s((uint8_t*)byteArray.data() + L2CAP_LENGTH_HEADER, length, data, length);
    hci_send_acl((uint8_t*)byteArray.data(), byteArray.length());
}

void l2cap_set_max_mtu(uint16_t mtu) {
    l2cap_max_mtu = mtu;
    att_set_max_mtu(l2cap_max_mtu - L2CAP_LENGTH_HEADER);
}

