#include "gatt.h"
#include "att.h"
#include <malloc.h>
#include <string.h>

#define GATT_SERVICE_GENERIC_ACCESS                         0x1800
#define GATT_SERVICE_GENERIC_ATTRIBUTE                      0x1801
#define GATT_SERVICE_IMMEDIATE_ALERT                        0x1802
#define GATT_SERVICE_LINK_LOSS                              0x1803
#define GATT_SERVICE_TX_POWER                               0x1804
#define GATT_SERVICE_CURRENT_TIME                           0x1805
#define GATT_SERVICE_REFERENCE_TIME_UPDATE                  0x1806
#define GATT_SERVICE_NEXT_DST_CHANGE                        0x1807
#define GATT_SERVICE_GLUCOSE                                0x1808
#define GATT_SERVICE_HEALTH_THERMOMETER                     0x1809
#define GATT_SERVICE_DEVICE_INFORMATION                     0x180a
#define GATT_SERVICE_HEART_RATE                             0x180d
#define GATT_SERVICE_PHONE_ALERT_STATUS                     0x180e
#define GATT_SERVICE_BATTERY                                0x180f
#define GATT_SERVICE_BLOOD_PRESSURE                         0x1810
#define GATT_SERVICE_ALERT_NOTIFICATION                     0x1811
#define GATT_SERVICE_HUMAN_INTERFACE_DEVICE                 0x1812
#define GATT_SERVICE_TEST                                   0x18ff


#define GATT_DECLARATION_PRIMARY_SERVICE                    0x2800
#define GATT_DECLARATION_SECONDARY_SERVICE                  0x2801
#define GATT_DECLARATION_INCLUDE                            0x2802
#define GATT_DECLARATION_CHARACTERISTIC                     0x2803


#define GATT_DESCRIPTOR_CHARACTERISTIC_EXTENDED_PROPERTY    0x2900
#define GATT_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION     0x2901
#define GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG        0x2902
#define GATT_DESCRIPTOR_SERVER_CHARACTERISTIC_CONFIG        0x2903
#define GATT_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT  0x2904
#define GATT_DESCRIPTOR_CHARACTERISTIC_AGGREGATE_FORMAT     0x2905


#define GATT_OBJECT_TYPE_DEVICE_NAME                        0x2a00
#define GATT_OBJECT_TYPE_APPEARANCE                         0x2a01
#define GATT_OBJECT_TYPE_SERVICE_CHANGED                    0x2a05
#define GATT_OBJECT_TYPE_BATTERY_LEVEL                      0x2a19
#define GATT_OBJECT_TYPE_TEST                               0x2aff


#define GATT_PERMISSION_READ                                0x01
#define GATT_PERMISSION_READ_ENCRYPTED                      0x02
#define GATT_PERMISSION_READ_ENCRYPTED_MITM                 0x04
#define GATT_PERMISSION_WRITE                               0x08
#define GATT_PERMISSION_WRITE_ENCRYPTED                     0x10
#define GATT_PERMISSION_WRITE_ENCRYPTED_MITM                0x20
#define GATT_PERMISSION_WRITE_SIGNED                        0x40
#define GATT_PERMISSION_WRITE_SIGNED_MITM                   0x80


#define GATT_CHARACTERISTIC_PROPERITY_BROADCAST             0x01
#define GATT_CHARACTERISTIC_PROPERITY_READ                  0x02
#define GATT_CHARACTERISTIC_PROPERITY_WRITE_NORESP          0x04
#define GATT_CHARACTERISTIC_PROPERITY_WRITE                 0x08
#define GATT_CHARACTERISTIC_PROPERITY_NOTIFY                0x10
#define GATT_CHARACTERISTIC_PROPERITY_INDICATE              0x20
#define GATT_CHARACTERISTIC_PROPERITY_AUTHENTICATE          0x40
#define GATT_CHARACTERISTIC_PROPERITY_EXTENDED_PROPERTY     0x80


#define GATT_SERVICE_MAX_COUNT                              10


typedef struct {
    ATT_ITEM *items;
    uint16_t items_cnt;
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t service_id;
} GATT_SERVICE;


