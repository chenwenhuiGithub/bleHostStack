#include <memory>
#include "aes.h"

void rot_word(uint8_t* word, uint8_t* result);
void sub_word(uint8_t* word, uint8_t* result);
void add_round_key(uint32_t round);
void sub_bytes();
void inv_sub_bytes();
void shift_rows();
void inv_shift_rows();
uint8_t gfmultby01(uint8_t b);
uint8_t gfmultby02(uint8_t b);
uint8_t gfmultby03(uint8_t b);
uint8_t gfmultby09(uint8_t b);
uint8_t gfmultby0b(uint8_t b);
uint8_t gfmultby0d(uint8_t b);
uint8_t gfmultby0e(uint8_t b);
void mix_columns();
void inv_mix_columns();
void key_expansion();


uint8_t Nb = 4;   // block size in 32-bit words. Always 4 for AES. (128 bits)
uint8_t Nk = 4;   // key size in 32-bit words. 4, 6, 8. (128, 192, 256 bits)
uint8_t Nr = 10;  // number of rounds. 10, 12, 14


uint8_t aes_key[32] = {0};
uint8_t aes_w[16*15] = {0};
uint8_t aes_state[4][4];

uint8_t sbox[16*16] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

uint8_t isbox[16*16] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

uint8_t rcon[11*4] = {
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x00, 0x00,
    0x1b, 0x00, 0x00, 0x00,
    0x36, 0x00, 0x00, 0x00
};

void rot_word(uint8_t* word, uint8_t* result) {
    result[0] = word[1];
    result[1] = word[2];
    result[2] = word[3];
    result[3] = word[0];
}

void sub_word(uint8_t* word, uint8_t* result) {
    result[0] = sbox[ 16*(word[0] >> 4) + (word[0] & 0x0f) ];
    result[1] = sbox[ 16*(word[1] >> 4) + (word[1] & 0x0f) ];
    result[2] = sbox[ 16*(word[2] >> 4) + (word[2] & 0x0f) ];
    result[3] = sbox[ 16*(word[3] >> 4) + (word[3] & 0x0f) ];
}

void add_round_key(uint32_t round) {
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            aes_state[r][c] = (uint8_t)((int)aes_state[r][c]^(int)aes_w[4*((round*4)+c)+r]);
        }
    }
}

void sub_bytes() {
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            aes_state[r][c] = sbox[ 16*(aes_state[r][c] >> 4)+ ( aes_state[r][c] & 0x0f) ];
        }
    }
}

void inv_sub_bytes() {
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            aes_state[r][c] = isbox[ 16*( aes_state[r][c] >> 4)+( aes_state[r][c] & 0x0f) ];
        }
    }
}

void shift_rows() {
    uint8_t temp[4*4] = {0};
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            temp[4*r+c] =  aes_state[r][c];
        }
    }

    for (r = 1; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            aes_state[r][c] = temp[ 4*r+ (c + r) % Nb ];
        }
    }
}

void inv_shift_rows() {
    uint8_t temp[4*4] = {0};
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            temp[4*r+c] =  aes_state[r][c];
        }
    }

    for (r = 1; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            aes_state[r][ (c + r) % Nb ] = temp[4*r+c];
        }
    }
}

uint8_t gfmultby01(uint8_t b) {
    return b;
}

uint8_t gfmultby02(uint8_t b) {
    if (b < 0x80)
        return (uint8_t)(int)(b << 1);
    else
        return (uint8_t)((int)(b << 1) ^ (int)(0x1b));
}

uint8_t gfmultby03(uint8_t b) {
    return (uint8_t)((int)gfmultby02(b) ^ (int)b);
}

uint8_t gfmultby09(uint8_t b) {
    return (uint8_t)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^ (int)b);
}

uint8_t gfmultby0b(uint8_t b) {
    return (uint8_t)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^ (int)gfmultby02(b) ^ (int)b);
}

uint8_t gfmultby0d(uint8_t b) {
    return (uint8_t)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^ (int)gfmultby02(gfmultby02(b)) ^ (int)(b));
}

uint8_t gfmultby0e(uint8_t b) {
    return (uint8_t)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^ (int)gfmultby02(gfmultby02(b)) ^ (int)gfmultby02(b));
}

void mix_columns() {
    uint8_t temp[4*4] = {0};
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            temp[4*r+c] =  aes_state[r][c];
        }
    }

    for (c = 0; c < 4; c++) {
        aes_state[0][c] = (uint8_t) ( (int)gfmultby02(temp[0+c]) ^ (int)gfmultby03(temp[4*1+c]) ^ (int)gfmultby01(temp[4*2+c]) ^ (int)gfmultby01(temp[4*3+c]) );
        aes_state[1][c] = (uint8_t) ( (int)gfmultby01(temp[0+c]) ^ (int)gfmultby02(temp[4*1+c]) ^ (int)gfmultby03(temp[4*2+c]) ^ (int)gfmultby01(temp[4*3+c]) );
        aes_state[2][c] = (uint8_t) ( (int)gfmultby01(temp[0+c]) ^ (int)gfmultby01(temp[4*1+c]) ^ (int)gfmultby02(temp[4*2+c]) ^ (int)gfmultby03(temp[4*3+c]) );
        aes_state[3][c] = (uint8_t) ( (int)gfmultby03(temp[0+c]) ^ (int)gfmultby01(temp[4*1+c]) ^ (int)gfmultby01(temp[4*2+c]) ^ (int)gfmultby02(temp[4*3+c]) );
    }
}

