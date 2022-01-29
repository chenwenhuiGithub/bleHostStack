#include "gatt.h"
#include <vector>
#include <QByteArray>

std::vector<gatt_service> services;

void gatt_recv_read_by_group_type_req(uint16_t start_handle, uint16_t end_handle, att_uuid &group_type) {
    QByteArray byteArray;

    for (std::vector<gatt_service>::const_iterator iter = services.begin(); iter != services.end(); ++iter) {
        if ((start_handle <= iter->start_handle) && (iter->end_handle <= end_handle)) {
            if (iter->items[0].type == group_type) {
                byteArray.resize(6 + iter->items[0].value.length);
                byteArray[0] = ATT_OPERATE_READ_BY_GROUP_TYPE_RESP;
                byteArray[1] = 4 + iter->items[0].value.length;
                byteArray[2] = iter->start_handle;
                byteArray[3] = iter->start_handle >> 8;
                byteArray[4] = iter->end_handle;
                byteArray[5] = iter->end_handle >> 8;
                memcpy_s((uint8_t*)byteArray.data() + 6, iter->items[0].value.length, iter->items[0].value.data, iter->items[0].value.length);
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
