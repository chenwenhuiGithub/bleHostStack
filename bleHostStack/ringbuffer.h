#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

#define RINGBUFFER_SIZE     1024

void ringbuffer_reset();
bool ringbuffer_write(uint8_t *data, uint32_t length);
bool ringbuffer_read(uint8_t *data, uint32_t length);
uint32_t ringbuffer_used();
uint32_t ringbuffer_left();
bool ringbuffer_is_empty();
bool ringbuffer_is_full();

#endif // RINGBUFFER_H