/*
att_handle(2B)	att_type(UUID, 2B/16B)							att_value(0-512B)                               att_permission(1B)

// GAP service
0x0001			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x1800(GATT_SERVICE_GENERIC_ACCESS)             0x01(GATT_PERMISSION_READ)
0x0002			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x02(GATT_CHARACTERISTIC_PROPERITY_READ)        0x01(GATT_PERMISSION_READ)
                                                                0x0003(handle)
                                                                0x2a00(GATT_OBJECT_TYPE_DEVICE_NAME)
0x0003			0x2a00(GATT_OBJECT_TYPE_DEVICE_NAME)			"ble_demo"                                      0x01(GATT_PERMISSION_READ)

// GATT service
0x0010			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x1801(GATT_SERVICE_GATT)                       0x01(GATT_PERMISSION_READ)
0x0011			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x20(GATT_CHARACTERISTIC_PROPERITY_INDICATE)    0x01(GATT_PERMISSION_READ)
                                                                0x0012(handle)
                                                                0x2a05(GATT_OBJECT_TYPE_SERVICE_CHANGED)
0x0012			0x2a05(GATT_OBJECT_TYPE_SERVICE_CHANGED)		0x0000, 0x0000                                  0x01(GATT_PERMISSION_READ)
0x0013          0x2902(GATT_CLIENT_CHARACTER_CONFIG)            0x0000                                          0x01(GATT_PERMISSION_READ)

// BATTERY service
0x0100			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x180f(GATT_SERVICE_BATTERY)                    0x01(GATT_PERMISSION_READ)
0x0101			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x12(GATT_CHARACTERISTIC_PROPERITY_READ/NOTIFY) 0x01(GATT_PERMISSION_READ)
                                                                0x0102(handle)
                                                                0x2a19(GATT_OBJECT_TYPE_BATTERY_LEVEL)
0x0102			0x2a19(GATT_OBJECT_TYPE_BATTERY_LEVEL)			0x62(98%)                                       0x01(GATT_PERMISSION_READ)
0x0103          0x2902(GATT_CLIENT_CHARACTER_CONFIG)            0x0000                                          0x01(GATT_PERMISSION_READ)

// TEST service
0x1000          0x2800(GATT_DECLARATION_PRIMARY_SERVICE)        0x18ff(GATT_SERVICE_TEST)                       0x01(GATT_PERMISSION_READ)
0x1001          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x16(GATT_CHARACTERISTIC_PROPERITY_READ/WRITE_NORESP/NOTIFY)  0x01(GATT_PERMISSION_READ)
                                                                0x1002(handle)
                                                                0x2aff(GATT_OBJECT_TYPE_TEST)
0x1002          0x2aff(GATT_OBJECT_TYPE_TEST)                   [36 Bytes]                                      0x01(GATT_PERMISSION_READ)
0x1003          0x2902(GATT_CLIENT_CHARACTER_CONFIG)            0x0000                                          0x01(GATT_PERMISSION_READ)
*/

uint8_t uuid_gencric_access[] = {(uint8_t)GATT_SERVICE_GENERIC_ACCESS, GATT_SERVICE_GENERIC_ACCESS >> 8};
uint8_t characteristic_gencric_access[] = {GATT_CHARACTERISTIC_PROPERITY_READ,
                                           0x03, 0x00,
                                           (uint8_t)GATT_OBJECT_TYPE_DEVICE_NAME, GATT_OBJECT_TYPE_DEVICE_NAME >> 8};
uint8_t device_name[] = {'b', 'l', 'e', '_', 'd', 'e', 'm', 'o'};


uint8_t uuid_gencric_attribute[] = {(uint8_t)GATT_SERVICE_GENERIC_ATTRIBUTE, GATT_SERVICE_GENERIC_ATTRIBUTE >> 8};
uint8_t characteristic_gencric_attribute[] = {GATT_CHARACTERISTIC_PROPERITY_INDICATE,
                                              0x12, 0x00,
                                              (uint8_t)GATT_OBJECT_TYPE_SERVICE_CHANGED, GATT_OBJECT_TYPE_SERVICE_CHANGED >> 8};
uint8_t gencric_attribute[] = {0x00, 0x00, 0x00, 0x00};


uint8_t uuid_battery[] = {(uint8_t)GATT_SERVICE_BATTERY, GATT_SERVICE_BATTERY >> 8};
uint8_t characteristic_battery[] = {GATT_CHARACTERISTIC_PROPERITY_READ | GATT_CHARACTERISTIC_PROPERITY_NOTIFY,
                                    0x02, 0x01,
                                    (uint8_t)GATT_OBJECT_TYPE_BATTERY_LEVEL, GATT_OBJECT_TYPE_BATTERY_LEVEL >> 8};
uint8_t battery_level[] = {0x62}; // 98%


uint8_t uuid_test[] = {(uint8_t)GATT_SERVICE_TEST, GATT_SERVICE_TEST >> 8};
uint8_t characteristic_test[] = {GATT_CHARACTERISTIC_PROPERITY_READ | GATT_CHARACTERISTIC_PROPERITY_WRITE_NORESP| GATT_CHARACTERISTIC_PROPERITY_NOTIFY,
                                 0x02, 0x10,
                                 (uint8_t)GATT_OBJECT_TYPE_TEST, GATT_OBJECT_TYPE_TEST >> 8};
uint8_t buff_test[36] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                         's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};


