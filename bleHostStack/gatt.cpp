#include "gatt.h"
#include <vector>
#include <QByteArray>

/*
att_handle(2B)	att_type(UUID, 2B/16B)							att_value(0-512B)                               att_permission(2B)

// GAP service
0x0001			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x1800(GATT_SERVICE_GENERIC_ACCESS)             0x01(GATT_PERMISSION_READ)
0x0002			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x02(GATT_CHARACTERISTIC_PROPERITY_READ)        0x01(GATT_PERMISSION_READ)
                                                                0x0003(handle)
                                                                0x2a00(GATT_OBJECT_TYPE_DEVICE_NAME)
0x0003			0x2a00(GATT_OBJECT_TYPE_DEVICE_NAME)			"gatt_demo"                                     0x01(GATT_PERMISSION_READ)

// GATT service
0x0010			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x1801(GATT_SERVICE_GATT)                       0x01(GATT_PERMISSION_READ)
0x0011			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x20(GATT_CHARACTERISTIC_PROPERITY_INDICATE)    0x01(GATT_PERMISSION_READ)
                                                                0x0012(handle)
                                                                0x2a05(GATT_OBJECT_TYPE_SERVICE_CHANGED)
0x0012			0x2a05(GATT_OBJECT_TYPE_SERVICE_CHANGED)		0x0000, 0x0000                                  0x01(GATT_PERMISSION_READ)
0x0013          0x2902(GATT_CLIENT_CHARACTER_CONFIG)                                                            0x0001(GATT_PERMISSION_READ)

// BATTERY service
0x0100			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x180f(GATT_SERVICE_BATTERY)                    0x01(GATT_PERMISSION_READ)
0x0101			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x12(GATT_CHARACTERISTIC_PROPERITY_READ/NOTIFY) 0x01(GATT_PERMISSION_READ)
                                                                0x0102(handle)
                                                                0x2a19(GATT_OBJECT_TYPE_BATTERY_LEVEL)
0x0102			0x2a19(GATT_OBJECT_TYPE_BATTERY_LEVEL)			0x62(98%)                                       0x01(GATT_PERMISSION_READ)
0x0103          0x2902(GATT_CLIENT_CHARACTER_CONFIG)                                                            0x0001(GATT_PERMISSION_READ)

// TEST service
0x1000          0x2800(GATT_DECLARATION_PRIMARY_SERVICE)        0x18ff(GATT_SERVICE_TEST)                       0x0001(GATT_PERMISSION_READ)
0x1001          0x2803(GATT_DECLARATION_CHARACTERISTIC)         0x16(GATT_CHARACTERISTIC_PROPERITY_READ/WRITE_NORESP/NOTIFY)  0x0001(GATT_PERMISSION_READ)
                                                                0x1002(handle)
                                                                0x2aff(GATT_OBJECT_TYPE_TEST)
0x1002          0x2aff(GATT_OBJECT_TYPE_TEST)                   [512 Bytes]                                     0x0001(GATT_PERMISSION_READ)
0x1003          0x2902(GATT_CLIENT_CHARACTER_CONFIG)                                                            0x0001(GATT_PERMISSION_READ)
*/

uint8_t uuid_gencric_access[] = {(uint8_t)GATT_SERVICE_GENERIC_ACCESS, GATT_SERVICE_GENERIC_ACCESS >> 8};
uint8_t characteristic_gencric_access[] = {GATT_CHARACTERISTIC_PROPERITY_READ, 0x03, 0x00,
                                           (uint8_t)GATT_OBJECT_TYPE_DEVICE_NAME, GATT_OBJECT_TYPE_DEVICE_NAME >> 8};
uint8_t device_name[] = {'b', 'l', 'e', '_', 'd', 'e', 'm', 'o'};


uint8_t uuid_gencric_attribute[] = {(uint8_t)GATT_SERVICE_GENERIC_ATTRIBUTE, GATT_SERVICE_GENERIC_ATTRIBUTE >> 8};
uint8_t characteristic_gencric_attribute[] = {GATT_CHARACTERISTIC_PROPERITY_INDICATE, 0x12, 0x00,
                                              (uint8_t)GATT_OBJECT_TYPE_SERVICE_CHANGED, GATT_OBJECT_TYPE_SERVICE_CHANGED >> 8};
uint8_t gencric_attribute[] = {0x00, 0x00, 0x00, 0x00};


