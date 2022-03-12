#include "gatt.h"
#include <vector>
#include <QByteArray>

/*
att_handle(2B)	att_type(UUID, 2B/16B)							att_value(0-512B)                           att_permission(2B)

0x0001			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x1800(GATT_SERVICE_GENERIC_ACCESS)         0x01(GATT_PERMISSION_READ)
0x0002			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x02(GATT_CHARACTERISTIC_PROPERITY_READ)    0x01(GATT_PERMISSION_READ)
                                                                0x0003(handle)
                                                                0x2A00(GATT_OBJECT_TYPE_DEVICE_NAME)
0x0003			0x2A00(GATT_OBJECT_TYPE_DEVICE_NAME)			"gatt_demo"                                 0x01(GATT_PERMISSION_READ)


0x0010			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x1801(GATT_SERVICE_GATT)                   0x01(GATT_PERMISSION_READ)
0x0011			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x02(GATT_CHARACTERISTIC_PROPERITY_READ)    0x01(GATT_PERMISSION_READ)
                                                                0x0012(handle)
                                                                0x2A05(GATT_OBJECT_TYPE_SERVICE_CHANGED)
0x0012			0x2A05(GATT_OBJECT_TYPE_SERVICE_CHANGED)		0x0000, 0x0000                              0x01(GATT_PERMISSION_READ)


0x0100			0x2800(GATT_DECLARATION_PRIMARY_SERVICE)		0x180F(GATT_SERVICE_BATTERY)                0x01(GATT_PERMISSION_READ)
0x0101			0x2803(GATT_DECLARATION_CHARACTERISTIC)			0x02(GATT_CHARACTERISTIC_PROPERITY_READ)    0x01(GATT_PERMISSION_READ)
                                                                0x0102(handle)
                                                                0x2A19(GATT_OBJECT_TYPE_BATTERY_LEVEL)
0x0102			0x2A19(GATT_OBJECT_TYPE_BATTERY_LEVEL)			0x62(98%)                                   0x01(GATT_PERMISSION_READ)
*/

uint8_t gencric_access_uuid[] = {(uint8_t)GATT_SERVICE_GENERIC_ACCESS, GATT_SERVICE_GENERIC_ACCESS >> 8};
uint8_t gencric_access_characteristic[] = {GATT_CHARACTERISTIC_PROPERITY_READ, 0x03, 0x00,
                                           (uint8_t)GATT_OBJECT_TYPE_DEVICE_NAME, GATT_OBJECT_TYPE_DEVICE_NAME >> 8};
uint8_t gencric_access_device_name[] = {'w', 'e', 'n', 'h', 'u', 'i', '_', 'B', 'L', 'E'};
att_item service_gencric_access[] = {
    {0x0001, GATT_DECLARATION_PRIMARY_SERVICE, gencric_access_uuid, sizeof(gencric_access_uuid), GATT_PERMISSION_READ},
    {0x0002, GATT_DECLARATION_CHARACTERISTIC, gencric_access_characteristic, sizeof(gencric_access_characteristic), GATT_PERMISSION_READ},
    {0x0003, GATT_OBJECT_TYPE_DEVICE_NAME, gencric_access_device_name, sizeof(gencric_access_device_name), GATT_PERMISSION_READ}
};


uint8_t gencric_attribute_uuid[] = {(uint8_t)GATT_SERVICE_GENERIC_ATTRIBUTE, GATT_SERVICE_GENERIC_ATTRIBUTE >> 8};
uint8_t gencric_attribute_characteristic[] = {GATT_CHARACTERISTIC_PROPERITY_READ, 0x12, 0x00,
                                              (uint8_t)GATT_OBJECT_TYPE_SERVICE_CHANGED, GATT_OBJECT_TYPE_SERVICE_CHANGED >> 8};
uint8_t gencric_attribute[] = {0x00, 0x00, 0x00, 0x00};
att_item service_gencric_attribute[] = {
    {0x0010, GATT_DECLARATION_PRIMARY_SERVICE, gencric_attribute_uuid, sizeof(gencric_attribute_uuid), GATT_PERMISSION_READ},
    {0x0011, GATT_DECLARATION_CHARACTERISTIC, gencric_attribute_characteristic, sizeof(gencric_attribute_characteristic), GATT_PERMISSION_READ},
    {0x0012, GATT_OBJECT_TYPE_SERVICE_CHANGED, gencric_attribute, sizeof(gencric_attribute), GATT_PERMISSION_READ}
};


uint8_t battery_uuid[] = {(uint8_t)GATT_SERVICE_BATTERY, GATT_SERVICE_BATTERY >> 8};
uint8_t battery_characteristic[] = {GATT_CHARACTERISTIC_PROPERITY_READ, 0x02, 0x01,
                                    (uint8_t)GATT_OBJECT_TYPE_BATTERY_LEVEL, GATT_OBJECT_TYPE_BATTERY_LEVEL >> 8};
uint8_t battery_level[] = {0x62}; // 98%
att_item service_battery[] = {
    {0x0100, GATT_DECLARATION_PRIMARY_SERVICE, battery_uuid, sizeof(battery_uuid), GATT_PERMISSION_READ},
    {0x0101, GATT_DECLARATION_CHARACTERISTIC, battery_characteristic, sizeof(battery_characteristic), GATT_PERMISSION_READ},
    {0x0102, GATT_OBJECT_TYPE_BATTERY_LEVEL, battery_level, sizeof(battery_level), GATT_PERMISSION_READ}
};


gatt_service services[GATT_SERVICE_SIZE];
uint8_t gatt_service_count = 0;

void gatt_init() {
    gatt_add_service(service_gencric_access, 0x0001, 0x0003, GATT_SERVICE_GENERIC_ACCESS);
    gatt_add_service(service_gencric_attribute, 0x0010, 0x0012, GATT_SERVICE_GENERIC_ATTRIBUTE);
    gatt_add_service(service_battery, 0x0100, 0x0ffff, GATT_SERVICE_BATTERY); // latest service end_handle must be 0xffff?
}

void gatt_add_service(att_item *items, uint16_t start_handle, uint16_t end_handle, uint16_t service) {
    if (gatt_service_count < GATT_SERVICE_SIZE) {
        services[gatt_service_count].items = items;
        services[gatt_service_count].start_handle = start_handle;
        services[gatt_service_count].end_handle = end_handle;
        services[gatt_service_count].service = service;
        gatt_service_count++;
    }
}

void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t group_type) {
    QByteArray byteArray;

    for (uint8_t index = 0; index < gatt_service_count; index++) {
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

    gatt_send_error_resp(ATT_OPERATE_READ_BY_GROUP_TYPE_RESP, start_handle, ATT_ERROR_ATTRIBUTE_NOT_FOUND);
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
