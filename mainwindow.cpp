#include "mainwindow.h"
#include "ui_mainwindow.h"

static QString serialBuffer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ScaleSerial(new QSerialPort(this))
{
    ui->setupUi(this);

    // === CRITICAL: Ensure this connection exists and is correct ===
    connect(ScaleSerial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
    // =============================================================

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
        QString receivedString = QString::fromUtf8(data);
        DisplayInfo(receivedString);
        //ui->ShowWeight->display(data); // Display what was sent
        qDebug() << "Sent data: " << dataToSend;
        qDebug() << "Data recv: " << data;
    }
}

void MainWindow::DisplayInfo(QString &SerialRecvInfo){
    // Check if the input string is empty or just whitespace
    if (SerialRecvInfo.trimmed().isEmpty()) {
        qDebug() << "DisplayInfo: Received empty or whitespace-only string.";
        ui->ShowWeight->display(0.0); // Clear or reset display
        ui->ScaleUnits->setText("");
        return;
    }

    // Your regex for "number text" format
    // Ensure the regex is valid; consider adding regex.isValid() check here too
    QRegularExpression regex("^([+-]?\\d*\\.?\\d+)\\s*(.*)$");
    QRegularExpressionMatch match = regex.match(SerialRecvInfo);

    double numericValue = 0.0;
    QString unitText = "";

    if (match.hasMatch()) {
        numericValue = match.captured(1).toDouble();
        unitText = match.captured(2).trimmed();
        qDebug() << "Parsed: Numeric=" << numericValue << ", Unit='" << unitText << "'";
    } else {
        qDebug() << "DisplayInfo: Formato de string no reconocido: '" << SerialRecvInfo << "'";
        // Handle cases where the string doesn't match the expected format
        ui->ShowWeight->display(0.0);
        ui->ScaleUnits->setText("ERROR"); // Indicate parsing failure
        return;
    }

    ui->ShowWeight->display(numericValue);
    ui->ScaleUnits->setText(unitText);
}


void MainWindow::readSerialData()
{
    // Read all available bytes from the serial port.
    // Use readAll() as readyRead() indicates new data has arrived.
    const QByteArray receivedBytes = ScaleSerial->readAll();

    // Convert the received QByteArray to QString.
    // IMPORTANT: Use the correct encoding for your scale's output. UTF-8 is common.
    QString receivedStringChunk = QString::fromUtf8(receivedBytes);

    // Append to the buffer to handle incomplete messages (if data comes in chunks).
    serialBuffer += receivedStringChunk;

    // Check for the message terminator (e.g., '\r' for "0.084 kg\r").
    // If your scale sends '\r\n', check for that or just '\n' depending on consistency.
    while (serialBuffer.contains('\r')) { // Use a while loop to process multiple messages if they arrive in one go
        int endIndex = serialBuffer.indexOf('\r');
        QString completeMessage = serialBuffer.left(endIndex).trimmed(); // Extract and trim the message

        // Process the complete message
        qDebug() << "Complete message received: '" << completeMessage << "'";

        // Pass the complete, parsed message to DisplayInfo
        // Make sure DisplayInfo handles the "0.084 kg" format correctly.
        DisplayInfo(completeMessage);

        // Remove the processed message (and its terminator) from the buffer
        serialBuffer.remove(0, endIndex + 1); // +1 to remove the '\r' itself
    }
}







