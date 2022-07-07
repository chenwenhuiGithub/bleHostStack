#include "l2cap.h"
#include "hci.h"
#include "att.h"
#include "sm.h"
#include "log.h"


void l2cap_recv(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint16_t cid = data[2] | (data[3] << 8);

    switch (cid) {
    case L2CAP_CID_ATT:
        att_recv(connect_handle, data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER);
        break;
    case L2CAP_CID_SM:
        sm_recv(connect_handle, data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER);
        break;
    default:
        LOG_WARNING("l2cap_recv invalid, cid:0x%04x", cid);
        break;
    }
}

void l2cap_send(uint16_t connect_handle, l2cap_cid_t cid, uint8_t *data, uint32_t length) {
    uint32_t offset = HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER;

    data[offset] = length;
    offset++;
    data[offset] = length >> 8;
    offset++;
    data[offset] = cid;
    offset++;
    data[offset] = cid >> 8;
    offset++;
    hci_send_acl(connect_handle, data, length + L2CAP_LENGTH_HEADER);
}
