#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qserialport.h>
#include <qtimer.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonConnection_clicked();

    void on_pushButtonSend_clicked();

private:
    Ui::MainWindow *ui;
    void pushButtonSerialSendFunc();
    void readData();
    void populateSerialPorts();
    void updateConnectButtons(bool connected);

    QSerialPort *ScaleSerial;


    static constexpr int DEFAULT_BAUD_RATE = 9600; // Define a default baud rate


};
#endif // MAINWINDOW_H