uint8_t uuid_battery[] = {(uint8_t)GATT_SERVICE_BATTERY, GATT_SERVICE_BATTERY >> 8};
uint8_t characteristic_battery[] = {GATT_CHARACTERISTIC_PROPERITY_READ | GATT_CHARACTERISTIC_PROPERITY_NOTIFY, 0x02, 0x01,
                                    (uint8_t)GATT_OBJECT_TYPE_BATTERY_LEVEL, GATT_OBJECT_TYPE_BATTERY_LEVEL >> 8};
uint8_t battery_level[] = {0x62}; // 98%


uint8_t uuid_test[] = {(uint8_t)GATT_SERVICE_TEST, GATT_SERVICE_TEST >> 8};
uint8_t characteristic_test[] = {GATT_CHARACTERISTIC_PROPERITY_READ | GATT_CHARACTERISTIC_PROPERITY_WRITE_NORESP| GATT_CHARACTERISTIC_PROPERITY_NOTIFY,
                                 0x02, 0x10, (uint8_t)GATT_OBJECT_TYPE_TEST, GATT_OBJECT_TYPE_TEST >> 8};
uint8_t buff_test[8] = {0x01, 0x03, 0x05, 0x07, 0x02, 0x04, 0x06, 0x08};


att_item items_gencric_access[] = {
    {0x0001, GATT_DECLARATION_PRIMARY_SERVICE, uuid_gencric_access, sizeof(uuid_gencric_access), GATT_PERMISSION_READ},
    {0x0002, GATT_DECLARATION_CHARACTERISTIC, characteristic_gencric_access, sizeof(characteristic_gencric_access), GATT_PERMISSION_READ},
    {0x0003, GATT_OBJECT_TYPE_DEVICE_NAME, device_name, sizeof(device_name), GATT_PERMISSION_READ}
};

att_item items_gencric_attribute[] = {
    {0x0010, GATT_DECLARATION_PRIMARY_SERVICE, uuid_gencric_attribute, sizeof(uuid_gencric_attribute), GATT_PERMISSION_READ},
    {0x0011, GATT_DECLARATION_CHARACTERISTIC, characteristic_gencric_attribute, sizeof(characteristic_gencric_attribute), GATT_PERMISSION_READ},
    {0x0012, GATT_OBJECT_TYPE_SERVICE_CHANGED, gencric_attribute, sizeof(gencric_attribute), GATT_PERMISSION_READ},
    {0x0013, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, nullptr, 0, GATT_PERMISSION_READ}
};

att_item items_battery[] = {
    {0x0100, GATT_DECLARATION_PRIMARY_SERVICE, uuid_battery, sizeof(uuid_battery), GATT_PERMISSION_READ},
    {0x0101, GATT_DECLARATION_CHARACTERISTIC, characteristic_battery, sizeof(characteristic_battery), GATT_PERMISSION_READ},
    {0x0102, GATT_OBJECT_TYPE_BATTERY_LEVEL, battery_level, sizeof(battery_level), GATT_PERMISSION_READ},
    {0x0103, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, nullptr, 0, GATT_PERMISSION_READ}
};

att_item items_test[] = {
    {0x1000, GATT_DECLARATION_PRIMARY_SERVICE, uuid_test, sizeof(uuid_test), GATT_PERMISSION_READ},
    {0x1001, GATT_DECLARATION_CHARACTERISTIC, characteristic_test, sizeof(characteristic_test), GATT_PERMISSION_READ},
    {0x1002, GATT_OBJECT_TYPE_TEST, buff_test, sizeof(buff_test), GATT_PERMISSION_READ},
    {0x1003, GATT_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIG, nullptr, 0, GATT_PERMISSION_READ}
};


gatt_service services[GATT_SERVICE_SIZE];
uint8_t service_count = 0;

void gatt_init() {
    gatt_add_service(items_gencric_access, 3, 0x0001, 0x0003, GATT_SERVICE_GENERIC_ACCESS);
    gatt_add_service(items_gencric_attribute, 4, 0x0010, 0x0013, GATT_SERVICE_GENERIC_ATTRIBUTE);
    gatt_add_service(items_battery, 4, 0x0100, 0x0103, GATT_SERVICE_BATTERY);
    gatt_add_service(items_test, 4, 0x1000, 0x1003, GATT_SERVICE_TEST);
}

void gatt_add_service(att_item *items, uint16_t items_cnt, uint16_t start_handle, uint16_t end_handle, uint16_t service) {
    if (service_count < GATT_SERVICE_SIZE) {
        services[service_count].items = items;
        services[service_count].items_cnt = items_cnt;
        services[service_count].start_handle = start_handle;
        services[service_count].end_handle = end_handle;
        services[service_count].service = service;
        service_count++;
    }
}

