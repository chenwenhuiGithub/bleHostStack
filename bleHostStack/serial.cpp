#include "serial.h"

static QSerialPort serialPort;

QSerialPort* serial_get_instance() {
    return &serialPort;
}

bool serial_open(QString portName) {
    if (serialPort.isOpen()) {
        serialPort.close();
    }

    serialPort.setPortName(portName);
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);
    return serialPort.open(QIODevice::ReadWrite);
}

QByteArray serial_read() {
    return serialPort.readAll();
}

int64_t serial_write(uint8_t *data, uint32_t length) {
    return serialPort.write((char*)data, length);
}

void serial_close() {
    if (serialPort.isOpen()) {
        serialPort.close();
    }
}
