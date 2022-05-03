#ifndef SM_H
#define SM_H

#include <stdint.h>

#define SM_OPERATE_PAIRING_REQ                              0x01
#define SM_OPERATE_PAIRING_RESP                             0x02
#define SM_OPERATE_PAIRING_CONFIRM                          0x03
#define SM_OPERATE_PAIRING_RANDOM                           0x04
#define SM_OPERATE_PAIRING_FAILED                           0x05
#define SM_OPERATE_ENCRYPTION_INFO                          0x06
#define SM_OPERATE_CENTRAL_IDENTIFICATION                   0x07
#define SM_OPERATE_IDENTITY_INFO                            0x08
#define SM_OPERATE_IDENTITY_ADDRESS_INFO                    0x09
#define SM_OPERATE_SIGNING_INFO                             0x0a
#define SM_OPERATE_SECURITY_REQ                             0x0b
#define SM_OPERATE_PAIRING_PUBLIC_KEY                       0x0c
#define SM_OPERATE_PAIRING_DHKEY_CHECK                      0x0d
#define SM_OPERATE_PAIRING_KEYPRESS_NOTIFICATION            0x0e

#define SM_ERROR_PASSKEY_ENTRY_FAILED                       0x01
#define SM_ERROR_OOB_NOT_AVALIABLE                          0x02
#define SM_ERROR_AUTHENTICATION_REQUIREMENT                 0x03
#define SM_ERROR_CONFIRM_VALUE_FAILED                       0x04
#define SM_ERROR_PAIRING_NOT_SUPPORTED                      0x05
#define SM_ERROR_ENCRYPTION_KEY_SIZE                        0x06
#define SM_ERROR_CMD_NOT_SUPPORTED                          0x07
#define SM_ERROR_UNSPECIFIED_REASON                         0x08
#define SM_ERROR_REPEATED_ATTEMPTS                          0x09
#define SM_ERROR_INVALID_PARAMETERS                         0x0a
#define SM_ERROR_DHKEY_CHECK_FAILED                         0x0b
#define SM_ERROR_NUMERIC_COMPARISON_FAILED                  0x0c
#define SM_ERROR_BREDR_PARING_IN_PROGRESS                   0x0d
#define SM_ERROR_TRANSPORT_DERIVATION_NOT_ALLOWED           0x0e
#define SM_ERROR_KEY_REJECTED                               0x0f

#define SM_IOCAP_DISPLAY_ONLY                               0x00
#define SM_IOCAP_DISPLAY_YESNO                              0x01
#define SM_IOCAP_KEYBORAD_ONLY                              0x02
#define SM_IOCAP_NO_INPUT_NO_OUTPUT                         0x03
#define SM_IOCAP_KEYBOARD_DISPLAY                           0x04
#define SM_IOCAP                                            SM_IOCAP_DISPLAY_YESNO

#define SM_OOB_DATA_FLAG_NOT_PRESENT                        0x00
#define SM_OOB_DATA_FLAG_PRESENT                            0x01
#define SM_OOB_DATA_FLAG                                    SM_OOB_DATA_FLAG_NOT_PRESENT

#define SM_AUTH_BONDING                                     0x01
#define SM_AUTH_MITM                                        0x04
#define SM_AUTH_SECURE_CONNECTION                           0x08
#define SM_AUTH_KEYPRESS                                    0x10
#define SM_AUTH                                             SM_AUTH_BONDING | SM_AUTH_MITM | SM_AUTH_SECURE_CONNECTION

#define SM_MIN_ENCRYPT_KEY_SIZE                             7
#define SM_MAX_ENCRYPT_KEY_SIZE                             16
#define SM_ENCRYPT_KEY_SIZE                                 16

#define SM_KEY_DISTRIBUTION_ENCRYPTION                      0x01
#define SM_KEY_DISTRIBUTION_ID                              0x02
#define SM_KEY_DISTRIBUTION_SIGNATURE                       0x04
#define SM_KEY_DISTRIBUTION_LINK                            0x08
#define SM_KEY_DISTRIBUTION                                 SM_KEY_DISTRIBUTION_ENCRYPTION | SM_KEY_DISTRIBUTION_ID


#define SM_LENGTH_HEADER                                    1
#define SM_LENGTH_PAIRING_REQ                               6
#define SM_LENGTH_PAIRING_RESP                              6
#define SM_LENGTH_PAIRING_PUBLIC_KEY                        64
#define SM_LENGTH_PAIRING_CONFIRM                           16
#define SM_LENGTH_PAIRING_RANDOM                            16
#define SM_LENGTH_PAIRING_FAILED                            1
#define SM_LENGTH_DHKEY                                     32
#define SM_LENGTH_DHKEY_CHECK                               16
#define SM_LENGTH_MACKEY                                    16
#define SM_LENGTH_LTK                                       16
#define SM_LENGTH_IOCAP                                     3
#define SM_LENGTH_EDIV                                      2
#define SM_LENGTH_RAND                                      8
#define SM_LENGTH_IRK                                       16
#define SM_LENGTH_ADDR_TYPE                                 1
#define SM_LENGTH_ADDR                                      6
#define SM_LENGTH_SIGNING_INFO                              16



void sm_c1(uint8_t* k, uint8_t* r, uint8_t* preq, uint8_t *pres, uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out_confirm);
void sm_s1(uint8_t* k, uint8_t* r1, uint8_t* r2, uint8_t* out_stk);
void sm_f4(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t z, uint8_t* out_confirm);
void sm_f5(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_mackey, uint8_t* out_ltk);
void sm_f6(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t* r, uint8_t* iocap, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_dhkey_check);
void sm_g2(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t* y, uint32_t* out_passkey);

void sm_recv(uint8_t *data, uint16_t length);
void sm_recv_pairing_req(uint8_t *data, uint16_t length);
void sm_recv_pairing_public_key(uint8_t *data, uint16_t length);
void sm_recv_pairing_random(uint8_t *data, uint16_t length);
void sm_recv_pairing_dhkey_check(uint8_t *data, uint16_t length);
void sm_send_pairing_resp(uint8_t *data);
void sm_send_pairing_public_key(uint8_t *data);
void sm_send_pairing_confirm(uint8_t *data);
void sm_send_pairing_random(uint8_t *data);
void sm_send_pairing_dhkey_check(uint8_t *data);
void sm_send_pairing_failed(uint8_t reason);
void sm_send_encryption_information(uint8_t *data);
void sm_send_central_identification(uint8_t *data);
void sm_send_identity_information(uint8_t *data);
void sm_send_identity_address_information(uint8_t *data);
void sm_send_signing_information(uint8_t *data);
void sm_set_local_pairing_public_key(uint8_t *data);
void sm_set_local_dhkey(uint8_t *data);
void sm_set_local_address(uint8_t *data);
void sm_set_local_address_type(uint8_t type);
void sm_set_remote_address(uint8_t *data);
void sm_set_remote_address_type(uint8_t type);
void sm_set_local_ediv(uint8_t *data);
void sm_set_local_rand(uint8_t *data);
void sm_get_local_ltk(uint8_t *data);
void sm_send(uint8_t *data, uint16_t length);
void sm_get_pairing_method();
void sm_key_distribution();

#endif // SM_H
