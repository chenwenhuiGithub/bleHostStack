#include "qbtsnoop.h"
#include <QDateTime>

#define BTSNOOP_HTONL(x)    ((((x) & 0x000000ffUL) << 24) | \
                             (((x) & 0x0000ff00UL) <<  8) | \
                             (((x) & 0x00ff0000UL) >>  8) | \
                             (((x) & 0xff000000UL) >> 24))
#define BTSNOOP_HTONLL(x)   ((((uint64_t)BTSNOOP_HTONL(x)) << 32) + BTSNOOP_HTONL((x) >> 32))

#define BTSNOOP_HCI_TYPE_CMD  0x01
#define BTSNOOP_HCI_TYPE_ACL  0x02
#define BTSNOOP_HCI_TYPE_SCO  0x03
#define BTSNOOP_HCI_TYPE_EVT  0x04

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

void QBtsnoop::btsnoop_open(void)
{
    if (btsnoopFile.isOpen()) {
        btsnoopFile.close();
    }

    QDateTime curDateTime = QDateTime::currentDateTime();
    QString fileName = curDateTime.toString("yyyy_MM_dd_hh_mm_ss") + ".dat";
    btsnoopFile.setFileName(fileName);
    btsnoopFile.open(QIODevice::WriteOnly);

    uint8_t header[16] = {'b', 't', 's', 'n', 'o', 'o', 'p', 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0xea}; // HCI type: H4
    btsnoopFile.write((char*)header, sizeof(header));
}

void QBtsnoop::btsnoop_wirte(uint8_t* data, uint32_t len, BTSNOOP_DIRECT direct)
{
    uint32_t flags = 0; // bit0: 0 - Host to Controller, 1 - Controller to Host
                        // bit1: 0 - ACL/SCO, 1 - CMD/EVT
    uint8_t type = data[0];

    switch (type) {
    case BTSNOOP_HCI_TYPE_CMD: flags = 0x02; break;
    case BTSNOOP_HCI_TYPE_EVT: flags = 0x03; break;
    case BTSNOOP_HCI_TYPE_ACL:
    case BTSNOOP_HCI_TYPE_SCO:
        if (direct == BTSNOOP_DIRECT_HOST_TO_CONTROLLER) {
            flags = 0x00;
        } else {
            flags = 0x01;
        }
        break;
    default: break;
    }

    struct btsnoop_header header;
    header.length_original = BTSNOOP_HTONL(len);
    header.length_captured = BTSNOOP_HTONL(len);
    header.flags = flags;
    header.dropped_packets = 0;
    header.timestamp = 0;

    if (btsnoopFile.isOpen()) {
        btsnoopFile.write((char*)&header, sizeof(header));
        btsnoopFile.write((char*)data, len);
    }
}

void QBtsnoop::btsnoop_close(void)
{
    if (btsnoopFile.isOpen()) {
        btsnoopFile.close();
    }
}
