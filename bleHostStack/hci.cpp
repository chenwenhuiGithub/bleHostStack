#include "hci.h"
#include "serial.h"
#include "btsnoop.h"
#include "l2cap.h"
#include "log.h"
#include "sm.h"
#include "att.h"

// Opcode Group Field (OGF) values
#define HCI_OGF_LINK_CONTROL                                            0x01
#define HCI_OGF_LINK_POLICY                                             0x02
#define HCI_OGF_CONTROLLER_BASEBAND                                     0x03
#define HCI_OGF_INFORMATIONAL_PARAM                                     0x04
#define HCI_OGF_STATUS_PARAM                                            0x05
#define HCI_OGF_TESTING                                                 0x06
#define HCI_OGF_LE_CONTROLLER                                           0x08
#define HCI_OGF_VENDOR                                                  0x3f

// Opcode Command Field (OCF) values
// Link Control Commands
#define HCI_OCF_INQUIRY                                                 0x01
#define HCI_OCF_INQUIRY_CANCEL                                          0x02
#define HCI_OCF_PERIODIC_INQUIRY                                        0x03
#define HCI_OCF_EXIT_PERIODIC_INQUIRY                                   0x04
#define HCI_OCF_CREATE_CONNECT                                          0x05
#define HCI_OCF_DISCONNECT                                              0x06
#define HCI_OCF_CREATE_CONNECT_CANCEL                                   0x08
#define HCI_OCF_ACCEPT_CONNECT_REQ                                      0x09
#define HCI_OCF_REJECT_CONNECT_REQ                                      0x0a
#define HCI_OCF_LINK_KEY_REQ_REPLY                                      0x0b
#define HCI_OCF_LINK_KEY_REQ_NEG_REPLY                                  0x0c
#define HCI_OCF_PIN_CODE_REQ_REPLY                                      0x0d
#define HCI_OCF_PIN_CODE_REQ_NEG_REPLY                                  0x0e
#define HCI_OCF_CHANGE_CONNECT_PACKET_TYPE                              0x0f
#define HCI_OCF_AUTH_REQ                                                0x11
#define HCI_OCF_SET_CONNECT_ENCRYPTION                                  0x13
#define HCI_OCF_CHANGE_CONNECT_LINK_KEY                                 0x15
#define HCI_OCF_LINK_KEY_SELECTION                                      0x17
#define HCI_OCF_REMOTE_NAME_REQ                                         0x19
#define HCI_OCF_REMOTE_NAME_REQ_CANCEL                                  0x1a
#define HCI_OCF_READ_REMOTE_SUPPORT_FEATURES                            0x1b
#define HCI_OCF_READ_REMOTE_EXTEND_FEATURES                             0x1c
#define HCI_OCF_READ_REMOTE_VERSION_INFO                                0x1d
#define HCI_OCF_READ_CLOCK_OFFSET                                       0x1f
#define HCI_OCF_READ_LMP_HANDLE                                         0x20
#define HCI_OCF_SETUP_SCO_CONNECT_REQ                                   0x28
#define HCI_OCF_ACCEP_SCO_CONNECT_REQ                                   0x29
#define HCI_OCF_REJECT_SCO_CONNECT_REQ                                  0x2a
#define HCI_OCF_IO_CAP_REQ_REPLY                                        0x2b
#define HCI_OCF_USER_CONFIRM_REQ_REPLY                                  0x2c
#define HCI_OCF_USER_CONFIRM_REQ_NEG_REPLY                              0x2d
#define HCI_OCF_USER_PASSKEY_REQ_REPLY                                  0x2e
#define HCI_OCF_USER_PASSKEY_REQ_NEG_REPLY                              0x2f
#define HCI_OCF_REMOTE_OOB_REQ_REPLY                                    0x30
#define HCI_OCF_REMOTE_OOB_REQ_NEG_REPLY                                0x33
#define HCI_OCF_IO_CAP_REQ_NEG_REPLY                                    0x34

// Link Policy Commands
#define HCI_OCF_HOLD_MODE                                               0x01
#define HCI_OCF_SNIFF_MODE                                              0x03
#define HCI_OCF_EXIT_SNIFF_MODE                                         0x04
#define HCI_OCF_QOS_SETUP                                               0x07
#define HCI_OCF_ROLE_DISCOVERY                                          0x09
#define HCI_OCF_SWITCH_ROLE                                             0x0b
#define HCI_OCF_READ_LINK_POLICY                                        0x0c
#define HCI_OCF_WRITE_LINK_POLICY                                       0x0d
#define HCI_OCF_READ_DEFAULT_LINK_POLICY                                0x0e
#define HCI_OCF_WRITE_DEFAULT_LINK_POLICY                               0x0f
#define HCI_OCF_FLOW_SPEC                                               0x10
#define HCI_OCF_SNIFF_SUBRATING                                         0x11

