#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "hci.h"
#include "l2cap.h"
#include "gatt.h"
#include "att.h"
#include "log.h"


#define GATT_MAX_COUNT_SERVICE                              10
#define GATT_MAX_SIZE_DEVICE_NAME                           32

#define GATT_LENGTH_PACKET_HEADER                           (HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_HEADER_ACL + L2CAP_LENGTH_HEADER)


static void __gatt_print_hex(uint8_t *data, uint32_t length);
static uint8_t __gatt_check_read_permission(uint16_t connect_handle, uint8_t att_permission);
static uint8_t __gatt_check_write_permission(uint16_t connect_handle, uint8_t att_permission);
static uint8_t __gatt_check_notify_permission(uint16_t connect_handle, uint16_t service_id, uint16_t characteristic_id);
static uint8_t __gatt_check_indicate_permission(uint16_t connect_handle, uint16_t service_id, uint16_t characteristic_id);


typedef struct {
    att_item_t *items;
    uint16_t items_cnt;
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t service_id;
} gatt_service_t;


/*
att_handle(2B)  att_type(UUID, 2B/16B)                          att_value(0-512B)                                           att_permission(1B)

// GAP service
0x0001          0x2800(GATT_DECLARATION_PRIMARY_SERVICE)        0x1800(GATT_SERVICE_GENERIC_ACCESS)                         0x01(GATT_PERMISSION_READ)
0x0002          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x02(GATT_CHARACTERISTIC_PROPERITY_READ)                    0x01(GATT_PERMISSION_READ)
                                                                0x0003(handle)
                                                                0x2a00(GATT_OBJECT_TYPE_DEVICE_NAME)
0x0003          0x2a00(GATT_OBJECT_TYPE_DEVICE_NAME)            [32B]"ble_demo"                                             0x01(GATT_PERMISSION_READ)

// GATT service
0x0011          0x2800(GATT_DECLARATION_PRIMARY_SERVICE)        0x1801(GATT_SERVICE_GATT)                                   0x01(GATT_PERMISSION_READ)
0x0012          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x20(GATT_CHARACTERISTIC_PROPERITY_INDICATE)                0x01(GATT_PERMISSION_READ)
                                                                0x0013(handle)
                                                                0x2a05(GATT_OBJECT_TYPE_SERVICE_CHANGED)
0x0013          0x2a05(GATT_OBJECT_TYPE_SERVICE_CHANGED)        [4B]0x000000000                                             0x01(GATT_PERMISSION_READ)
0x0014          0x2902(GATT_CLIENT_CHARACTER_CONFIG)            [2B]0x0000                                                  0x11(GATT_PERMISSION_READ|WRITE)

// BATTERY service
0x0101          0x2800(GATT_DECLARATION_PRIMARY_SERVICE)        0x180f(GATT_SERVICE_BATTERY)                                0x01(GATT_PERMISSION_READ)
0x0102          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x32(GATT_CHARACTERISTIC_PROPERITY_READ/NOTIFY/INDICATE)    0x01(GATT_PERMISSION_READ)
                                                                0x0103(handle)
                                                                0x2a19(GATT_OBJECT_TYPE_BATTERY_LEVEL)
0x0103          0x2a19(GATT_OBJECT_TYPE_BATTERY_LEVEL)          [1B]0x62(98%)                                               0x04(GATT_PERMISSION_READ_AUTHENTICATE)
0x0104          0x2902(GATT_CLIENT_CHARACTER_CONFIG)            [2B]0x0000                                                  0x41(GATT_PERMISSION_READ|WRITE_AUTHENTICATE)

// TEST service
0x1001          0x2800(GATT_DECLARATION_PRIMARY_SERVICE)        0x18ff(GATT_SERVICE_TEST)                                   0x01(GATT_PERMISSION_READ)
0x1002          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x04(GATT_CHARACTERISTIC_PROPERITY_WRITE_NORESP)            0x01(GATT_PERMISSION_READ)
                                                                0x1003(handle)
                                                                0x2afe(GATT_OBJECT_TYPE_TEST_RX)
0x1003          0x2afe(GATT_OBJECT_TYPE_TEST_RX)                nullptr                                                     0x10(GATT_PERMISSION_WRITE)
0x1011          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x10(GATT_CHARACTERISTIC_PROPERITY_NOTIFY)                  0x01(GATT_PERMISSION_READ)
                                                                0x1012(handle)
                                                                0x2aff(GATT_OBJECT_TYPE_TEST_TX)
0x1012          0x2aff(GATT_OBJECT_TYPE_TEST_TX)                nullptr                                                     0x01(GATT_PERMISSION_READ)
0x1013          0x2902(GATT_CLIENT_CHARACTER_CONFIG)            [2B]0x0000                                                  0x11(GATT_PERMISSION_READ|WRITE)
*/

