#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#define READ_BUFFER_SIZE                            1024

typedef enum {
    STATUS_PACKET_TYPE,
    STATUS_HEADRER_EVT,
    STATUS_HEADRER_ACL,
    STATUS_DATA_EVT,
    STATUS_DATA_ACL
} read_status_t;

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
    void on_pushButtonAdv_clicked();
    void on_pushButtonTestPeripheral_clicked();
    void serialPort_readyRead();

private:
    Ui::MainWindow *ui;
    read_status_t read_status;
    uint8_t read_buffer[READ_BUFFER_SIZE];
    uint32_t read_buffer_length;
    uint8_t is_processing;
};
#endif // MAINWINDOW_H