// Controller and Baseband Commands
#define HCI_OCF_SET_EVENT_MASK                                          0x01
#define HCI_OCF_RESET                                                   0x03
#define HCI_OCF_SET_EVENT_FILTER                                        0x05
#define HCI_OCF_FLUSH                                                   0x08
#define HCI_OCF_READ_PIN_TYPE                                           0x09
#define HCI_OCF_WRITE_PIN_TYPE                                          0x0a
#define HCI_OCF_READ_STORED_LINK_KEY                                    0x0d
#define HCI_OCF_WRITE_STORED_LINK_KEY                                   0x11
#define HCI_OCF_DELETE_STORED_LINK_KEY                                  0x12
#define HCI_OCF_WRITE_LOCAL_NAME                                        0x13
#define HCI_OCF_READ_LOCAL_NAME                                         0x14
#define HCI_OCF_READ_CONNECT_ACCEPT_TIMEOUT                             0x15
#define HCI_OCF_WRITE_CONNECT_ACCEPT_TIMEOUT                            0x16
#define HCI_OCF_READ_PAGE_TIMEOUT                                       0x17
#define HCI_OCF_WRITE_PAGE_TIMEOUT                                      0x18
#define HCI_OCF_READ_SCAN_ENABLE                                        0x19
#define HCI_OCF_WRITE_SCAN_ENABLE                                       0x1a
#define HCI_OCF_READ_PAGE_SCAN_ACTIVITY                                 0x1b
#define HCI_OCF_WRITE_PAGE_SCAN_ACTIVITY                                0x1c
#define HCI_OCF_READ_INQUIRY_SCAN_ACTIVITY                              0x1d
#define HCI_OCF_WRITE_INQUIRY_SCAN_ACTIVITY                             0x1e
#define HCI_OCF_READ_AUTH_ENABLE                                        0x1f
#define HCI_OCF_WRITE_AUTH_ENABLE                                       0x20
#define HCI_OCF_READ_CLASS_OF_DEVICE                                    0x23
#define HCI_OCF_WRITE_CLASS_OF_DEVICE                                   0x24
#define HCI_OCF_READ_VOICE_SETTING                                      0x25
#define HCI_OCF_WRITE_VOICE_SETTING                                     0x26
#define HCI_OCF_READ_AUTO_FLUSH_TIMEOUT                                 0x27
#define HCI_OCF_WRITE_AUTO_FLUSH_TIMEOUT                                0x28
#define HCI_OCF_READ_NUM_BROADCAST_RETRANS                              0x29
#define HCI_OCF_WRITE_NUM_BROADCAST_RETRANS                             0x2a
#define HCI_OCF_READ_HOLD_MODE_ACTIVITY                                 0x2b
#define HCI_OCF_WRITE_HOLD_MODE_ACTIVITY                                0x2c
#define HCI_OCF_READ_TRANSMIT_POWER_LEVEL                               0x2d
#define HCI_OCF_READ_SCO_FLOW_CONTROL_ENABLE                            0x2e
#define HCI_OCF_WRITE_SCO_FLOW_CONTROL_ENABLE                           0x2f
#define HCI_OCF_SET_C2H_FLOW_CONTROL                                    0x31
#define HCI_OCF_HOST_BUFF_SIZE                                          0x33
#define HCI_OCF_HOST_NUM_OF_COMPLETED_PACKET                            0x35
#define HCI_OCF_READ_LINK_SUPERVISION_TIMEOUT                           0x36
#define HCI_OCF_WRITE_LINK_SUPERVISION_TIMEOUT                          0x37
#define HCI_OCF_READ_NUM_OF_SUPPORTED_IAC                               0x38
#define HCI_OCF_READ_CURRENT_IAC_LAP                                    0x39
#define HCI_OCF_WRITE_CURRENT_IAC_LAP                                   0x3a
#define HCI_OCF_SET_AFH_HOST_CHANNEL_CLASSIFICATION                     0x3f
#define HCI_OCF_READ_INQUIRY_SCAN_TYPE                                  0x42
#define HCI_OCF_WRITE_INQUIRY_SCAN_TYPE                                 0x43
#define HCI_OCF_READ_INQUIRY_MODE                                       0x44
#define HCI_OCF_WRITE_INQUIRY_MODE                                      0x45
#define HCI_OCF_READ_PAGE_SCAN_TYPE                                     0x46
#define HCI_OCF_WRITE_PAGE_SCAN_TYPE                                    0x47
#define HCI_OCF_READ_EXTENDED_INQUIRY_RESP                              0x51
#define HCI_OCF_WRITE_EXTENDED_INQUIRY_RESP                             0x52
#define HCI_OCF_WRITE_SIMPLE_PAIRING_MODE                               0x56
#define HCI_OCF_READ_LE_HOST_SUPPORT                                    0x6c
#define HCI_OCF_WRITE_LE_HOST_SUPPORT                                   0x6d

// Informational Parameters
#define HCI_OCF_READ_LOCAL_VERSION_INFO                                 0x01
#define HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS                           0x02
#define HCI_OCF_RAED_LOCAL_SUPPORTED_FEATURE                            0x03
#define HCI_OCF_READ_LOCAL_EXTENDED_FEATURE                             0x04
#define HCI_OCF_READ_BUFFER_SIZE                                        0x05
#define HCI_OCF_READ_BD_ADDR                                            0x09
#define HCI_OCF_READ_DATA_BLOCK_SIZE                                    0x0a

// Status Parameters
#define HCI_OCF_READ_FAILED_CONTACT_COUNTER                             0x01
#define HCI_OCF_RESET_FAILED_CONTACT_COUNTER                            0x02
#define HCI_OCF_READ_LINK_QUALITY                                       0x03
#define HCI_OCF_READ_RSSI                                               0x05
#define HCI_OCF_READ_AFH_CHANNEL_MAP                                    0x06
#define HCI_OCF_READ_CLOCK                                              0x07
#define HCI_OCF_READ_ENCRYPTION_KEY_SIZE                                0x08

// Testing Commands
#define HCI_OCF_READ_LOOPBACK_MODE                                      0x01
#define HCI_OCF_WRITE_LOOPBACK_MODE                                     0x02
#define HCI_OCF_ENABLE_DEVICE_UNDER_TEST_MODE                           0x03
#define HCI_OCF_WRITE_SIMPLE_PAIRING_DEBUG_MODE                         0x04
#define HCI_OCF_WRITE_SECURE_CONNECT_TEST_MODE                          0x0a

// LE Commands
#define HCI_OCF_LE_SET_EVENT_MASK                                       0x01
#define HCI_OCF_LE_READ_BUFFER_SIZE                                     0x02
#define HCI_OCF_LE_READ_LOCAL_SUPPORTED_FEATURE                         0x03
#define HCI_OCF_LE_SET_RANDOM_ADDRESS                                   0x05
#define HCI_OCF_LE_SET_ADV_PARAM                                        0x06
#define HCI_OCF_LE_SET_ADV_DATA                                         0x08
#define HCI_OCF_LE_SET_ADV_ENABLE                                       0x0a
#define HCI_OCF_LE_SET_SCAN_PARAM                                       0x0b
#define HCI_OCF_LE_SET_SCAN_ENABLE                                      0x0c
#define HCI_OCF_LE_CREATE_CONNECT                                       0x0d
#define HCI_OCF_LE_LTK_REQ_REPLY                                        0x1a
#define HCI_OCF_LE_LTK_REQ_NEG_REPLY                                    0x1b
#define HCI_OCF_LE_REMOTE_CONN_PARAM_REQ_REPLY                          0x20
#define HCI_OCF_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY                      0x21
#define HCI_OCF_LE_SET_DATA_LENGTH                                      0x22
#define HCI_OCF_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH                   0x23
#define HCI_OCF_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH                  0x24
#define HCI_OCF_LE_READ_LOCAL_P256_PUBLIC_KEY                           0x25
#define HCI_OCF_LE_GENERATE_DHKEY                                       0x26

