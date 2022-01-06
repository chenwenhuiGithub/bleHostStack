#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&serial, &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);

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
        serial.setPortName(ui->comboBoxNum->currentText());
        serial.setBaudRate(QSerialPort::Baud115200);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        if (! serial.open(QIODevice::ReadWrite)) {
            qDebug() << "open serial fail";
            return;
        }
        ui->pushButtonOpen->setText("Close");
        qDebug() << "open serial success";
    } else {
        serial.close();
        ui->pushButtonOpen->setText("Open");
        qDebug() << "close serial";
    }
}


void MainWindow::on_pushButtonSend_clicked()
{
    uint8_t bufSend[4] = {0x01, 0x03, 0x0c, 0x00};
    serial.write((char*)bufSend, sizeof(bufSend));

    snoop.btsnoop_open();
    snoop.btsnoop_wirte(bufSend, sizeof(bufSend), BTSNOOP_DIRECT_HOST_TO_CONTROLLER);
}


void MainWindow::serialPort_readyRead()
{
    QByteArray buf = serial.readAll();
    qDebug() << "receive: " << buf;

    uint8_t bufRecv[7] = {0x04, 0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};
    snoop.btsnoop_wirte(bufRecv, sizeof(bufRecv), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
    snoop.btsnoop_close();
}
