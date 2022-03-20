#include "att.h"
#include "l2cap.h"
#include "gatt.h"
#include "log.h"

uint16_t att_mtu = ATT_MTU_DEFAULT;
uint16_t att_max_mtu = ATT_MTU_DEFAULT;

void att_recv(uint8_t *data, uint16_t length) {
    uint8_t op_code = data[0];

    switch (op_code) {
    case ATT_OPERATE_EXCHANGE_MTU_REQ:
        att_recv_exchange_mtu_req(data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER); break;
    case ATT_OPERATE_FIND_INFORMATION_REQ:
        att_recv_find_information_req(data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER); break;
    case ATT_OPERATE_READ_BY_TYPE_REQ:
        att_recv_read_by_type_req(data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER); break;
    case ATT_OPERATE_READ_REQ:
        att_recv_read_req(data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER); break;
    case ATT_OPERATE_READ_BY_GROUP_TYPE_REQ:
        att_recv_read_by_group_type_req(data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER); break;
    default:
        LOG_WARNING("att_recv invalid, op_code:%u", op_code); break;
    }
}

void att_recv_read_req(uint8_t *data, uint16_t length) {
    uint16_t handle = data[0] | (data[1] << 8);

    gatt_recv_read_req(handle);
}

void att_recv_find_information_req(uint8_t *data, uint16_t length) {
    uint16_t start_handle = data[0] | (data[1] << 8);
    uint16_t end_handle = data[2] | (data[3] << 8);

    gatt_recv_find_information_req(start_handle, end_handle);
}

void att_recv_read_by_type_req(uint8_t *data, uint16_t length) {
    uint16_t start_handle = data[0] | (data[1] << 8);
    uint16_t end_handle = data[2] | (data[3] << 8);
    uint16_t att_type = data[4] | (data[5] << 8);

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

    gatt_recv_read_by_type_req(start_handle, end_handle, att_type);
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

void att_set_max_mtu(uint16_t mtu) {
    att_max_mtu = mtu;
}

void att_recv_exchange_mtu_req(uint8_t *data, uint16_t length) {
    uint16_t client_mtu = data[0] | (data[1] << 8);
    uint16_t min = (att_max_mtu > client_mtu) ? client_mtu : att_max_mtu;
    att_mtu = (ATT_MTU_DEFAULT > min) ? ATT_MTU_DEFAULT : min;
    LOG_INFO("set att_mtu:%u", att_mtu);

    att_send_exchange_mtu_resp(att_mtu);
}

void att_send_exchange_mtu_resp(uint16_t mtu) {
    uint8_t data[ATT_LENGTH_EXCHANGE_MTU_RESP] = { 0x00 };

    data[0] = ATT_OPERATE_EXCHANGE_MTU_RESP;
    data[1] = mtu;
    data[2] = mtu >> 8;
    att_send(data, ATT_LENGTH_EXCHANGE_MTU_RESP);
}

uint16_t att_get_mtu() {
    return att_mtu;
}