// event codes
#define HCI_EVENT_INQUIRY_COMPLETE                                      0x01
#define HCI_EVENT_INQUIRY_RESULT                                        0x02
#define HCI_EVENT_CONN_COMPLETE                                         0x03
#define HCI_EVENT_CONN_REQ                                              0x04
#define HCI_EVENT_DISCONN_COMPLETE                                      0x05
#define HCI_EVENT_AUTH_COMPLETE                                         0x06
#define HCI_EVENT_REMOTE_NAME_REQ_COMPLETE                              0x07
#define HCI_EVENT_ENCRYPTION_CHANGE                                     0x08
#define HCI_EVENT_CHANGE_CONN_LINK_KEY_COMPLETE                         0x09
#define HCI_EVENT_LINK_KEY_TYPE_CHANGED                                 0x0a
#define HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE               0x0b
#define HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE              0x0c
#define HCI_EVENT_QOS_SETUP_COMPLETE                                    0x0d
#define HCI_EVENT_COMMAND_COMPLETE                                      0x0e
#define HCI_EVENT_COMMAND_STATUS                                        0x0f
#define HCI_EVENT_HARDWARE_ERROR                                        0x10
#define HCI_EVENT_FLUSH_OCCURRED                                        0x11
#define HCI_EVENT_ROLE_CHANGE                                           0x12
#define HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS                           0x13
#define HCI_EVENT_MODE_CHANGE                                           0x14
#define HCI_EVENT_RETURN_LINK_KEYS                                      0x15
#define HCI_EVENT_PIN_CODE_REQ                                          0x16
#define HCI_EVENT_LINK_KEY_REQ                                          0x17
#define HCI_EVENT_LINK_KEY_NOTIFY                                       0x18
#define HCI_EVENT_LOOPBACK_COMMAND                                      0x19
#define HCI_EVENT_DATA_BUFFER_OVERFLOW                                  0x1a
#define HCI_EVENT_MAX_SLOTS_CHANGE                                      0x1b
#define HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE                            0x1c
#define HCI_EVENT_CONN_PACKET_TYPE_CHANGED                              0x1d
#define HCI_EVENT_QOS_VIOLATION                                         0x1e
#define HCI_EVENT_PAGE_SCAN_REPETITION_MODE_CHANGE                      0x20
#define HCI_EVENT_FLOW_SPECIFICATION_COMPLETE                           0x21
#define HCI_EVENT_INQUIRY_RESULT_WITH_RSSI                              0x22
#define HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE                0x23
#define HCI_EVENT_SCO_CONN_COMPLETE                                     0x2c
#define HCI_EVENT_SCO_CONN_CHANGED                                      0x2d
#define HCI_EVENT_SNIFF_SUBRATING                                       0x2e
#define HCI_EVENT_EXTENDED_INQUIRY_RESULT                               0x2f
#define HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE                       0x30
#define HCI_EVENT_IO_CAPABILITY_REQ                                     0x31
#define HCI_EVENT_IO_CAPABILITY_RESP                                    0x32
#define HCI_EVENT_USER_CONFIRMATION_REQ                                 0x33
#define HCI_EVENT_USER_PASSKEY_REQ                                      0x34
#define HCI_EVENT_REMOTE_OOB_DATA_REQ                                   0x35
#define HCI_EVENT_SIMPLE_PAIRING_COMPLETE                               0x36
#define HCI_EVENT_LINK_SUPERVISION_TIMEOUT_CHANGED                      0x38
#define HCI_EVENT_ENHANCED_FLUSH_COMPLETE                               0x39
#define HCI_EVENT_USER_PASSKEY_NOTIFY                                   0x3b
#define HCI_EVENT_KEYPRESS_NOTIFY                                       0x3c
#define HCI_EVENT_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFY                 0x3d
#define HCI_EVENT_LE_META                                               0x3e
#define HCI_EVENT_NUMBER_OF_COMPLETED_DATA_BLOCKS                       0x48

// le meta event codes
#define HCI_EVENT_LE_CONN_COMPLETE                                      0x01
#define HCI_EVENT_LE_ADV_REPORT                                         0x02
#define HCI_EVENT_LE_CONN_UPDATE_COMPLETE                               0x03
#define HCI_EVENT_LE_READ_REMOTE_FEATURES_COMPLETE                      0x04
#define HCI_EVENT_LE_LTK_REQ                                            0x05
#define HCI_EVENT_LE_REMOTE_CONN_PARAM_REQ                              0x06
#define HCI_EVENT_LE_DATA_LENGTH_CHANGE                                 0x07
#define HCI_EVENT_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE                0x08
#define HCI_EVENT_LE_GENERATE_DHKEY_COMPLETE                            0x09
#define HCI_EVENT_LE_ENHANCED_CONN_COMPLETE                             0x0a
#define HCI_EVENT_LE_DIRECTED_ADV_REPORT                                0x0b


#define HCI_LENGTH_CMD_HEADER                                           4
#define HCI_LENGTH_CMD_PARAM_RESET                                      0
#define HCI_LENGTH_CMD_PARAM_READ_BD_ADDR                               0
#define HCI_LENGTH_CMD_PARAM_READ_LOCAL_VERSION_INFO                    0
#define HCI_LENGTH_CMD_PARAM_READ_LOCAL_SUPPORTED_COMMANDS              0
#define HCI_LENGTH_CMD_PARAM_SET_EVENT_MASK                             8
#define HCI_LENGTH_CMD_PARAM_WRITE_CLASS_OF_DEVICE                      3
#define HCI_LENGTH_CMD_PARAM_WRITE_LE_HOST_SUPPORT                      2
#define HCI_LENGTH_CMD_PARAM_LE_READ_BUFFER_SIZE                        0
#define HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_REPLY             14
#define HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY         3
#define HCI_LENGTH_CMD_PARAM_LE_SET_EVENT_MASK                          8
#define HCI_LENGTH_CMD_PARAM_LE_SET_ADV_PARAM                           15
#define HCI_LENGTH_CMD_PARAM_LE_SET_ADV_DATA                            32
#define HCI_LENGTH_CMD_PARAM_LE_SET_ADV_ENABLE                          1
#define HCI_LENGTH_CMD_PARAM_LE_SET_DATA_LENGTH                         6
#define HCI_LENGTH_CMD_PARAM_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH      0
#define HCI_LENGTH_CMD_PARAM_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH     4
#define HCI_LENGTH_CMD_PARAM_LE_READ_LOCAL_P256_PUBLIC_KEY              0
#define HCI_LENGTH_CMD_PARAM_LE_GENERATE_DHKEY                          64
#define HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_REPLY                           18
#define HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_NEG_REPLY                       2

#define HCI_ACL_SEGMENT_PACKET_FIRST                                    0x20
#define HCI_ACL_SEGMENT_PACKET_CONTINUE                                 0x10


