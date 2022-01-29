#ifndef GATT_H
#define GATT_H

#include "att.h"

typedef struct {
    att_item *items;
    uint8_t items_count;
    uint16_t start_handle;
    uint16_t end_handle;
    att_uuid service;
} gatt_service;


void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, att_uuid &group_type);
void gatt_send_error_resp(uint8_t op_code, uint16_t handle, uint8_t error_code);

#endif // GATT_H
