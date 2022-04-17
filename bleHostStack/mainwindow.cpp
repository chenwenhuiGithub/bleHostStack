#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include "serial.h"
#include "hci.h"
#include "btsnoop.h"
#include "ringbuffer.h"
#include "gatt.h"
#include "log.h"
#include "sm.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ringbuffer_read_status = STATUS_PACKET_TYPE;
    connect(serial_get_instance(), &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBoxNum->addItem(info.portName());
    }
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


void MainWindow::on_pushButtonTest_clicked()
{
    gatt_init();
    hci_send_cmd_reset();
}


void MainWindow::serialPort_readyRead()
{
    uint8_t packet_type = 0;
    uint8_t data_length_evt = 0;
    uint8_t data_length_sco = 0;
    uint16_t data_length_acl = 0;
    bool result = false;

    QByteArray byteArray = serial_read();
    ringbuffer_write((uint8_t*)byteArray.data(), byteArray.length());

    while (!ringbuffer_is_empty()) {
        switch (ringbuffer_read_status) {
        case STATUS_PACKET_TYPE:
            ringbuffer_read_data.clear();
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data(), HCI_LENGTH_PACKET_TYPE);
            if (result) {
                packet_type = ringbuffer_read_data[0];
                if (packet_type == HCI_PACKET_TYPE_ACL) {
                    ringbuffer_read_status = STATUS_HEADRER_ACL;
                } else if (packet_type == HCI_PACKET_TYPE_SCO) {
                    ringbuffer_read_status = STATUS_HEADRER_SCO;
                } else if (packet_type == HCI_PACKET_TYPE_EVT) {
                    ringbuffer_read_status = STATUS_HEADRER_EVT;
                } else {
                    LOG_WARNING("ringbuffer_read invalid, packet_type:%u", packet_type);
                    return;
                }
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_PACKET_TYPE");
                return;
            }
            break;
        case STATUS_HEADRER_EVT:
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_EVT_HEADER);
            if (result) {
                ringbuffer_read_status = STATUS_DATA_EVT;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_HEADRER_EVT");
                return;
            }
            break;
        case STATUS_HEADRER_ACL:
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_ACL_HEADER);
            if (result) {
                ringbuffer_read_status = STATUS_DATA_ACL;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_HEADRER_ACL");
                return;
            }
            break;
        case STATUS_HEADRER_SCO:
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_SCO_HEADER);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_SCO_HEADER);
            if (result) {
                ringbuffer_read_status = STATUS_DATA_SCO;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_HEADRER_SCO");
                return;
            }
            break;
        case STATUS_DATA_EVT:
            data_length_evt = ringbuffer_read_data[2];
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER + data_length_evt);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_EVT_HEADER, data_length_evt);
            if (result) {
                btsnoop_wirte((uint8_t*)ringbuffer_read_data.data(), ringbuffer_read_data.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
                hci_recv_evt((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE, ringbuffer_read_data.length() - HCI_LENGTH_PACKET_TYPE);
                ringbuffer_read_status = STATUS_PACKET_TYPE;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_DATA_EVT");
                return;
            }
            break;
        case STATUS_DATA_ACL:
            data_length_acl = ringbuffer_read_data[3] | (ringbuffer_read_data[4] << 8);
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER + data_length_acl);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_ACL_HEADER, data_length_acl);
            if (result) {
                btsnoop_wirte((uint8_t*)ringbuffer_read_data.data(), ringbuffer_read_data.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
                hci_recv_acl((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE, ringbuffer_read_data.length() - HCI_LENGTH_PACKET_TYPE);
                ringbuffer_read_status = STATUS_PACKET_TYPE;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_DATA_ACL");
                return;
            }
            break;
        case STATUS_DATA_SCO:
            data_length_sco = ringbuffer_read_data[3];
            ringbuffer_read_data.resize(HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_SCO_HEADER + data_length_sco);
            result = ringbuffer_read((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_SCO_HEADER, data_length_sco);
            if (result) {
                btsnoop_wirte((uint8_t*)ringbuffer_read_data.data(), ringbuffer_read_data.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
                hci_recv_sco((uint8_t*)ringbuffer_read_data.data() + HCI_LENGTH_PACKET_TYPE, ringbuffer_read_data.length() - HCI_LENGTH_PACKET_TYPE);
                ringbuffer_read_status = STATUS_PACKET_TYPE;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, status:STATUS_DATA_SCO");
                return;
            }
            break;
        default:
            LOG_WARNING("ringbuffer_read invalid, status:%u", ringbuffer_read_status);
            return;
        }
    }
}