static uint8_t gacc_uuid[] = {(uint8_t)GATT_SERVICE_GENERIC_ACCESS, GATT_SERVICE_GENERIC_ACCESS >> 8};
static uint8_t gacc_characteristic[] = {GATT_CHARACTERISTIC_PROPERITY_READ,
                                        0x03, 0x00,
                                        (uint8_t)GATT_OBJECT_TYPE_DEVICE_NAME, GATT_OBJECT_TYPE_DEVICE_NAME >> 8};
static uint8_t gacc_device_name[GATT_MAX_SIZE_DEVICE_NAME] = {'b', 'l', 'e', '_', 'd', 'e', 'm', 'o'};


static uint8_t gatt_uuid[] = {(uint8_t)GATT_SERVICE_GENERIC_ATTRIBUTE, GATT_SERVICE_GENERIC_ATTRIBUTE >> 8};
static uint8_t gatt_characteristic[] = {GATT_CHARACTERISTIC_PROPERITY_INDICATE,
                                        0x13, 0x00,
                                        (uint8_t)GATT_OBJECT_TYPE_SERVICE_CHANGED, GATT_OBJECT_TYPE_SERVICE_CHANGED >> 8};
static uint8_t gatt_service_changed[] = {0x00, 0x00, 0x00, 0x00};
static uint8_t gatt_ccc[] = {0x00, 0x00};


static uint8_t battery_uuid[] = {(uint8_t)GATT_SERVICE_BATTERY, GATT_SERVICE_BATTERY >> 8};
static uint8_t battery_characteristic[] = {GATT_CHARACTERISTIC_PROPERITY_READ | GATT_CHARACTERISTIC_PROPERITY_NOTIFY | GATT_CHARACTERISTIC_PROPERITY_INDICATE,
                                           0x03, 0x01,
                                           (uint8_t)GATT_OBJECT_TYPE_BATTERY_LEVEL, GATT_OBJECT_TYPE_BATTERY_LEVEL >> 8};
static uint8_t battery_level[] = {0x62}; // 98%
static uint8_t battery_ccc[] = {0x00, 0x00};


static uint8_t test_uuid[] = {(uint8_t)GATT_SERVICE_TEST, GATT_SERVICE_TEST >> 8};
static uint8_t test_characteristic_rx[] = {GATT_CHARACTERISTIC_PROPERITY_WRITE_NORESP,
                                           0x03, 0x10,
                                           (uint8_t)GATT_OBJECT_TYPE_TEST_RX, GATT_OBJECT_TYPE_TEST_RX >> 8};
static uint8_t test_characteristic_tx[] = {GATT_CHARACTERISTIC_PROPERITY_NOTIFY,
                                           0x12, 0x10,
                                           (uint8_t)GATT_OBJECT_TYPE_TEST_TX, GATT_OBJECT_TYPE_TEST_TX >> 8};
static uint8_t test_tx_ccc[] = {0x00, 0x00};


static att_item_t items_gacc[] = {
    {0x0001, GATT_DECLARATION_PRIMARY_SERVICE, gacc_uuid, sizeof(gacc_uuid), GATT_PERMISSION_READ},
    {0x0002, GATT_DECLARATION_CHARACTERISTIC, gacc_characteristic, sizeof(gacc_characteristic), GATT_PERMISSION_READ},
    {0x0003, GATT_OBJECT_TYPE_DEVICE_NAME, gacc_device_name, sizeof(gacc_device_name), GATT_PERMISSION_READ}
};