static void __hci_recv_evt_command_complete(uint8_t *data, uint32_t length);
static void __hci_recv_evt_command_status(uint8_t *data, uint32_t length);
static void __hci_recv_evt_le_meta(uint8_t *data, uint32_t length);
static void __hci_recv_evt_disconn_complete(uint8_t* data, uint32_t length);
static void __hci_recv_evt_number_of_completed_packets(uint8_t* data, uint32_t length);
static void __hci_recv_evt_encryption_change(uint8_t* data, uint32_t length);
static void __hci_assign_cmd_header(uint8_t *buffer, uint8_t ogf, uint16_t ocf, uint8_t param_length);

typedef struct {
    uint8_t class_of_device[HCI_LENGTH_CLASS_OF_DEVICE];
    uint8_t event_mask[HCI_LENGTH_EVENT_MASK];
    uint8_t le_event_mask[HCI_LENGTH_LE_EVENT_MASK];

    hci_le_adv_param_t le_adv_param;
    hci_le_adv_data_t le_adv_data;
    hci_le_conn_param_t le_conn_param;

    uint16_t le_acl_packet_length;
    uint8_t num_of_allowed_le_acl_packets;
    uint8_t num_of_allowed_cmd_packets;

    uint8_t local_addr[HCI_LENGTH_ADDR];
    uint8_t local_addr_type;
    uint8_t local_p256_public_key[HCI_LENGTH_P256_PUBLIC_KEY];

    hci_connection_t *hci_connections;
} hci_stack_t;

static hci_stack_t hci_stack = {
    .class_of_device = {0x0c, 0x02, 0x7a},
    .event_mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f},
    .le_event_mask = {0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00},

    .le_adv_param = {
        .adv_interval_min = 0x0800, // 1.28s(N*0.625ms)
        .adv_interval_max = 0x0800, // 1.28s(N*0.625ms)
        .adv_type = HCI_ADV_TYPE_ADV_IND,
        .own_addr_type = HCI_ADDR_TYPE_PUBLIC_DEVICE,
        .peer_addr_type = HCI_ADDR_TYPE_PUBLIC_DEVICE,
        .peer_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .adv_channel_map = HCI_ADV_CHANNEL_MAP_37 | HCI_ADV_CHANNEL_MAP_38 | HCI_ADV_CHANNEL_MAP_39,
        .adv_filter_policy = HCI_ADV_FILTER_POLICY_SCAN_ALL_CONN_ALL
    },

    .le_adv_data = {
        .adv_data_length = 0x0d,
        .adv_data = {
            0x02, 0x01, 0x06, // flag:0x06
            0x09, 0x09, 0x62, 0x6c, 0x65, 0x5f, 0x64, 0x65, 0x6d, 0x6f, // device_name:ble_demo
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        }
    },

    .le_conn_param = {
        .conn_interval_min = 0x0018, // 30ms(N*1.25ms)
        .conn_interval_max = 0x0018, // 30ms(N*1.25ms)
        .max_latency = 10,
        .timeout = 0x0080, // 1.28s(N*10ms)
        .min_ce_length = 0x0028, // 25ms(N*0.625ms)
        .max_ce_length = 0x0028  // 25ms(N*0.625ms)
    }
};

static void __hci_assign_cmd_header(uint8_t *buffer, uint8_t ogf, uint16_t ocf, uint8_t param_length) {
    buffer[0] = HCI_PACKET_TYPE_CMD;
    buffer[1] = ocf;
    buffer[2] = (ocf >> 8) | (ogf << 2);
    buffer[3] = param_length;
}

static void __hci_recv_evt_command_complete(uint8_t *data, uint32_t length) {
    (void)length;
    uint8_t ogf = data[2] >> 2;
    uint16_t ocf = ((data[2] & 0x03) << 8) | data[1];

    hci_stack.num_of_allowed_cmd_packets = data[0];
    switch (ogf) {
    case HCI_OGF_CONTROLLER_BASEBAND:
        switch (ocf) {
        case HCI_OCF_RESET:
            LOG_INFO("reset status:%u", data[3]);
            hci_send_cmd_read_local_version_info();
            break;
        case HCI_OCF_WRITE_CLASS_OF_DEVICE:
            LOG_INFO("write_class_of_device status:%u", data[3]);
            hci_send_cmd_set_event_mask(hci_stack.event_mask);
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
            LOG_WARNING("__hci_recv_evt_command_complete invalid, ogf:0x%02x, ocf:0x%04x", ogf, ocf);
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
            // HCI_Read_BD_ADDR used to get public address
            // HCI_LE_Set_Random_Address used to set random address, once set, controller will use random address for hci connection
            hci_send_cmd_read_bd_addr();
            break;
        case HCI_OCF_READ_BD_ADDR:
            LOG_INFO("read_bd_addr status:%u, bd_addr:%02x:%02x:%02x:%02x:%02x:%02x",
                     data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
            memcpy_s(hci_stack.local_addr, HCI_LENGTH_ADDR, data + 4, HCI_LENGTH_ADDR);
            hci_stack.local_addr_type = HCI_ADDR_TYPE_PUBLIC_DEVICE;
            hci_send_cmd_write_class_of_device(hci_stack.class_of_device);
            break;
        default:
            LOG_WARNING("__hci_recv_evt_command_complete invalid, ogf:0x%02x, ocf:0x%04x", ogf, ocf);
            break;
        }
        break;
    case HCI_OGF_LE_CONTROLLER:
        switch (ocf) {
        case HCI_OCF_LE_READ_BUFFER_SIZE:
            LOG_INFO("le_read_buffer_size status:%u, le_acl_packet_length:%u, total_num_le_acl_packets:%u",
                     data[3], (data[4] | (data[5] << 8)), data[6]);
            hci_stack.le_acl_packet_length = data[4] | (data[5] << 8);
            hci_stack.num_of_allowed_le_acl_packets = data[6];
            hci_send_cmd_le_set_event_mask(hci_stack.le_event_mask);
            break;
        case HCI_OCF_LE_SET_EVENT_MASK:
            LOG_INFO("le_set_event_mask status:%u", data[3]);
            hci_send_cmd_le_read_local_P256_public_key();
            break;
        case HCI_OCF_LE_SET_ADV_PARAM:
            LOG_INFO("le_set_adv_param status:%u", data[3]);
            hci_send_cmd_le_set_adv_data(&hci_stack.le_adv_data);
            break;
        case HCI_OCF_LE_SET_ADV_DATA:
            LOG_INFO("le_set_adv_data status:%u", data[3]);
            hci_send_cmd_le_set_adv_enable(HCI_LE_ADV_ENABLE);
            break;
        case HCI_OCF_LE_SET_ADV_ENABLE:
            LOG_INFO("le_set_adv_enable status:%u", data[3]);
            LOG_INFO("/***** wait peer devices to connect *****/");
            break;
        case HCI_OCF_LE_LTK_REQ_REPLY:
            LOG_INFO("le_ltk_req_reply status:%u, connect_handle:0x%02x%02x", data[3], data[4], (data[5] & 0x0f));
            break;
        case HCI_OCF_LE_LTK_REQ_NEG_REPLY:
            LOG_INFO("le_ltk_req_neg_reply status:%u, connect_handle:0x%02x%02x", data[3], data[4], (data[5] & 0x0f));
            break;
        case HCI_OCF_LE_REMOTE_CONN_PARAM_REQ_REPLY:
            LOG_INFO("le_remote_conn_param_req_reply status:%u, connect_handle:0x%02x%02x",
                     data[3], data[4], (data[5] & 0x0f));
            break;
        case HCI_OCF_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY:
            LOG_INFO("hci_send_cmd_le_remote_conn_param_req_neg_reply status:%u, connect_handle:0x%02x%02x",
                     data[3], data[4], (data[5] & 0x0f));
            break;
        default:
            LOG_WARNING("__hci_recv_evt_command_complete invalid, ogf:0x%02x, ocf:0x%04x", ogf, ocf);
            break;
        }
        break;
    default:
        LOG_WARNING("__hci_recv_evt_command_complete invalid, ogf:0x%02x, ocf:0x%04x", ogf, ocf);
        break;
    }
}

static void __hci_recv_evt_command_status(uint8_t *data, uint32_t length) {
    (void)length;
    uint8_t ogf = data[3] >> 2;
    uint16_t ocf = ((data[3] & 0x03) << 8) | data[2];

    hci_stack.num_of_allowed_cmd_packets = data[1];
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
            LOG_WARNING("__hci_recv_evt_command_status invalid, ogf:0x%02x, ocf:0x%04x", ogf, ocf);
            break;
        }
        break;
    default:
        LOG_WARNING("__hci_recv_evt_command_status invalid, ogf:0x%02x, ocf:0x%04x", ogf, ocf);
        break;
    }
}

