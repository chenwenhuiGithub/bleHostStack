#include "ringbuffer.h"
#include <QByteArray>

uint8_t buffer[RINGBUFFER_SIZE] = { 0x00 };
uint32_t read_pos = 0;
uint32_t write_pos = 0;

void ringbuffer_reset() {
    read_pos = 0;
    write_pos = 0;
}

bool ringbuffer_write(uint8_t *data, uint32_t length) {
    uint32_t left_to_end = RINGBUFFER_SIZE - write_pos; // left buffer from write_pos to the end

    if (ringbuff_left() < length) { // left buffer not enough to write
        return false;
    }

    if (left_to_end >= length) { // left_to_end buffer enough to write
        memcpy_s(buffer + write_pos, length, data, length);
        write_pos += length;
        if (write_pos == RINGBUFFER_SIZE) {
            write_pos = 0;
        }
    } else {
        memcpy_s(buffer + write_pos, left_to_end, data, left_to_end); // left_to_end buffer not enough to write
        memcpy_s(buffer, length - left_to_end, data, length - left_to_end); // continue write from begin
        write_pos = length - left_to_end;
    }

    return true;
}

uint8_t* ringbuffer_read(uint32_t length) {
    uint32_t left_to_end = RINGBUFFER_SIZE - read_pos; // used buffer from read_pos to the end
    uint32_t backup_read_pos = 0;
    QByteArray byteArray;

    if (length > ringbuff_used()) { // used buffer not enough to read
        return nullptr;
    }

    if (left_to_end >= length) { // left_to_end buffer enough to read
        backup_read_pos = read_pos;
        read_pos += length;
        if (read_pos == RINGBUFFER_SIZE) {
            read_pos = 0;
        }

        return buffer + backup_read_pos;
    } else {
        byteArray.resize(length);
        memcpy_s(&byteArray[0], left_to_end, buffer + read_pos, left_to_end); // left_to_end buffer not enough to read
        memcpy_s(&byteArray[left_to_end], length - left_to_end, buffer, length - left_to_end); // continue read from begin
        read_pos = length - left_to_end;

        return (uint8_t*)byteArray.data();
    }
}

uint32_t ringbuff_used() {
    uint32_t used = 0;

    if (write_pos >= read_pos) {
        used = write_pos - read_pos;
    } else {
        used = RINGBUFFER_SIZE + write_pos - read_pos;
    }

    return used;
}

uint32_t ringbuff_left() {
    return RINGBUFFER_SIZE - ringbuff_used() - 1;
}

bool ringbuffer_is_empty() {
    return (0 == ringbuff_used());
}

bool ringbuffer_is_full() {
    return (0 == ringbuff_left());
}
