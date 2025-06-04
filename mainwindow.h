#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qserialport.h>
#include <qserialportinfo.h>
#include <qtimer.h>
#include <QRegularExpression>
#include <QMessageBox>

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

    void on_pushButtonSerialConfig_clicked();

private:
    Ui::MainWindow *ui;
    void pushButtonSerialSendFunc();
    void readData();
    void populateSerialPorts();
    void readSerialData();
    void DisplayInfo(QString &SerialRecvInfo);
    void sendDataLoop(); // Slot for the timer to call, handles sending 'P' command

    QSerialPort *ScaleSerial;

    QTimer *loopTimer; // Declare a QTimer object
    bool isLoopActive = false; // To track if the loop is running

    static constexpr int DEFAULT_BAUD_RATE = 9600; // Define a default baud rate


};
#endif // MAINWINDOW_H
