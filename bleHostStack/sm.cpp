#include <memory>
#include "sm.h"
#include "log.h"
#include "hci.h"
#include "l2cap.h"
#include "tinycrypt/include/cmac_mode.h"

uint32_t get_be32(uint8_t* data) {
    uint32_t x = 0;

    x = (uint32_t)data[0] << 24;
    x |= (uint32_t)data[1] << 16;
    x |= (uint32_t)data[2] << 8;
    x |= data[3];

    return x;
}

void mem_swap_copy(uint8_t* dst, uint8_t* src, uint32_t length) {
    uint32_t i = 0;

    for (i = 0; i < length; i++) {
        dst[length - 1 - i] = src[i];
    }
}

void mem_swap_self(uint8_t* data, uint32_t length) {
    uint8_t tmp = 0;
    uint32_t i = 0, j = 0;

    for (i = 0, j = length - 1; i < j; i++, j--) {
        tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

void sm_encrypt(uint8_t* k, uint8_t* plain_data, uint8_t* enc_data) {
    uint8_t tmp[16] = {0};
    struct tc_aes_key_sched_struct s;

    mem_swap_copy(tmp, k, 16);
    tc_aes128_set_encrypt_key(&s, tmp);
    mem_swap_copy(tmp, plain_data, 16);
    tc_aes_encrypt(enc_data, tmp, &s);
    mem_swap_self(enc_data, 16);
}

void sm_aes_cmac(uint8_t* k, uint8_t* input, uint32_t length, uint8_t* output) {
    struct tc_aes_key_sched_struct sched;
    struct tc_cmac_struct state;

    tc_cmac_setup(&state, k, &sched);
    tc_cmac_update(&state, input, length);
    tc_cmac_final(output, &state);
}

// k:16B, r:16B, preq:7B, pres:7B, ia:6B, ra:6B, out_confirm:16B
void sm_c1(uint8_t* k, uint8_t* r, uint8_t* preq, uint8_t *pres, uint8_t iat, uint8_t *ia, uint8_t rat, uint8_t *ra, uint8_t *out_confirm) {
    uint8_t p1[16] = {0};
    uint8_t p2[16] = {0};
    uint8_t tmp[16] = {0};
    uint32_t i = 0;

    p1[0] = iat;
    p1[1] = rat;
    memcpy(p1 + 2, preq, 7);
    memcpy(p1 + 9, pres, 7);

    memcpy(p2, ra, 6);
    memcpy(p2 + 6, ia, 6);

    for (i = 0; i < 16; i++) {
        tmp[i] = r[i] ^ p1[i];
    }
    sm_encrypt(k, tmp, tmp);
    for (i = 0; i < 16; i++) {
        tmp[i] ^= p2[i];
    }
    sm_encrypt(k, tmp, out_confirm);
}

// k:16B, r1:16B, r2:16B, out_stk:16B
void sm_s1(uint8_t* k, uint8_t* r1, uint8_t* r2, uint8_t* out_stk) {
    uint8_t tmp[16] = {0};

    memcpy(tmp, r2, 8);
    memcpy(tmp + 8, r1, 8);

    sm_encrypt(k, tmp, out_stk);
}

// u:32B, v:32B, x:16B, out_confirm:16B
void sm_f4(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t z, uint8_t* out_confirm) {
    uint8_t xs[16] = {0};
    uint8_t m[65] = {0};

    mem_swap_copy(m, u, 32);
    mem_swap_copy(m + 32, v, 32);
    m[64] = z;

    mem_swap_copy(xs, x, 16);

    sm_aes_cmac(xs, m, sizeof(m), out_confirm);
    mem_swap_self(out_confirm, 16);
}

// w:32B, n1:16B, n2:16B, a1:6B, a2:6B, out_mackey:16B, out_ltk:16B
void sm_f5(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_mackey, uint8_t* out_ltk) {
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

    mem_swap_copy(ws, w, 32);
    sm_aes_cmac(salt, ws, sizeof(ws), t);

    mem_swap_copy(m + 5, n1, 16);
    mem_swap_copy(m + 21, n2, 16);
    m[37] = at1;
    mem_swap_copy(m + 38, a1, 6);
    m[44] = at2;
    mem_swap_copy(m + 45, a2, 6);
    sm_aes_cmac(t, m, sizeof(m), out_mackey);
    mem_swap_self(out_mackey, 16);

    m[0] = 0x01;
    sm_aes_cmac(t, m, sizeof(m), out_ltk);
    mem_swap_self(out_ltk, 16);
}

// w:16B, n1:16B, n2:16B, r:16B, iocap:3B, a1:6B, a2:6B, out_dhkey_check:16B
void sm_f6(uint8_t* w, uint8_t* n1, uint8_t* n2, uint8_t* r, uint8_t* iocap, uint8_t at1, uint8_t* a1, uint8_t at2, uint8_t* a2, uint8_t* out_dhkey_check) {
    uint8_t ws[16] = {0};
    uint8_t m[65] = {0};

    mem_swap_copy(m, n1, 16);
    mem_swap_copy(m + 16, n2, 16);
    mem_swap_copy(m + 32, r, 16);
    mem_swap_copy(m + 48, iocap, 3);
    m[51] = at1;
    memcpy(m + 52, a1, 6); // no need?
    mem_swap_copy(m + 52, a1, 6);
    m[58] = at2;
    memcpy(m + 59, a2, 6); // no need?
    mem_swap_copy(m + 59, a2, 6);

    mem_swap_copy(ws, w, 16);

    sm_aes_cmac(ws, m, sizeof(m), out_dhkey_check);
    mem_swap_self(out_dhkey_check, 16);
}

// u:32B, v:32B, x:16B, y:16B, out_passkey:4B
void sm_g2(uint8_t* u, uint8_t* v, uint8_t* x, uint8_t* y, uint32_t* out_passkey) {
    uint8_t m[80] = {0};
    uint8_t xs[16] = {0};

    mem_swap_copy(m, u, 32);
    mem_swap_copy(m + 32, v, 32);
    mem_swap_copy(m + 64, y, 16);

    mem_swap_copy(xs, x, 16);

    sm_aes_cmac(xs, m, sizeof(m), xs);
    *out_passkey = get_be32(xs + 12) % 1000000;
}

uint8_t pairing_req[SM_LENGTH_PAIRING_REQ] = {0};
uint8_t pairing_resp[SM_LENGTH_PAIRING_RESP] = {0};
uint8_t local_pairing_public_key[SM_LENGTH_PAIRING_PUBLIC_KEY] = {0};
uint8_t remote_pairing_public_key[SM_LENGTH_PAIRING_PUBLIC_KEY] = {0};
uint8_t local_dhkey[SM_LENGTH_DHKEY] = {0};

void sm_recv(uint8_t *data, uint16_t length) {
    uint8_t op_code = data[0];

    switch (op_code) {
    case SM_OPERATE_PAIRING_REQ:
        sm_recv_pairing_req(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_PUBLIC_KEY:
        sm_recv_pairing_public_key(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_RANDOM:
        sm_recv_pairing_random(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    case SM_OPERATE_PAIRING_DHKEY_CHECK:
        sm_recv_pairing_dhkey_check(data + SM_LENGTH_HEADER, length - SM_LENGTH_HEADER); break;
    default:
        LOG_WARNING("sm_recv invalid, op_code:%u", op_code); break;
    }
}

void sm_recv_pairing_req(uint8_t *data, uint16_t length) {
    LOG_INFO("sm_recv_pairing_req iocap:%u, oob_flag:%u, auth_req:%u, max_encrypt_key_size:%u, initator_key_distribution:%u, responder_key_distribution:%u",
             data[1], data[2], data[3], data[4], data[5], data[6]);
    memcpy(pairing_req, data, SM_LENGTH_PAIRING_REQ);

    pairing_resp[0] = SM_OPERATE_PAIRING_RESP;
    pairing_resp[1] = SM_IOCAP;
    pairing_resp[2] = SM_OOB_DATA_FLAG;
    pairing_resp[3] = SM_AUTH;
    pairing_resp[4] = SM_MAX_ENCRYPT_KEY_SIZE;
    pairing_resp[5] = SM_KEY_DISTRIBUTION;
    pairing_resp[6] = SM_KEY_DISTRIBUTION;
    sm_send(pairing_resp, SM_LENGTH_PAIRING_RESP);
}

void sm_recv_pairing_public_key(uint8_t *data, uint16_t length) {
    memcpy(remote_pairing_public_key, data, SM_LENGTH_PAIRING_PUBLIC_KEY);
    hci_send_cmd_le_read_local_P256_public_key(); // wait HCI_EVENT_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE
    hci_send_cmd_le_generate_dhkey(remote_pairing_public_key, SM_LENGTH_PAIRING_PUBLIC_KEY); // wait HCI_EVENT_LE_GENERATE_DHKEY_COMPLETE
    sm_send_local_pairing_public_key(local_pairing_public_key, SM_LENGTH_PAIRING_PUBLIC_KEY);
}

void sm_recv_pairing_random(uint8_t *data, uint16_t length) {

}

void sm_recv_pairing_dhkey_check(uint8_t *data, uint16_t length) {

}

void sm_send_local_pairing_public_key(uint8_t *data, uint16_t length) {
    uint8_t buf[SM_LENGTH_HEADER + SM_LENGTH_PAIRING_PUBLIC_KEY] = { 0x00 };

    buf[0] = SM_OPERATE_PAIRING_PUBLIC_KEY;
    memcpy(&buf[1], data, length);
    sm_send(buf, SM_LENGTH_HEADER + SM_LENGTH_PAIRING_PUBLIC_KEY);
}

void sm_set_local_pairing_public_key(uint8_t *data) {
    memcpy(local_pairing_public_key, data, SM_LENGTH_PAIRING_PUBLIC_KEY);
}

void sm_set_local_dhkey(uint8_t *data) {
    memcpy(local_dhkey, data, SM_LENGTH_DHKEY);
}

void sm_send(uint8_t *data, uint16_t length) {
    l2cap_send(L2CAP_CID_SM, data, length);
}

