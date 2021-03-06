#ifndef GATT_H
#define GATT_H

#include <stdint.h>

typedef struct {
    uint16_t handle;
    uint16_t type; // TODO: support 16 Bytes uuid
    uint8_t *value;
    uint16_t value_length;
    uint8_t permission;
} att_item_t;

typedef struct {
    att_item_t *items;
    uint16_t items_cnt;
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t service_id;
} gatt_service_t;

void gatt_init();
void gatt_add_service(att_item_t *items, uint16_t items_cnt, uint16_t start_handle, uint16_t end_handle, uint16_t service_id);
void gatt_recv_read_req(uint16_t connect_handle, uint16_t att_handle);
void gatt_recv_find_information_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end);
void gatt_recv_find_by_type_value_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end,
                                      uint16_t att_type, uint8_t *att_value, uint32_t att_value_length);
void gatt_recv_read_by_type_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end, uint16_t att_type);
void gatt_recv_read_by_group_type_req(uint16_t connect_handle, uint16_t att_handle_start, uint16_t att_handle_end, uint16_t group_type);
void gatt_recv_read_blob_req(uint16_t connect_handle, uint16_t att_handle, uint16_t value_offset);
void gatt_recv_write_req(uint16_t connect_handle, uint16_t att_handle, uint8_t *value, uint32_t value_length);
void gatt_recv_write_cmd(uint16_t connect_handle, uint16_t att_handle, uint8_t *value, uint32_t value_length);
void gatt_recv_handle_value_cfm(uint16_t connect_handle);
void gatt_send_handle_value_notify(uint16_t connect_handle, uint16_t att_handle, uint8_t *value, uint32_t value_length);
void gatt_send_handle_value_indication(uint16_t connect_handle, uint16_t att_handle, uint8_t *value, uint32_t value_length);
void gatt_send_error_resp(uint16_t connect_handle, uint8_t op_code, uint16_t handle, uint8_t error_code);

#endif // GATT_H