static void __hci_recv_evt_le_meta(uint8_t *data, uint32_t length) {
    uint8_t sub_event = data[0];
    uint16_t connect_handle = 0x0000;

    switch (sub_event) {
    case HCI_EVENT_LE_CONN_COMPLETE:
        LOG_INFO("le_conn_complete status:%u, connect_handle:0x%02x%02x, peer_addr_type:0x%02x, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x",
                 data[1], data[2], (data[3] & 0x0f), data[5], data[6],  data[7], data[8], data[9], data[10], data[11]);
        connect_handle = data[2] | ((data[3] & 0x0f) << 8);
        hci_add_connection(connect_handle, data + 6, data[5]);
        LOG_INFO("/***** peer device connects success *****/");
        break;
    case HCI_EVENT_LE_CONN_UPDATE_COMPLETE:
        LOG_INFO("le_conn_update_complete status:%u, connect_handle:0x%02x%02x, interval:%0.2fms, latency:%u, timeout:%ums",
                 data[1], data[2], (data[3] & 0x0f), (data[4] | (data[5] << 8)) * 1.25, data[6] | (data[7] << 8), (data[8] | (data[9] << 8)) * 10);
        break;
    case HCI_EVENT_LE_LTK_REQ:
        LOG_INFO("le_ltk_req connect_handle:0x%02x%02x, random_number:0x%02x%02x%02x%02x%02x%02x%02x%02x, encrypted_diversifier:0x%02x%02x",
                 data[1], (data[2] & 0x0f), data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12]);
        connect_handle = data[1] | ((data[2] & 0x0f) << 8);
        sm_recv_evt_le_ltk_req(connect_handle, data + 3, data + 11);
        break;
    case HCI_EVENT_LE_REMOTE_CONN_PARAM_REQ:
        LOG_INFO("le_remote_conn_param_req connect_handle:0x%02x%02x, interval_min:%0.2fms, interval_max:%0.2fms, max_latency:%u, timeout:%ums",
                 data[1], (data[2] & 0x0f), (data[3] | (data[4] << 8)) * 1.25, (data[5] | (data[6] << 8)) * 1.25,
                 data[7] | (data[8] << 8), (data[9] | (data[10] << 8)) * 10);
        connect_handle = data[1] | ((data[2] & 0x0f) << 8);
        hci_send_cmd_le_remote_conn_param_req_reply(connect_handle, &hci_stack.le_conn_param);
        break;
    case HCI_EVENT_LE_ENHANCED_CONN_COMPLETE:
        LOG_INFO("le_enhanced_conn_complete status:%u, connect_handle:0x%02x%02x, peer_addr_type:0x%02x, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x",
                 data[1], data[2], (data[3] & 0x0f), data[5], data[6],  data[7], data[8], data[9], data[10], data[11]);
        connect_handle = data[2] | ((data[3] & 0x0f) << 8);
        hci_add_connection(connect_handle, data + 6, data[5]);
        LOG_INFO("/***** peer device connects success *****/");
        break;
    case HCI_EVENT_LE_DATA_LENGTH_CHANGE:
        LOG_INFO("le_data_length_change connect_handle:0x%02x%02x, max_tx_bytes:%u, max_tx_time:%uµs, max_rx_bytes:%u, max_rx_time:%uµs",
                 data[1], (data[2] & 0x0f), data[3] | (data[4] << 8), data[5] | (data[6] << 8),
                 data[7] | (data[8] << 8), data[9] | (data[10] << 8));
        break;
    case HCI_EVENT_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE:
        LOG_INFO("le_read_local_p256_public_key_complete status:%u", data[1]);
        memcpy_s(hci_stack.local_p256_public_key, HCI_LENGTH_P256_PUBLIC_KEY, data + 2, HCI_LENGTH_P256_PUBLIC_KEY);
        hci_send_cmd_le_set_adv_param(&hci_stack.le_adv_param);
        break;
    case HCI_EVENT_LE_GENERATE_DHKEY_COMPLETE:
        LOG_INFO("le_generate_dhkey_complete status:%u", data[1]);
        sm_recv_evt_le_generate_dhkey_complete(data + 2); // all connections re-used one local pub-priv key pair?
        break;
    default:
        LOG_WARNING("__hci_recv_evt_le_meta invalid, sub_event:0x%02x", sub_event);
        break;
    }
}