static att_item_t items_gatt[] = {
    {0x0011, GATT_DECLARATION_PRIMARY_SERVICE, gatt_uuid, sizeof(gatt_uuid), GATT_PERMISSION_READ},
    {0x0012, GATT_DECLARATION_CHARACTERISTIC, gatt_characteristic, sizeof(gatt_characteristic), GATT_PERMISSION_READ},
    {0x0013, GATT_OBJECT_TYPE_SERVICE_CHANGED, gatt_service_changed, sizeof(gatt_service_changed), GATT_PERMISSION_READ},
    {0x0014, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, gatt_ccc, sizeof(gatt_ccc), GATT_PERMISSION_READ | GATT_PERMISSION_WRITE}
};

static att_item_t items_battery[] = {
    {0x0101, GATT_DECLARATION_PRIMARY_SERVICE, battery_uuid, sizeof(battery_uuid), GATT_PERMISSION_READ},
    {0x0102, GATT_DECLARATION_CHARACTERISTIC, battery_characteristic, sizeof(battery_characteristic), GATT_PERMISSION_READ},
    {0x0103, GATT_OBJECT_TYPE_BATTERY_LEVEL, battery_level, sizeof(battery_level), GATT_PERMISSION_READ_AUTHENTICATE},
    {0x0104, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, battery_ccc, sizeof(battery_ccc), GATT_PERMISSION_READ | GATT_PERMISSION_WRITE_AUTHENTICATE}
};

static att_item_t items_test[] = {
    {0x1001, GATT_DECLARATION_PRIMARY_SERVICE, test_uuid, sizeof(test_uuid), GATT_PERMISSION_READ},
    {0x1002, GATT_DECLARATION_CHARACTERISTIC, test_characteristic_rx, sizeof(test_characteristic_rx), GATT_PERMISSION_READ},
    {0x1003, GATT_OBJECT_TYPE_TEST_RX, nullptr, 0, GATT_PERMISSION_WRITE},
    {0x1011, GATT_DECLARATION_CHARACTERISTIC, test_characteristic_tx, sizeof(test_characteristic_tx), GATT_PERMISSION_READ},
    {0x1012, GATT_OBJECT_TYPE_TEST_TX, nullptr, 0, GATT_PERMISSION_READ},
    {0x1013, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, test_tx_ccc, sizeof(test_tx_ccc), GATT_PERMISSION_READ | GATT_PERMISSION_WRITE}
};

static gatt_service_t services[GATT_MAX_COUNT_SERVICE];
static uint8_t service_count = 0;


void gatt_init() {
    gatt_add_service(items_gacc, 3, 0x0001, 0x0003, GATT_SERVICE_GENERIC_ACCESS);
    gatt_add_service(items_gatt, 4, 0x0011, 0x0014, GATT_SERVICE_GENERIC_ATTRIBUTE);
    gatt_add_service(items_battery, 4, 0x0101, 0x0104, GATT_SERVICE_BATTERY);
    gatt_add_service(items_test, 6, 0x1001, 0x1013, GATT_SERVICE_TEST);
}

void gatt_add_service(att_item_t *items, uint16_t items_cnt, uint16_t start_handle, uint16_t end_handle, uint16_t service_id) {
    if (service_count < GATT_MAX_COUNT_SERVICE) {
        services[service_count].items = items;
        services[service_count].items_cnt = items_cnt;
        services[service_count].start_handle = start_handle;
        services[service_count].end_handle = end_handle;
        services[service_count].service_id = service_id;
        service_count++;
    }
}

