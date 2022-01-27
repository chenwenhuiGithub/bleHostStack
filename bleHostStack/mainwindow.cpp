#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include "serial.h"
#include "hci.h"
#include "btsnoop.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialPort_buf.clear();
    connect(&serialPort_timer, &QTimer::timeout, this, &MainWindow::serialPort_timeout);
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
            qDebug() << "serial open failed";
            return;
        }
        ui->pushButtonOpen->setText("Close");
        btsnoop_open();
        qDebug() << "serial open success";
    } else {
        serial_close();
        btsnoop_close();
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
    serialPort_timer.start(100); // TODO: use ringbuffer
    serialPort_buf.append(serial_read());
}


void MainWindow::serialPort_timeout()
{
    serialPort_timer.stop();

    // qDebug() << "recv len: " << serialPort_buf.length() << " " << serialPort_buf;
    btsnoop_wirte((uint8_t*)(serialPort_buf.data()), serialPort_buf.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
    hci_recv((uint8_t*)(serialPort_buf.data()), serialPort_buf.length());
    serialPort_buf.clear();
}
