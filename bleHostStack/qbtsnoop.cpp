#include "qbtsnoop.h"
#include <QDateTime>

#define BTSNOOP_HTONL(x)    ((((x) & 0x000000ffUL) << 24) | \
                             (((x) & 0x0000ff00UL) <<  8) | \
                             (((x) & 0x00ff0000UL) >>  8) | \
                             (((x) & 0xff000000UL) >> 24))
#define BTSNOOP_HTONLL(x)   ((((uint64_t)BTSNOOP_HTONL(x)) << 32) + BTSNOOP_HTONL((x) >> 32))

typedef enum {
    HCI_PACKET_TYPE_CMD = 1,
    HCI_PACKET_TYPE_ACL,
    HCI_PACKET_TYPE_SCO,
    HCI_PACKET_TYPE_EVT
} HCI_PACKET_TYPE;

struct btsnoop_header {
    uint32_t length_original;
    uint32_t length_captured;
    uint32_t flags;
    uint32_t dropped_packets;
    uint64_t timestamp;
};


QBtsnoop::QBtsnoop()
{

}

void QBtsnoop::open(void)
{
    if (file.isOpen()) {
        file.close();
    }

    QDateTime curDateTime = QDateTime::currentDateTime();
    QString fileName = curDateTime.toString("yyyy_MM_dd_hh_mm_ss") + ".dat";
    file.setFileName(fileName);
    file.open(QIODevice::WriteOnly);

    uint8_t header[16] = {'b', 't', 's', 'n', 'o', 'o', 'p', 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0xea}; // HCI type: H4
    file.write((char*)header, sizeof(header));
}

void QBtsnoop::wirte(uint8_t* data, uint32_t len, BTSNOOP_DIRECT direct)
{
    uint8_t type = data[0];
    uint32_t flags = 0x00; // bit0: 0 - Host to Controller, 1 - Controller to Host
                           // bit1: 0 - ACL/SCO, 1 - CMD/EVT
    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() * 1000ULL + 0x00dcddb30f2f8000ULL; // 1970-01-01 00:00:00

    if (HCI_PACKET_TYPE_CMD == type) {
        flags = 0x02;
    } else if (HCI_PACKET_TYPE_EVT == type) {
        flags = 0x03;
    } else if ((HCI_PACKET_TYPE_ACL == type) || (HCI_PACKET_TYPE_SCO == type)) {
        if (BTSNOOP_DIRECT_HOST_TO_CONTROLLER == direct) {
            flags = 0x00;
        } else {
            flags = 0x01;
        }
    } else {

    }

    struct btsnoop_header header;
    header.length_original = BTSNOOP_HTONL(len);
    header.length_captured = BTSNOOP_HTONL(len);
    header.flags = flags;
    header.dropped_packets = 0;
    header.timestamp = BTSNOOP_HTONLL(timestamp);

    if (file.isOpen()) {
        file.write((char*)&header, sizeof(header));
        file.write((char*)data, len);
    }
}

void QBtsnoop::close(void)
{
    if (file.isOpen()) {
        file.close();
    }
}
