#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , hci(serialPort, btsnoop)
{
    ui->setupUi(this);

    serialPort_buf.clear();
    connect(&serialPort_timer, &QTimer::timeout, this, &MainWindow::serialPort_timeout);
    connect(&serialPort, &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);

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
        serialPort.setPortName(ui->comboBoxNum->currentText());
        serialPort.setBaudRate(QSerialPort::Baud115200);
        serialPort.setDataBits(QSerialPort::Data8);
        serialPort.setParity(QSerialPort::NoParity);
        serialPort.setStopBits(QSerialPort::OneStop);
        serialPort.setFlowControl(QSerialPort::NoFlowControl);
        if (! serialPort.open(QIODevice::ReadWrite)) {
            qDebug() << "open serial fail";
            return;
        }
        ui->pushButtonOpen->setText("Close");
        btsnoop.open();
        qDebug() << "open serial success";
    } else {
        serialPort.close();
        btsnoop.close();
        ui->pushButtonOpen->setText("Open");
        qDebug() << "close serial";
    }
}


void MainWindow::on_pushButtonTest_clicked()
{
    hci.send_cmd_reset();
}


void MainWindow::serialPort_readyRead()
{
    serialPort_timer.start(100);
    serialPort_buf.append(serialPort.readAll());
}


void MainWindow::serialPort_timeout()
{
    serialPort_timer.stop();

    // qDebug() << "recv len: " << serialPort_buf.length() << " " << serialPort_buf;
    btsnoop.wirte((uint8_t*)(serialPort_buf.data()), serialPort_buf.length(), BTSNOOP_DIRECT_CONTROLLER_TO_HOST);
    hci.recv((uint8_t*)(serialPort_buf.data()), serialPort_buf.length());
    serialPort_buf.clear();
}