void inv_mix_columns() {
    uint8_t temp[4*4] = {0};
    uint32_t r = 0, c = 0;

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            temp[4*r+c] =  aes_state[r][c];
        }
    }

    for (c = 0; c < 4; c++) {
        aes_state[0][c] = (unsigned char) ( (int)gfmultby0e(temp[c]) ^ (int)gfmultby0b(temp[4+c]) ^ (int)gfmultby0d(temp[4*2+c]) ^ (int)gfmultby09(temp[4*3+c]) );
        aes_state[1][c] = (unsigned char) ( (int)gfmultby09(temp[c]) ^ (int)gfmultby0e(temp[4+c]) ^ (int)gfmultby0b(temp[4*2+c]) ^ (int)gfmultby0d(temp[4*3+c]) );
        aes_state[2][c] = (unsigned char) ( (int)gfmultby0d(temp[c]) ^ (int)gfmultby09(temp[4+c]) ^ (int)gfmultby0e(temp[4*2+c]) ^ (int)gfmultby0b(temp[4*3+c]) );
        aes_state[3][c] = (unsigned char) ( (int)gfmultby0b(temp[c]) ^ (int)gfmultby0d(temp[4+c]) ^ (int)gfmultby09(temp[4*2+c]) ^ (int)gfmultby0e(temp[4*3+c]) );
    }
}

void key_expansion() {
    uint8_t result[4] = {0}, result2[4] = {0}, temp[4] = {0};
    uint32_t row = 0;
    memset(aes_w, 0, sizeof(aes_w));

    for (row = 0; row < Nk; row++) {
        aes_w[4*row+0] = aes_key[4*row];
        aes_w[4*row+1] = aes_key[4*row+1];
        aes_w[4*row+2] = aes_key[4*row+2];
        aes_w[4*row+3] = aes_key[4*row+3];
    }

    for (row = Nk; row < Nb * (Nr+1); row++) {
        temp[0] = aes_w[4*(row-1)+0];
        temp[1] = aes_w[4*(row-1)+1];
        temp[2] = aes_w[4*(row-1)+2];
        temp[3] = aes_w[4*(row-1)+3];

        if ((row % Nk) == 0) {
            rot_word(temp, result);
            sub_word(result, result2);
            memcpy(temp, result2, 4);
            temp[0] = (uint8_t)( (int)temp[0] ^ (int) rcon[4*(row/Nk)+0] );
            temp[1] = (uint8_t)( (int)temp[1] ^ (int) rcon[4*(row/Nk)+1] );
            temp[2] = (uint8_t)( (int)temp[2] ^ (int) rcon[4*(row/Nk)+2] );
            temp[3] = (uint8_t)( (int)temp[3] ^ (int) rcon[4*(row/Nk)+3] );
        } else if (Nk > 6 && (row % Nk == 4)) {
            sub_word(temp, result);
            memcpy(temp, result, 4);
        }

        aes_w[4*row+0] = (uint8_t) ( (int) aes_w[4*(row-Nk)+0] ^ (int)temp[0] );
        aes_w[4*row+1] = (uint8_t) ( (int) aes_w[4*(row-Nk)+1] ^ (int)temp[1] );
        aes_w[4*row+2] = (uint8_t) ( (int) aes_w[4*(row-Nk)+2] ^ (int)temp[2] );
        aes_w[4*row+3] = (uint8_t) ( (int) aes_w[4*(row-Nk)+3] ^ (int)temp[3] );
    }
}

void aes_set_key(uint8_t* key, uint8_t key_size) {
    Nb = 4;
    if (key_size == 16) {
        Nk = 4;
        Nr = 10;
    }
    if (key_size == 24) {
        Nk = 6;
        Nr = 12;
    }
    if (key_size == 32) {
        Nk = 8;
        Nr = 14;
    }

    memcpy(aes_key, key, key_size);
    key_expansion();
}

void aes_encrypt(uint8_t* input, uint8_t* output) {
    uint32_t i = 0, round = 0;

    memset(&aes_state[0][0], 0, sizeof(aes_state));

    for (i = 0; i < (4 * Nb); i++){
        aes_state[i % 4][ i / 4] = input[i];
    }

    add_round_key(0);

    for (round = 1; round <= (Nr - 1); round++) {
        sub_bytes();
        shift_rows();
        mix_columns();
        add_round_key(round);
    }

    sub_bytes();
    shift_rows();
    add_round_key(Nr);

    for (i = 0; i < (4 * Nb); i++){
        output[i] =  aes_state[i % 4][ i / 4];
    }
}

void aes_decrypt(uint8_t* input, uint8_t* output) {
    uint32_t i = 0, round = 0;

    memset(&aes_state[0][0], 0, sizeof(aes_state));

    for (i = 0; i < (4 * Nb); i++){
        aes_state[i % 4][ i / 4] = input[i];
    }

    add_round_key(Nr);

    for (round = Nr-1; round >= 1; round--) {
        inv_shift_rows();
        inv_sub_bytes();
        add_round_key(round);
        inv_mix_columns();
    }

    inv_shift_rows();
    inv_sub_bytes();
    add_round_key(0);

    for (i = 0; i < (4 * Nb); i++){
        output[i] =  aes_state[i % 4][ i / 4];
    }
}

