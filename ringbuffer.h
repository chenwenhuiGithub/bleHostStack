#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

void ringbuffer_reset();
bool ringbuffer_write(uint8_t *data, uint32_t length);
bool ringbuffer_read(uint8_t *data, uint32_t length);
bool ringbuffer_is_empty();

#endif // RINGBUFFER_H
