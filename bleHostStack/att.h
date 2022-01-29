#ifndef ATT_H
#define ATT_H

#include <stdint.h>
#include <memory>

#define ATT_ERROR_INVALID_HANDLE                        0x01
#define ATT_ERROR_READ_NOT_PERMITTED                    0x02
#define ATT_ERROR_WRITE_NOT_PERMITTED                   0x03
#define ATT_ERROR_INVALID_PDU                           0x04
#define ATT_ERROR_INSUFFICIENT_AUTHENTICATION           0x05
#define ATT_ERROR_REQUEST_NOT_SUPPORTED                 0x06
#define ATT_ERROR_INVALID_OFFSET                        0x07
#define ATT_ERROR_INSUFFICIENT_AUTHORIZATION            0x08
#define ATT_ERROR_PREPARE_QUEUE_FULL                    0x09
#define ATT_ERROR_ATTRIBUTE_NOT_FOUND                   0x0a
#define ATT_ERROR_ATTRIBUTE_NOT_LONG                    0x0b
#define ATT_ERROR_ENCRYPTION_KEY_SIZE_TOO_SHORT         0x0c
#define ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LENGTH        0x0d
#define ATT_ERROR_UNLIKELY_ERROR                        0x0e
#define ATT_ERROR_INSUFFICIENT_ENCRYPTION               0x0f
#define ATT_ERROR_UNSUPPORTED_GROUP_TYPE                0x10
#define ATT_ERROR_INSUFFICIENT_RESOURCE                 0x11
#define ATT_ERROR_DATABASE_OUT_OF_SYNC                  0x12
#define ATT_ERROR_VALUE_NOT_ALLOWED                     0x13


#define ATT_OPERATE_ERROR_RESP                          0x01
#define ATT_OPERATE_EXCHANGE_MTU_REQ                    0x02
#define ATT_OPERATE_EXCHANGE_MTU_RESP                   0x03
#define ATT_OPERATE_FIND_INFORMATION_REQ                0x04
#define ATT_OPERATE_FIND_INFORMATION_RESP               0x05
#define ATT_OPERATE_FIND_BY_TYPE_VALUE_REQ              0x06
#define ATT_OPERATE_FIND_BY_TYPE_VALUE_RESP             0x07
#define ATT_OPERATE_READ_BY_TYPE_REQ                    0x08
#define ATT_OPERATE_READ_BY_TYPE_RESP                   0x09
#define ATT_OPERATE_READ_REQ                            0x0a
#define ATT_OPERATE_READ_RESP                           0x0b
#define ATT_OPERATE_READ_BLOB_REQ                       0x0c
#define ATT_OPERATE_READ_BLOB_RESP                      0x0d
#define ATT_OPERATE_READ_MULTIPLE_REQ                   0x0e
#define ATT_OPERATE_READ_MULTIPLE_RESP                  0x0f
#define ATT_OPERATE_READ_BY_GROUP_TYPE_REQ              0x10
#define ATT_OPERATE_READ_BY_GROUP_TYPE_RESP             0x11
#define ATT_OPERATE_WRITE_REQ                           0x12
#define ATT_OPERATE_WRITE_RESP                          0x13
#define ATT_OPERATE_PREPARE_WRITE_REQ                   0x16
#define ATT_OPERATE_PREPARE_WRITE_RESP                  0x17
#define ATT_OPERATE_EXECUTE_WRITE_REQ                   0x18
#define ATT_OPERATE_EXECUTE_WRITE_RESP                  0x19
#define ATT_OPERATE_HANDLE_VALUE_NTF                    0x1b
#define ATT_OPERATE_HANDLE_VALUE_IND                    0x1d
#define ATT_OPERATE_HANDLE_VALUE_CFM                    0x1e
#define ATT_OPERATE_READ_MULTIPLE_VARIABLE_REQ          0x20
#define ATT_OPERATE_READ_MULTIPLE_VARIABLE_RESP         0x21
#define ATT_OPERATE_MULTIPLE_HANDLE_VALUE_NTF           0x23
#define ATT_OPERATE_WRITE_CMD                           0x52
#define ATT_OPERATE_SIGNED_WRITE_CMD                    0xd2


#define ATT_LENGTH_HEADER								1
#define ATT_LENGTH_UUID16								2
#define ATT_LENGTH_UUID128								16
#define ATT_LENGTH_ERROR_RESP							5


struct att_uuid {
    uint8_t uuid[16]; // 2 or 16 Bytes uuid
    uint8_t length;

    bool operator == (const att_uuid &id) {
        return ((length == id.length) && (0 == memcmp(uuid, id.uuid, length)));
    }
};

typedef struct {
    uint8_t *data;
    uint16_t length;
} att_value;

typedef struct {
    uint16_t handle;
    att_uuid type;
    att_value value;
    uint8_t permission;
} att_item;


void att_recv(uint8_t *data, uint16_t length);
void att_recv_read_by_group_type_req(uint8_t *data, uint16_t length);
void att_send(uint8_t *data, uint16_t length);

#endif // ATT_H
