#include <memory>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include "sm.h"
#include "log.h"
#include "hci.h"
#include "l2cap.h"
#include "tinycrypt/include/cmac_mode.h"


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

#define SM_LENGTH_PACKET_HEADER                             (HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_HEADER_ACL + L2CAP_LENGTH_HEADER)


#pragma pack(1)
typedef struct {
    uint8_t remote_addr_type;
    uint8_t remote_addr[HCI_LENGTH_ADDR];
    uint8_t remote_irk[SM_LENGTH_IRK];

    uint8_t local_ltk[HCI_LENGTH_LTK];
    uint8_t local_ediv[SM_LENGTH_EDIV];
    uint8_t local_rand[SM_LENGTH_RAND];

    uint8_t local_csrk[SM_LENGTH_CSRK];
    uint8_t remote_csrk[SM_LENGTH_CSRK];

    uint8_t is_encrypted;
    uint8_t encryption_key_size;
    uint8_t is_authenticated;
    uint8_t is_authorizated;
    uint8_t is_secure_connection;
    uint8_t pairing_method;
} sm_device_db_entry_t;
#pragma pack()


static uint32_t __get_be32(uint8_t* data);
static void __mem_swap_copy(uint8_t* dst, uint8_t* src, uint32_t length);
static void __mem_swap_self(uint8_t* data, uint32_t length);
static void __sm_encrypt(uint8_t* k, uint8_t* plain_data, uint8_t* enc_data);
static void __sm_aes_cmac(uint8_t* k, uint8_t* input, uint32_t length, uint8_t* output);

static void __sm_c1(uint8_t* k, uint8_t* r, uint8_t* preq, uint8_t *pres, uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out_confirm);
static void __sm_s1(uint8_t* k, uint8_t* r1, uint8_t* r2, uint8_t* out_stk);
static void __sm_f4(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t z, uint8_t* out_confirm);
static void __sm_f5(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_mackey, uint8_t* out_ltk);
static void __sm_f6(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t* r, uint8_t* iocap, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_dhkey_check);
static void __sm_g2(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t* y, uint32_t* out_passkey);

static bool __sm_device_db_entry_add(uint8_t remote_addr_type, uint8_t *remote_addr, sm_device_db_entry_t *entry);
static bool __sm_device_db_entry_get(uint8_t remote_addr_type, uint8_t *remote_addr, sm_device_db_entry_t *entry);
static bool __sm_device_db_entry_update(uint8_t remote_addr_type, uint8_t *remote_addr, sm_device_db_entry_t *entry);
static bool __sm_device_db_entry_delete(uint8_t remote_addr_type, uint8_t *remote_addr);

