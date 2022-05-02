#include "hci.h"
#include "serial.h"
#include "btsnoop.h"
#include "l2cap.h"
#include "log.h"
#include "sm.h"

uint8_t class_of_device[3] = {0x92, 0x07, 0x14};
uint8_t event_mask[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f};
uint8_t le_event_mask[8] = {0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00};
uint8_t le_advertising_parameters[15] = {0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00};
// flag:0x06, device_name:wenhui_BLE
uint8_t le_advertising_data[32] = {0x0f, 0x02, 0x01, 0x06, 0x0b, 0x09, 0x77, 0x65, 0x6e, 0x68, 0x75, 0x69, 0x5f, 0x42, 0x4c, 0x45,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint16_t connect_handle = 0; // TODO: support multiple connections

QByteArray segment_l2cap_bytearray;
uint16_t segment_l2cap_total_length = 0;
uint16_t segment_l2cap_current_length = 0;
uint16_t segment_l2cap_received_length = 0;

void hci_recv_evt(uint8_t *data, uint8_t length) {
    uint8_t event_code = data[0];

    switch (event_code) {
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        hci_recv_evt_disconnection_complete(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_COMMAND_COMPLETE:
        hci_recv_evt_command_complete(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_COMMAND_STATUS:
        hci_recv_evt_command_status(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_LE_META:
        hci_recv_evt_le_meta(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
        hci_recv_evt_number_of_completed_packets(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    default:
        LOG_WARNING("hci_recv_evt invalid, event_code:%u", event_code); break;
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
            LOG_INFO("reset status:%u", data[3]);
            hci_send_cmd_read_local_version_info();
            break;
        case HCI_OCF_WRITE_CLASS_OF_DEVICE:
            LOG_INFO("write_class_of_device status:%u", data[3]);
            hci_send_cmd_set_event_mask();
            break;
        case HCI_OCF_SET_EVENT_MASK:
            LOG_INFO("set_event_mask status:%u", data[3]);
            hci_send_cmd_write_le_host_support(HCI_LE_HOST_SUPPORT_ENABLE);
            break;
        case HCI_OCF_WRITE_LE_HOST_SUPPORT:
            LOG_INFO("write_le_host_support status:%u", data[3]);
            hci_send_cmd_le_read_buffer_size();
            break;
        default:
            LOG_WARNING("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    case HCI_OGF_INFORMATIONAL_PARAM:
        switch (ocf) {
        case HCI_OCF_READ_LOCAL_VERSION_INFO:
            LOG_INFO("read_local_version_info status:%u, hci_version:%u", data[3], data[4]);
            hci_send_cmd_read_local_supported_commands();
            break;
        case HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS:
            LOG_INFO("read_local_supported_commands status:%u", data[3]);
            hci_send_cmd_read_bd_addr();
            break;
        case HCI_OCF_READ_BD_ADDR:
            LOG_INFO("read_bd_addr status:%u, bd_addr:%02x:%02x:%02x:%02x:%02x:%02x",
                     data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
            sm_set_local_address(data + 4);
            sm_set_local_address_type(HCI_ADDRESS_TYPE_PUBLIC_DEVICE); // TODO: how to get local address type?
            hci_send_cmd_write_class_of_device();
            break;
        default:
            LOG_WARNING("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    case HCI_OGF_LE_CONTROLLER:
        switch (ocf) {
        case HCI_OCF_LE_READ_BUFFER_SIZE:
            LOG_INFO("le_read_buffer_size status:%u, le_acl_data_packet_length:%u, le_acl_data_packet_total_num:%u",
                     data[3], (data[4] | (data[5] << 8)), data[6]);
            l2cap_set_max_mtu(data[4] | (data[5] << 8)); // TODO: set att_mtu directly
            hci_send_cmd_le_set_event_mask();
            break;
        case HCI_OCF_LE_SET_EVENT_MASK:
            LOG_INFO("le_set_event_mask status:%u", data[3]);
            hci_send_cmd_le_read_local_P256_public_key();
            break;
        case HCI_OCF_LE_SET_ADV_PARAM:
            LOG_INFO("le_set_advertising_parameters status:%u", data[3]);
            hci_send_cmd_le_set_advertising_data();
            break;
        case HCI_OCF_LE_SET_ADV_DATA:
            LOG_INFO("le_set_advertising_data status:%u", data[3]);
            hci_send_cmd_le_set_advertising_enable(HCI_LE_ADVERTISING_ENABLE);
            break;
        case HCI_OCF_LE_SET_ADV_ENABLE:
            LOG_INFO("le_set_advertising_enable status:%u", data[3]);
            LOG_INFO("/***** wait peer device to connect *****/");
            break;
        case HCI_OCF_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY:
            LOG_INFO("le_remote_connection_parameter_request_reply status:%u, connect_handle:0x%02x%02x",
                     data[3], data[4], (data[5] & 0x0f));
            break;
        case HCI_OCF_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_NEG_REPLY:
            LOG_INFO("le_remote_connection_parameter_request_neg_reply status:%u, connect_handle:0x%02x%02x",
                     data[3], data[4], (data[5] & 0x0f));
            break;
        default:
            LOG_WARNING("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    default:
        LOG_WARNING("hci_recv_evt_command_complete invalid, ogf:%u, ocf:%u", ogf, ocf);
        break;
    }
}

void hci_recv_evt_command_status(uint8_t *data, uint8_t length) {
    (void)length;
    uint8_t ogf = data[3] >> 2;
    uint16_t ocf = ((data[3] & 0x03) << 8) | data[2];

    switch (ogf) {
    case HCI_OGF_LE_CONTROLLER:
        switch (ocf) {
        case HCI_OCF_LE_READ_LOCAL_P256_PUBLIC_KEY:
            LOG_INFO("le_read_local_p256_public_key status:%u", data[0]);
            break;
        case HCI_OCF_LE_GENERATE_DHKEY:
            LOG_INFO("le_generate_dhkey status:%u", data[0]);
            break;
        default:
            LOG_WARNING("hci_recv_evt_command_status invalid, ogf:%u, ocf:%u", ogf, ocf);
            break;
        }
        break;
    default:
        LOG_WARNING("hci_recv_evt_command_status invalid, ogf:%u, ocf:%u", ogf, ocf);
        break;
    }
}

void hci_recv_evt_le_meta(uint8_t *data, uint8_t length) {
    (void)length;
    uint8_t sub_event = data[0];
    uint8_t remote_connection_parameter_request_reply[14] = { 0 };

    switch (sub_event) {
    case HCI_EVENT_LE_CONNECTION_COMPLETE:
        LOG_INFO("le_connection_complete status:%u, connect_handle:0x%02x%02x, "
                 "peer_address_type:%u, peer_address:%02x:%02x:%02x:%02x:%02x:%02x",
                 data[1], data[2], (data[3] & 0x0f), data[5], data[6],  data[7], data[8], data[9], data[10], data[11]);
        sm_set_remote_address(data + 6);
        sm_set_remote_address_type(data[5]);
        LOG_INFO("/***** peer device connects success *****/");
        break;
    case HCI_EVENT_LE_CONNECTION_UPDATE_COMPLETE:
        LOG_INFO("le_connection_update_complete status:%u, connect_handle:0x%02x%02x, interval:%0.2fms, latency:%u, timeout:%ums",
                 data[1], data[2], (data[3] & 0x0f), (data[4] | (data[5] << 8)) * 1.25, data[6] | (data[7] << 8), (data[8] | (data[9] << 8)) * 10);
        break;
    case HCI_EVENT_LE_REMOTE_CONNECTION_PARAMETER_REQUEST:
        LOG_INFO("le_remote_connection_parameter_request connect_handle:0x%02x%02x, interval_min:%0.2fms, interval_max:%0.2fms, max_latency:%u, timeout:%ums",
                 data[1], (data[2] & 0x0f), (data[3] | (data[4] << 8)) * 1.25, (data[5] | (data[6] << 8)) * 1.25,
                 data[7] | (data[8] << 8), (data[9] | (data[10] << 8)) * 10);
        memcpy_s(remote_connection_parameter_request_reply, 10, data + 1, 10);
        remote_connection_parameter_request_reply[12] = 0xff;
        remote_connection_parameter_request_reply[13] = 0xff;
        hci_send_cmd_le_remote_connection_parameter_request_reply(remote_connection_parameter_request_reply);
        break;
    case HCI_EVENT_LE_ENHANCED_CONNECTION_COMPLETE:
        LOG_INFO("le_enhanced_connection_complete status:%u, connect_handle:0x%02x%02x, "
                 "peer_address_type:%u, peer_address:%02x:%02x:%02x:%02x:%02x:%02x",
                 data[1], data[2], (data[3] & 0x0f), data[5], data[6],  data[7], data[8], data[9], data[10], data[11]);
        sm_set_remote_address(data + 6);
        sm_set_remote_address_type(data[5]);
        LOG_INFO("/***** peer device connects success *****/");
        break;
    case HCI_EVENT_LE_DATA_LENGTH_CHANGE:
        LOG_INFO("le_data_length_change connect_handle:0x%02x%02x, max_tx_bytes:%u, max_tx_time:%uµs, max_rx_bytes:%u, max_rx_time:%uµs",
                 data[1], (data[2] & 0x0f), data[3] | (data[4] << 8), data[5] | (data[6] << 8),
                 data[7] | (data[8] << 8), data[9] | (data[10] << 8));
        break;
    case HCI_EVENT_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE:
        LOG_INFO("le_read_local_p256_public_key_complete status:%u", data[1]);
        sm_set_local_pairing_public_key(&data[2]);
        hci_send_cmd_le_set_advertising_parameters();
        break;
    case HCI_EVENT_LE_GENERATE_DHKEY_COMPLETE:
        LOG_INFO("le_generate_dhkey_complete status:%u", data[1]);
        sm_set_local_dhkey(&data[2]);
        break;
    default:
        LOG_WARNING("hci_recv_evt_le_meta invalid, sub_event:%u", sub_event);
        break;
    }
}

void hci_recv_evt_disconnection_complete(uint8_t* data, uint8_t length) {
    (void)length;
    LOG_INFO("disconnection_complete status:%u, connect_handle:0x%02x%02x, reason:0x%02x", data[0], data[1], (data[2] & 0x0f), data[3]);
    LOG_INFO("/***** peer device disconnect *****/");
}

void hci_recv_evt_number_of_completed_packets(uint8_t* data, uint8_t length) {
    (void)length;
    LOG_INFO("number_of_completed_packets number_handles:%u, connect_handle[0]:0x%02x%02x, num_completed_packets[0]:%u",
             data[0], data[1], (data[2] & 0x0f), data[3] | (data[4] << 8));
}

void hci_recv_acl(uint8_t *data, uint16_t length) {
    connect_handle = data[0] | ((data[1] & 0x0f) << 8);
    uint8_t pb_flag = data[1] & 0x30;

    if (HCI_ACL_SEGMENTATION_PACKET_FIRST == pb_flag) {
        segment_l2cap_total_length = data[4] | (data[5] << 8);
        segment_l2cap_current_length = length - HCI_LENGTH_ACL_HEADER;
        segment_l2cap_received_length = segment_l2cap_current_length;
        segment_l2cap_bytearray.resize(L2CAP_LENGTH_HEADER + segment_l2cap_total_length);
        memcpy_s(&segment_l2cap_bytearray[0], segment_l2cap_received_length, data + HCI_LENGTH_ACL_HEADER, segment_l2cap_received_length);
    } else if (HCI_ACL_SEGMENTATION_PACKET_CONTINUE == pb_flag) {
        segment_l2cap_current_length = length - HCI_LENGTH_ACL_HEADER;
        memcpy_s(&segment_l2cap_bytearray[segment_l2cap_received_length], segment_l2cap_current_length , data + HCI_LENGTH_ACL_HEADER, segment_l2cap_current_length);
        segment_l2cap_received_length += segment_l2cap_current_length;
    } else {
        LOG_ERROR("hci_recv_acl invalid, pb_flag:%02x", pb_flag);
        return;
    }

    if (segment_l2cap_received_length == segment_l2cap_total_length + L2CAP_LENGTH_HEADER) {
        l2cap_recv((uint8_t*)(segment_l2cap_bytearray.data()), L2CAP_LENGTH_HEADER + segment_l2cap_total_length);
    }
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

static void hci_assign_cmd(uint8_t *buffer, uint8_t ogf, uint16_t ocf) {
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

void hci_send_cmd_le_remote_connection_parameter_request_reply(uint8_t *data) {
    uint8_t buffer[HCI_LENGTH_CMD_LE_REMOTE_CONN_PARAM_REQ_REPLY] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY);
    buffer[3] = 14;
    memcpy_s(&buffer[4], 14, data, 14);
    serial_write(buffer, HCI_LENGTH_CMD_LE_REMOTE_CONN_PARAM_REQ_REPLY);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_REMOTE_CONN_PARAM_REQ_REPLY, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
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

void hci_send_cmd_le_set_data_length(uint16_t connect_handle, uint16_t tx_octets, uint16_t tx_time) {
    uint8_t buffer[HCI_LENGTH_CMD_LE_SET_DATA_LENGTH] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_DATA_LENGTH);
    buffer[3] = 6;
    buffer[4] = connect_handle;
    buffer[5] = (connect_handle >> 8) & 0x0f;
    buffer[6] = tx_octets;
    buffer[7] = (tx_octets >> 8) & 0x0f;
    buffer[8] = tx_time;
    buffer[9] = (tx_time >> 8) & 0x0f;
    serial_write(buffer, HCI_LENGTH_CMD_LE_SET_DATA_LENGTH);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_SET_DATA_LENGTH, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_read_suggested_default_data_length() {
    uint8_t buffer[HCI_LENGTH_CMD_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH);
    serial_write(buffer, HCI_LENGTH_CMD_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_write_suggested_default_data_length(uint16_t tx_octets, uint16_t tx_time) {
    uint8_t buffer[HCI_LENGTH_CMD_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH);
    buffer[3] = 4;
    buffer[4] = tx_octets;
    buffer[5] = (tx_octets >> 8) & 0x0f;
    buffer[6] = tx_time;
    buffer[7] = (tx_time >> 8) & 0x0f;
    serial_write(buffer, HCI_LENGTH_CMD_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_read_local_P256_public_key() {
    uint8_t buffer[HCI_LENGTH_CMD_LE_READ_LOCAL_P256_PUBLIC_KEY] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_READ_LOCAL_P256_PUBLIC_KEY);
    serial_write(buffer, HCI_LENGTH_CMD_LE_READ_LOCAL_P256_PUBLIC_KEY);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_READ_LOCAL_P256_PUBLIC_KEY, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}

void hci_send_cmd_le_generate_dhkey(uint8_t* data, uint8_t length) {
    uint8_t buffer[HCI_LENGTH_CMD_LE_GENERATE_DHKEY] = { 0x00 };
    hci_assign_cmd(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_GENERATE_DHKEY);
    buffer[3] = length;
    memcpy_s(&buffer[4], length, data, length);
    serial_write(buffer, HCI_LENGTH_CMD_LE_GENERATE_DHKEY);
    btsnoop_wirte(buffer, HCI_LENGTH_CMD_LE_GENERATE_DHKEY, BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}