ATT_ITEM items_gencric_access[] = {
    {0x0001, GATT_DECLARATION_PRIMARY_SERVICE, uuid_gencric_access, sizeof(uuid_gencric_access), GATT_PERMISSION_READ},
    {0x0002, GATT_DECLARATION_CHARACTERISTIC, characteristic_gencric_access, sizeof(characteristic_gencric_access), GATT_PERMISSION_READ},
    {0x0003, GATT_OBJECT_TYPE_DEVICE_NAME, device_name, sizeof(device_name), GATT_PERMISSION_READ}
};

ATT_ITEM items_gencric_attribute[] = {
    {0x0010, GATT_DECLARATION_PRIMARY_SERVICE, uuid_gencric_attribute, sizeof(uuid_gencric_attribute), GATT_PERMISSION_READ},
    {0x0011, GATT_DECLARATION_CHARACTERISTIC, characteristic_gencric_attribute, sizeof(characteristic_gencric_attribute), GATT_PERMISSION_READ},
    {0x0012, GATT_OBJECT_TYPE_SERVICE_CHANGED, gencric_attribute, sizeof(gencric_attribute), GATT_PERMISSION_READ},
    {0x0013, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, nullptr, 0, GATT_PERMISSION_READ}
};

ATT_ITEM items_battery[] = {
    {0x0100, GATT_DECLARATION_PRIMARY_SERVICE, uuid_battery, sizeof(uuid_battery), GATT_PERMISSION_READ},
    {0x0101, GATT_DECLARATION_CHARACTERISTIC, characteristic_battery, sizeof(characteristic_battery), GATT_PERMISSION_READ},
    {0x0102, GATT_OBJECT_TYPE_BATTERY_LEVEL, battery_level, sizeof(battery_level), GATT_PERMISSION_READ},
    {0x0103, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, nullptr, 0, GATT_PERMISSION_READ}
};

ATT_ITEM items_test[] = {
    {0x1000, GATT_DECLARATION_PRIMARY_SERVICE, uuid_test, sizeof(uuid_test), GATT_PERMISSION_READ},
    {0x1001, GATT_DECLARATION_CHARACTERISTIC, characteristic_test, sizeof(characteristic_test), GATT_PERMISSION_READ},
    {0x1002, GATT_OBJECT_TYPE_TEST, buff_test, sizeof(buff_test), GATT_PERMISSION_READ},
    {0x1003, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, nullptr, 0, GATT_PERMISSION_READ}
};


GATT_SERVICE services[GATT_SERVICE_MAX_COUNT];
uint8_t service_count = 0;

void gatt_init() {
    gatt_add_service(items_gencric_access, 3, 0x0001, 0x0003, GATT_SERVICE_GENERIC_ACCESS);
    gatt_add_service(items_gencric_attribute, 4, 0x0010, 0x0013, GATT_SERVICE_GENERIC_ATTRIBUTE);
    gatt_add_service(items_battery, 4, 0x0100, 0x0103, GATT_SERVICE_BATTERY);
    gatt_add_service(items_test, 4, 0x1000, 0x1003, GATT_SERVICE_TEST);
}

void gatt_add_service(ATT_ITEM *items, uint16_t items_cnt, uint16_t start_handle, uint16_t end_handle, uint16_t service_id) {
    if (service_count < GATT_SERVICE_MAX_COUNT) {
        services[service_count].items = items;
        services[service_count].items_cnt = items_cnt;
        services[service_count].start_handle = start_handle;
        services[service_count].end_handle = end_handle;
        services[service_count].service_id = service_id;
        service_count++;
    }
}

// read part value of one characteristic
void gatt_recv_read_blob_req(uint16_t handle, uint16_t value_offset) {
    uint8_t *buffer = nullptr;
    ATT_ITEM *item = nullptr;
    uint16_t copy_bytes = 0;
    uint16_t offset= 1;
    uint16_t att_mtu = att_get_mtu();

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < handle) {
                continue;
            }

            copy_bytes = item->value_length - value_offset;
            if (copy_bytes > att_mtu - 1) {
                copy_bytes = att_mtu - 1;
            }

            buffer = (uint8_t *)malloc(att_mtu);
            buffer[0] = ATT_OPERATE_READ_BLOB_RESP;
            memcpy_s(buffer + offset, copy_bytes, item->value + value_offset, copy_bytes);
            offset += copy_bytes;
            att_send(buffer, offset);
            free(buffer);
            return;
        }
    }
}

// read value of one characteristic
void gatt_recv_read_req(uint16_t handle) {
    uint8_t *buffer = nullptr;
    ATT_ITEM *item = nullptr;
    uint16_t copy_bytes = 0;
    uint16_t offset= 1;
    uint16_t att_mtu = att_get_mtu();

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < handle) {
                continue;
            }

            copy_bytes = item->value_length;
            if (copy_bytes > att_mtu - 1) {
                copy_bytes = att_mtu - 1;
            }

            buffer = (uint8_t *)malloc(att_mtu);
            buffer[0] = ATT_OPERATE_READ_RESP;
            memcpy_s(buffer + offset, copy_bytes, item->value, copy_bytes);
            offset += copy_bytes;
            att_send(buffer, offset);
            free(buffer);
            return;
        }
    }
}

