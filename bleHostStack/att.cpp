#include "att.h"
#include "hci.h"
#include "l2cap.h"
#include "gatt.h"
#include "log.h"

#define ATT_LENGTH_PACKET_HEADER           (HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_HEADER_ACL + L2CAP_LENGTH_HEADER)


static void __att_recv_find_info_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_find_by_type_value_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_read_by_type_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_read_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_read_by_group_type_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_read_blob_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_write_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_write_cmd(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_recv_handle_value_cfm(uint16_t connect_handle);
static void __att_recv_exchange_mtu_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __att_send_exchange_mtu_resp(uint16_t connect_handle, uint16_t mtu);


void att_recv(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint8_t op_code = data[0];

    switch (op_code) {
    case ATT_OPERATE_EXCHANGE_MTU_REQ:
        __att_recv_exchange_mtu_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_FIND_INFO_REQ:
        __att_recv_find_info_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_FIND_BY_TYPE_VALUE_REQ:
        __att_recv_find_by_type_value_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_READ_BY_TYPE_REQ:
        __att_recv_read_by_type_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_READ_REQ:
        __att_recv_read_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_READ_BLOB_REQ:
        __att_recv_read_blob_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_READ_BY_GROUP_TYPE_REQ:
        __att_recv_read_by_group_type_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_WRITE_REQ:
        __att_recv_write_req(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_WRITE_CMD:
        __att_recv_write_cmd(connect_handle, data + ATT_LENGTH_HEADER, length - ATT_LENGTH_HEADER);
        break;
    case ATT_OPERATE_HANDLE_VALUE_CFM:
        __att_recv_handle_value_cfm(connect_handle);
        break;
    default:
        LOG_WARNING("att_recv invalid, op_code:0x%02x", op_code);
        break;
    }
}

static void __att_recv_read_blob_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    uint16_t att_handle = data[0] | (data[1] << 8);
    uint16_t offset = data[2] | (data[3] << 8);

    gatt_recv_read_blob_req(connect_handle, att_handle, offset);
}

static void __att_recv_read_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    uint16_t att_handle = data[0] | (data[1] << 8);

    gatt_recv_read_req(connect_handle, att_handle);
}

static void __att_recv_find_info_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    uint16_t att_handle_start = data[0] | (data[1] << 8);
    uint16_t att_handle_end = data[2] | (data[3] << 8);

    gatt_recv_find_information_req(connect_handle, att_handle_start, att_handle_end);
}

static void __att_recv_find_by_type_value_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint16_t att_handle_start = data[0] | (data[1] << 8);
    uint16_t att_handle_end = data[2] | (data[3] << 8);
    uint16_t att_type = data[4] | (data[5] << 8);
    uint32_t offset = 6;

    // this command only support 16 Bytes uuid
    gatt_recv_find_by_type_value_req(connect_handle, att_handle_start, att_handle_end, att_type, data + offset, length - offset);
}

static void __att_recv_read_by_type_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    uint16_t att_handle_start = data[0] | (data[1] << 8);
    uint16_t att_handle_end = data[2] | (data[3] << 8);
    uint16_t att_type = data[4] | (data[5] << 8);

    gatt_recv_read_by_type_req(connect_handle, att_handle_start, att_handle_end, att_type);
}

static void __att_recv_read_by_group_type_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    uint16_t att_handle_start = data[0] | (data[1] << 8);
    uint16_t att_handle_end = data[2] | (data[3] << 8);
    uint16_t group_type = data[4] | (data[5] << 8);

    gatt_recv_read_by_group_type_req(connect_handle, att_handle_start, att_handle_end, group_type);
}

static void __att_recv_write_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint16_t att_handle = data[0] | (data[1] << 8);
    uint32_t offset = 2;

    gatt_recv_write_req(connect_handle, att_handle, data + offset, length - offset);
}

static void __att_recv_write_cmd(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint16_t att_handle = data[0] | (data[1] << 8);
    uint32_t offset = 2;

    gatt_recv_write_cmd(connect_handle, att_handle, data + offset, length - offset);
}

static void __att_recv_handle_value_cfm(uint16_t connect_handle) {
    gatt_recv_handle_value_cfm(connect_handle);
}

static void __att_recv_exchange_mtu_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    uint16_t client_mtu = data[0] | (data[1] << 8);
    uint16_t max_mtu = hci_get_le_acl_packet_length() - L2CAP_LENGTH_HEADER;
    hci_connection_t *conn = hci_find_connection_by_handle(connect_handle);

    conn->att_mtu = (max_mtu > client_mtu) ? client_mtu : max_mtu;
    if (conn->att_mtu < ATT_MTU_DEFAULT) {
        conn->att_mtu = ATT_MTU_DEFAULT;
    }
    LOG_INFO("set att_mtu:%u", conn->att_mtu);
    __att_send_exchange_mtu_resp(connect_handle, conn->att_mtu);
}

static void __att_send_exchange_mtu_resp(uint16_t connect_handle, uint16_t mtu) {
    uint8_t buffer[ATT_LENGTH_PACKET_HEADER + ATT_LENGTH_HEADER + ATT_LENGTH_EXCHANGE_MTU_RESP] = { 0x00 };
    uint32_t offset = ATT_LENGTH_PACKET_HEADER;

    buffer[offset] = ATT_OPERATE_EXCHANGE_MTU_RESP;
    offset++;
    buffer[offset] = (uint8_t)mtu;
    offset++;
    buffer[offset] = mtu >> 8;
    offset++;
    att_send(connect_handle, buffer, offset - ATT_LENGTH_PACKET_HEADER);
}

uint16_t att_get_mtu(uint16_t connect_handle) {
    return hci_find_connection_by_handle(connect_handle)->att_mtu;
}

void att_send(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    l2cap_send(connect_handle, L2CAP_CID_ATT, data, length);
}
