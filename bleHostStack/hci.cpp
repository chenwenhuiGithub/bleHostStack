#include <QDebug>
#include "hci.h"
#include "serial.h"
#include "config.h"
#include "btsnoop.h"
#include "l2cap.h"

uint16_t connect_handle = 0; // TODO: support multiple connections

void hci_recv_evt(uint8_t *data, uint8_t length) {
    uint8_t event_code = data[0];

    switch (event_code) {
    case HCI_EVENT_DISCONNECTION_COMPLETE: hci_recv_evt_disconnection_complete(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_COMMAND_COMPLETE: hci_recv_evt_command_complete(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_LE_META: hci_recv_evt_le_meta(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
        hci_recv_evt_number_of_completed_packets(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    default: qDebug("hci_recv_evt invalid, event_code:%u", event_code); break;
    }
}

void hci_recv_evt_command_complete(uint8_t *data, uint8_t length) {
    (void)length;
    // uint8_t num_packet = data[0]; // TODO: support flow control based on le_acl_data_packet_total_num
    uint8_t ogf = data[2] >> 2;
    uint16_t ocf = ((data[2] & 0x03) << 8) | data[1];

    switch (ogf) {
    case HCI_OGF_CONTROLLER_BASEBAND:
        switch (ocf) {
        case HCI_OCF_RESET:
            qDebug("reset status:%u", data[3]);
            hci_send_cmd_read_local_version_info();
            break;
        case HCI_OCF_WRITE_CLASS_OF_DEVICE:
            qDebug("write_class_of_device status:%u", data[3]);
            hci_send_cmd_set_event_mask();
            break;
        case HCI_OCF_SET_EVENT_MASK:
            qDebug("set_event_mask status:%u", data[3]);
            hci_send_cmd_write_le_host_support(HCI_LE_HOST_SUPPORT_ENABLE);
            break;
        case HCI_OCF_WRITE_LE_HOST_SUPPORT:
            qDebug("write_le_host_support status:%u", data[3]);
            hci_send_cmd_le_read_buffer_size();
            break;
        default:
            qDebug("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    case HCI_OGF_INFORMATIONAL_PARAM:
        switch (ocf) {
        case HCI_OCF_READ_LOCAL_VERSION_INFO:
            qDebug("read_local_version_info status:%u, hci_version:%u", data[3], data[4]);
            hci_send_cmd_read_local_supported_commands();
            break;
        case HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS:
            qDebug("read_local_supported_commands status:%u", data[3]);
            hci_send_cmd_read_bd_addr();
            break;
        case HCI_OCF_READ_BD_ADDR:
            qDebug("read_bd_addr status:%u, bd_addr:%02x:%02x:%02x:%02x:%02x:%02x",
                   data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
            hci_send_cmd_write_class_of_device();
            break;
        default:
            qDebug("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    case HCI_OGF_LE_CONTROLLER:
        switch (ocf) {
        case HCI_OCF_LE_READ_BUFFER_SIZE:
            qDebug("le_read_buffer_size status:%u, le_acl_data_packet_length:%u, le_acl_data_packet_total_num:%u",
                   data[3], (data[4] | (data[5] << 8)), data[6]);
            hci_send_cmd_le_set_event_mask();
            break;
        case HCI_OCF_LE_SET_EVENT_MASK:
            qDebug("le_set_event_mask status:%u", data[3]);
            hci_send_cmd_le_set_advertising_parameters();
            break;
        case HCI_OCF_LE_SET_ADV_PARAM:
            qDebug("le_set_advertising_parameters status:%u", data[3]);
            hci_send_cmd_le_set_advertising_data();
            break;
        case HCI_OCF_LE_SET_ADV_DATA:
            qDebug("le_set_advertising_data status:%u", data[3]);
            hci_send_cmd_le_set_advertising_enable(HCI_LE_ADVERTISING_ENABLE);
            break;
        case HCI_OCF_LE_SET_ADV_ENABLE:
            qDebug("le_set_advertising_enable status:%u", data[3]);
            qDebug("/***** wait peer device to connect *****/");
            break;
        case HCI_OCF_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY:
            qDebug("le_remote_connection_parameter_request_reply status:%u, connect_handle:0x%02x%02x",
                   data[3], data[4], (data[5] & 0x0f));
            break;
        case HCI_OCF_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_NEG_REPLY:
            qDebug("le_remote_connection_parameter_request_neg_reply status:%u, connect_handle:0x%02x%02x",
                   data[3], data[4], (data[5] & 0x0f));
            break;
        default:
            qDebug("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    default:
        qDebug("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
        break;
    }
}

void hci_recv_evt_le_meta(uint8_t *data, uint8_t length) {
    (void)length;
    uint8_t sub_event = data[0];

    switch (sub_event) {
    case HCI_EVENT_LE_CONNECTION_COMPLETE:
        qDebug("le_connection_complete status:%u, connect_handle:0x%02x%02x, peer_address:%02x:%02x:%02x:%02x:%02x:%02x",
               data[1], data[2], (data[3] & 0x0f), data[6],  data[7], data[8], data[9], data[10], data[11]);
        qDebug("/***** peer device connects success *****/");
        break;
    case HCI_EVENT_LE_REMOTE_CONNECTION_PARAMETER_REQUEST:
        qDebug("le_remote_connection_parameter_request connect_handle:0x%02x%02x, interval_min:%0.2fms, interval_max:%0.2fms, max_latency:%u, timeout:%ums",
               data[1], (data[2] & 0x0f), (data[3] | (data[4] << 8)) * 1.25, (data[5] | (data[6] << 8)) * 1.25,
               data[7] | (data[8] << 8), (data[9] | (data[10] << 8)) * 10);
        hci_send_cmd_le_remote_connection_parameter_request_negative_reply();
        break;
    case HCI_EVENT_LE_ENHANCED_CONNECTION_COMPLETE:
        qDebug("le_enhanced_connection_complete status:%u, connect_handle:0x%02x%02x, peer_address:%02x:%02x:%02x:%02x:%02x:%02x",
               data[1], data[2], (data[3] & 0x0f), data[6],  data[7], data[8], data[9], data[10], data[11]);
        qDebug("/***** peer device connects success *****/");
        break;
    default:
        qDebug("hci_recv_evt_le_meta invalid, sub_event:%u", sub_event);
        break;
    }
}

void hci_recv_evt_disconnection_complete(uint8_t* data, uint8_t length) {
    (void)length;
    qDebug("disconnection_complete status:%u, connect_handle:0x%02x%02x, reason:0x%02x", data[0], data[1], (data[2] & 0x0f), data[3]);
    qDebug("/***** peer device disconnect *****/");
}

void hci_recv_evt_number_of_completed_packets(uint8_t* data, uint8_t length) {
    (void)length;
    qDebug("number_of_completed_packets number_handles:%u, connect_handle[0]:0x%02x%02x, num_completed_packets[0]:%u",
           data[0], data[1], (data[2] & 0x0f), data[3] | (data[4] << 8));
}

void hci_recv_acl(uint8_t *data, uint16_t length) {
    connect_handle = data[0] | ((data[1] & 0x0f) << 8);
    // uint8_t pb_flag = data[1] & 0x30; // TODO: support segmentation based on le_acl_data_packet_length

    l2cap_recv(data + HCI_LENGTH_ACL_HEADER, length - HCI_LENGTH_ACL_HEADER);
}

void hci_recv_sco(uint8_t *data, uint8_t length) {
    (void)data;
    (void)length;
}

void hci_send_acl(uint8_t *data, uint16_t length) {
    QByteArray byteArray;

    byteArray.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + length);
    byteArray[0] = HCI_PACKET_TYPE_ACL;
    byteArray[1] = connect_handle;
    byteArray[2] = (connect_handle >> 8) & 0x0f;
    byteArray[2] |= HCI_ACL_SEGMENTATION_PACKET_FIRST;  // TODO: support segmentation
    byteArray[3] = length;
    byteArray[4] = length >> 8;
    memcpy_s(&byteArray[5], length, data, length);
    serial_write((uint8_t*)(byteArray.data()), byteArray.length());
    btsnoop_wirte((uint8_t*)(byteArray.data()), byteArray.length(), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_assign_cmd(uint8_t *buffer, uint8_t ogf, uint16_t ocf) {
    buffer[0] = HCI_PACKET_TYPE_CMD;
    buffer[1] = ocf;
    buffer[2] = (ocf >> 8) | (ogf << 2);
}

void hci_send_cmd_reset() {
    uint8_t buffer[HCI_LENGTH_CMD_RESET] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_RESET);
    serial_write(buffer, HCI_LENGTH_CMD_RESET);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_RESET, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_read_local_version_info() {
    uint8_t buffer[HCI_LENGTH_CMD_READ_LOCAL_VERSION_INFO] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_VERSION_INFO);
    serial_write(buffer, HCI_LENGTH_CMD_READ_LOCAL_VERSION_INFO);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_READ_LOCAL_VERSION_INFO, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_read_local_supported_commands() {
    uint8_t buffer[HCI_LENGTH_CMD_READ_LOCAL_SUPPORTED_COMMANDS] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS);
    serial_write(buffer, HCI_LENGTH_CMD_READ_LOCAL_SUPPORTED_COMMANDS);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_READ_LOCAL_SUPPORTED_COMMANDS, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_set_event_mask() {
    uint8_t buffer[HCI_LENGTH_CMD_SET_EVENT_MASK] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_SET_EVENT_MASK);
    buffer[3] = sizeof(event_mask);
    memcpy_s(&buffer[4], sizeof(event_mask), event_mask, sizeof(event_mask));
    serial_write(buffer, HCI_LENGTH_CMD_SET_EVENT_MASK);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_SET_EVENT_MASK, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_write_le_host_support(HCI_LE_HOST_SUPPORT enable) {
    uint8_t buffer[HCI_LENGTH_CMD_WRITE_LE_HOST_SUPPORT] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_LE_HOST_SUPPORT);
    buffer[3] = 2;
    buffer[4] = enable;
    buffer[5] = 0x00;
    serial_write(buffer, HCI_LENGTH_CMD_WRITE_LE_HOST_SUPPORT);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_WRITE_LE_HOST_SUPPORT, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_read_buffer_size() {
    uint8_t buffer[HCI_LENGTH_CMD_LE_READ_BUFFER_SIZE] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_READ_BUFFER_SIZE);
    serial_write(buffer, HCI_LENGTH_CMD_LE_READ_BUFFER_SIZE);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_READ_BUFFER_SIZE, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_read_bd_addr() {
    uint8_t buffer[HCI_LENGTH_CMD_READ_BD_ADDR] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_BD_ADDR);
    serial_write(buffer, HCI_LENGTH_CMD_READ_BD_ADDR);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_READ_BD_ADDR, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_write_class_of_device() {
    uint8_t buffer[HCI_LENGTH_CMD_WRITE_CLASS_OF_DEVICE] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_CLASS_OF_DEVICE);
    buffer[3] = sizeof(class_of_device);
    memcpy_s(&buffer[4], sizeof(class_of_device), class_of_device, sizeof(class_of_device));
    serial_write(buffer, HCI_LENGTH_CMD_WRITE_CLASS_OF_DEVICE);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_WRITE_CLASS_OF_DEVICE, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_remote_connection_parameter_request_negative_reply() {
    uint8_t buffer[HCI_LENGTH_CMD_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_NEG_REPLY);
    buffer[3] = 3;
    buffer[4] = connect_handle;
    buffer[5] = (connect_handle >> 8) & 0x0f;
    buffer[6] = 0x3b; // reason that the connection parameter request was rejected: Unacceptable Connection Parameters
    serial_write(buffer, HCI_LENGTH_CMD_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_set_event_mask() {
    uint8_t buffer[HCI_LENGTH_CMD_LE_SET_EVENT_MASK] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_EVENT_MASK);
    buffer[3] = sizeof(le_event_mask);
    memcpy_s(&buffer[4], sizeof(le_event_mask), le_event_mask, sizeof(le_event_mask));
    serial_write(buffer, HCI_LENGTH_CMD_LE_SET_EVENT_MASK);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_SET_EVENT_MASK, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_set_advertising_parameters(){
    uint8_t buffer[HCI_LENGTH_CMD_LE_SET_ADVERTISING_PARAMETERS] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_ADV_PARAM);
    buffer[3] = sizeof(le_advertising_parameters);
    memcpy_s(&buffer[4], sizeof(le_advertising_parameters), le_advertising_parameters, sizeof(le_advertising_parameters));
    serial_write(buffer, HCI_LENGTH_CMD_LE_SET_ADVERTISING_PARAMETERS);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_SET_ADVERTISING_PARAMETERS, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_set_advertising_data() {
    uint8_t buffer[HCI_LENGTH_CMD_LE_SET_ADVERTISING_DATA] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_ADV_DATA);
    buffer[3] = sizeof(le_advertising_data);
    memcpy_s(&buffer[4], sizeof(le_advertising_data), le_advertising_data, sizeof(le_advertising_data));
    serial_write(buffer, HCI_LENGTH_CMD_LE_SET_ADVERTISING_DATA);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_SET_ADVERTISING_DATA, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_set_advertising_enable(HCI_LE_ADVERTISING enable) {
    uint8_t buffer[HCI_LENGTH_CMD_LE_SET_ADVERTISING_ENABLE] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_ADV_ENABLE);
    buffer[3] = 1;
    buffer[4] = enable;
    serial_write(buffer, HCI_LENGTH_CMD_LE_SET_ADVERTISING_ENABLE);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_SET_ADVERTISING_ENABLE, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}
