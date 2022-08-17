#ifndef SERIAL_H
#define SERIAL_H

#include <QSerialPort>

QSerialPort* serial_get_instance();
bool serial_open(QString portName); // 115200-8-N-1
QByteArray serial_read();
int64_t serial_write(uint8_t *data, uint32_t length);
void serial_close();

#endif // SERIAL_H
