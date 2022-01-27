#include "btsnoop.h"
#include <QFile>
#include <QDateTime>

QFile file;

void btsnoop_open() {
    if (file.isOpen()) {
        file.close();
    }

    QDateTime curDateTime = QDateTime::currentDateTime();
    QString fileName = curDateTime.toString("yyyy_MM_dd_hh_mm_ss") + ".dat";
    file.setFileName(fileName);
    file.open(QIODevice::WriteOnly);

    // HCI type: H4
    uint8_t header[16] = {'b', 't', 's', 'n', 'o', 'o', 'p', 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0xea};
    file.write((char*)header, sizeof(header));
}

void btsnoop_wirte(uint8_t *data, uint32_t length, BTSNOOP_DIRECT direct) {
    uint8_t type = data[0];
    // bit0: 0 - Host to Controller, 1 - Controller to Host
    // bit1: 0 - ACL/SCO, 1 - CMD/EVT
    uint32_t flags = 0x00;
    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() * 1000ULL + BTSNOOP_BASE_TIMESTAMP;

    if (BTSNOOP_HCI_PACKET_TYPE_CMD == type) {
        flags = 0x02;
    } else if (BTSNOOP_HCI_PACKET_TYPE_EVT == type) {
        flags = 0x03;
    } else if ((BTSNOOP_HCI_PACKET_TYPE_ACL == type) || (BTSNOOP_HCI_PACKET_TYPE_SCO == type)) {
        if (BTSNOOP_DIRECT_HOST_TO_CONTROLLER == direct) {
            flags = 0x00;
        } else {
            flags = 0x01;
        }
    } else {

    }

    struct btsnoop_header header;
    header.length_original = BTSNOOP_HTONL(length);
    header.length_captured = BTSNOOP_HTONL(length);
    header.flags = flags;
    header.dropped_packets = 0;
    header.timestamp = BTSNOOP_HTONLL(timestamp);

    if (file.isOpen()) {
        file.write((char*)&header, sizeof(header));
        file.write((char*)data, length);
    }
}

void btsnoop_close(void) {
    if (file.isOpen()) {
        file.close();
    }
}




