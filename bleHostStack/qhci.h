#ifndef QHCI_H
#define QHCI_H

#include <stdint.h>
#include <QSerialPort>
#include "qbtsnoop.h"

class QHci
{
public:
    QHci(QSerialPort& serialPort, QBtsnoop& btsnoop);
    void reset();

private:
    void _assign_command(uint8_t* buf, uint8_t ogf, uint16_t ocf);

    QSerialPort& serialPort;
    QBtsnoop& btsnoop;
};

#endif // QHCI_H
