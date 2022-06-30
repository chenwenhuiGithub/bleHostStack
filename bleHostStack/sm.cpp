#include <memory>
#include "sm.h"
#include "log.h"
#include "hci.h"
#include "l2cap.h"
#include "tinycrypt/include/cmac_mode.h"
#include <QFile>


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
#define SM_LENGTH_IOCAP                                     3
#define SM_LENGTH_LTK                                       16
#define SM_LENGTH_EDIV                                      2
#define SM_LENGTH_RAND                                      8
#define SM_LENGTH_IRK                                       16
#define SM_LENGTH_ADDR_TYPE                                 1
#define SM_LENGTH_ADDR                                      6
#define SM_LENGTH_CSRK                                      16

#define SM_LENGTH_PACKET_HEADER                             (HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + L2CAP_LENGTH_HEADER)


typedef enum {
    JUST_WORKS,
    PASSKEY_I_INPUT_R_DISPLAY,
    PASSKEY_I_DISPLAY_R_INPUT,
    PASSKEY_I_INPUT_R_INPUT,
    NUMERIC_COMPARISON,
    OOB
} SM_PAIRING_METHOD;

typedef struct {
    uint8_t local_ltk[SM_LENGTH_LTK];
    uint8_t local_ediv[SM_LENGTH_EDIV];
    uint8_t local_rand[SM_LENGTH_RAND];
    uint8_t local_irk[SM_LENGTH_IRK];
    uint8_t local_addr_type;
    uint8_t local_addr[SM_LENGTH_ADDR];
    uint8_t local_csrk[SM_LENGTH_CSRK];
    uint8_t remote_ltk[SM_LENGTH_LTK];
    uint8_t remote_ediv[SM_LENGTH_EDIV];
    uint8_t remote_rand[SM_LENGTH_RAND];
    uint8_t remote_irk[SM_LENGTH_IRK];
    uint8_t remote_addr_type;
    uint8_t remote_addr[SM_LENGTH_ADDR];
    uint8_t remote_csrk[SM_LENGTH_CSRK];
    uint8_t key_size;
    uint8_t is_authenticated;
    uint8_t is_authorized;
    uint8_t is_secure_connection;
} SM_DEVICE_INFO;

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

static void __sm_recv_pairing_req(uint8_t *data, uint32_t length);
static void __sm_recv_encryption_info(uint8_t *data, uint32_t length);
static void __sm_recv_central_identification(uint8_t *data, uint32_t length);
static void __sm_recv_identity_info(uint8_t *data, uint32_t length);
static void __sm_recv_identity_address_info(uint8_t *data, uint32_t length);
static void __sm_recv_signing_info(uint8_t *data, uint32_t length);
static void __sm_recv_pairing_public_key(uint8_t *data, uint32_t length);
static void __sm_recv_pairing_random(uint8_t *data, uint32_t length);
static void __sm_recv_pairing_dhkey_check(uint8_t *data, uint32_t length);
static void __sm_save_device_info();
static void __sm_get_pairing_method();

uint8_t pairing_req[SM_LENGTH_HEADER + SM_LENGTH_PAIRING_REQ] = {0};
uint8_t pairing_resp[SM_LENGTH_HEADER + SM_LENGTH_PAIRING_RESP] = {0};
uint8_t local_pairing_public_key[SM_LENGTH_PAIRING_PUBLIC_KEY] = {0};
uint8_t remote_pairing_public_key[SM_LENGTH_PAIRING_PUBLIC_KEY] = {0};
uint8_t local_dhkey[SM_LENGTH_DHKEY] = {0};
uint8_t local_random[SM_LENGTH_PAIRING_RANDOM] = {0};
uint8_t remote_random[SM_LENGTH_PAIRING_RANDOM] = {0};
uint8_t local_dhkey_check[SM_LENGTH_DHKEY_CHECK] = {0};
uint8_t remote_dhkey_check[SM_LENGTH_DHKEY_CHECK] = {0};
uint8_t local_mackey[SM_LENGTH_MACKEY] = {0};
uint8_t local_iocap[SM_LENGTH_IOCAP] = {0};
uint8_t remote_iocap[SM_LENGTH_IOCAP] = {0};
uint8_t local_ltk[SM_LENGTH_LTK] = {0};
uint8_t remote_ltk[SM_LENGTH_LTK] = {0};
uint8_t local_ediv[SM_LENGTH_EDIV] = {0};
uint8_t remote_ediv[SM_LENGTH_EDIV] = {0};
uint8_t local_rand[SM_LENGTH_RAND] = {0};
uint8_t remote_rand[SM_LENGTH_RAND] = {0};
uint8_t local_irk[SM_LENGTH_IRK] = {0};
uint8_t remote_irk[SM_LENGTH_IRK] = {0};
uint8_t local_addr_type = 0;
uint8_t local_addr[SM_LENGTH_ADDR] = {0};
uint8_t remote_addr_type = 0;
uint8_t remote_addr[SM_LENGTH_ADDR] = {0};
uint8_t local_csrk[SM_LENGTH_CSRK] = {0};
uint8_t remote_csrk[SM_LENGTH_CSRK] = {0};

