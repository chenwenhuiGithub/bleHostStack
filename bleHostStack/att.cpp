#include "att.h"
#include "l2cap.h"
#include "gatt.h"
#include "log.h"

void att_recv(uint8_t *data, uint16_t length) {
    uint8_t op_code = data[0];

    switch (op_code) {
    // case ATT_OPERATE_ERROR_RESP: att_recv_error_resp(); break;
    // case ATT_OPERATE_EXCHANGE_MTU_REQ: att_recv_exchange_mtu_req(); break;
    // case ATT_OPERATE_EXCHANGE_MTU_RESP: att_recv_exchange_mtu_resp(); break;
    case ATT_OPERATE_READ_BY_GROUP_TYPE_REQ:
        att_recv_read_by_group_type_req(data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER); break;
    // case ATT_OPERATE_READ_BY_GROUP_TYPE_RESP: att_recv_read_by_group_type_resp(); break;
    default:
        LOG_WARNING("att_recv invalid, op_code:%u", op_code); break;
    }
}

void att_recv_read_by_group_type_req(uint8_t *data, uint16_t length) {
    uint16_t start_handle = data[0] | (data[1] << 8);
    uint16_t end_handle = data[2] | (data[3] << 8);
    uint16_t group_type = data[4] | (data[5] << 8);

// TODO: support 16 Bytes uuid
#if 0
    if ((length - 4) == ATT_LENGTH_UUID16) {
        memcpy_s(group_type.uuid, 2, data + 4, 2);
        group_type.length = 2;
    } else if ((length - 4) == ATT_LENGTH_UUID128) {
        memcpy_s(group_type.uuid, 16, data + 4, 16);
        group_type.length = 16;
    } else {
        qDebug("att_recv_read_by_group_type_req invalid, length:%u", length);
        return;
    }
#endif

    gatt_recv_read_by_group_type_req(start_handle, end_handle, group_type);
}

void att_send(uint8_t *data, uint16_t length) {
    l2cap_send(L2CAP_CID_ATT, data, length);
}
