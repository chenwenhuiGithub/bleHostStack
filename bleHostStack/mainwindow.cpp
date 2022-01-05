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
    QByteArray buf;
    buf.resize(4);
    buf[0] = 0x00;
    buf[1] = 0x01;
    buf[2] = 0x02;
    buf[3] = 0x03;
    serial.write(buf);
}


void MainWindow::serialPort_readyRead()
{
    QByteArray buf = serial.readAll();
    qDebug() << "receive: " << buf;
}