void gatt_recv_read_req(uint16_t handle) {
    QByteArray byteArray;
    att_item *item = nullptr;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < handle) {
                continue;
            }

            byteArray.resize(1 + item->value_length);
            byteArray[0] = ATT_OPERATE_READ_RESP;
            memcpy_s((uint8_t*)byteArray.data() + 1, item->value_length, item->value, item->value_length);
            att_send((uint8_t*)byteArray.data(), byteArray.length());
            return; // TODO: support multiple services based on ATT_MTU check
        }
    }
}

// discover descriptors in one characteristic
void gatt_recv_find_information_req(uint16_t start_handle, uint16_t end_handle) {
    QByteArray byteArray;
    att_item *item = nullptr;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < start_handle) {
                continue;
            }

            byteArray.resize(6);
            byteArray[0] = ATT_OPERATE_FIND_INFORMATION_RESP;
            byteArray[1] = 0x01; // 16 bits UUID, TODO: macro define
            byteArray[2] = item->handle;
            byteArray[3] = item->handle >> 8;
            byteArray[4] = item->type;
            byteArray[5] = item->type >> 8;
            att_send((uint8_t*)byteArray.data(), byteArray.length());
            return; // TODO: support multiple services based on ATT_MTU check

            if (item->handle > end_handle) {
                gatt_send_error_resp(ATT_OPERATE_FIND_INFORMATION_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
                return;
            }
        }
    }

    gatt_send_error_resp(ATT_OPERATE_FIND_INFORMATION_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
}

// discover include and characteristics in one service
void gatt_recv_read_by_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t att_type) {
    QByteArray byteArray;
    att_item *item = nullptr;

    for (uint8_t index_service = 0; index_service < service_count; index_service++) {
        for (uint16_t index_item = 0; index_item < services[index_service].items_cnt; index_item++) {
            item = &(services[index_service].items[index_item]);

            if (item->handle < start_handle) {
                continue;
            }

            if (item->type == att_type) {
                byteArray.resize(4 + item->value_length);
                byteArray[0] = ATT_OPERATE_READ_BY_TYPE_RESP;
                byteArray[1] = 2 + item->value_length;
                byteArray[2] = item->handle;
                byteArray[3] = item->handle >> 8;
                memcpy_s((uint8_t*)byteArray.data() + 4, item->value_length, item->value, item->value_length);
                att_send((uint8_t*)byteArray.data(), byteArray.length());
                return; // TODO: support multiple services based on ATT_MTU check
            }

            if (item->handle > end_handle) {
                gatt_send_error_resp(ATT_OPERATE_READ_BY_TYPE_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
                return;
            }
        }
    }

    gatt_send_error_resp(ATT_OPERATE_READ_BY_TYPE_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
}

// discover all services and it's start and end handle, just used for primary and second service
void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t group_type) {
    QByteArray byteArray;

    for (uint8_t index = 0; index < service_count; index++) {
        if ((start_handle <= services[index].start_handle) && (services[index].end_handle <= end_handle)) {
            if (services[index].items[0].type == group_type) {
                byteArray.resize(6 + services[index].items[0].value_length);
                byteArray[0] = ATT_OPERATE_READ_BY_GROUP_TYPE_RESP;
                byteArray[1] = 4 + services[index].items[0].value_length;
                byteArray[2] = services[index].start_handle;
                byteArray[3] = services[index].start_handle >> 8;
                byteArray[4] = services[index].end_handle;
                byteArray[5] = services[index].end_handle >> 8;
                memcpy_s((uint8_t*)byteArray.data() + 6, services[index].items[0].value_length,
                        services[index].items[0].value, services[index].items[0].value_length);
                att_send((uint8_t*)byteArray.data(), byteArray.length());
                return; // TODO: support multiple services based on ATT_MTU check
            }
        }
    }

    gatt_send_error_resp(ATT_OPERATE_READ_BY_GROUP_TYPE_REQ, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
}

void gatt_send_error_resp(uint8_t op_code, uint16_t handle, uint8_t error_code) {
    uint8_t data[ATT_LENGTH_ERROR_RESP] = { 0x00 };

    data[0] = ATT_OPERATE_ERROR_RESP;
    data[1] = op_code;
    data[2] = handle;
    data[3] = handle >> 8;
    data[4] = error_code;
    att_send(data, ATT_LENGTH_ERROR_RESP);
}
