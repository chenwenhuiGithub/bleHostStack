#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

typedef enum {
    STATUS_PACKET_TYPE,
    STATUS_HEADRER_EVT,
    STATUS_HEADRER_ACL,
    STATUS_HEADRER_SCO,
    STATUS_DATA_EVT,
    STATUS_DATA_ACL,
    STATUS_DATA_SCO
} RINGBUFFER_READ_STATUS;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonOpen_clicked();
    void on_pushButtonTest_clicked();
    void serialPort_readyRead();

private:
    Ui::MainWindow *ui;
    RINGBUFFER_READ_STATUS ringbuffer_read_status;
    QByteArray ringbuffer_read_data;
};
#endif // MAINWINDOW_H