static void __hci_recv_evt_disconn_complete(uint8_t* data, uint32_t length) {
    (void)length;
    uint16_t connect_handle = data[1] | ((data[2] & 0x0f) << 8);
    hci_connection_t *deleted_connection = hci_delete_connection_by_handle(connect_handle);

    if (deleted_connection->segment_buffer_recv) {
        free(deleted_connection->segment_buffer_recv);
        deleted_connection->segment_buffer_recv = nullptr;
    }
    free(deleted_connection);
    deleted_connection = nullptr;
    LOG_INFO("disconn_complete status:%u, connect_handle:0x%02x%02x, reason:0x%02x", data[0], data[1], (data[2] & 0x0f), data[3]);
    LOG_INFO("/***** peer device disconnect *****/");
}

static void __hci_recv_evt_number_of_completed_packets(uint8_t* data, uint32_t length) {
    (void)length;
    uint8_t index = 0;
    uint8_t number_handles = data[0];
    uint8_t all_completed_packets_number = 0;

    for (index = 0; index < number_handles; index++) {
        LOG_INFO("number_of_completed_packets number_handles:%u, connect_handle[%u]:0x%02x%02x, num_completed_packets:%u",
                 number_handles, index, data[2 * index + 1], (data[2 * index + 2] & 0x0f), data[2 * (number_handles + index) + 1]);
        all_completed_packets_number += data[2 * (number_handles + index) + 1];
    }
    hci_stack.num_of_allowed_le_acl_packets += all_completed_packets_number;
}

static void __hci_recv_evt_encryption_change(uint8_t* data, uint32_t length) {
    (void)length;
    uint16_t connect_handle = data[1] | ((data[2] & 0x0f) << 8);

    LOG_INFO("encryption_change status:%u, connect_handle[0]:0x%02x%02x, encryption_enabled:0x%02x", data[0], data[1], (data[2] & 0x0f), data[3]);
    sm_recv_evt_encryption_change(connect_handle, data[3]);
}

void hci_init() {
    hci_stack.le_acl_packet_length = 0;
    hci_stack.num_of_allowed_le_acl_packets = 0;
    hci_stack.num_of_allowed_cmd_packets = 1;
    hci_stack.hci_connections = nullptr;

    hci_send_cmd_reset();
}

void hci_recv_evt(uint8_t *data, uint32_t length) {
    uint8_t event_code = data[0];

    switch (event_code) {
    case HCI_EVENT_DISCONN_COMPLETE:
        __hci_recv_evt_disconn_complete(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_ENCRYPTION_CHANGE:
        __hci_recv_evt_encryption_change(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_COMMAND_COMPLETE:
        __hci_recv_evt_command_complete(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_COMMAND_STATUS:
        __hci_recv_evt_command_status(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_LE_META:
        __hci_recv_evt_le_meta(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
        __hci_recv_evt_number_of_completed_packets(data + HCI_LENGTH_EVT_HEADER, length - HCI_LENGTH_EVT_HEADER); break;
    default:
        LOG_WARNING("hci_recv_evt invalid, event_code:0x%02x", event_code); break;
    }
}

void hci_recv_acl(uint8_t *data, uint32_t length) {
    uint16_t connect_handle = data[0] | ((data[1] & 0x0f) << 8);
    hci_connection_t *conn = hci_find_connection_by_handle(connect_handle);
    uint8_t pb_flag = data[1] & 0x30;
    uint16_t segment_length_current = 0;

    if (HCI_ACL_SEGMENT_PACKET_FIRST == pb_flag) {
        conn->segment_length_total = data[4] | (data[5] << 8); // TODO: based on connect_handle
        segment_length_current = length - HCI_LENGTH_ACL_HEADER;
        conn->segment_length_received = length - HCI_LENGTH_ACL_HEADER;

        if (conn->segment_length_received == conn->segment_length_total + L2CAP_LENGTH_HEADER) {
            l2cap_recv(connect_handle, data + HCI_LENGTH_ACL_HEADER, length - HCI_LENGTH_ACL_HEADER);
            return;
        }
        conn->segment_buffer_recv = (uint8_t *)malloc(L2CAP_LENGTH_HEADER + conn->segment_length_total);
        memcpy_s(conn->segment_buffer_recv, segment_length_current, data + HCI_LENGTH_ACL_HEADER, segment_length_current);
    } else if (HCI_ACL_SEGMENT_PACKET_CONTINUE == pb_flag) {
        segment_length_current = length - HCI_LENGTH_ACL_HEADER;
        memcpy_s(conn->segment_buffer_recv + conn->segment_length_received, segment_length_current, data + HCI_LENGTH_ACL_HEADER, segment_length_current);
        conn->segment_length_received += segment_length_current;

        if (conn->segment_length_received == conn->segment_length_total + L2CAP_LENGTH_HEADER) {
            l2cap_recv(connect_handle, conn->segment_buffer_recv, conn->segment_length_received);
            free(conn->segment_buffer_recv);
            conn->segment_buffer_recv = nullptr;
        }
    } else {
        LOG_ERROR("hci_recv_acl invalid, pb_flag:0x%02x", pb_flag);
    }
}

void hci_send_acl(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint32_t packet_length = 0;
    uint32_t offset = HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER;
    bool first_segment_sent = false;
    uint8_t *segment_buffer_send = nullptr;

    if (length <= hci_stack.le_acl_packet_length) {
        if (hci_send_acl_allowed()) {
            data[0] = HCI_PACKET_TYPE_ACL;
            data[1] = connect_handle;
            data[2] = (connect_handle >> 8) & 0x0f;
            data[2] |= HCI_ACL_SEGMENT_PACKET_FIRST;
            data[3] = length;
            data[4] = length >> 8;

            packet_length = HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + length;
            serial_write(data, packet_length);
            btsnoop_wirte(data, packet_length, BTSNOOP_PACKET_FLAG_ACL_SCO_SEND);
            hci_stack.num_of_allowed_le_acl_packets--;
        } else {
            LOG_ERROR("hci_send_acl error, send acl not allowed");
        }
    } else {
        segment_buffer_send = (uint8_t *)malloc(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + hci_stack.le_acl_packet_length);
        while (true) {
            if (hci_send_acl_allowed()) {
                segment_buffer_send[0] = HCI_PACKET_TYPE_ACL;
                segment_buffer_send[1] = connect_handle;
                segment_buffer_send[2] = (connect_handle >> 8) & 0x0f;
                if (false == first_segment_sent) {
                    segment_buffer_send[2] |= HCI_ACL_SEGMENT_PACKET_FIRST;
                    first_segment_sent = true;
                } else {
                    segment_buffer_send[2] |= HCI_ACL_SEGMENT_PACKET_CONTINUE;
                }

                if (length > hci_stack.le_acl_packet_length) {
                    segment_buffer_send[3] = hci_stack.le_acl_packet_length;
                    segment_buffer_send[4] = hci_stack.le_acl_packet_length >> 8;
                    memcpy_s(segment_buffer_send + 5, hci_stack.le_acl_packet_length, data + offset, hci_stack.le_acl_packet_length);

                    packet_length = HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + hci_stack.le_acl_packet_length;
                    serial_write(segment_buffer_send, packet_length);
                    btsnoop_wirte(segment_buffer_send, packet_length, BTSNOOP_PACKET_FLAG_ACL_SCO_SEND);
                    length -= hci_stack.le_acl_packet_length;
                    offset += hci_stack.le_acl_packet_length;
                    hci_stack.num_of_allowed_le_acl_packets--;
                } else {
                    if (length > 0) { // the last segment packet
                        segment_buffer_send[3] = length;
                        segment_buffer_send[4] = length >> 8;
                        memcpy_s(segment_buffer_send + 5, length, data + offset, length);

                        packet_length = HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + length;
                        serial_write(segment_buffer_send, packet_length);
                        btsnoop_wirte(segment_buffer_send, packet_length, BTSNOOP_PACKET_FLAG_ACL_SCO_SEND);
                        hci_stack.num_of_allowed_le_acl_packets--;
                    }
                    goto RET;
                }
            } else {
                LOG_ERROR("hci_send_acl error, send acl not allowed");
                goto RET;
            }
        }
    }

RET:
    if (segment_buffer_send) {
        free(segment_buffer_send);
        segment_buffer_send = nullptr;
    }
}

bool hci_send_cmd_allowed() {
    return hci_stack.num_of_allowed_cmd_packets > 0;
}

bool hci_send_acl_allowed() {
    return hci_stack.num_of_allowed_le_acl_packets > 0;
}

void hci_send_cmd_reset() {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_RESET;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_RESET] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_RESET, HCI_LENGTH_CMD_PARAM_RESET);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_reset error, send cmd not allowed");
    }
}

void hci_send_cmd_read_local_version_info() {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_READ_LOCAL_VERSION_INFO;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_READ_LOCAL_VERSION_INFO] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_VERSION_INFO,
                                HCI_LENGTH_CMD_PARAM_READ_LOCAL_VERSION_INFO);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_read_local_version_info error, send cmd not allowed");
    }
}

