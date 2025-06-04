#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ScaleSerial(new QSerialPort(this))
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::readData()
{
    QByteArray data = ScaleSerial->readAll();
    ui->ShowWeight->display(data);
    qDebug() << "Received:" << data;
}

void MainWindow::on_pushButtonConnection_clicked()
{
    //QString portName = ui->comboBoxPort->currentData().toString(); // Gets the actual port name (e.g., "COM3")
    QString portName = "/dev/cu.usbmodem00000000001A1";
    ScaleSerial->setPortName(portName);
    ScaleSerial->setBaudRate(static_cast<QSerialPort::BaudRate>(9600)); // THIS WILL BE 9600!
    ScaleSerial->setDataBits(QSerialPort::Data8);
    ScaleSerial->setParity(QSerialPort::NoParity);
    ScaleSerial->setStopBits(QSerialPort::OneStop);
    ScaleSerial->setFlowControl(QSerialPort::NoFlowControl);

    if (ScaleSerial->open(QIODevice::ReadWrite)) {
        // Connection successful!
        //ui->textEditReceive->append("--- Connected to " + portName + " at " + QString::number(m_serialPort->baudRate()) + " baud ---");
        //updateConnectButtons(true); // Enable Send, Disable Connect/Port/Baud
        qDebug() << "Serial port opened successfully.";
    } else {
        // Connection failed, show error message
        //QMessageBox::critical(this, "Serial Port Error", m_serialPort->errorString());
        qCritical() << "Failed to open serial port:" << ScaleSerial->errorString();
    }
}




void MainWindow::on_pushButtonSend_clicked()
{
    // --- Check if the serial port is open before sending ---
    if (!ScaleSerial->isOpen()) {
        qDebug() << "Serial port is not open. Please connect first.";
        return; // Exit if not connected
    }

    QByteArray dataToSend = "P"; // Create a QByteArray with the character 'P'

    qint64 bytesWritten = ScaleSerial->write(dataToSend);

    if (bytesWritten == -1) {
        //QMessageBox::critical(this, "Write Error", ScaleSerial->errorString());
        qCritical() << "Failed to write to serial port:" << ScaleSerial->errorString();
    } else if (bytesWritten != dataToSend.size()) {
        qWarning() << "Warning: Not all data written to serial port.";
    } else {
        // Optional: clear the lineEditSend if you want it blank after sending
        // ui->lineEditSend->clear();
        QByteArray data = ScaleSerial->readAll();
        ui->ShowWeight->display(data); // Display what was sent
        qDebug() << "Sent data: " << dataToSend;
        qDebug() << "Data recv: " << data;
    }
}








