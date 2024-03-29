#ifndef SM_H
#define SM_H

#include <stdint.h>

#define SM_LENGTH_HEADER                                    1
#define SM_LENGTH_PAIRING_REQ                               6
#define SM_LENGTH_PAIRING_RESP                              6
#define SM_LENGTH_PAIRING_CONFIRM                           16
#define SM_LENGTH_PAIRING_RANDOM                            16
#define SM_LENGTH_PAIRING_FAILED                            1
#define SM_LENGTH_DHKEY                                     32
#define SM_LENGTH_DHKEY_CHECK                               16
#define SM_LENGTH_MACKEY                                    16
#define SM_LENGTH_IOCAP                                     3
#define SM_LENGTH_EDIV                                      2
#define SM_LENGTH_RAND                                      8
#define SM_LENGTH_IRK                                       16
#define SM_LENGTH_CSRK                                      16
#define SM_LENGTH_P256_PUBLIC_KEY                           64
#define SM_LENGTH_LTK                                       16
#define SM_LENGTH_TK                                        16
#define SM_LENGTH_ADDR                                      6
#define SM_LENGTH_KEYPRESS_NOTIFICATION                     1


typedef enum {
    KEYPRESS_ENTRY_STARTED,
    KEYPRESS_DIGIT_ENTERED,
    KEYPRESS_DIGIT_ERASED,
    KEYPRESS_CLEARED,
    KEYPRESS_ENTRY_COMPLETED
} sm_keypress_notification_t;

typedef enum {
    JUST_WORKS,
    PASSKEY_I_INPUT_R_DISPLAY,
    PASSKEY_I_DISPLAY_R_INPUT,
    PASSKEY_I_INPUT_R_INPUT,
    NUMERIC_COMPARISON,
    OOB
} sm_pairing_method_t;

typedef struct {
    uint8_t pairing_req[SM_LENGTH_HEADER + SM_LENGTH_PAIRING_REQ];
    uint8_t pairing_resp[SM_LENGTH_HEADER + SM_LENGTH_PAIRING_RESP];
    uint8_t remote_pairing_public_key[SM_LENGTH_P256_PUBLIC_KEY];
    uint8_t remote_pairing_confirm[SM_LENGTH_PAIRING_CONFIRM];
    uint8_t local_dhkey[SM_LENGTH_DHKEY];
    uint8_t local_random[SM_LENGTH_PAIRING_RANDOM];
    uint8_t remote_random[SM_LENGTH_PAIRING_RANDOM];
    uint8_t local_tk[SM_LENGTH_TK];
    uint8_t remote_tk[SM_LENGTH_TK];
    uint8_t local_ltk[SM_LENGTH_LTK];
    uint8_t remote_ltk[SM_LENGTH_LTK];
    uint8_t local_ediv[SM_LENGTH_EDIV];
    uint8_t remote_ediv[SM_LENGTH_EDIV];
    uint8_t local_rand[SM_LENGTH_RAND];
    uint8_t remote_rand[SM_LENGTH_RAND];
    uint8_t local_irk[SM_LENGTH_IRK];
    uint8_t remote_irk[SM_LENGTH_IRK];
    uint8_t local_csrk[SM_LENGTH_CSRK];
    uint8_t remote_csrk[SM_LENGTH_CSRK];
    uint8_t remote_addr_type;
    uint8_t remote_addr[SM_LENGTH_ADDR];
    sm_pairing_method_t pairing_method;

    uint32_t passkey_num; // 0 ~ 999999(0xF423F), 20 bits
    uint8_t passkey_index;
    uint8_t passkey_r;

    uint8_t is_secure_connection;
    uint8_t is_encrypted;
    uint8_t encryption_key_size; // TODO: add permission check
    uint8_t is_authenticated;
    uint8_t is_authorizated;     // TODO: add permission check
    uint8_t is_received_key_distribution_ltk;
    uint8_t is_received_key_distribution_id;
    uint8_t is_received_key_distribution_irk;
    uint8_t is_received_key_distribution_addr;
    uint8_t is_received_key_distribution_csrk;
} sm_connection_t;


void sm_recv(uint16_t connect_handle, uint8_t *data, uint32_t length);
void sm_recv_evt_le_ltk_req(uint16_t connect_handle, uint8_t *rand, uint8_t *ediv);
void sm_recv_evt_le_generate_dhkey_complete(uint8_t *dhkey);
void sm_recv_evt_encryption_change(uint16_t connect_handle, uint8_t encryption_enabled);
void sm_send_pairing_resp(uint16_t connect_handle, uint8_t *data);
void sm_send_pairing_public_key(uint16_t connect_handle, uint8_t *data);
void sm_send_pairing_confirm(uint16_t connect_handle, uint8_t *data);
void sm_send_pairing_random(uint16_t connect_handle, uint8_t *data);
void sm_send_pairing_dhkey_check(uint16_t connect_handle, uint8_t *data);
void sm_send_pairing_failed(uint16_t connect_handle, uint8_t reason);
void sm_send_encryption_information(uint16_t connect_handle, uint8_t *data);
void sm_send_central_identification(uint16_t connect_handle, uint8_t *data);
void sm_send_identity_information(uint16_t connect_handle, uint8_t *data);
void sm_send_identity_address_information(uint16_t connect_handle, uint8_t *data);
void sm_send_signing_information(uint16_t connect_handle, uint8_t *data);
void sm_send_keypress_notification(uint16_t connect_handle, sm_keypress_notification_t notification);
void sm_send(uint16_t connect_handle, uint8_t *data, uint32_t length);
void sm_ah(uint8_t* k, uint8_t* r, uint8_t* out_hash);
void sm_generate_random(uint8_t* data, uint32_t length);

#endif // SM_H
