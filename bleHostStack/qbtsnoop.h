#ifndef QBTSNOOP_H
#define QBTSNOOP_H

#include <stdint.h>
#include <QFile>

typedef enum {
    BTSNOOP_DIRECT_HOST_TO_CONTROLLER,
    BTSNOOP_DIRECT_CONTROLLER_TO_HOST
} BTSNOOP_DIRECT;

class QBtsnoop
{
public:
    QBtsnoop();
    void open(void);
    void wirte(uint8_t* data, uint32_t len, BTSNOOP_DIRECT direct);
    void close(void);

private:
    QFile file;
};

#endif // QBTSNOOP_H
