#include "ringbufferreader.h"
#include "ringbuffer.h"
#include "hci.h"
#include "btsnoop.h"

typedef enum {
    STATUS_PACKET_TYPE,
    STATUS_HEADRER_EVT,
    STATUS_HEADRER_ACL,
    STATUS_HEADRER_SCO,
    STATUS_DATA_EVT,
    STATUS_DATA_ACL,
    STATUS_DATA_SCO
} RINGBUFFER_READER_STATUS;

QRingbufferReader::QRingbufferReader(QObject *parent)
    : QThread{parent}
{
    running_flag = true;
}

void QRingbufferReader::set_running_flag(bool flag) {
    running_flag = flag;
}

void QRingbufferReader::run() {
    RINGBUFFER_READER_STATUS status = STATUS_PACKET_TYPE;
    QByteArray byteArray;
    uint8_t packet_type = 0;
    uint8_t data_length_evt = 0;
    uint16_t data_length_acl = 0;
    bool result = false;

    while (running_flag) {
        if (!ringbuffer_is_empty()) {
            switch (status) {
            case STATUS_PACKET_TYPE:
                byteArray.clear();
                byteArray.resize(HCI_LENGTH_PACKET_TYPE);
                result = ringbuffer_read((uint8_t*)byteArray.data(), HCI_LENGTH_PACKET_TYPE);
                if (result) {
                    packet_type = byteArray[0];
                    if (packet_type == HCI_PACKET_TYPE_ACL) {
                        status = STATUS_HEADRER_ACL;
                    } else if (packet_type == HCI_PACKET_TYPE_SCO) {
                        status = STATUS_HEADRER_SCO;
                    } else if (packet_type == HCI_PACKET_TYPE_EVT) {
                        status = STATUS_HEADRER_EVT;
                    } else {
                        qDebug("ringbuffer_read invalid, packet_type:%u", packet_type);
                        return;
                    }
                } else {
                    qDebug("ringbuffer_read invalid, status:%u", STATUS_PACKET_TYPE);
                    return;
                }
                break;
            case STATUS_HEADRER_EVT:
                byteArray.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER);
                result = ringbuffer_read((uint8_t*)byteArray.data() + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_EVT_HEADER);
                if (result) {
                    status = STATUS_DATA_EVT;
                } else {
                    qDebug("ringbuffer_read invalid, status:%u", STATUS_HEADRER_EVT);
                    return;
                }
                break;
            case STATUS_HEADRER_ACL:
                byteArray.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER);
                result = ringbuffer_read((uint8_t*)byteArray.data() + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_ACL_HEADER);
                if (result) {
                    status = STATUS_DATA_ACL;
                } else {
                    qDebug("ringbuffer_read invalid, status:%u", STATUS_HEADRER_ACL);
                    return;
                }
                break;
            case STATUS_DATA_EVT:
                data_length_evt = byteArray[2];
                byteArray.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER + data_length_evt);
                result = ringbuffer_read((uint8_t*)byteArray.data() + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER, data_length_evt);
                if (result) {
                    btsnoop_wirte((uint8_t*)byteArray.data(), byteArray.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
                    hci_recv_evt((uint8_t*)byteArray.data() + HCI_LENGTH_PACKET_TYPE, byteArray.length() - HCI_LENGTH_PACKET_TYPE);
                    status = STATUS_PACKET_TYPE;
                } else {
                    qDebug("ringbuffer_read invalid, status:%u", STATUS_DATA_EVT);
                    return;
                }
                break;
            case STATUS_DATA_ACL:
                data_length_acl = byteArray[3] | (byteArray[4] << 8);
                byteArray.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + data_length_acl);
                result = ringbuffer_read((uint8_t*)byteArray.data() + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER, data_length_acl);
                if (result) {
                    btsnoop_wirte((uint8_t*)byteArray.data(), byteArray.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
                    hci_recv_acl((uint8_t*)byteArray.data() + HCI_LENGTH_PACKET_TYPE, byteArray.length() - HCI_LENGTH_PACKET_TYPE);
                    status = STATUS_PACKET_TYPE;
                } else {
                    qDebug("ringbuffer_read invalid, status:%u", STATUS_DATA_ACL);
                    return;
                }
                break;
            default:
                qDebug("ringbuffer_read invalid, status:%u", status);
                return;
            }
        } else {
            msleep(500);
        }
    }
}
