#ifndef GATT_H
#define GATT_H

#include <stdint.h>

typedef struct {
    uint16_t handle;
    uint16_t type; // TODO: support 16 Bytes uuid
    uint8_t *value;
    uint16_t value_length;
    uint8_t permission;
} ATT_ITEM;


void gatt_init();
void gatt_add_service(ATT_ITEM *items, uint16_t items_cnt, uint16_t start_handle, uint16_t end_handle, uint16_t service_id);
void gatt_recv_read_req(uint16_t handle);
void gatt_recv_find_information_req(uint16_t start_handle, uint16_t end_handle);
void gatt_recv_read_by_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t att_type);
void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, uint16_t group_type);
void gatt_recv_read_blob_req(uint16_t handle, uint16_t value_offset);
void gatt_send_error_resp(uint8_t op_code, uint16_t handle, uint8_t error_code);

#endif // GATT_H
