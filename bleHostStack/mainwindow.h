#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

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
    void serialPort_timeout();

private:
    Ui::MainWindow *ui;
    QByteArray serialPort_buf; // serialPort recv buffer
    QTimer serialPort_timer;   // wait 100ms to avoid can't read all data
};
#endif // MAINWINDOW_H
