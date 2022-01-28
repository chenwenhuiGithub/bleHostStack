#ifndef RINGBUFFERREADER_H
#define RINGBUFFERREADER_H

#include <QThread>

class QRingbufferReader : public QThread
{
public:
    explicit QRingbufferReader(QObject *parent = nullptr);

    void set_running_flag(bool flag);
    void run() override;

private:
    bool running_flag;
};

#endif // RINGBUFFERREADER_H