void hci_send_cmd_read_local_supported_commands() {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_READ_LOCAL_SUPPORTED_COMMANDS;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_READ_LOCAL_SUPPORTED_COMMANDS] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_LOCAL_SUPPORTED_COMMANDS,
                                HCI_LENGTH_CMD_PARAM_READ_LOCAL_SUPPORTED_COMMANDS);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_read_local_supported_commands error, send cmd not allowed");
    }
}

void hci_send_cmd_set_event_mask(uint8_t *event_mask) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_SET_EVENT_MASK;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_SET_EVENT_MASK] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_SET_EVENT_MASK, HCI_LENGTH_CMD_PARAM_SET_EVENT_MASK);
        memcpy_s(&buffer[4], HCI_LENGTH_EVENT_MASK, event_mask, HCI_LENGTH_EVENT_MASK);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_set_event_mask error, send cmd not allowed");
    }
}

void hci_send_cmd_write_le_host_support(hci_le_host_support_t enable) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_WRITE_LE_HOST_SUPPORT;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_WRITE_LE_HOST_SUPPORT] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_LE_HOST_SUPPORT, HCI_LENGTH_CMD_PARAM_WRITE_LE_HOST_SUPPORT);
        buffer[4] = enable;
        buffer[5] = 0x00;
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_write_le_host_support error, send cmd not allowed");
    }
}

void hci_send_cmd_le_read_buffer_size() {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_READ_BUFFER_SIZE;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_READ_BUFFER_SIZE] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_READ_BUFFER_SIZE, HCI_LENGTH_CMD_PARAM_LE_READ_BUFFER_SIZE);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_read_buffer_size error, send cmd not allowed");
    }
}

void hci_send_cmd_read_bd_addr() {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_READ_BD_ADDR;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_READ_BD_ADDR] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_INFORMATIONAL_PARAM, HCI_OCF_READ_BD_ADDR, HCI_LENGTH_CMD_PARAM_READ_BD_ADDR);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_read_bd_addr error, send cmd not allowed");
    }
}

void hci_send_cmd_write_class_of_device(uint8_t *class_of_device) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_WRITE_CLASS_OF_DEVICE;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_WRITE_CLASS_OF_DEVICE] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_CONTROLLER_BASEBAND, HCI_OCF_WRITE_CLASS_OF_DEVICE, HCI_LENGTH_CMD_PARAM_WRITE_CLASS_OF_DEVICE);
        memcpy_s(&buffer[4], HCI_LENGTH_CLASS_OF_DEVICE, class_of_device, HCI_LENGTH_CLASS_OF_DEVICE);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_write_class_of_device error, send cmd not allowed");
    }
}

void hci_send_cmd_le_remote_conn_param_req_reply(uint16_t connect_handle, hci_le_conn_param_t *conn_param) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_REPLY;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_REPLY] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_REMOTE_CONN_PARAM_REQ_REPLY,
                                HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_REPLY);
        buffer[4] = connect_handle;
        buffer[5] = (connect_handle >> 8) & 0x0f;
        buffer[6] = conn_param->conn_interval_min;
        buffer[7] = conn_param->conn_interval_min >> 8;
        buffer[8] = conn_param->conn_interval_max;
        buffer[9] = conn_param->conn_interval_max >> 8;
        buffer[10] = conn_param->max_latency;
        buffer[11] = conn_param->max_latency >> 8;
        buffer[12] = conn_param->timeout;
        buffer[13] = conn_param->timeout >> 8;
        buffer[14] = conn_param->min_ce_length;
        buffer[15] = conn_param->min_ce_length >> 8;
        buffer[16] = conn_param->max_ce_length;
        buffer[17] = conn_param->max_ce_length >> 8;
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_remote_conn_param_req_reply error, send cmd not allowed");
    }
}

void hci_send_cmd_le_remote_conn_param_req_neg_reply(uint16_t connect_handle, uint8_t reason) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY,
                                HCI_LENGTH_CMD_PARAM_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY);
        buffer[4] = connect_handle;
        buffer[5] = (connect_handle >> 8) & 0x0f;
        buffer[6] = reason;
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_remote_conn_param_req_neg_reply error, send cmd not allowed");
    }
}

void hci_send_cmd_le_set_event_mask(uint8_t *le_event_mask) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_EVENT_MASK;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_EVENT_MASK] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_EVENT_MASK, HCI_LENGTH_CMD_PARAM_LE_SET_EVENT_MASK);
        memcpy_s(&buffer[4], HCI_LENGTH_LE_EVENT_MASK, le_event_mask, HCI_LENGTH_LE_EVENT_MASK);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_set_event_mask error, send cmd not allowed");
    }
}

