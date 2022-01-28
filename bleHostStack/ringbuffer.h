#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

#define RINGBUFFER_SIZE     1024

void ringbuffer_reset();
bool ringbuffer_write(uint8_t *data, uint32_t length);
uint8_t* ringbuffer_read(uint32_t length);
uint32_t ringbuff_used();
uint32_t ringbuff_left();
bool ringbuffer_is_empty();
bool ringbuffer_is_full();

#endif // RINGBUFFER_H