static void __gatt_print_hex(uint8_t *data, uint32_t length) {
    uint8_t buffer[128] = {0};
    uint32_t i = 0, j = 0;

    for (i = 0; i < length; i++) {
        sprintf_s((char*)(buffer + j*3), sizeof(buffer) - j*3, "%02x ", data[i]);
        j++;
        if ((i % 32) == 31) {
            LOG_INFO("%s", buffer);
            memset(buffer, 0, sizeof(buffer));
            j = 0;
        }
    }

    if (j != 0) {
        LOG_INFO("%s", buffer);
    }
}

static uint8_t __gatt_check_read_permission(uint16_t connect_handle, uint8_t att_permission) {
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    if (att_permission & GATT_PERMISSION_READ_AUTHORIZATE) {
        if (!sm_connection.is_authorizated) {
            LOG_ERROR("check read permission failed, no authorization, connect_handle:0x%04x", connect_handle);
            return ATT_ERROR_INSUFFICIENT_AUTHORIZATION;
        }
    }

    if (att_permission & GATT_PERMISSION_READ_AUTHENTICATE) {
        if (!sm_connection.is_authenticated) {
            LOG_ERROR("check read permission failed, no authentication, connect_handle:0x%04x", connect_handle);
            return ATT_ERROR_INSUFFICIENT_AUTHENTICATION;
        }
    }

    if (att_permission & GATT_PERMISSION_READ_ENCRYPT) {
        if (!sm_connection.is_encrypted) {
            LOG_ERROR("check read permission failed, no encryption, connect_handle:0x%04x", connect_handle);
            return ATT_ERROR_INSUFFICIENT_ENCRYPTION;
        }
    }

    return 0;
}

static uint8_t __gatt_check_write_permission(uint16_t connect_handle, uint8_t att_permission) {
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    if (att_permission & GATT_PERMISSION_WRITE_AUTHORIZATE) {
        if (!sm_connection.is_authorizated) {
            LOG_ERROR("check write permission failed, no authorization, connect_handle:0x%04x", connect_handle);
            return ATT_ERROR_INSUFFICIENT_AUTHORIZATION;
        }
    }

    if (att_permission & GATT_PERMISSION_WRITE_AUTHENTICATE) {
        if (!sm_connection.is_authenticated) {
            LOG_ERROR("check write permission failed, no authentication, connect_handle:0x%04x", connect_handle);
            return ATT_ERROR_INSUFFICIENT_AUTHENTICATION;
        }
    }

    if (att_permission & GATT_PERMISSION_WRITE_ENCRYPT) {
        if (!sm_connection.is_encrypted) {
            LOG_ERROR("check write permission failed, no encryption, connect_handle:0x%04x", connect_handle);
            return ATT_ERROR_INSUFFICIENT_ENCRYPTION;
        }
    }

    return 0;
}

static uint8_t __gatt_check_notify_permission(uint16_t connect_handle, uint16_t service_id, uint16_t characteristic_id) {
    (void)connect_handle; // TODO: every connection has own ccc
    att_item_t *item = nullptr;
    uint8_t index_service = 0;
    uint16_t index_item = 0;
    uint8_t found_characteristic = 0;

    for (index_service = 0; index_service < service_count; index_service++) {
        if (services[index_service].service_id == service_id) {
            for (index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
                item = &(services[index_service].items[index_item]);
                if (item->type == characteristic_id) {
                    found_characteristic = 1;
                }

                // the next ccc descriptor behind characteristic clarification
                if ((found_characteristic) && (GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG == item->type)) {
                    if (item->value[0] & 0x01) { // bit0
                        return 0;
                    } else {
                        return ATT_ERROR_REQUEST_NOT_SUPPORTED;
                    }
                }
            }
        }
    }

    return ATT_ERROR_REQUEST_NOT_SUPPORTED;
}

