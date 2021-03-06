#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QTime>
#include "ringbuffer.h"
#include "serial.h"
#include "hci.h"
#include "btsnoop.h"
#include "att.h"
#include "gatt.h"
#include "log.h"
#include "sm.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    read_status = STATUS_PACKET_TYPE;
    read_buffer_length = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBoxNum->addItem(info.portName());
    }
    connect(serial_get_instance(), &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);
    srand(QTime::currentTime().msecsSinceStartOfDay());
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButtonOpen_clicked()
{
    if (ui->pushButtonOpen->text() == "Open") {
        if (! serial_open(ui->comboBoxNum->currentText())) {
            LOG_ERROR("serial open failed");
            return;
        }
        btsnoop_open();
        ringbuffer_reset();
        ui->pushButtonOpen->setText("Close");
        LOG_INFO("serial open success");
    } else {
        serial_close();
        btsnoop_close();
        ui->pushButtonOpen->setText("Open");
        LOG_INFO("serial closed");
    }
}


void MainWindow::on_pushButtonStart_clicked()
{
    gatt_init();
    hci_init();
}


void MainWindow::on_pushButtonTest_clicked()
{
    const uint32_t data_length = 2048;
    uint32_t i = 0;
    uint8_t data[data_length] = {0};
    uint16_t connect_handle = 0x0000;
    uint16_t att_mtu = att_get_mtu(connect_handle);
    uint16_t maxPacketSize = att_mtu - 3;

    for (i = 0; i < data_length; i++) {
        data[i] = i;
    }

    for (i = 0; i < data_length/maxPacketSize; i++) {
        gatt_send_handle_value_notify(connect_handle, 0x1012, data + i*maxPacketSize, maxPacketSize);
    }
    if (data_length % maxPacketSize) {
        gatt_send_handle_value_notify(connect_handle, 0x1012, data + i*maxPacketSize, data_length % maxPacketSize);
    }
}


void MainWindow::serialPort_readyRead()
{
    uint8_t packet_type = 0;
    uint16_t acl_length = 0;
    bool ret = false;

    QByteArray byteArray = serial_read();
    ret = ringbuffer_write((uint8_t*)byteArray.data(), byteArray.length());
    if (false == ret) {
        LOG_ERROR("ringbuffer_write invalid, no enough memory");
        return;
    }

    while (!ringbuffer_is_empty()) {
        switch (read_status) {
        case STATUS_PACKET_TYPE:
            ret = ringbuffer_read(read_buffer, HCI_LENGTH_PACKET_TYPE);
            if (ret) {
                read_buffer_length = HCI_LENGTH_PACKET_TYPE;
                packet_type = read_buffer[0];
                if (packet_type == HCI_PACKET_TYPE_ACL) {
                    read_status = STATUS_HEADRER_ACL;
                }  else if (packet_type == HCI_PACKET_TYPE_EVT) {
                    read_status = STATUS_HEADRER_EVT;
                } else {
                    LOG_ERROR("ringbuffer_read invalid, packet_type:%u", packet_type);
                    return;
                }
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_PACKET_TYPE");
                return;
            }
            break;
        case STATUS_HEADRER_EVT:
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_EVT_HEADER);
            if (ret) {
                read_buffer_length += HCI_LENGTH_EVT_HEADER;
                read_status = STATUS_DATA_EVT;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_HEADRER_EVT");
                return;
            }
            break;
        case STATUS_HEADRER_ACL:
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_ACL_HEADER);
            if (ret) {
                read_buffer_length += HCI_LENGTH_ACL_HEADER;
                read_status = STATUS_DATA_ACL;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_HEADRER_ACL");
                return;
            }
            break;
        case STATUS_DATA_EVT:
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER, read_buffer[2]);
            if (ret) {
                read_buffer_length += read_buffer[2];
                btsnoop_wirte(read_buffer, read_buffer_length, BTSNOOP_PACKET_FLAG_EVT_RECV);
                hci_recv_evt(read_buffer + HCI_LENGTH_PACKET_TYPE, read_buffer_length - HCI_LENGTH_PACKET_TYPE);
                read_status = STATUS_PACKET_TYPE;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_DATA_EVT");
                return;
            }
            break;
        case STATUS_DATA_ACL:
            acl_length = read_buffer[3] | (read_buffer[4] << 8);
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER, acl_length);
            if (ret) {
                read_buffer_length += acl_length;
                btsnoop_wirte(read_buffer, read_buffer_length, BTSNOOP_PACKET_FLAG_ACL_SCO_RECV);
                hci_recv_acl(read_buffer + HCI_LENGTH_PACKET_TYPE, read_buffer_length - HCI_LENGTH_PACKET_TYPE);
                read_status = STATUS_PACKET_TYPE;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_DATA_ACL");
                return;
            }
            break;
        default:
            break;
        }
    }
}
