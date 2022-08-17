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
    is_processing = 0;
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
        if (!serial_open(ui->comboBoxNum->currentText())) {
            LOG_ERROR("serial open failed");
            return;
        }

        ui->pushButtonOpen->setText("Close");
        btsnoop_open();
        ringbuffer_reset();
        gatt_init();
        hci_init();
        LOG_INFO("serial open success");
    } else {
        serial_close();

        ui->pushButtonOpen->setText("Open");
        btsnoop_close();
        LOG_INFO("serial closed");
    }
}


void MainWindow::on_pushButtonAdv_clicked()
{
    if (ui->pushButtonAdv->text() == "Start Adv") {
        hci_send_cmd_le_set_adv_enable(HCI_LE_ADV_ENABLE);
        ui->pushButtonAdv->setText("Stop Adv");
        LOG_INFO("/***** adv started *****/");
    } else {
        hci_send_cmd_le_set_adv_enable(HCI_LE_ADV_DISABLE);
        ui->pushButtonAdv->setText("Start Adv");
        LOG_INFO("/***** adv stoped *****/");
    }
}


void MainWindow::on_pushButtonTestPeripheral_clicked()
{
    const uint32_t data_length = 1024;
    uint32_t i = 0;
    uint8_t data[data_length] = {0};
    uint16_t connect_handle = 0x0000;

    for (i = 0; i < data_length; i++) {
        data[i] = i;
    }
    gatt_send_passthrough(connect_handle, data, sizeof(data));
}


void MainWindow::on_pushButtonScan_clicked()
{
    if (ui->pushButtonScan->text() == "Start Scan") {
        hci_send_cmd_le_set_scan_enable(HCI_LE_SCAN_ENABLE, HCI_LE_SCAN_FILTER_DUPLICATE_ENABLE);
        ui->pushButtonScan->setText("Stop Scan");
        LOG_INFO("/***** scan started *****/");
    } else {
        hci_send_cmd_le_set_scan_enable(HCI_LE_SCAN_DISABLE, HCI_LE_SCAN_FILTER_DUPLICATE_ENABLE);
        ui->pushButtonScan->setText("Start Scan");
        LOG_INFO("/***** scan stoped *****/");
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

    if (is_processing) { // avoid function re-entrant, just write data to ringbuffer if is processing event or acl
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
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_HEADER_EVT);
            if (ret) {
                read_buffer_length += HCI_LENGTH_HEADER_EVT;
                read_status = STATUS_DATA_EVT;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_HEADRER_EVT");
                return;
            }
            break;
        case STATUS_HEADRER_ACL:
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE, HCI_LENGTH_HEADER_ACL);
            if (ret) {
                read_buffer_length += HCI_LENGTH_HEADER_ACL;
                read_status = STATUS_DATA_ACL;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_HEADRER_ACL");
                return;
            }
            break;
        case STATUS_DATA_EVT:
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_HEADER_EVT, read_buffer[2]);
            if (ret) {
                is_processing = 1;
                read_buffer_length += read_buffer[2];
                btsnoop_wirte(read_buffer, read_buffer_length, BTSNOOP_PACKET_FLAG_EVT_RECV);
                hci_recv_evt(read_buffer + HCI_LENGTH_PACKET_TYPE, read_buffer_length - HCI_LENGTH_PACKET_TYPE);
                read_status = STATUS_PACKET_TYPE;
                is_processing = 0;
            } else {
                LOG_DEBUG("ringbuffer_read invalid, no enough data, status:STATUS_DATA_EVT");
                return;
            }
            break;
        case STATUS_DATA_ACL:
            acl_length = read_buffer[3] | (read_buffer[4] << 8);
            ret = ringbuffer_read(read_buffer + HCI_LENGTH_PACKET_TYPE + HCI_LENGTH_HEADER_ACL, acl_length);
            if (ret) {
                is_processing = 1;
                read_buffer_length += acl_length;
                btsnoop_wirte(read_buffer, read_buffer_length, BTSNOOP_PACKET_FLAG_ACL_SCO_RECV);
                hci_recv_acl(read_buffer + HCI_LENGTH_PACKET_TYPE, read_buffer_length - HCI_LENGTH_PACKET_TYPE);
                read_status = STATUS_PACKET_TYPE;
                is_processing = 0;
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