static uint8_t __gatt_check_indicate_permission(uint16_t connect_handle, uint16_t service_id, uint16_t characteristic_id) {
    (void)connect_handle; // TODO: every connection has own ccc
    att_item_t *item = nullptr;
    uint8_t index_service = 0;
    uint16_t index_item = 0;
    uint8_t found_characteristic = 0;

    for (index_service = 0; index_service < service_count; index_service++) {
        if (services[index_service].service_id == service_id) {
            for (index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
                item = &(services[index_service].items[index_item]);
                if (item->type == characteristic_id) {
                    found_characteristic = 1;
                }

                // the next ccc descriptor behind characteristic clarification
                if ((found_characteristic) && (GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG == item->type)) {
                    if (item->value[0] & 0x02) { // bit1
                        return 0;
                    } else {
                        return ATT_ERROR_REQUEST_NOT_SUPPORTED;
                    }
                }
            }
        }
    }

    return ATT_ERROR_REQUEST_NOT_SUPPORTED;
}

// read part value for one characteristic
void gatt_recv_read_blob_req(uint16_t connect_handle, uint16_t att_handle, uint16_t value_offset) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t copy_length = 0;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint8_t error_code = 0;
    uint8_t found = 0;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle) {
                continue;
            }

            if (item->handle > att_handle) {
                goto RET;
            }

            error_code = __gatt_check_read_permission(connect_handle, item->permission);
            if (!error_code) {
                if (item->value_length < value_offset) {
                    error_code = ATT_ERROR_INVALID_OFFSET;
                    goto RET;
                }

                if (item->value_length <= att_mtu - 1) {
                    error_code = ATT_ERROR_ATTRIBUTE_NOT_LONG;
                    goto RET;
                }

                found = 1;
                buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
                buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_READ_BLOB_RESP;
                copy_length = item->value_length - value_offset;
                if (copy_length > att_mtu - 1) {
                    copy_length = att_mtu - 1;
                }
                memcpy_s(&buffer[GATT_LENGTH_PACKET_HEADER + 1], copy_length, item->value + value_offset, copy_length);
            }
            goto RET;
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_INVALID_HANDLE;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_READ_BLOB_REQ, att_handle, error_code);
    } else {
        att_send(connect_handle, buffer, copy_length + 1);
    }

    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// read first att_mtu-1 value for one characteristic
void gatt_recv_read_req(uint16_t connect_handle, uint16_t att_handle) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t copy_length = 0;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint8_t error_code = 0;
    uint8_t found = 0;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle) {
                continue;
            }

            if (item->handle > att_handle) {
                error_code = ATT_ERROR_INVALID_HANDLE;
                goto RET;
            }

            error_code = __gatt_check_read_permission(connect_handle, item->permission);
            if (!error_code) {
                found = 1;
                buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
                buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_READ_RESP;
                copy_length = item->value_length;
                if (copy_length > att_mtu - 1) {
                    copy_length = att_mtu - 1;
                }
                memcpy_s(&buffer[GATT_LENGTH_PACKET_HEADER + 1], copy_length, item->value, copy_length);
            }
            goto RET;
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_INVALID_HANDLE;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_READ_REQ, att_handle, error_code);
    } else {
        att_send(connect_handle, buffer, copy_length + 1);
    }

    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// find descriptors for one characteristic
void gatt_recv_find_information_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint32_t offset = GATT_LENGTH_PACKET_HEADER;
    uint8_t pair_value_length = 4;
    uint8_t error_code = 0;
    uint8_t found = 0;

    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
    buffer[offset] = ATT_OPERATE_FIND_INFO_RESP;
    offset++;
    buffer[offset] = ATT_UUID_TYPE_BITS_16;
    offset++;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle_start) {
                continue;
            }

            if (item->handle > att_handle_end) {
                goto RET;
            }

            found = 1;
            buffer[offset] = (uint8_t)item->handle;
            offset++;
            buffer[offset] = item->handle >> 8;
            offset++;
            buffer[offset] = (uint8_t)item->type;
            offset++;
            buffer[offset] = item->type >> 8;
            offset++;
            if ((offset - GATT_LENGTH_PACKET_HEADER + pair_value_length) > att_mtu) {
                goto RET;
            }
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_FIND_INFO_REQ, att_handle_start, error_code);
    } else {
        att_send(connect_handle, buffer, offset - GATT_LENGTH_PACKET_HEADER);
    }

    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// find att_handle, group_end_handle for one att_value
