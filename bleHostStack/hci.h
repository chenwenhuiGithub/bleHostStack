#ifndef HCI_H
#define HCI_H

#include <stdint.h>

#define HCI_PACKET_TYPE_CMD                                             1
#define HCI_PACKET_TYPE_ACL                                             2
#define HCI_PACKET_TYPE_SCO                                             3
#define HCI_PACKET_TYPE_EVT                                             4

#define HCI_ADV_CHANNEL_MAP_37                                          0x01
#define HCI_ADV_CHANNEL_MAP_38                                          0x02
#define HCI_ADV_CHANNEL_MAP_39                                          0x04

#define HCI_LENGTH_PACKET_TYPE                                          1
#define HCI_LENGTH_EVT_HEADER                                           2
#define HCI_LENGTH_SCO_HEADER                                           3
#define HCI_LENGTH_ACL_HEADER                                           4

#define HCI_LENGTH_ADDR                                                 6
#define HCI_LENGTH_ADV_DATA                                             31
#define HCI_LENGTH_P256_PUBLIC_KEY_COORDINATE                           32
#define HCI_LENGTH_LTK                                                  16
#define HCI_LENGTH_EVENT_MASK                                           8
#define HCI_LENGTH_LE_EVENT_MASK                                        8
#define HCI_LENGTH_CLASS_OF_DEVICE                                      3


typedef enum {
    HCI_LE_HOST_SUPPORT_DISABLE,
    HCI_LE_HOST_SUPPORT_ENABLE
} hci_le_host_support_t;

typedef struct {
    uint16_t connect_handle;
    uint16_t interval_min;
    uint16_t interval_max;
    uint16_t max_latency;
    uint16_t timeout;
    uint16_t min_ce_length;
    uint16_t max_ce_length;
} hci_le_remote_conn_param_req_reply_t;

typedef struct {
    uint16_t connect_handle;
    uint8_t reason;
} hci_le_remote_conn_param_req_neg_reply_t;

typedef enum {
    HCI_ADV_TYPE_ADV_IND,
    HCI_ADV_TYPE_ADV_DIRECT_IND_HIGH_DUTY,
    HCI_ADV_TYPE_ADV_SCAN_IND,
    HCI_ADV_TYPE_ADV_NONCONN_IND,
    HCI_ADV_TYPE_ADV_DIRECT_IND_LOW_DUTY
} hci_adv_type_t;

typedef enum {
    HCI_ADDR_TYPE_PUBLIC_DEVICE,
    HCI_ADDR_TYPE_RANDOM_DEVICE,
    HCI_ADDR_TYPE_PUBLIC_IDENTITY,
    HCI_ADDR_TYPE_RANDOM_IDENTITY
} hci_addr_type_t;

typedef enum {
    HCI_ADV_FILTER_POLICY_SCAN_ALL_CONN_ALL,
    HCI_ADV_FILTER_POLICY_SCAN_FILTER_CONN_ALL,
    HCI_ADV_FILTER_POLICY_SCAN_ALL_CONN_FILETER,
    HCI_ADV_FILTER_POLICY_SCAN_FILTER_CONN_FILETER
} hci_adv_filter_policy_t;

typedef struct {
    uint16_t adv_interval_min;
    uint16_t adv_interval_max;
    hci_adv_type_t adv_type;
    hci_addr_type_t own_addr_type;
    hci_addr_type_t peer_addr_type;
    uint8_t peer_addr[HCI_LENGTH_ADDR];
    uint8_t adv_channel_map;
    hci_adv_filter_policy_t adv_filter_policy;
} hci_le_adv_param_t;

typedef struct {
    uint8_t adv_data_length;
    uint8_t adv_data[HCI_LENGTH_ADV_DATA];
} hci_le_adv_data_t;

typedef enum {
    HCI_LE_ADV_DISABLE,
    HCI_LE_ADV_ENABLE
} hci_le_adv_enable_t;

typedef struct {
    uint8_t key_x[HCI_LENGTH_P256_PUBLIC_KEY_COORDINATE];
    uint8_t key_y[HCI_LENGTH_P256_PUBLIC_KEY_COORDINATE];
} hci_le_generate_dhkey_t;

typedef struct {
    uint16_t connect_handle;
    uint8_t ltk[HCI_LENGTH_LTK];
} hci_le_ltk_req_reply_t;

void hci_recv_evt(uint8_t *data, uint32_t length);
void hci_recv_acl(uint8_t *data, uint32_t length);
void hci_send_acl(uint8_t *data, uint32_t length);
void hci_send_cmd_reset();
void hci_send_cmd_read_local_version_info();
void hci_send_cmd_read_local_supported_commands();
void hci_send_cmd_le_read_buffer_size();
void hci_send_cmd_read_bd_addr();
void hci_send_cmd_set_event_mask(uint8_t *event_mask);
void hci_send_cmd_write_le_host_support(hci_le_host_support_t enable);
void hci_send_cmd_write_class_of_device(uint8_t *class_of_device);
void hci_send_cmd_le_remote_conn_param_req_reply(hci_le_remote_conn_param_req_reply_t *param);
void hci_send_cmd_le_remote_conn_param_req_neg_reply(hci_le_remote_conn_param_req_neg_reply_t *param);
void hci_send_cmd_le_set_event_mask(uint8_t *le_event_mask);
void hci_send_cmd_le_set_adv_param(hci_le_adv_param_t *param);
void hci_send_cmd_le_set_adv_data(hci_le_adv_data_t *param);
void hci_send_cmd_le_set_adv_enable(hci_le_adv_enable_t enable);
void hci_send_cmd_le_read_local_P256_public_key();
void hci_send_cmd_le_generate_dhkey(hci_le_generate_dhkey_t *param);
void hci_send_cmd_le_ltk_req_reply(hci_le_ltk_req_reply_t *param);

#endif // HCI_H
