#include "ringbuffer.h"
#include <memory>

uint8_t ringbuffer_buf[RINGBUFFER_SIZE] = { 0x00 };
uint32_t ringbuffer_read_pos = 0;
uint32_t ringbuffer_write_pos = 0;

void ringbuffer_reset() {
    ringbuffer_read_pos = 0;
    ringbuffer_write_pos = 0;
}

bool ringbuffer_write(uint8_t *data, uint32_t length) {
    uint32_t left_to_end = RINGBUFFER_SIZE - ringbuffer_write_pos; // left buffer from write_pos to the end

    if (ringbuffer_left() < length) { // left buffer not enough to write
        return false;
    }

    if (left_to_end >= length) { // left_to_end buffer enough to write
        memcpy_s(ringbuffer_buf + ringbuffer_write_pos, length, data, length);
        ringbuffer_write_pos += length;
        if (ringbuffer_write_pos == RINGBUFFER_SIZE) {
            ringbuffer_write_pos = 0;
        }
    } else {
        memcpy_s(ringbuffer_buf + ringbuffer_write_pos, left_to_end, data, left_to_end); // left_to_end buffer not enough to write
        memcpy_s(ringbuffer_buf, length - left_to_end, data, length - left_to_end); // continue write from begin
        ringbuffer_write_pos = length - left_to_end;
    }

    return true;
}

bool ringbuffer_read(uint8_t *data, uint32_t length) {
    uint32_t left_to_end = RINGBUFFER_SIZE - ringbuffer_read_pos; // used buffer from read_pos to the end

    if (length > ringbuffer_used()) { // used buffer not enough to read
        return false;
    }

    if (left_to_end >= length) { // left_to_end buffer enough to read
        memcpy_s(data, length, ringbuffer_buf + ringbuffer_read_pos, length);
        ringbuffer_read_pos += length;
        if (ringbuffer_read_pos == RINGBUFFER_SIZE) {
            ringbuffer_read_pos = 0;
        }
    } else {
        memcpy_s(data, left_to_end, ringbuffer_buf + ringbuffer_read_pos, left_to_end); // left_to_end buffer not enough to read
        memcpy_s(data + left_to_end, length - left_to_end, ringbuffer_buf, length - left_to_end); // continue read from begin
        ringbuffer_read_pos = length - left_to_end;
    }

    return true;
}

uint32_t ringbuffer_used() {
    uint32_t used = 0;

    if (ringbuffer_write_pos >= ringbuffer_read_pos) {
        used = ringbuffer_write_pos - ringbuffer_read_pos;
    } else {
        used = RINGBUFFER_SIZE + ringbuffer_write_pos - ringbuffer_read_pos;
    }

    return used;
}

uint32_t ringbuffer_left() {
    return RINGBUFFER_SIZE - ringbuffer_used() - 1;
}

bool ringbuffer_is_empty() {
    return (0 == ringbuffer_used());
}

bool ringbuffer_is_full() {
    return (0 == ringbuffer_left());
}