void gatt_recv_find_by_type_value_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end,
                                      uint16_t att_type, uint8_t *att_value, uint32_t att_value_length) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint32_t offset = GATT_LENGTH_PACKET_HEADER;
    uint8_t pair_value_length = 4;
    uint8_t error_code = 0;
    uint8_t found = 0;

    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
    buffer[offset] = ATT_OPERATE_FIND_BY_TYPE_VALUE_RESP;
    offset++;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle_start) {
                continue;
            }

            if (item->handle > att_handle_end) {
                goto RET;
            }

            if ((item->type == att_type) && (item->value_length == att_value_length) && (!memcmp(item->value, att_value, att_value_length))) {
                found = 1;
                buffer[offset] = (uint8_t)item->handle;
                offset++;
                buffer[offset] = item->handle >> 8;
                offset++;
                buffer[offset] = (uint8_t)services[index_service].end_handle;
                offset++;
                buffer[offset] = services[index_service].end_handle >> 8;
                offset++;
                if ((offset - GATT_LENGTH_PACKET_HEADER + pair_value_length) > att_mtu) {
                    goto RET;
                }
            }
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_FIND_BY_TYPE_VALUE_REQ, att_handle_start, error_code);
    } else {
        att_send(connect_handle, buffer, offset - GATT_LENGTH_PACKET_HEADER);
    }

    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// read include,characteristics for one service
void gatt_recv_read_by_type_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end, uint16_t att_type) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint32_t offset = GATT_LENGTH_PACKET_HEADER;
    uint8_t pair_value_length = 0;
    uint8_t error_code = 0;
    uint8_t found = 0;

    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
    buffer[offset] = ATT_OPERATE_READ_BY_TYPE_RESP;
    offset++;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle_start) {
                continue;
            }

            if (item->handle > att_handle_end) {
                goto RET;
            }

            if (item->type == att_type) {
                if (found == 0) { // the first item matched
                    pair_value_length = (uint8_t)(item->value_length);
                    buffer[offset] = 2 + pair_value_length;
                    offset++;
                } else { // subsequent items not matched
                    if (pair_value_length != item->value_length) {
                        goto RET;
                    }
                }

                found = 1;
                buffer[offset] = (uint8_t)item->handle;
                offset++;
                buffer[offset] = item->handle >> 8;
                offset++;
                memcpy_s(&buffer[offset], pair_value_length, item->value, pair_value_length);
                offset += pair_value_length;
                if ((offset - GATT_LENGTH_PACKET_HEADER + 2 + pair_value_length) > att_mtu) {
                    goto RET;
                }
            }
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_READ_BY_TYPE_REQ, att_handle_start, error_code);
    } else {
        att_send(connect_handle, buffer, offset - GATT_LENGTH_PACKET_HEADER);
    }

    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// read all primary service_id, start_handle, end_handle
void gatt_recv_read_by_group_type_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end, uint16_t group_type) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint32_t offset = GATT_LENGTH_PACKET_HEADER;
    uint8_t pair_value_length = 6; // start_handle, end_handle, uuid
    uint8_t error_code = 0;
    uint8_t found = 0;

    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
    buffer[offset] = ATT_OPERATE_READ_BY_GROUP_TYPE_RESP;
    offset++;
    buffer[offset] = pair_value_length;
    offset++;

    for (uint8_t index = 0; index < service_count; index++) {
        if ((att_handle_start <= services[index].start_handle) && (services[index].end_handle <= att_handle_end)) {
            item = &services[index].items[0];
            if (item->type == group_type) {

                found = 1;
                buffer[offset] = (uint8_t)services[index].start_handle;
                offset++;
                buffer[offset] = services[index].start_handle >> 8;
                offset++;
                buffer[offset] = (uint8_t)services[index].end_handle;
                offset++;
                buffer[offset] = services[index].end_handle >> 8;
                offset++;
                memcpy_s(&buffer[offset], item->value_length, item->value, item->value_length);
                offset += item->value_length;
                if ((offset - GATT_LENGTH_PACKET_HEADER + pair_value_length) > att_mtu) {
                    goto RET;
                }
            }
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_READ_BY_GROUP_TYPE_REQ, att_handle_start, error_code);
    } else {
        att_send(connect_handle, buffer, offset - GATT_LENGTH_PACKET_HEADER);
    }

    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// write for one characteristics with response