void hci_send_cmd_le_set_adv_param(hci_le_adv_param_t *adv_param){
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_ADV_PARAM;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_ADV_PARAM] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_ADV_PARAM, HCI_LENGTH_CMD_PARAM_LE_SET_ADV_PARAM);
        buffer[4] = adv_param->adv_interval_min;
        buffer[5] = adv_param->adv_interval_min >> 8;
        buffer[6] = adv_param->adv_interval_max;
        buffer[7] = adv_param->adv_interval_max >> 8;
        buffer[8] = adv_param->adv_type;
        buffer[9] = adv_param->own_addr_type;
        buffer[10] = adv_param->peer_addr_type;
        memcpy_s(&buffer[11], HCI_LENGTH_ADDR, adv_param->peer_addr, HCI_LENGTH_ADDR);
        buffer[17] = adv_param->adv_channel_map;
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_set_adv_param error, send cmd not allowed");
    }
}

void hci_send_cmd_le_set_adv_data(hci_le_adv_data_t *adv_data) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_ADV_DATA;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_ADV_DATA] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_ADV_DATA, HCI_LENGTH_CMD_PARAM_LE_SET_ADV_DATA);
        buffer[4] = adv_data->adv_data_length;
        memcpy_s(&buffer[5], HCI_LENGTH_ADV_DATA, adv_data->adv_data, HCI_LENGTH_ADV_DATA);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_set_adv_data error, send cmd not allowed");
    }
}

void hci_send_cmd_le_set_adv_enable(hci_le_adv_enable_t enable) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_ADV_ENABLE;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_SET_ADV_ENABLE] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_SET_ADV_ENABLE, HCI_LENGTH_CMD_PARAM_LE_SET_ADV_ENABLE);
        buffer[4] = enable;
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_set_adv_enable error, send cmd not allowed");
    }
}

void hci_send_cmd_le_read_local_P256_public_key() {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_READ_LOCAL_P256_PUBLIC_KEY;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_READ_LOCAL_P256_PUBLIC_KEY] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_READ_LOCAL_P256_PUBLIC_KEY,
                                HCI_LENGTH_CMD_PARAM_LE_READ_LOCAL_P256_PUBLIC_KEY);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_read_local_P256_public_key error, send cmd not allowed");
    }
}

void hci_send_cmd_le_generate_dhkey(uint8_t *pub_key) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_GENERATE_DHKEY;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_GENERATE_DHKEY] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_GENERATE_DHKEY, HCI_LENGTH_CMD_PARAM_LE_GENERATE_DHKEY);
        memcpy_s(&buffer[4], HCI_LENGTH_P256_PUBLIC_KEY, pub_key, HCI_LENGTH_P256_PUBLIC_KEY);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_generate_dhkey error, send cmd not allowed");
    }
}

void hci_send_cmd_le_ltk_req_reply(uint16_t connect_handle, uint8_t *ltk) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_REPLY;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_REPLY] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_LTK_REQ_REPLY, HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_REPLY);
        buffer[4] = connect_handle;
        buffer[5] = (connect_handle >> 8) & 0x0f;
        memcpy_s(&buffer[6], HCI_LENGTH_LTK, ltk, HCI_LENGTH_LTK);
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_ltk_req_reply error, send cmd not allowed");
    }
}

void hci_send_cmd_le_ltk_req_neg_reply(uint16_t connect_handle) {
    uint32_t cmd_length = HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_NEG_REPLY;
    uint8_t buffer[HCI_LENGTH_CMD_HEADER + HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_NEG_REPLY] = { 0x00 };

    if (hci_send_cmd_allowed()) {
        __hci_assign_cmd_header(buffer, HCI_OGF_LE_CONTROLLER, HCI_OCF_LE_LTK_REQ_NEG_REPLY, HCI_LENGTH_CMD_PARAM_LE_LTK_REQ_NEG_REPLY);
        buffer[4] = connect_handle;
        buffer[5] = (connect_handle >> 8) & 0x0f;
        serial_write(buffer, cmd_length);
        btsnoop_wirte(buffer, cmd_length, BTSNOOP_PACKET_FLAG_CMD_SEND);
        hci_stack.num_of_allowed_cmd_packets--;
    } else {
        LOG_ERROR("hci_send_cmd_le_ltk_req_neg_reply error, send cmd not allowed");
    }
}

void hci_get_local_addr_info(uint8_t *addr, uint8_t *addr_type) {
    memcpy_s(addr, HCI_LENGTH_ADDR, hci_stack.local_addr, HCI_LENGTH_ADDR);
    *addr_type = hci_stack.local_addr_type;
}

uint16_t hci_get_le_acl_packet_length() {
    return hci_stack.le_acl_packet_length;
}

uint8_t* hci_get_local_p256_public_key() {
    return hci_stack.local_p256_public_key;
}

hci_connection_t* hci_add_connection(uint16_t connect_handle, uint8_t *peer_addr, uint8_t peer_addr_type) {
    hci_connection_t *conn = (hci_connection_t *)malloc(sizeof(hci_connection_t));
    memset(conn, 0, sizeof(hci_connection_t));
    conn->connect_handle = connect_handle;
    memcpy_s(conn->peer_addr, HCI_LENGTH_ADDR, peer_addr, HCI_LENGTH_ADDR);
    conn->peer_addr_type = peer_addr_type;
    conn->att_mtu = ATT_MTU_DEFAULT;

    conn->next = hci_stack.hci_connections;
    hci_stack.hci_connections = conn; // insert to list head

    return conn;
}

hci_connection_t* hci_find_connection_by_handle(uint16_t connect_handle) {
    hci_connection_t *temp = hci_stack.hci_connections;

    while (temp) {
        if (temp->connect_handle == connect_handle) {
            return temp;
        }
        temp = temp->next;
    }

    return nullptr;
}

hci_connection_t* hci_delete_connection_by_handle(uint16_t connect_handle) {
    hci_connection_t *temp_node = hci_stack.hci_connections;
    hci_connection_t *deleted_node = nullptr;

    if (temp_node->connect_handle == connect_handle) { // delete head node
        hci_stack.hci_connections = hci_stack.hci_connections->next;
        deleted_node = temp_node;
    } else {
        while (temp_node->next) {
            if (temp_node->next->connect_handle == connect_handle) {
                deleted_node = temp_node->next;
                temp_node->next = temp_node->next->next;
            }
            temp_node = temp_node->next;
        }
    }

    return deleted_node;
}
