#ifndef SM_H
#define SM_H

#include <stdint.h>

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


void sm_recv(uint8_t *data, uint32_t length);
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
void sm_key_distribution();
void sm_send(uint8_t *data, uint32_t length);

#endif // SM_H