void gatt_recv_write_req(uint16_t connect_handle, uint16_t att_handle, uint8_t *value, uint32_t value_length) {
    att_item_t *item = nullptr;
    uint8_t buffer[GATT_LENGTH_PACKET_HEADER + ATT_LENGTH_HEADER] = {0};
    uint8_t error_code = 0;
    uint8_t found = 0;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle) {
                continue;
            }

            if (item->handle > att_handle) {
                goto RET;
            }

            error_code = __gatt_check_write_permission(connect_handle, item->permission);
            if (!error_code) {
                if (item->value) { // has memory to save att value
                    if (value_length > item->value_length) {
                        LOG_ERROR("gatt_recv_write_req error, value too long, att_length:%u, value_length:%u", item->value_length, value_length);
                        error_code = ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LENGTH;
                        goto RET;
                    } else {
                        found = 1;
                        buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_WRITE_RESP;
                        memcpy_s(item->value, value_length, value, value_length);
                    }
                }
                LOG_INFO("gatt_recv_write_req value_length:%u", value_length);
                __gatt_print_hex(value, value_length);
            }
            goto RET;
        }
    }

RET:
    if (!found) {
        if (!error_code) {
            error_code = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        }
        gatt_send_error_resp(connect_handle, ATT_OPERATE_WRITE_REQ, att_handle, error_code);
    } else {
        att_send(connect_handle, buffer, 1);
    }
}

// write for one characteristics without response
void gatt_recv_write_cmd(uint16_t connect_handle, uint16_t att_handle, uint8_t *value, uint32_t value_length) {
    att_item_t *item = nullptr;
    uint8_t error_code = 0;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < att_handle) {
                continue;
            }

            if (item->handle > att_handle) {
                return;
            }

            error_code = __gatt_check_write_permission(connect_handle, item->permission);
            if (!error_code) {
                if (item->value) { // has memory to save att value
                    if (value_length > item->value_length) {
                        LOG_ERROR("gatt_recv_write_cmd error, value too long, att_length:%u, value_length:%u", item->value_length, value_length);
                        return;
                    } else {
                        memcpy_s(item->value, value_length, value, value_length);
                    }
                }

                LOG_INFO("gatt_recv_write_cmd value_length:%u", value_length);
                __gatt_print_hex(value, value_length);
            }
            return;
        }
    }
}

void gatt_recv_handle_value_cfm(uint16_t connect_handle) {
    (void)connect_handle;
    // TODO: set flag to enable send next indication
}

void gatt_send_notify(uint16_t connect_handle, uint16_t service_id, uint16_t characteristic_id) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t copy_length = 0;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint8_t index_service = 0;
    uint16_t index_item = 0;

    if (__gatt_check_notify_permission(connect_handle, service_id, characteristic_id)) {
        LOG_ERROR("gatt_send_notify no permission, connect_handle:0x%04x, service_id:0x%04x, characteristic_id:0x%04x", connect_handle, service_id, characteristic_id);
        return;
    }

    for (index_service = 0; index_service < service_count; index_service++) {
        if (services[index_service].service_id == service_id) {
            for (index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
                item = &(services[index_service].items[index_item]);
                if (item->type == characteristic_id) {
                    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
                    buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_HANDLE_VALUE_NTF;
                    buffer[GATT_LENGTH_PACKET_HEADER + 1] = (uint8_t)item->handle;
                    buffer[GATT_LENGTH_PACKET_HEADER + 2] = item->handle >> 8;
                    if (item->value) {
                        copy_length = item->value_length;
                        if (copy_length > att_mtu - 3) {
                            copy_length = att_mtu - 3;
                        }
                        memcpy_s(&buffer[GATT_LENGTH_PACKET_HEADER + 3], copy_length, item->value, copy_length);
                    }
                    att_send(connect_handle, buffer, copy_length + 3);
                    free(buffer);
                    return;
                }
            }
        }
    }
}

