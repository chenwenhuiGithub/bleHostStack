#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ringbufferreader.h"

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
    QRingbufferReader *readerThread;
};
#endif // MAINWINDOW_H
