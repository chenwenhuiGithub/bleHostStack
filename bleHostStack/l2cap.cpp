#include "l2cap.h"
#include "hci.h"
#include "att.h"
#include "sm.h"
#include "log.h"

void l2cap_recv(uint8_t *data, uint32_t length) {
    uint16_t cid = data[2] | (data[3] << 8);

    switch (cid) {
    case L2CAP_CID_ATT:
        att_recv(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER);
        break;
    case L2CAP_CID_SM:
        sm_recv(data + L2CAP_LENGTH_HEADER, length - L2CAP_LENGTH_HEADER);
        break;
    default:
        LOG_WARNING("l2cap_recv invalid, cid:0x%04x", cid);
        break;
    }
}

void l2cap_send(L2CAP_CID cid, uint8_t *data, uint32_t length) {
    uint8_t *buffer = nullptr;
    uint32_t buffer_length = L2CAP_LENGTH_HEADER + length;

    buffer = (uint8_t *)malloc(buffer_length);
    if (nullptr == buffer) {
        LOG_ERROR("l2cap_send malloc error");
        return;
    }
    buffer[0] = length;
    buffer[1] = length >> 8;
    buffer[2] = cid;
    buffer[3] = cid >> 8;  // TODO: support segmentation
    memcpy_s(&buffer[4], length, data, length);
    hci_send_acl(buffer, buffer_length);

    free(buffer);
    buffer = nullptr;
}