// discover descriptors in one characteristic
void gatt_recv_find_information_req(uint16_t start_handle, uint16_t end_handle) {
    uint8_t *buffer = nullptr;
    ATT_ITEM *item = nullptr;
    uint16_t att_mtu = att_get_mtu();
    uint16_t offset= 2;
    bool found = false;

    buffer = (uint8_t *)malloc(att_mtu);
    buffer[0] = ATT_OPERATE_FIND_INFO_RESP;
    buffer[1] = ATT_UUID_TYPE_BITS_16; // TODO: 128 bits uuid

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < start_handle) {
                continue;
            }
            found = true;

            buffer[offset] = (uint8_t)item->handle;
            offset++;
            buffer[offset] = item->handle >> 8;
            offset++;
            buffer[offset] = (uint8_t)item->type;
            offset++;
            buffer[offset] = item->type >> 8;
            offset++;

            if ((offset + 4) > att_mtu) {
                break;
            }

            if (item->handle > end_handle) {
                break;
            }
        }
    }

    if (found) {
        att_send(buffer, offset);
    } else {
        gatt_send_error_resp(ATT_OPERATE_FIND_INFO_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
    }

    free(buffer);
}

// discover include and characteristics in one service
void gatt_recv_read_by_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t att_type) {
    uint8_t *buffer = nullptr;
    ATT_ITEM *item = nullptr;
    uint16_t att_mtu = att_get_mtu();
    uint16_t offset= 2;
    uint16_t pair_value_length = 0;
    bool found = false;

    buffer = (uint8_t *)malloc(att_mtu);
    buffer[0] = ATT_OPERATE_READ_BY_TYPE_RESP;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < start_handle) {
                continue;
            }

            if (item->type == att_type) {
                if (found == false) { // the first item matched
                    pair_value_length = item->value_length;
                    buffer[1] = 2 + pair_value_length;
                }
                found = true;

                if (pair_value_length != item->value_length) { // subsequent items not matched
                    break;
                }

                buffer[offset] = (uint8_t)item->handle;
                offset++;
                buffer[offset] = item->handle >> 8;
                offset++;
                memcpy_s(buffer + offset, pair_value_length, item->value, pair_value_length);
                offset += pair_value_length;
                if ((offset + 2 + pair_value_length) > att_mtu) {
                    break;
                }
            }

            if (item->handle > end_handle) {
                break;
            }
        }
    }

    if (found) {
        att_send(buffer, offset);
    } else {
        gatt_send_error_resp(ATT_OPERATE_READ_BY_TYPE_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
    }

    free(buffer);
}

// discover all services and it's start and end handle, just used for primary and second service
void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t group_type) {
    uint8_t *buffer = nullptr;
    uint16_t att_mtu = att_get_mtu();
    uint16_t offset= 2;
    bool found = false;

    buffer = (uint8_t *)malloc(att_mtu);
    buffer[0] = ATT_OPERATE_READ_BY_GROUP_TYPE_RESP;
    buffer[1] = 6; // start_handle, end_handle, uuid, TODO: 128 bits uuid

    for (uint8_t index = 0; index < service_count; index++) {
        if ((start_handle <= services[index].start_handle) && (services[index].end_handle <= end_handle)) {
            if (services[index].items[0].type == group_type) {
                found = true;

                buffer[offset] = (uint8_t)services[index].start_handle;
                offset++;
                buffer[offset] = services[index].start_handle >> 8;
                offset++;
                buffer[offset] = (uint8_t)services[index].end_handle;
                offset++;
                buffer[offset] = services[index].end_handle >> 8;
                offset++;
                memcpy_s(buffer + offset, services[index].items[0].value_length, services[index].items[0].value,
                         services[index].items[0].value_length);
                offset += services[index].items[0].value_length;
                if ((offset + 6) > att_mtu) {
                    break;
                }
            }
        }
    }

    if (found) {
        att_send(buffer, offset);
    } else {
        gatt_send_error_resp(ATT_OPERATE_READ_BY_GROUP_TYPE_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
    }

    free(buffer);
}

void gatt_send_error_resp(uint8_t op_code, uint16_t handle, uint8_t error_code) {
    uint8_t data[ATT_LENGTH_ERROR_RESP] = { 0x00 };

    data[0] = ATT_OPERATE_ERROR_RESP;
    data[1] = op_code;
    data[2] = (uint8_t)handle;
    data[3] = handle >> 8;
    data[4] = error_code;
    att_send(data, ATT_LENGTH_ERROR_RESP);
}
