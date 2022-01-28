#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include "serial.h"
#include "hci.h"
#include "btsnoop.h"
#include "ringbuffer.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(serial_get_instance(), &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBoxNum->addItem(info.portName());
    }

    readerThread = new QRingbufferReader();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButtonOpen_clicked()
{
    if (ui->pushButtonOpen->text() == "Open") {
        if (! serial_open(ui->comboBoxNum->currentText())) {
            qDebug() << "serial open failed";
            return;
        }
        btsnoop_open();
        ringbuffer_reset();
        readerThread->set_running_flag(true);
        readerThread->start();
        ui->pushButtonOpen->setText("Close");
        qDebug() << "serial open success";
    } else {
        serial_close();
        btsnoop_close();
        readerThread->set_running_flag(false);
        ui->pushButtonOpen->setText("Open");
        qDebug() << "serial closed";
    }
}


void MainWindow::on_pushButtonTest_clicked()
{
    hci_send_cmd_reset();
}


void MainWindow::serialPort_readyRead()
{
    QByteArray byteArray = serial_read();
    ringbuffer_write((uint8_t*)byteArray.data(), byteArray.length());
}