SM_PAIRING_METHOD pairing_method = JUST_WORKS;
uint8_t is_local_ltk_generated = 0;
uint8_t is_secure_connection = 0;
uint8_t is_received_key_distribution_ltk = 0;
uint8_t is_received_key_distribution_id = 0;
uint8_t is_received_key_distribution_irk = 0;
uint8_t is_received_key_distribution_addr = 0;
uint8_t is_received_key_distribution_csrk = 0;
SM_DEVICE_INFO device_info;
QFile device_info_file;

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
    memcpy_s(m + 52, 6, a1, 6); // no need?
    __mem_swap_copy(m + 52, a1, 6);
    m[58] = at2;
    memcpy_s(m + 59, 6, a2, 6); // no need?
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



static void __sm_get_pairing_method() {
    SM_PAIRING_METHOD legacy_pairing_method_table[5][5] = {
        {JUST_WORKS, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY},
        {JUST_WORKS, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY},
        {PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_INPUT_R_INPUT, JUST_WORKS, PASSKEY_I_DISPLAY_R_INPUT},
        {JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS},
        {PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_INPUT_R_DISPLAY,JUST_WORKS, PASSKEY_I_DISPLAY_R_INPUT},
    };

    SM_PAIRING_METHOD secure_connection_pairing_method_table[5][5] = {
        {JUST_WORKS, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, PASSKEY_I_INPUT_R_DISPLAY},
        {JUST_WORKS, NUMERIC_COMPARISON, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, NUMERIC_COMPARISON},
        {PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_DISPLAY_R_INPUT, PASSKEY_I_INPUT_R_INPUT, JUST_WORKS, PASSKEY_I_DISPLAY_R_INPUT},
        {JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS, JUST_WORKS},
        {PASSKEY_I_DISPLAY_R_INPUT, NUMERIC_COMPARISON, PASSKEY_I_INPUT_R_DISPLAY, JUST_WORKS, NUMERIC_COMPARISON},
    };

    is_secure_connection = (pairing_req[3] & SM_AUTH_SECURE_CONNECTION) && (pairing_resp[3] & SM_AUTH_SECURE_CONNECTION);
    LOG_INFO("is_secure_connection:0x%02x", is_secure_connection);

    if (is_secure_connection) {
        if (pairing_req[2] || pairing_resp[2]) {
            pairing_method = OOB;
            LOG_INFO("pairing_method:OOB");
            return;
        }
    } else {
        if (pairing_req[2] && pairing_resp[2]) {
            pairing_method = OOB;
            LOG_INFO("pairing_method:OOB");
            return;
        }
    }

    if ((pairing_req[3] & SM_AUTH_MITM) || (pairing_resp[3] & SM_AUTH_MITM)) {
        if (is_secure_connection) {
            pairing_method = secure_connection_pairing_method_table[pairing_resp[1]][pairing_req[1]];
        } else {
            pairing_method = legacy_pairing_method_table[pairing_resp[1]][pairing_req[1]];
        }

        if (JUST_WORKS == pairing_method) {
            LOG_INFO("pairing_method:JUST_WORKS");
        } else if (PASSKEY_I_INPUT_R_DISPLAY == pairing_method) {
            LOG_INFO("pairing_method:PASSKEY_I_INPUT_R_DISPLAY");
        } else if (PASSKEY_I_DISPLAY_R_INPUT == pairing_method) {
            LOG_INFO("pairing_method:PASSKEY_I_DISPLAY_R_INPUT");
        } else if (PASSKEY_I_INPUT_R_INPUT == pairing_method) {
            LOG_INFO("pairing_method:PASSKEY_I_INPUT_R_INPUT");
        } else if (NUMERIC_COMPARISON == pairing_method) {
            LOG_INFO("pairing_method:NUMERIC_COMPARISON");
        } else {
        }
    } else {
        pairing_method = JUST_WORKS;
        LOG_INFO("pairing_method:JUST_WORKS");
    }
}

