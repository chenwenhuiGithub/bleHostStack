#include <memory>
#include "ringbuffer.h"
#include "config.h"

static uint32_t __ringbuffer_used();
static uint32_t __ringbuffer_left();

static uint8_t ringbuffer_buf[RINGBUFFER_SIZE] = { 0x00 };
static uint32_t ringbuffer_read_pos = 0;
static uint32_t ringbuffer_write_pos = 0;

void ringbuffer_reset() {
    ringbuffer_read_pos = 0;
    ringbuffer_write_pos = 0;
}

bool ringbuffer_write(uint8_t *data, uint32_t length) {
    uint32_t left_to_end = RINGBUFFER_SIZE - ringbuffer_write_pos;

    if (__ringbuffer_left() < length) {
        return false;
    }

    if (left_to_end >= length) {
        memcpy_s(&ringbuffer_buf[ringbuffer_write_pos], length, data, length);
        ringbuffer_write_pos += length;
        if (ringbuffer_write_pos == RINGBUFFER_SIZE) {
            ringbuffer_write_pos = 0;
        }
    } else {
        memcpy_s(&ringbuffer_buf[ringbuffer_write_pos], left_to_end, data, left_to_end);
        memcpy_s(ringbuffer_buf, length - left_to_end, data + left_to_end, length - left_to_end);
        ringbuffer_write_pos = length - left_to_end;
    }

    return true;
}

bool ringbuffer_read(uint8_t *data, uint32_t length) {
    uint32_t left_to_end = RINGBUFFER_SIZE - ringbuffer_read_pos;

    if (length > __ringbuffer_used()) {
        return false;
    }

    if (left_to_end >= length) {
        memcpy_s(data, length, ringbuffer_buf + ringbuffer_read_pos, length);
        ringbuffer_read_pos += length;
        if (ringbuffer_read_pos == RINGBUFFER_SIZE) {
            ringbuffer_read_pos = 0;
        }
    } else {
        memcpy_s(data, left_to_end, ringbuffer_buf + ringbuffer_read_pos, left_to_end);
        memcpy_s(&data[left_to_end], length - left_to_end, ringbuffer_buf, length - left_to_end);
        ringbuffer_read_pos = length - left_to_end;
    }

    return true;
}

bool ringbuffer_is_empty() {
    return (0 == __ringbuffer_used());
}

static uint32_t __ringbuffer_used() {
    uint32_t used = 0;

    if (ringbuffer_write_pos >= ringbuffer_read_pos) {
        used = ringbuffer_write_pos - ringbuffer_read_pos;
    } else {
        used = RINGBUFFER_SIZE + ringbuffer_write_pos - ringbuffer_read_pos;
    }

    return used;
}

static uint32_t __ringbuffer_left() {
    return RINGBUFFER_SIZE - __ringbuffer_used() - 1;
}