void gatt_send_indicate(uint16_t connect_handle, uint16_t service_id, uint16_t characteristic_id) {
    // TODO: check previous cfm is received
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t copy_length = 0;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint8_t index_service = 0;
    uint16_t index_item = 0;

    if (__gatt_check_indicate_permission(connect_handle, service_id, characteristic_id)) {
        LOG_ERROR("gatt_send_indicate no permission, connect_handle:0x%04x, service_id:0x%04x, characteristic_id:0x%04x", connect_handle, service_id, characteristic_id);
        return;
    }

    for (index_service = 0; index_service < service_count; index_service++) {
        if (services[index_service].service_id == service_id) {
            for (index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
                item = &(services[index_service].items[index_item]);
                if (item->type == characteristic_id) {
                    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
                    buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_HANDLE_VALUE_IND;
                    buffer[GATT_LENGTH_PACKET_HEADER + 1] = (uint8_t)item->handle;
                    buffer[GATT_LENGTH_PACKET_HEADER + 2] = item->handle >> 8;
                    if (item->value) {
                        copy_length = item->value_length;
                        if (copy_length > att_mtu - 3) {
                            copy_length = att_mtu - 3;
                        }
                        memcpy_s(&buffer[GATT_LENGTH_PACKET_HEADER + 3], copy_length, item->value, copy_length);
                    }
                    att_send(connect_handle, buffer, copy_length + 3);
                    free(buffer);
                    return;
                }
            }
        }
    }
}

void gatt_send_passthrough(uint16_t connect_handle, uint8_t *value, uint32_t value_length) {
    att_item_t *item = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t copy_length = 0;
    uint32_t sent_length = 0;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint8_t index_service = 0;
    uint16_t index_item = 0;

    for (index_service = 0; index_service < service_count; index_service++) {
        if (services[index_service].service_id == GATT_SERVICE_TEST) {
            for (index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
                item = &(services[index_service].items[index_item]);
                if (item->type == GATT_OBJECT_TYPE_TEST_TX) {
                    buffer = (uint8_t *)malloc(GATT_LENGTH_PACKET_HEADER + att_mtu);
                    buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_HANDLE_VALUE_NTF;
                    buffer[GATT_LENGTH_PACKET_HEADER + 1] = (uint8_t)item->handle;
                    buffer[GATT_LENGTH_PACKET_HEADER + 2] = item->handle >> 8;
                    while (value_length > 0) {
                        copy_length = value_length;
                        if (copy_length > att_mtu - 3) {
                            copy_length = att_mtu - 3;
                        }
                        memcpy_s(&buffer[GATT_LENGTH_PACKET_HEADER + 3], copy_length, value + sent_length, copy_length);
                        att_send(connect_handle, buffer, copy_length + 3);

                        sent_length += copy_length;
                        value_length -= copy_length;
                    }
                    free(buffer);
                    return;
                }
            }
        }
    }
}

void gatt_send_error_resp(uint16_t connect_handle, uint8_t op_code, uint16_t handle, uint8_t error_code) {
    uint8_t buffer[GATT_LENGTH_PACKET_HEADER + ATT_LENGTH_HEADER + ATT_LENGTH_ERROR_RESP] = {0};

    buffer[GATT_LENGTH_PACKET_HEADER] = ATT_OPERATE_ERROR_RESP;
    buffer[GATT_LENGTH_PACKET_HEADER + 1] = op_code;
    buffer[GATT_LENGTH_PACKET_HEADER + 2] = (uint8_t)handle;
    buffer[GATT_LENGTH_PACKET_HEADER + 3] = handle >> 8;
    buffer[GATT_LENGTH_PACKET_HEADER + 4] = error_code;
    att_send(connect_handle, buffer, ATT_LENGTH_HEADER + ATT_LENGTH_ERROR_RESP);
}
