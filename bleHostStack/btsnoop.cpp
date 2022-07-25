#include <QFile>
#include <QDateTime>
#include "btsnoop.h"
#include "hci.h"

#define BTSNOOP_BASE_TIMESTAMP                  0x00dcddb30f2f8000ULL  // 1970-01-01 00:00:00

#pragma pack(1)
typedef struct {
    uint8_t length_original[4];
    uint8_t length_included[4];
    uint8_t packet_flag[4];
    uint8_t cumulative_drops[4];
    uint8_t timestamp_μs[8];
} btsnoop_packet_header_t;
#pragma pack()

static QFile file_btsnoop;

void btsnoop_open() {
    if (file_btsnoop.isOpen()) {
        file_btsnoop.close();
    }

    QDateTime curDateTime = QDateTime::currentDateTime();
    QString fileName = curDateTime.toString("yyyy_MM_dd_hh_mm_ss") + ".dat";
    file_btsnoop.setFileName(fileName);
    file_btsnoop.open(QIODevice::WriteOnly);

    uint8_t file_header[16] = {
        'b', 't', 's', 'n', 'o', 'o', 'p', 0x00, // identification pattern:"btsnoop"
        0x00, 0x00, 0x00, 0x01, // version number:1
        0x00, 0x00, 0x03, 0xea  // datalink type:1002(HCI UART - H4)
    };
    file_btsnoop.write((char*)file_header, sizeof(file_header));
}

void btsnoop_wirte(uint8_t *data, uint32_t length, btsnoop_packet_flag_t flag) {
    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() * 1000ULL + BTSNOOP_BASE_TIMESTAMP;

    btsnoop_packet_header_t packet_header {
        .length_original = {(uint8_t)(length >> 24), (uint8_t)(length >> 16), (uint8_t)(length >> 8), (uint8_t)length},
        .length_included = {(uint8_t)(length >> 24), (uint8_t)(length >> 16), (uint8_t)(length >> 8), (uint8_t)length},
        .packet_flag = {0x00, 0x00, 0x00, (uint8_t)flag},
        .cumulative_drops = {0x00, 0x00, 0x00, 0x00},
        .timestamp_μs = {(uint8_t)(timestamp >> 56), (uint8_t)(timestamp >> 48), (uint8_t)(timestamp >> 40), (uint8_t)(timestamp >> 32),
                         (uint8_t)(timestamp >> 24), (uint8_t)(timestamp >> 16), (uint8_t)(timestamp >> 8), (uint8_t)timestamp}
    };

    if (file_btsnoop.isOpen()) {
        file_btsnoop.write((char*)&packet_header, sizeof(packet_header));
        file_btsnoop.write((char*)data, length);
    }
}

void btsnoop_close(void) {
    if (file_btsnoop.isOpen()) {
        file_btsnoop.close();
    }
}