static void __sm_recv_pairing_req(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_encryption_info(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_central_identification(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_identity_info(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_identity_address_info(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_signing_info(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_pairing_public_key(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_pairing_random(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_pairing_dhkey_check(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_recv_pairing_confirm(uint16_t connect_handle, uint8_t *data, uint32_t length);
static void __sm_save_device_info(uint16_t connect_handle);
static void __sm_get_pairing_method(uint16_t connect_handle);


static uint16_t connect_handle_last_send_le_generate_dhkey = 0;


static uint32_t __get_be32(uint8_t* data) {
    uint32_t x = 0;

    x = (uint32_t)data[0] << 24;
    x |= (uint32_t)data[1] << 16;
    x |= (uint32_t)data[2] << 8;
    x |= data[3];

    return x;
}

static void __mem_swap_copy(uint8_t* dst, uint8_t* src, uint32_t length) {
    uint32_t i = 0;

    for (i = 0; i < length; i++) {
        dst[length - 1 - i] = src[i];
    }
}

static void __mem_swap_self(uint8_t* data, uint32_t length) {
    uint8_t tmp = 0;
    uint32_t i = 0, j = 0;

    for (i = 0, j = length - 1; i < j; i++, j--) {
        tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

static void __sm_encrypt(uint8_t* k, uint8_t* plain_data, uint8_t* enc_data) {
    uint8_t tmp[16] = {0};
    struct tc_aes_key_sched_struct s;

    __mem_swap_copy(tmp, k, 16);
    tc_aes128_set_encrypt_key(&s, tmp);
    __mem_swap_copy(tmp, plain_data, 16);
    tc_aes_encrypt(enc_data, tmp, &s);
    __mem_swap_self(enc_data, 16);
}

static void __sm_aes_cmac(uint8_t* k, uint8_t* input, uint32_t length, uint8_t* output) {
    struct tc_aes_key_sched_struct sched;
    struct tc_cmac_struct state;

    tc_cmac_setup(&state, k, &sched);
    tc_cmac_update(&state, input, length);
    tc_cmac_final(output, &state);
}

// k:16B, r:16B, preq:7B, pres:7B, ia:6B, ra:6B, out_confirm:16B
static void __sm_c1(uint8_t* k, uint8_t* r, uint8_t* preq, uint8_t *pres, uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out_confirm) {
    uint8_t p1[16] = {0};
    uint8_t p2[16] = {0};
    uint8_t tmp[16] = {0};
    uint32_t i = 0;

    p1[0] = iat;
    p1[1] = rat;
    memcpy_s(p1 + 2, 7, preq, 7);
    memcpy_s(p1 + 9, 7, pres, 7);

    memcpy_s(p2, 6, ra, 6);
    memcpy_s(p2 + 6, 6, ia, 6);

    for (i = 0; i < 16; i++) {
        tmp[i] = r[i] ^ p1[i];
    }
    __sm_encrypt(k, tmp, tmp);
    for (i = 0; i < 16; i++) {
        tmp[i] ^= p2[i];
    }
    __sm_encrypt(k, tmp, out_confirm);
}

// k:16B, r1:16B, r2:16B, out_stk:16B
static void __sm_s1(uint8_t* k, uint8_t* r1, uint8_t* r2, uint8_t* out_stk) {
    uint8_t tmp[16] = {0};

    memcpy_s(tmp, 8, r2, 8);
    memcpy_s(tmp + 8, 8, r1, 8);

    __sm_encrypt(k, tmp, out_stk);
}

// u:32B, v:32B, x:16B, out_confirm:16B
static void __sm_f4(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t z, uint8_t* out_confirm) {
    uint8_t xs[16] = {0};
    uint8_t m[65] = {0};

    __mem_swap_copy(m, u, 32);
    __mem_swap_copy(m + 32, v, 32);
    m[64] = z;

    __mem_swap_copy(xs, x, 16);

    __sm_aes_cmac(xs, m, sizeof(m), out_confirm);
    __mem_swap_self(out_confirm, 16);
}

// w:32B, n1:16B, n2:16B, a1:6B, a2:6B, out_mackey:16B, out_ltk:16B
static void __sm_f5(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_mackey, uint8_t* out_ltk) {
    uint8_t salt[16] = {0x6c, 0x88, 0x83, 0x91, 0xaa, 0xf5, 0xa5, 0x38, 0x60, 0x37, 0x0b, 0xdb, 0x5a, 0x60, 0x83, 0xbe};
    uint8_t m[53] = {
        0x00, /* counter */
        0x62, 0x74, 0x6c, 0x65, /* keyID */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* n1 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* n2 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a1 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a2 */
        0x01, 0x00 /* length */
    };
    uint8_t ws[32] = {0};
    uint8_t t[16] = {0};

    __mem_swap_copy(ws, w, 32);
    __sm_aes_cmac(salt, ws, sizeof(ws), t);

    __mem_swap_copy(m + 5, n1, 16);
    __mem_swap_copy(m + 21, n2, 16);
    m[37] = at1;
    __mem_swap_copy(m + 38, a1, 6);
    m[44] = at2;
    __mem_swap_copy(m + 45, a2, 6);
    __sm_aes_cmac(t, m, sizeof(m), out_mackey);
    __mem_swap_self(out_mackey, 16);

    m[0] = 0x01;
    __sm_aes_cmac(t, m, sizeof(m), out_ltk);
    __mem_swap_self(out_ltk, 16);
}

// w:16B, n1:16B, n2:16B, r:16B, iocap:3B, a1:6B, a2:6B, out_dhkey_check:16B
static void __sm_f6(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t* r, uint8_t* iocap, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_dhkey_check) {
    uint8_t ws[16] = {0};
    uint8_t m[65] = {0};

    __mem_swap_copy(m, n1, 16);
    __mem_swap_copy(m + 16, n2, 16);
    __mem_swap_copy(m + 32, r, 16);
    __mem_swap_copy(m + 48, iocap, 3);
    m[51] = at1;
    memcpy_s(m + 52, 6, a1, 6);
    __mem_swap_copy(m + 52, a1, 6);
    m[58] = at2;
    memcpy_s(m + 59, 6, a2, 6);
    __mem_swap_copy(m + 59, a2, 6);

    __mem_swap_copy(ws, w, 16);

    __sm_aes_cmac(ws, m, sizeof(m), out_dhkey_check);
    __mem_swap_self(out_dhkey_check, 16);
}

// u:32B, v:32B, x:16B, y:16B, out_passkey:4B
static void __sm_g2(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t* y, uint32_t* out_passkey) {
    uint8_t m[80] = {0};
    uint8_t xs[16] = {0};

    __mem_swap_copy(m, u, 32);
    __mem_swap_copy(m + 32, v, 32);
    __mem_swap_copy(m + 64, y, 16);

    __mem_swap_copy(xs, x, 16);

    __sm_aes_cmac(xs, m, sizeof(m), xs);
    *out_passkey = __get_be32(xs + 12) % 1000000;
}

static bool __sm_device_db_entry_add(uint8_t remote_addr_type, uint8_t *remote_addr, sm_device_db_entry_t *entry) {
    bool result = true;
    QFile file(SM_DEVICE_DB_FILE_NAME);
    sm_device_db_entry_t read_entry = {0};
    uint32_t offset = 0;
    uint8_t calc_hash[3] = {0};

    file.open(QIODevice::ReadWrite);
    while (!file.atEnd()) {
        file.read((char*)&read_entry, sizeof(sm_device_db_entry_t));

        if (read_entry.remote_addr_type == remote_addr_type && (!memcmp(read_entry.remote_addr, remote_addr, HCI_LENGTH_ADDR))) {
                result = false;
                goto RET;
        }

        if (HCI_ADDR_TYPE_RANDOM == remote_addr_type) {
            if (HCI_RANDOM_ADDR_SUBTYPE_RESOLVABLE == (remote_addr[5] >> 6)) { // [47:46] = 0b01
                sm_ah(read_entry.remote_irk, remote_addr + 3, calc_hash);
                if (!memcmp(calc_hash, remote_addr, 3)) {
                    result = false;
                    goto RET;
                }
            }
        }

        offset += sizeof(sm_device_db_entry_t);
    }

RET:
    if (result) {
        file.seek(offset);
        file.write((char*)entry, sizeof(sm_device_db_entry_t));
    }
    file.close();

    return result;
}

static bool __sm_device_db_entry_get(uint8_t remote_addr_type, uint8_t *remote_addr, sm_device_db_entry_t *entry) {
    bool result = false;
    QFile file(SM_DEVICE_DB_FILE_NAME);
    sm_device_db_entry_t read_entry = {0};
    uint8_t calc_hash[3] = {0};

    file.open(QIODevice::ReadOnly);
    while (!file.atEnd()) {
        file.read((char*)&read_entry, sizeof(sm_device_db_entry_t));

        if (read_entry.remote_addr_type == remote_addr_type && (!memcmp(read_entry.remote_addr, remote_addr, HCI_LENGTH_ADDR))) {
            result = true;
            goto RET;
        }

        if (HCI_ADDR_TYPE_RANDOM == remote_addr_type) {
            if (HCI_RANDOM_ADDR_SUBTYPE_RESOLVABLE == (remote_addr[5] >> 6)) {
                sm_ah(read_entry.remote_irk, remote_addr + 3, calc_hash);
                if (!memcmp(calc_hash, remote_addr, 3)) {
                    result = true;
                    goto RET;
                }
            }
        }
    }

RET:
    if (result) {
        memcpy_s(entry, sizeof(sm_device_db_entry_t), &read_entry, sizeof(sm_device_db_entry_t));
    }
    file.close();

    return result;
}

static bool __sm_device_db_entry_update(uint8_t remote_addr_type, uint8_t *remote_addr, sm_device_db_entry_t *entry) {
    bool result = false;
    QFile file(SM_DEVICE_DB_FILE_NAME);
    sm_device_db_entry_t read_entry = {0};
    uint32_t offset = 0;
    uint8_t calc_hash[3] = {0};

    file.open(QIODevice::ReadWrite);
    while (!file.atEnd()) {
        file.read((char*)&read_entry, sizeof(sm_device_db_entry_t));

        if (read_entry.remote_addr_type == remote_addr_type && (!memcmp(read_entry.remote_addr, remote_addr, HCI_LENGTH_ADDR))) {
            result = true;
            goto RET;
        }

        if (HCI_ADDR_TYPE_RANDOM == remote_addr_type) {
            if (HCI_RANDOM_ADDR_SUBTYPE_RESOLVABLE == (remote_addr[5] >> 6)) {
                sm_ah(read_entry.remote_irk, remote_addr + 3, calc_hash);
                if (!memcmp(calc_hash, remote_addr, 3)) {
                    result = true;
                    goto RET;
                }
            }
        }

        offset += sizeof(sm_device_db_entry_t);
    }

RET:
    if (result) {
        file.seek(offset);
        file.write((char*)entry, sizeof(sm_device_db_entry_t));
    }
    file.close();

    return result;
}

static bool __sm_device_db_entry_delete(uint8_t remote_addr_type, uint8_t *remote_addr) {
    bool result = false;
    QFile file(SM_DEVICE_DB_FILE_NAME);
    sm_device_db_entry_t read_entry = {0};
    uint32_t offset = 0;
    uint8_t calc_hash[3] = {0};

    file.open(QIODevice::ReadWrite);
    while (!file.atEnd()) {
        file.read((char*)&read_entry, sizeof(sm_device_db_entry_t));

        if (read_entry.remote_addr_type == remote_addr_type && (!memcmp(read_entry.remote_addr, remote_addr, HCI_LENGTH_ADDR))) {
            result = true;
            goto RET;
        }

        if (HCI_ADDR_TYPE_RANDOM == remote_addr_type) {
            if (HCI_RANDOM_ADDR_SUBTYPE_RESOLVABLE == (remote_addr[5] >> 6)) {
                sm_ah(read_entry.remote_irk, remote_addr + 3, calc_hash);
                if (!memcmp(calc_hash, remote_addr, 3)) {
                    result = true;
                    goto RET;
                }
            }
        }

        offset += sizeof(sm_device_db_entry_t);
    }

RET:
    if (result) {
        file.seek(offset);
        memset(&read_entry, 0, sizeof(sm_device_db_entry_t));
        file.write((char*)&read_entry, sizeof(sm_device_db_entry_t));
    }
    file.close();

    return result;
}

// k:16B, r:3B, out_hash:3B
void sm_ah(uint8_t* k, uint8_t* r, uint8_t* out_hash) {
    uint8_t tmp[16] = {0};
    uint8_t output[16] = {0};

    memcpy_s(tmp, 3, r, 3);
    __sm_encrypt(k, tmp, output);
    memcpy_s(out_hash, 3, output, 3);
}

void sm_generate_random(uint8_t* data, uint32_t length) {
    uint32_t index = 0;

    for (index = 0; index < length / 2; index++) {
        *(uint16_t *)(data + index * 2) = rand();
    }

    if (length % 2) {
        data[length - 1] = rand();
    }
}


static void __sm_get_pairing_method(uint16_t connect_handle) {
    sm_pairing_method_t legacy_pairing_method_table[5][5] = {
        {JUST_WORKS, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY},
        {JUST_WORKS, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY},
        {PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_INPUT_R_INPUT, JUST_WORKS, PASSKEY_I_DISPLAY_R_INPUT},
        {JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS},
        {PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_INPUT_R_DISPLAY,JUST_WORKS, PASSKEY_I_DISPLAY_R_INPUT},
    };
    sm_pairing_method_t secure_connection_pairing_method_table[5][5] = {
        {JUST_WORKS, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY},
        {JUST_WORKS, NUMERIC_COMPARISON, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, NUMERIC_COMPARISON},
        {PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_INPUT_R_INPUT, JUST_WORKS, PASSKEY_I_DISPLAY_R_INPUT},
        {JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS},
        {PASSKEY_I_DISPLAY_R_INPUT, NUMERIC_COMPARISON, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, NUMERIC_COMPARISON},
    };
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    sm_connection.pairing_method = JUST_WORKS;
    sm_connection.is_secure_connection = (sm_connection.pairing_req[3] & SM_AUTH_SECURE_CONNECTION) && (sm_connection.pairing_resp[3] & SM_AUTH_SECURE_CONNECTION);
    LOG_INFO("is_secure_connection:0x%02x", sm_connection.is_secure_connection);

    if (sm_connection.is_secure_connection) {
        if (sm_connection.pairing_req[2] || sm_connection.pairing_resp[2]) {
            sm_connection.pairing_method = OOB;
            goto RET;
        }
    } else {
        if (sm_connection.pairing_req[2] && sm_connection.pairing_resp[2]) {
            sm_connection.pairing_method = OOB;
            goto RET;
        }
    }

    if ((sm_connection.pairing_req[3] & SM_AUTH_MITM) || (sm_connection.pairing_resp[3] & SM_AUTH_MITM)) {
        if (sm_connection.is_secure_connection) {
            sm_connection.pairing_method = secure_connection_pairing_method_table[sm_connection.pairing_resp[1]][sm_connection.pairing_req[1]];
        } else {
            sm_connection.pairing_method = legacy_pairing_method_table[sm_connection.pairing_resp[1]][sm_connection.pairing_req[1]];
        }
        goto RET;
    }

RET:
    if (JUST_WORKS == sm_connection.pairing_method) {
        LOG_INFO("pairing_method:just_works");
    } else if (PASSKEY_I_INPUT_R_DISPLAY == sm_connection.pairing_method) {
        LOG_INFO("pairing_method:passkey_i_input_r_display");
    } else if (PASSKEY_I_DISPLAY_R_INPUT == sm_connection.pairing_method) {
        LOG_INFO("pairing_method:passkey_i_display_r_input");
    } else if (PASSKEY_I_INPUT_R_INPUT == sm_connection.pairing_method) {
        LOG_INFO("pairing_method:passkey_i_input_r_input");
    } else if (NUMERIC_COMPARISON == sm_connection.pairing_method) {
        LOG_INFO("pairing_method:numeric_comparison");
    } else {
    }
}

void sm_recv(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    uint8_t op_code = data[0];

    switch (op_code) {
    case SM_OPERATE_PAIRING_REQ:
        __sm_recv_pairing_req(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_ENCRYPTION_INFO:
        __sm_recv_encryption_info(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_CENTRAL_IDENTIFICATION:
        __sm_recv_central_identification(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_IDENTITY_INFO:
        __sm_recv_identity_info(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_IDENTITY_ADDRESS_INFO:
        __sm_recv_identity_address_info(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_SIGNING_INFO:
        __sm_recv_signing_info(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_PUBLIC_KEY:
        __sm_recv_pairing_public_key(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_RANDOM:
        __sm_recv_pairing_random(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_DHKEY_CHECK:
        __sm_recv_pairing_dhkey_check(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_CONFIRM:
        __sm_recv_pairing_confirm(connect_handle, data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    default:
        LOG_WARNING("sm_recv invalid, op_code:0x%02x", op_code); break;
    }
}

// step 1
static void __sm_recv_pairing_req(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    sm_connection.pairing_req[0] = SM_OPERATE_PAIRING_REQ;
    memcpy_s(sm_connection.pairing_req + 1, SM_LENGTH_PAIRING_REQ, data, SM_LENGTH_PAIRING_REQ);

    sm_connection.pairing_resp[0] = SM_OPERATE_PAIRING_RESP;
    sm_connection.pairing_resp[1] = SM_IOCAP;
    sm_connection.pairing_resp[2] = SM_OOB_DATA_FLAG;
    sm_connection.pairing_resp[3] = SM_AUTH;
    sm_connection.pairing_resp[4] = SM_MAX_ENCRYPT_KEY_SIZE;
    sm_connection.pairing_resp[5] = SM_INITIATOR_KEY_DISTRIBUTION;
    sm_connection.pairing_resp[6] = SM_RESPONDER_KEY_DISTRIBUTION;

    LOG_INFO("pairing_req iocap:0x%02x, oob_flag:0x%02x, auth_req:0x%02x, max_encrypt_key_size:%u, i_key_distribution:0x%02x, r_key_distribution:0x%02x",
             sm_connection.pairing_req[1], sm_connection.pairing_req[2], sm_connection.pairing_req[3],
             sm_connection.pairing_req[4], sm_connection.pairing_req[5], sm_connection.pairing_req[6]);
    LOG_INFO("pairing_resp iocap:0x%02x, oob_flag:0x%02x, auth_req:0x%02x, max_encrypt_key_size:%u, i_key_distribution:0x%02x, r_key_distribution:0x%02x",
             sm_connection.pairing_resp[1], sm_connection.pairing_resp[2], sm_connection.pairing_resp[3],
             sm_connection.pairing_resp[4], sm_connection.pairing_resp[5], sm_connection.pairing_resp[6]);

    sm_send_pairing_resp(connect_handle, sm_connection.pairing_resp + 1);

    __sm_get_pairing_method(connect_handle);
    if (!sm_connection.is_secure_connection) {
        if (PASSKEY_I_INPUT_R_DISPLAY == sm_connection.pairing_method) {
            sm_generate_random((uint8_t *)&sm_connection.passkey_num, sizeof(sm_connection.passkey_num));
            sm_connection.passkey_num %= 1000000;
            memset(sm_connection.local_tk, 0, SM_LENGTH_TK);
            sm_connection.local_tk[0] = (uint8_t)sm_connection.passkey_num;
            sm_connection.local_tk[1] = (uint8_t)(sm_connection.passkey_num >> 8);
            sm_connection.local_tk[2] = (uint8_t)(sm_connection.passkey_num >> 16);
            memcpy_s(sm_connection.remote_tk, SM_LENGTH_TK, sm_connection.local_tk, SM_LENGTH_TK);
            LOG_INFO("passkey:%06u, please input on peer device", sm_connection.passkey_num);
        }
    }
}

// step 2
static void __sm_recv_pairing_public_key(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    memcpy_s(sm_connection.remote_pairing_public_key, HCI_LENGTH_P256_PUBLIC_KEY, data, HCI_LENGTH_P256_PUBLIC_KEY);
    connect_handle_last_send_le_generate_dhkey = connect_handle;
    hci_send_cmd_le_generate_dhkey(sm_connection.remote_pairing_public_key);
}

// step 3
void sm_recv_evt_le_generate_dhkey_complete(uint8_t *dhkey) {
    uint16_t connect_handle = connect_handle_last_send_le_generate_dhkey;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;
    uint8_t *local_pairing_public_key = hci_get_local_p256_public_key();
    uint8_t local_pairing_confirm[SM_LENGTH_PAIRING_CONFIRM] = {0};
    bool pressed_ok = false;

    memcpy_s(sm_connection.local_dhkey, SM_LENGTH_DHKEY, dhkey, SM_LENGTH_DHKEY);
    sm_send_pairing_public_key(connect_handle, local_pairing_public_key);

    if ((JUST_WORKS == sm_connection.pairing_method) || (NUMERIC_COMPARISON == sm_connection.pairing_method)) {
        memset(sm_connection.local_tk, 0, SM_LENGTH_TK); // for just_work or numeric_comparison, set ra and rb to 0
        memset(sm_connection.remote_tk, 0, SM_LENGTH_TK);

        sm_generate_random(sm_connection.local_random, SM_LENGTH_PAIRING_RANDOM);
        __sm_f4(local_pairing_public_key, sm_connection.remote_pairing_public_key, sm_connection.local_random, 0, local_pairing_confirm);
        sm_send_pairing_confirm(connect_handle, local_pairing_confirm);
    } else if (PASSKEY_I_INPUT_R_DISPLAY == sm_connection.pairing_method) {
        sm_generate_random((uint8_t *)&sm_connection.passkey_num, sizeof(sm_connection.passkey_num));
        sm_connection.passkey_num %= 1000000;
        sm_connection.passkey_index = 0;
        memset(sm_connection.local_tk, 0, SM_LENGTH_TK); // for passkey, inject ra and rb
        sm_connection.local_tk[0] = (uint8_t)sm_connection.passkey_num;
        sm_connection.local_tk[1] = (uint8_t)(sm_connection.passkey_num >> 8);
        sm_connection.local_tk[2] = (uint8_t)(sm_connection.passkey_num >> 16);
        memcpy_s(sm_connection.remote_tk, SM_LENGTH_TK, sm_connection.local_tk, SM_LENGTH_TK);
        LOG_INFO("passkey:%06u, please input on peer device", sm_connection.passkey_num);
    } else if (PASSKEY_I_DISPLAY_R_INPUT == sm_connection.pairing_method) {
        sm_send_keypress_notification(connect_handle, KEYPRESS_ENTRY_STARTED);
        sm_connection.passkey_num = QInputDialog::getInt(nullptr, "PASSKEY_R_INPUT", "please input peer device num", 0, 0, 999999, 1, &pressed_ok);
        if (pressed_ok) {
            sm_connection.passkey_index = 0;
            memset(sm_connection.local_tk, 0, SM_LENGTH_TK); // for passkey, inject ra and rb
            sm_connection.local_tk[0] = (uint8_t)sm_connection.passkey_num;
            sm_connection.local_tk[1] = (uint8_t)(sm_connection.passkey_num >> 8);
            sm_connection.local_tk[2] = (uint8_t)(sm_connection.passkey_num >> 16);
            memcpy_s(sm_connection.remote_tk, SM_LENGTH_TK, sm_connection.local_tk, SM_LENGTH_TK);
            sm_send_keypress_notification(connect_handle, KEYPRESS_ENTRY_COMPLETED); // trigger initor start to send pairing confirm(20 times)
        } else {
            sm_send_pairing_failed(connect_handle, SM_ERROR_PASSKEY_ENTRY_FAILED);
        }
    } else {
        // TODO: other methods
        LOG_ERROR("pairing method not supported");
    }
}

// step 4.2.1(PASSKEY), step 2(legacy JUST_WORKS, PASSKEY)
static void __sm_recv_pairing_confirm(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    hci_connection_t *hci_connection = hci_find_connection_by_handle(connect_handle);
    sm_connection_t& sm_connection = hci_connection->sm_connection;
    uint8_t *local_pairing_public_key = hci_get_local_p256_public_key();
    uint8_t local_pairing_confirm[SM_LENGTH_PAIRING_CONFIRM] = {0};
    hci_addr_type_t local_addr_type = HCI_ADDR_TYPE_RANDOM;
    uint8_t *local_addr_random = hci_get_local_addr_random();
    bool pressed_ok = false;

    memcpy_s(sm_connection.remote_pairing_confirm, SM_LENGTH_PAIRING_CONFIRM, data, SM_LENGTH_PAIRING_CONFIRM);
    sm_generate_random(sm_connection.local_random, SM_LENGTH_PAIRING_RANDOM);
    if (sm_connection.is_secure_connection) {
        if ((sm_connection.passkey_num >> sm_connection.passkey_index) & 0x01) {
            sm_connection.passkey_r = 0x81; // why add 0x80?
        } else {
            sm_connection.passkey_r = 0x80;
        }
        __sm_f4(local_pairing_public_key, sm_connection.remote_pairing_public_key, sm_connection.local_random, sm_connection.passkey_r, local_pairing_confirm);
    } else {
        if (JUST_WORKS == sm_connection.pairing_method) {
            memset(sm_connection.local_tk, 0, SM_LENGTH_TK); // for legacy just_work, set local_tk and remote_tk to 0
            memset(sm_connection.remote_tk, 0, SM_LENGTH_TK);
        } else if (PASSKEY_I_INPUT_R_DISPLAY == sm_connection.pairing_method) {
            // local_tk and remote_tk generated during recv pairing req
        } else if (PASSKEY_I_DISPLAY_R_INPUT == sm_connection.pairing_method) {
            sm_send_keypress_notification(connect_handle, KEYPRESS_ENTRY_STARTED);
            sm_connection.passkey_num = QInputDialog::getInt(nullptr, "PASSKEY_R_INPUT", "please input peer device num", 0, 0, 999999, 1, &pressed_ok);
            if (pressed_ok) {
                memset(sm_connection.local_tk, 0, SM_LENGTH_TK);
                sm_connection.local_tk[0] = (uint8_t)sm_connection.passkey_num;
                sm_connection.local_tk[1] = (uint8_t)(sm_connection.passkey_num >> 8);
                sm_connection.local_tk[2] = (uint8_t)(sm_connection.passkey_num >> 16);
                memcpy_s(sm_connection.remote_tk, SM_LENGTH_TK, sm_connection.local_tk, SM_LENGTH_TK);
                sm_send_keypress_notification(connect_handle, KEYPRESS_ENTRY_COMPLETED);
            } else {
                sm_send_pairing_failed(connect_handle, SM_ERROR_PASSKEY_ENTRY_FAILED);
                return;
            }
        }
        __sm_c1(sm_connection.local_tk, sm_connection.local_random, sm_connection.pairing_req, sm_connection.pairing_resp,
                hci_connection->peer_addr_type, hci_connection->peer_addr, local_addr_type, local_addr_random, local_pairing_confirm);
    }
    sm_send_pairing_confirm(connect_handle, local_pairing_confirm);
}

// step 4.1(JUST_WORKS or NUMERIC_COMPARISON) 4.2.2(PASSKEY) step 3(legacy JUST_WORKS, PASSKEY)
static void __sm_recv_pairing_random(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    hci_connection_t *hci_connection = hci_find_connection_by_handle(connect_handle);
    sm_connection_t& sm_connection = hci_connection->sm_connection;
    uint32_t local_v = 0;
    uint8_t *local_pairing_public_key = hci_get_local_p256_public_key();
    uint8_t calc_pairing_confirm[SM_LENGTH_PAIRING_CONFIRM] = {0};
    char message_info[128] = {0};
    uint32_t press_button_ret = 0;
    hci_addr_type_t local_addr_type = HCI_ADDR_TYPE_RANDOM;
    uint8_t *local_addr_random = hci_get_local_addr_random();
    sm_device_db_entry_t entry = {0};

    memcpy_s(sm_connection.remote_random, SM_LENGTH_PAIRING_RANDOM, data, SM_LENGTH_PAIRING_RANDOM);
    if (sm_connection.is_secure_connection) {
        if (JUST_WORKS == sm_connection.pairing_method) {
            sm_send_pairing_random(connect_handle, sm_connection.local_random);
        } else if (NUMERIC_COMPARISON == sm_connection.pairing_method) {
            sm_send_pairing_random(connect_handle, sm_connection.local_random);
            __sm_g2(sm_connection.remote_pairing_public_key, local_pairing_public_key, sm_connection.remote_random, sm_connection.local_random, &local_v);
            sprintf_s(message_info, sizeof(message_info), "%06u\nplease check num with peer device", local_v);
            press_button_ret = QMessageBox::question(nullptr, "NUMERIC_COMPARISON", message_info, QMessageBox::Yes | QMessageBox::No);
            if (QMessageBox::No == press_button_ret) {
                sm_send_pairing_failed(connect_handle, SM_ERROR_NUMERIC_COMPARISON_FAILED);
            }
        } else if ((PASSKEY_I_INPUT_R_DISPLAY == sm_connection.pairing_method) || (PASSKEY_I_DISPLAY_R_INPUT == sm_connection.pairing_method)) {
            __sm_f4(sm_connection.remote_pairing_public_key, local_pairing_public_key, sm_connection.remote_random, sm_connection.passkey_r, calc_pairing_confirm);
            if (!memcmp(calc_pairing_confirm, sm_connection.remote_pairing_confirm, SM_LENGTH_PAIRING_CONFIRM)) {
                sm_send_pairing_random(connect_handle, sm_connection.local_random);
                sm_connection.passkey_index++;
            } else {
                LOG_ERROR("confirm_check error, pairing failed");
                sm_send_pairing_failed(connect_handle, SM_ERROR_CONFIRM_VALUE_FAILED);
            }
        }
    } else {
        __sm_c1(sm_connection.remote_tk, sm_connection.remote_random, sm_connection.pairing_req, sm_connection.pairing_resp,
                hci_connection->peer_addr_type, hci_connection->peer_addr, local_addr_type, local_addr_random, calc_pairing_confirm);
        if (!memcmp(calc_pairing_confirm, sm_connection.remote_pairing_confirm, SM_LENGTH_PAIRING_CONFIRM)) {
            sm_send_pairing_random(connect_handle, sm_connection.local_random);

            __sm_s1(sm_connection.local_tk, sm_connection.local_random, sm_connection.remote_random, sm_connection.local_ltk);
            sm_generate_random(sm_connection.local_ediv, SM_LENGTH_EDIV);
            sm_generate_random(sm_connection.local_rand, SM_LENGTH_RAND);
            sm_generate_random(sm_connection.local_csrk, SM_LENGTH_CSRK);

            entry.remote_addr_type = hci_connection->peer_addr_type;
            memcpy_s(entry.remote_addr, HCI_LENGTH_ADDR, hci_connection->peer_addr, HCI_LENGTH_ADDR);
            memcpy_s(entry.local_ltk, SM_LENGTH_LTK, sm_connection.local_ltk, SM_LENGTH_LTK);
            memcpy_s(entry.local_ediv, SM_LENGTH_EDIV, sm_connection.local_ediv, SM_LENGTH_EDIV);
            memcpy_s(entry.local_rand, SM_LENGTH_RAND, sm_connection.local_rand, SM_LENGTH_RAND);
            memcpy_s(entry.local_csrk, SM_LENGTH_CSRK, sm_connection.local_csrk, SM_LENGTH_CSRK);
            entry.is_secure_connection = sm_connection.is_secure_connection;
            entry.pairing_method = (uint8_t)sm_connection.pairing_method;
            if (!__sm_device_db_entry_add(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry)) {
                __sm_device_db_entry_update(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry);
            }
        } else {
            LOG_ERROR("confirm_check error, pairing failed");
            sm_send_pairing_failed(connect_handle, SM_ERROR_CONFIRM_VALUE_FAILED);
        }
    }
}

// step 5
static void __sm_recv_pairing_dhkey_check(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    hci_connection_t *hci_connection = hci_find_connection_by_handle(connect_handle);
    sm_connection_t& sm_connection = hci_connection->sm_connection;
    uint8_t calc_dhkey_check[SM_LENGTH_DHKEY_CHECK] = {0};
    uint8_t local_dhkey_check[SM_LENGTH_DHKEY_CHECK] = {0};
    hci_addr_type_t local_addr_type = HCI_ADDR_TYPE_RANDOM;
    uint8_t *local_addr_random = hci_get_local_addr_random();
    uint8_t ltk[SM_LENGTH_LTK] = {0};
    uint8_t mackey[SM_LENGTH_MACKEY] = {0};
    uint8_t local_iocap[SM_LENGTH_IOCAP] = {0};
    uint8_t remote_iocap[SM_LENGTH_IOCAP] = {0};
    sm_device_db_entry_t entry = {0};

    local_iocap[0] = sm_connection.pairing_resp[1];
    local_iocap[1] = sm_connection.pairing_resp[2];
    local_iocap[2] = sm_connection.pairing_resp[3];
    remote_iocap[0] = sm_connection.pairing_req[1];
    remote_iocap[1] = sm_connection.pairing_req[2];
    remote_iocap[2] = sm_connection.pairing_req[3];

    __sm_f5(sm_connection.local_dhkey, sm_connection.remote_random, sm_connection.local_random, hci_connection->peer_addr_type, hci_connection->peer_addr,
            (uint8_t)local_addr_type, local_addr_random, mackey, ltk);
    __sm_f6(mackey, sm_connection.remote_random, sm_connection.local_random, sm_connection.local_tk, remote_iocap,
            hci_connection->peer_addr_type, hci_connection->peer_addr, (uint8_t)local_addr_type, local_addr_random, calc_dhkey_check);

    if (memcmp(data, calc_dhkey_check, SM_LENGTH_DHKEY_CHECK)) {
        LOG_ERROR("dhkey_check error, pairing failed");
        sm_send_pairing_failed(connect_handle, SM_ERROR_DHKEY_CHECK_FAILED);
    } else {
        __sm_f6(mackey, sm_connection.local_random, sm_connection.remote_random, sm_connection.remote_tk, local_iocap, (uint8_t)local_addr_type, local_addr_random,
                hci_connection->peer_addr_type, hci_connection->peer_addr, local_dhkey_check);
        sm_send_pairing_dhkey_check(connect_handle, local_dhkey_check);

        memcpy_s(sm_connection.local_ltk, SM_LENGTH_LTK, ltk, SM_LENGTH_LTK);
        if (sm_connection.is_secure_connection) {
            memset(sm_connection.local_ediv, 0, SM_LENGTH_EDIV); // for le secure connection, set to 0
            memset(sm_connection.local_rand, 0, SM_LENGTH_RAND);
        } else {
            // TODO: for legacy pairing
        }
        sm_generate_random(sm_connection.local_csrk, SM_LENGTH_CSRK);

        entry.remote_addr_type = hci_connection->peer_addr_type;
        memcpy_s(entry.remote_addr, HCI_LENGTH_ADDR, hci_connection->peer_addr, HCI_LENGTH_ADDR);
        memcpy_s(entry.local_ltk, SM_LENGTH_LTK, sm_connection.local_ltk, SM_LENGTH_LTK);
        memcpy_s(entry.local_ediv, SM_LENGTH_EDIV, sm_connection.local_ediv, SM_LENGTH_EDIV);
        memcpy_s(entry.local_rand, SM_LENGTH_RAND, sm_connection.local_rand, SM_LENGTH_RAND);
        memcpy_s(entry.local_csrk, SM_LENGTH_CSRK, sm_connection.local_csrk, SM_LENGTH_CSRK);
        entry.is_secure_connection = sm_connection.is_secure_connection;
        entry.pairing_method = (uint8_t)sm_connection.pairing_method;
        if (!__sm_device_db_entry_add(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry)) {
            __sm_device_db_entry_update(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry);
        }
    }
}

// step 6
void sm_recv_evt_le_ltk_req(uint16_t connect_handle, uint8_t *random_number, uint8_t *encrypted_diversifier) {
    (void)random_number;
    (void)encrypted_diversifier;
    hci_connection_t *hci_connection = hci_find_connection_by_handle(connect_handle);
    sm_device_db_entry_t entry = {0};

    if (__sm_device_db_entry_get(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry)) { // TODO: find also match ediv and rand
        hci_send_cmd_le_ltk_req_reply(connect_handle, entry.local_ltk);
    } else {
        LOG_ERROR("device get db error, remote_addr_type:0x%02x, remote_addr:%02x:%02x:%02x:%02x:%02x:%02x", hci_connection->peer_addr_type,
                  hci_connection->peer_addr[0], hci_connection->peer_addr[1], hci_connection->peer_addr[2],
                  hci_connection->peer_addr[3], hci_connection->peer_addr[4], hci_connection->peer_addr[5]);
        hci_send_cmd_le_ltk_req_neg_reply(connect_handle);
    }
}

// step 7
void sm_recv_evt_encryption_change(uint16_t connect_handle, uint8_t encryption_enabled) {
    hci_connection_t *hci_connection = hci_find_connection_by_handle(connect_handle);
    sm_connection_t& sm_connection = hci_connection->sm_connection;
    uint8_t central_id[SM_LENGTH_EDIV + SM_LENGTH_RAND] = {0};
    hci_addr_type_t local_addr_type = HCI_ADDR_TYPE_PUBLIC;
    uint8_t *local_addr_public = hci_get_local_addr_public();
    uint8_t addr_info[HCI_LENGTH_ADDR_TYPE + HCI_LENGTH_ADDR] = {0};
    uint8_t *local_random_irk = hci_get_local_addr_random_irk();
    uint8_t responder_key_distribution = sm_connection.pairing_req[6];
    sm_device_db_entry_t entry = {0};

    if (__sm_device_db_entry_get(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry)) {
        if (encryption_enabled == 1) {
            sm_connection.is_encrypted = 1;
            if (entry.pairing_method != JUST_WORKS) {
                sm_connection.is_authenticated = 1;
            } else {
                sm_connection.is_authenticated = 0;
            }

            if ((responder_key_distribution & SM_KEY_DISTRIBUTION_ENC) != 0) {
                sm_send_encryption_information(connect_handle, entry.local_ltk);

                memcpy_s(central_id, SM_LENGTH_EDIV, entry.local_ediv, SM_LENGTH_EDIV);
                memcpy_s(central_id + SM_LENGTH_EDIV, SM_LENGTH_RAND, entry.local_rand, SM_LENGTH_RAND);
                sm_send_central_identification(connect_handle, central_id);
            }

            if ((responder_key_distribution & SM_KEY_DISTRIBUTION_ID) != 0) {
                sm_send_identity_information(connect_handle, local_random_irk);

                addr_info[0] = (uint8_t)local_addr_type;
                memcpy_s(addr_info + HCI_LENGTH_ADDR_TYPE, HCI_LENGTH_ADDR, local_addr_public, HCI_LENGTH_ADDR);
                sm_send_identity_address_information(connect_handle, addr_info);
            }

            if ((responder_key_distribution & SM_KEY_DISTRIBUTION_SIGN) != 0) {
                sm_send_signing_information(connect_handle, entry.local_csrk);
            }

            if ((entry.is_encrypted != sm_connection.is_encrypted) || (entry.is_authenticated != sm_connection.is_authenticated)) {
                entry.is_encrypted = sm_connection.is_encrypted;
                entry.is_authenticated = sm_connection.is_authenticated;
                __sm_device_db_entry_update(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry);
            }
        } else {
            sm_connection.is_encrypted = 0;
            sm_connection.is_authenticated = 0;
            __sm_device_db_entry_delete(hci_connection->peer_addr_type, hci_connection->peer_addr);
        }
    } else {
        LOG_ERROR("device get db error, remote_addr_type:0x%02x, remote_addr:%02x:%02x:%02x:%02x:%02x:%02x", hci_connection->peer_addr_type,
                  hci_connection->peer_addr[0], hci_connection->peer_addr[1], hci_connection->peer_addr[2],
                  hci_connection->peer_addr[3], hci_connection->peer_addr[4], hci_connection->peer_addr[5]);
    }
}

// step 8
static void __sm_recv_encryption_info(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    memcpy_s(sm_connection.remote_ltk, HCI_LENGTH_LTK, data, HCI_LENGTH_LTK);
    sm_connection.is_received_key_distribution_ltk = 1;
    __sm_save_device_info(connect_handle);
}

static void __sm_recv_central_identification(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    memcpy_s(sm_connection.remote_ediv, SM_LENGTH_EDIV, data, SM_LENGTH_EDIV);
    memcpy_s(sm_connection.remote_rand, SM_LENGTH_RAND, data + SM_LENGTH_EDIV, SM_LENGTH_RAND);
    sm_connection.is_received_key_distribution_id = 1;
    __sm_save_device_info(connect_handle);
}

static void __sm_recv_identity_info(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    memcpy_s(sm_connection.remote_irk, SM_LENGTH_IRK, data, SM_LENGTH_IRK);
    sm_connection.is_received_key_distribution_irk = 1;
    __sm_save_device_info(connect_handle);
}

static void __sm_recv_identity_address_info(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    // maybe not same with hci, for example, peer use random address for hci connection and use public address after sm
    sm_connection.remote_addr_type = data[0];
    memcpy_s(sm_connection.remote_addr, HCI_LENGTH_ADDR, data + HCI_LENGTH_ADDR_TYPE, HCI_LENGTH_ADDR);
    sm_connection.is_received_key_distribution_addr = 1;
    __sm_save_device_info(connect_handle);
}

static void __sm_recv_signing_info(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    (void)length;
    sm_connection_t& sm_connection = hci_find_connection_by_handle(connect_handle)->sm_connection;

    memcpy_s(sm_connection.remote_csrk, SM_LENGTH_CSRK, data, SM_LENGTH_CSRK);
    sm_connection.is_received_key_distribution_csrk = 1;
    __sm_save_device_info(connect_handle);
}

// step 9
static void __sm_save_device_info(uint16_t connect_handle) {
    hci_connection_t *hci_connection = hci_find_connection_by_handle(connect_handle);
    sm_connection_t& sm_connection = hci_connection->sm_connection;
    uint8_t initiator_key_distribution = sm_connection.pairing_resp[5];
    sm_device_db_entry_t entry = {0};

    if (initiator_key_distribution & SM_KEY_DISTRIBUTION_ENC) {
        if ((sm_connection.is_received_key_distribution_ltk == 0) || (sm_connection.is_received_key_distribution_id == 0)) {
            return;
        }
    }

    if (initiator_key_distribution & SM_KEY_DISTRIBUTION_ID) {
        if ((sm_connection.is_received_key_distribution_irk == 0) || (sm_connection.is_received_key_distribution_addr == 0)) {
            return;
        }
    }

    if (initiator_key_distribution & SM_KEY_DISTRIBUTION_SIGN) {
        if (sm_connection.is_received_key_distribution_csrk == 0) {
            return;
        }
    }

    if (__sm_device_db_entry_get(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry)) {
        entry.remote_addr_type = sm_connection.remote_addr_type;
        memcpy_s(entry.remote_addr, HCI_LENGTH_ADDR, sm_connection.remote_addr, HCI_LENGTH_ADDR);
        memcpy_s(entry.remote_irk, SM_LENGTH_IRK, sm_connection.remote_irk, SM_LENGTH_IRK);
        memcpy_s(entry.remote_csrk, SM_LENGTH_CSRK, sm_connection.remote_csrk, SM_LENGTH_CSRK);
        __sm_device_db_entry_update(hci_connection->peer_addr_type, hci_connection->peer_addr, &entry);
        LOG_INFO("all key distributions received, device db saved");
    } else {
        LOG_ERROR("device get db error, remote_addr_type:0x%02x, remote_addr:%02x:%02x:%02x:%02x:%02x:%02x", hci_connection->peer_addr_type,
                  hci_connection->peer_addr[0], hci_connection->peer_addr[1], hci_connection->peer_addr[2],
                  hci_connection->peer_addr[3], hci_connection->peer_addr[4], hci_connection->peer_addr[5]);
    }
}

void sm_send_pairing_resp(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_RESP] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_RESP;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_RESP, data, SM_LENGTH_PAIRING_RESP);
    offset += SM_LENGTH_PAIRING_RESP;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_public_key(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + HCI_LENGTH_P256_PUBLIC_KEY] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_PUBLIC_KEY;
    offset++;
    memcpy_s(&buffer[offset], HCI_LENGTH_P256_PUBLIC_KEY, data, HCI_LENGTH_P256_PUBLIC_KEY);
    offset += HCI_LENGTH_P256_PUBLIC_KEY;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_confirm(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_CONFIRM] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_CONFIRM;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_CONFIRM, data, SM_LENGTH_PAIRING_CONFIRM);
    offset += SM_LENGTH_PAIRING_CONFIRM;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_random(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_RANDOM] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_RANDOM;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_RANDOM, data, SM_LENGTH_PAIRING_RANDOM);
    offset += SM_LENGTH_PAIRING_RANDOM;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_dhkey_check(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_DHKEY_CHECK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_DHKEY_CHECK;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_DHKEY_CHECK, data, SM_LENGTH_DHKEY_CHECK);
    offset += SM_LENGTH_DHKEY_CHECK;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_failed(uint16_t connect_handle, uint8_t reason) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_FAILED] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_FAILED;
    offset++;
    buffer[offset] = reason;
    offset++;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_encryption_information(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + HCI_LENGTH_LTK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_ENCRYPTION_INFO;
    offset++;
    memcpy_s(&buffer[offset], HCI_LENGTH_LTK, data, HCI_LENGTH_LTK);
    offset += HCI_LENGTH_LTK;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_central_identification(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_EDIV + SM_LENGTH_RAND] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_CENTRAL_IDENTIFICATION;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_EDIV + SM_LENGTH_RAND, data, SM_LENGTH_EDIV + SM_LENGTH_RAND);
    offset += SM_LENGTH_EDIV + SM_LENGTH_RAND;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_identity_information(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_IRK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_IDENTITY_INFO;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_IRK, data, SM_LENGTH_IRK);
    offset += SM_LENGTH_IRK;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_identity_address_information(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + HCI_LENGTH_ADDR_TYPE + HCI_LENGTH_ADDR] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_IDENTITY_ADDRESS_INFO;
    offset++;
    memcpy_s(&buffer[offset], HCI_LENGTH_ADDR_TYPE + HCI_LENGTH_ADDR, data, HCI_LENGTH_ADDR_TYPE + HCI_LENGTH_ADDR);
    offset += HCI_LENGTH_ADDR_TYPE + HCI_LENGTH_ADDR;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_signing_information(uint16_t connect_handle, uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_CSRK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_SIGNING_INFO;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_CSRK, data, SM_LENGTH_CSRK);
    offset += SM_LENGTH_CSRK;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_keypress_notification(uint16_t connect_handle, sm_keypress_notification_t notification) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_KEYPRESS_NOTIFICATION] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_KEYPRESS_NOTIFICATION;
    offset++;
    buffer[offset] = notification;
    offset += SM_LENGTH_KEYPRESS_NOTIFICATION;
    sm_send(connect_handle, buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send(uint16_t connect_handle, uint8_t *data, uint32_t length) {
    l2cap_send(connect_handle, L2CAP_CID_SM, data, length);
}