void sm_recv(uint8_t *data, uint32_t length) {
    uint8_t op_code = data[0];

    switch (op_code) {
    case SM_OPERATE_PAIRING_REQ:
        __sm_recv_pairing_req(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_ENCRYPTION_INFO:
        __sm_recv_encryption_info(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_CENTRAL_IDENTIFICATION:
        __sm_recv_central_identification(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_IDENTITY_INFO:
        __sm_recv_identity_info(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_IDENTITY_ADDRESS_INFO:
        __sm_recv_identity_address_info(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_SIGNING_INFO:
        __sm_recv_signing_info(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_PUBLIC_KEY:
        __sm_recv_pairing_public_key(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_RANDOM:
        __sm_recv_pairing_random(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_DHKEY_CHECK:
        __sm_recv_pairing_dhkey_check(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    default:
        LOG_WARNING("sm_recv invalid, op_code:0x%02x", op_code); break;
    }
}

static void __sm_recv_pairing_req(uint8_t *data, uint32_t length) {
    (void)length;
    pairing_req[0] = SM_OPERATE_PAIRING_REQ;
    memcpy_s(&pairing_req[1], SM_LENGTH_PAIRING_REQ, data, SM_LENGTH_PAIRING_REQ);

    pairing_resp[0] = SM_OPERATE_PAIRING_RESP;
    pairing_resp[1] = SM_IOCAP;
    pairing_resp[2] = SM_OOB_DATA_FLAG;
    pairing_resp[3] = SM_AUTH;
    pairing_resp[4] = SM_MAX_ENCRYPT_KEY_SIZE;
    pairing_resp[5] = SM_INITIATOR_KEY_DISTRIBUTION;
    pairing_resp[6] = SM_RESPONDER_KEY_DISTRIBUTION;

    LOG_INFO("pairing_req iocap:0x%02x, oob_flag:0x%02x, auth_req:0x%02x, max_encrypt_key_size:%u, i_key_distribution:0x%02x, r_key_distribution:0x%02x",
             pairing_req[1], pairing_req[2], pairing_req[3], pairing_req[4], pairing_req[5], pairing_req[6]);
    LOG_INFO("pairing_resp iocap:0x%02x, oob_flag:0x%02x, auth_req:0x%02x, max_encrypt_key_size:%u, i_key_distribution:0x%02x, r_key_distribution:0x%02x",
             pairing_resp[1], pairing_resp[2], pairing_resp[3], pairing_resp[4], pairing_resp[5], pairing_resp[6]);

    sm_send_pairing_resp(pairing_resp + 1);
}

static void __sm_recv_encryption_info(uint8_t *data, uint32_t length) {
    (void)length;
    sm_set_remote_ltk(data);

    is_received_key_distribution_ltk = 1;
    __sm_save_device_info();
}

static void __sm_recv_central_identification(uint8_t *data, uint32_t length) {
    (void)length;
    sm_set_remote_ediv(data);
    sm_set_remote_rand(data + SM_LENGTH_EDIV);

    is_received_key_distribution_id = 1;
    __sm_save_device_info();
}

static void __sm_recv_identity_info(uint8_t *data, uint32_t length) {
    (void)length;
    sm_set_remote_irk(data);

    is_received_key_distribution_irk = 1;
    __sm_save_device_info();
}

static void __sm_recv_identity_address_info(uint8_t *data, uint32_t length) {
    (void)length;
    sm_set_remote_address_type(data[0]); // TODO: data not same with connection event?
    sm_set_remote_address(data + SM_LENGTH_ADDR_TYPE);

    is_received_key_distribution_addr = 1;
    __sm_save_device_info();
}

static void __sm_recv_signing_info(uint8_t *data, uint32_t length) {
    (void)length;
    sm_set_remote_csrk(data);

    is_received_key_distribution_csrk = 1;
    __sm_save_device_info();
}

static void __sm_save_device_info() {
    uint8_t initiator_key_distribution = pairing_resp[5];

    if (initiator_key_distribution & SM_KEY_DISTRIBUTION_ENC) {
        if ((is_received_key_distribution_ltk == 0) || (is_received_key_distribution_id == 0)) {
            return;
        }
    }

    if (initiator_key_distribution & SM_KEY_DISTRIBUTION_ID) {
        if ((is_received_key_distribution_irk == 0) || (is_received_key_distribution_addr == 0)) {
            return;
        }
    }

    if (initiator_key_distribution & SM_KEY_DISTRIBUTION_SIGN) {
        if (is_received_key_distribution_csrk == 0) {
            return;
        }
    }

    memcpy_s(device_info.local_ltk, SM_LENGTH_LTK, local_ltk, SM_LENGTH_LTK);
    memcpy_s(device_info.local_ediv, SM_LENGTH_EDIV, local_ediv, SM_LENGTH_EDIV);
    memcpy_s(device_info.local_rand, SM_LENGTH_RAND, local_rand, SM_LENGTH_RAND);
    memcpy_s(device_info.local_irk, SM_LENGTH_IRK, local_irk, SM_LENGTH_IRK);
    device_info.local_addr_type = local_addr_type;
    memcpy_s(device_info.local_addr, SM_LENGTH_ADDR, local_addr, SM_LENGTH_ADDR);
    memcpy_s(device_info.local_csrk, SM_LENGTH_CSRK, local_csrk, SM_LENGTH_CSRK);
    memcpy_s(device_info.remote_ltk, SM_LENGTH_LTK, remote_ltk, SM_LENGTH_LTK);
    memcpy_s(device_info.remote_ediv, SM_LENGTH_EDIV, remote_ediv, SM_LENGTH_EDIV);
    memcpy_s(device_info.remote_rand, SM_LENGTH_RAND, remote_rand, SM_LENGTH_RAND);
    memcpy_s(device_info.remote_irk, SM_LENGTH_IRK, remote_irk, SM_LENGTH_IRK);
    device_info.remote_addr_type = remote_addr_type;
    memcpy_s(device_info.remote_addr, SM_LENGTH_ADDR, remote_addr, SM_LENGTH_ADDR);
    memcpy_s(device_info.remote_csrk, SM_LENGTH_CSRK, remote_csrk, SM_LENGTH_CSRK);
    device_info.key_size = 0; // TODO
    device_info.is_authenticated = 0; // TODO
    device_info.is_authorized = 0; // TODO
    device_info.is_secure_connection = is_secure_connection;

    device_info_file.setFileName(SM_DEVICE_INFO_FILE_NAME);
    device_info_file.open(QIODevice::WriteOnly);
    device_info_file.write((char*)&device_info, sizeof(device_info));
    device_info_file.close();
    LOG_INFO("all key distributions received, save device info to db");
}

static void __sm_recv_pairing_public_key(uint8_t *data, uint32_t length) {
    (void)length;

    memcpy_s(remote_pairing_public_key, SM_LENGTH_PAIRING_PUBLIC_KEY, data, SM_LENGTH_PAIRING_PUBLIC_KEY);
    hci_send_cmd_le_generate_dhkey(remote_pairing_public_key);
    sm_send_pairing_public_key(local_pairing_public_key);
}

static void __sm_recv_pairing_random(uint8_t *data, uint32_t length) {
    (void)length;
    uint32_t vb = 0;

    memcpy_s(remote_random, SM_LENGTH_PAIRING_RANDOM, data, SM_LENGTH_PAIRING_RANDOM);
    __sm_g2(remote_pairing_public_key, local_pairing_public_key, remote_random, local_random, &vb);
    LOG_INFO("__sm_g2:%u", vb);

    sm_send_pairing_random(local_random);
}

static void __sm_recv_pairing_dhkey_check(uint8_t *data, uint32_t length) {
    (void)length;
    uint8_t ra[16] = {0};
    uint8_t rb[16] = {0};
    uint8_t calc_remote_dhkey_check[SM_LENGTH_DHKEY_CHECK] = {0};

    local_iocap[0] = pairing_resp[1];
    local_iocap[1] = pairing_resp[2];
    local_iocap[2] = pairing_resp[3];
    remote_iocap[0] = pairing_req[1];
    remote_iocap[1] = pairing_req[2];
    remote_iocap[2] = pairing_req[3];

    memcpy_s(remote_dhkey_check, SM_LENGTH_DHKEY_CHECK, data, SM_LENGTH_DHKEY_CHECK);
    __sm_f5(local_dhkey, remote_random, local_random, remote_addr_type, remote_addr,
            local_addr_type, local_addr, local_mackey, local_ltk);
    __sm_f6(local_mackey, remote_random, local_random, rb, remote_iocap, remote_addr_type, remote_addr,
            local_addr_type, local_addr, calc_remote_dhkey_check);

    if (memcmp(remote_dhkey_check, calc_remote_dhkey_check, SM_LENGTH_DHKEY_CHECK)) {
        LOG_ERROR("remote_dhkey_check error");
        sm_send_pairing_failed(SM_ERROR_DHKEY_CHECK_FAILED);
    } else {
        __sm_f6(local_mackey, local_random, remote_random, ra, local_iocap, local_addr_type, local_addr,
                remote_addr_type, remote_addr, local_dhkey_check);
        sm_send_pairing_dhkey_check(local_dhkey_check);
        is_local_ltk_generated = 1;
    }
}

void sm_send_pairing_resp(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_RESP] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_RESP;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_RESP, data, SM_LENGTH_PAIRING_RESP);
    offset += SM_LENGTH_PAIRING_RESP;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_public_key(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_PUBLIC_KEY] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_PUBLIC_KEY;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_PUBLIC_KEY, data, SM_LENGTH_PAIRING_PUBLIC_KEY);
    offset += SM_LENGTH_PAIRING_PUBLIC_KEY;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_confirm(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_CONFIRM] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_CONFIRM;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_CONFIRM, data, SM_LENGTH_PAIRING_CONFIRM);
    offset += SM_LENGTH_PAIRING_CONFIRM;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_random(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_RANDOM] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_RANDOM;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_PAIRING_RANDOM, data, SM_LENGTH_PAIRING_RANDOM);
    offset += SM_LENGTH_PAIRING_RANDOM;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_dhkey_check(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_DHKEY_CHECK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_DHKEY_CHECK;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_DHKEY_CHECK, data, SM_LENGTH_DHKEY_CHECK);
    offset += SM_LENGTH_DHKEY_CHECK;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_pairing_failed(uint8_t reason) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_PAIRING_FAILED] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_PAIRING_FAILED;
    offset++;
    buffer[offset] = reason;
    offset++;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_encryption_information(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_LTK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_ENCRYPTION_INFO;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_LTK, data, SM_LENGTH_LTK);
    offset += SM_LENGTH_LTK;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_central_identification(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_EDIV + SM_LENGTH_RAND] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_CENTRAL_IDENTIFICATION;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_EDIV + SM_LENGTH_RAND, data, SM_LENGTH_EDIV + SM_LENGTH_RAND);
    offset += SM_LENGTH_EDIV + SM_LENGTH_RAND;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_identity_information(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_IRK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_IDENTITY_INFO;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_IRK, data, SM_LENGTH_IRK);
    offset += SM_LENGTH_IRK;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_identity_address_information(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_ADDR_TYPE + SM_LENGTH_ADDR] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_IDENTITY_ADDRESS_INFO;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_ADDR_TYPE + SM_LENGTH_ADDR, data, SM_LENGTH_ADDR_TYPE + SM_LENGTH_ADDR);
    offset += SM_LENGTH_ADDR_TYPE + SM_LENGTH_ADDR;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_send_signing_information(uint8_t *data) {
    uint8_t buffer[SM_LENGTH_PACKET_HEADER + SM_LENGTH_HEADER + SM_LENGTH_CSRK] = {0};
    uint32_t offset = SM_LENGTH_PACKET_HEADER;

    buffer[offset] = SM_OPERATE_SIGNING_INFO;
    offset++;
    memcpy_s(&buffer[offset], SM_LENGTH_CSRK, data, SM_LENGTH_CSRK);
    offset += SM_LENGTH_CSRK;
    sm_send(buffer, offset - SM_LENGTH_PACKET_HEADER);
}

void sm_key_distribution() {
    uint8_t central_id[SM_LENGTH_EDIV + SM_LENGTH_RAND] = {0};
    uint8_t addr_info[SM_LENGTH_ADDR_TYPE + SM_LENGTH_ADDR] = {0};
    uint8_t responder_key_distribution = pairing_req[6];

    if ((responder_key_distribution & SM_KEY_DISTRIBUTION_ENC) != 0) {
        sm_send_encryption_information(local_ltk);

        // local_ediv/local_rand: for le secure connection, set to 0, TODO: for legacy pairing
        memcpy_s(central_id, SM_LENGTH_EDIV, local_ediv, SM_LENGTH_EDIV);
        memcpy_s(&central_id[SM_LENGTH_EDIV], SM_LENGTH_RAND, local_rand, SM_LENGTH_RAND);
        sm_send_central_identification(central_id);
    }

    if ((responder_key_distribution & SM_KEY_DISTRIBUTION_ID) != 0) {
        *(uint32_t *)(local_irk) = rand();
        *(uint32_t *)(local_irk + 4) = rand();
        *(uint32_t *)(local_irk + 8) = rand();
        *(uint32_t *)(local_irk + 12) = rand();
        sm_send_identity_information(local_irk);

        addr_info[0] = local_addr_type;
        memcpy_s(&addr_info[1], SM_LENGTH_ADDR, local_addr, SM_LENGTH_ADDR);
        sm_send_identity_address_information(addr_info);
    }

    if ((responder_key_distribution & SM_KEY_DISTRIBUTION_SIGN) != 0) {
        *(uint32_t *)(local_csrk) = rand();
        *(uint32_t *)(local_csrk + 4) = rand();
        *(uint32_t *)(local_csrk + 8) = rand();
        *(uint32_t *)(local_csrk + 12) = rand();
        sm_send_signing_information(local_csrk);
    }
}

void sm_set_local_pairing_public_key(uint8_t *data) {
    memcpy_s(local_pairing_public_key, SM_LENGTH_PAIRING_PUBLIC_KEY, data, SM_LENGTH_PAIRING_PUBLIC_KEY);
}

void sm_set_local_dhkey(uint8_t *data) {
    memcpy_s(local_dhkey, SM_LENGTH_DHKEY, data, SM_LENGTH_DHKEY);

    __sm_get_pairing_method();
    if ((JUST_WORKS == pairing_method) || (NUMERIC_COMPARISON == pairing_method)) {
        uint8_t cb[16] = {0};
        *(uint32_t *)(local_random) = rand();
        *(uint32_t *)(local_random + 4) = rand();
        *(uint32_t *)(local_random + 8) = rand();
        *(uint32_t *)(local_random + 12) = rand();
        __sm_f4(local_pairing_public_key, remote_pairing_public_key, local_random, 0, cb);
        sm_send_pairing_confirm(cb);
    } else {
        // TODO: other methods
    }
}

void sm_set_remote_ltk(uint8_t *data) {
    memcpy_s(remote_ltk, SM_LENGTH_LTK, data, SM_LENGTH_LTK);
}

void sm_get_local_ltk(uint8_t *data) {
    char *buffer = nullptr;
    uint32_t device_info_size = sizeof(SM_DEVICE_INFO);

    if (is_local_ltk_generated) { // first encrypt, get ltk from memory
        memcpy_s(data, SM_LENGTH_LTK, local_ltk, SM_LENGTH_LTK);
    } else { // already encrypted, get ltk from db
        buffer = (char *)malloc(device_info_size);
        device_info_file.setFileName(SM_DEVICE_INFO_FILE_NAME);
        device_info_file.open(QIODevice::ReadOnly);
        device_info_file.read(buffer, device_info_size);
        device_info_file.close();

        memcpy_s(data, SM_LENGTH_LTK, buffer, SM_LENGTH_LTK); // TODO: find ltk in multy device_info
        free(buffer);
    }
}

void sm_set_remote_ediv(uint8_t *data) {
    memcpy_s(remote_ediv, SM_LENGTH_EDIV, data, SM_LENGTH_EDIV);
}

void sm_set_remote_rand(uint8_t *data) {
    memcpy_s(remote_rand, SM_LENGTH_RAND, data, SM_LENGTH_RAND);
}

void sm_set_remote_irk(uint8_t *data) {
    memcpy_s(remote_irk, SM_LENGTH_IRK, data, SM_LENGTH_IRK);
}

void sm_set_local_address_type(uint8_t type) {
    local_addr_type = type;
}

void sm_set_local_address(uint8_t *data) {
    memcpy_s(local_addr, SM_LENGTH_ADDR, data, SM_LENGTH_ADDR);
}

void sm_set_remote_address_type(uint8_t type) {
    remote_addr_type = type;
}

void sm_set_remote_address(uint8_t *data) {
    memcpy_s(remote_addr, SM_LENGTH_ADDR, data, SM_LENGTH_ADDR);
}

void sm_set_remote_csrk(uint8_t *data) {
    memcpy_s(remote_csrk, SM_LENGTH_CSRK, data, SM_LENGTH_CSRK);
}

void sm_send(uint8_t *data, uint32_t length) {
    l2cap_send(L2CAP_CID_SM, data, length);
}
