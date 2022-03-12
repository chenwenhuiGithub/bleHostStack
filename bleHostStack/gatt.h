#ifndef GATT_H
#define GATT_H

#include "att.h"

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


#define GATT_SERVICE_SIZE                                   10  // TODO: move to config.h


typedef struct {
    att_item *items;
    uint16_t items_cnt;
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t service;
} gatt_service;


void gatt_init();
void gatt_add_service(att_item *items, uint16_t items_cnt, uint16_t start_handle, uint16_t end_handle, uint16_t service);
void gatt_recv_read_req(uint16_t handle);
void gatt_recv_find_information_req(uint16_t start_handle, uint16_t end_handle);
void gatt_recv_read_by_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t att_type);
void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t group_type);
void gatt_send_error_resp(uint8_t op_code, uint16_t handle, uint8_t error_code);

#endif // GATT_H
