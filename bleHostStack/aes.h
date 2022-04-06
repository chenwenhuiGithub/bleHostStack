#ifndef AES_H
#define AES_H

#include <stdint.h>

void aes_set_key(uint8_t* key, uint8_t key_size);
void aes_encrypt(uint8_t* input, uint8_t* output);
void aes_decrypt(uint8_t* input, uint8_t* output);

#endif // AES_H
