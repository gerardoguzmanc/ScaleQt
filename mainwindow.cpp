#include "mainwindow.h"
#include "ui_mainwindow.h"

static QString serialBuffer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ScaleSerial(new QSerialPort(this))
    , loopTimer(new QTimer(this)) // Initialize the QTimer
{
    ui->setupUi(this);

    // === CRITICAL: Ensure this connection exists and is correct ===
    connect(ScaleSerial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
    // =============================================================

    // Connect the QTimer's timeout signal to your new slot
    connect(loopTimer, &QTimer::timeout, this, &MainWindow::sendDataLoop);

}

MainWindow::~MainWindow()
{
    // Stop the timer to prevent calls after the object is destroyed
    if (loopTimer->isActive()) {
        loopTimer->stop();
    }
    delete ui;
}

void MainWindow::readData()
{
    // This function is not currently used or connected to any signal.
    // If it's meant for something specific, ensure it's called appropriately.
    // Otherwise, it might be redundant.
    QByteArray data = ScaleSerial->readAll();
    ui->ShowWeight->display(data);
    qDebug() << "Received (in readData):" << data;
}

void MainWindow::on_pushButtonConnection_clicked()
{

    if(ScaleSerial->isOpen()){
        ScaleSerial->close();
        ui->pushButtonConnection->setText(("Conectar"));
    }
    else{

        // 1. Get the selected port name from the QComboBox
        // We stored the actual port name in the item's data (Qt::UserRole)
        QString selectedPortName = ui->comboBoxSerialPorts->currentData().toString();
        qint32 SelectedBaudRate;
        ui->comboBoxSerialPorts->clear();
        // Check if a valid port was selected (e.g., not "No serial ports found")
        if (selectedPortName.isEmpty() || selectedPortName.contains("No serial ports found")) {
            QMessageBox::warning(this, "Serial Port Error", "Please select a valid serial port.");
            qDebug() << "Attempted to open an invalid or empty port selection.";
            return;
        }

        // 2. Configure the QSerialPort object
        ScaleSerial->setPortName(selectedPortName);
        if(ui->comboBoxBaudRates->currentIndex() == 0){
            SelectedBaudRate = QSerialPort::Baud9600;
        }
        if(ui->comboBoxBaudRates->currentIndex() == 1){
           SelectedBaudRate = QSerialPort::Baud115200;
        }
        ScaleSerial->setBaudRate(SelectedBaudRate); // Common baud rate for many devices
        ScaleSerial->setDataBits(QSerialPort::Data8);
        ScaleSerial->setParity(QSerialPort::NoParity);
        ScaleSerial->setStopBits(QSerialPort::OneStop);
        ScaleSerial->setFlowControl(QSerialPort::NoFlowControl);

        // 3. Open the serial port
        if (ScaleSerial->open(QIODevice::ReadWrite)){
            qDebug() << "Successfully opened serial port:" << selectedPortName;
            qDebug() << "at " << SelectedBaudRate;
            // Optionally update UI to show connected status, disable open button, enable disconnect etc.
            ui->pushButtonConnection->setText(("Desconectar"));

        } else {
            // Handle error: port could not be opened
            qDebug() << "Failed to open serial port:" << selectedPortName << "Error:" << ScaleSerial->errorString();
            QMessageBox::critical(this, "Serial Port Error",
                                  QString("Could not open serial port %1. Error: %2")
                                      .arg(selectedPortName)
                                      .arg(ScaleSerial->errorString()));
        }
    }
}

// This function is now for a *manual, one-time* send.
// It still attempts to read immediately, which might not get data.
// For reliable reading, rely on readSerialData connected to readyRead.
void MainWindow::on_pushButtonSend_clicked()
{
    if (!ScaleSerial->isOpen()) {
        qDebug() << "Serial port is not open. Cannot start loop.";
        QMessageBox::warning(this, "Serial Port Not Open", "Please connect to the serial port before starting the weight loop.");
        return;
    }

    if (isLoopActive) {
        // If the loop is active, stop it
        loopTimer->stop();
        isLoopActive = false;
        //ui->PushButton->setText("Start Weight Loop"); // Change button text
        qDebug() << "Weight reading loop stopped.";
    } else {
        // If the loop is not active, start it
        // Set the interval for the timer in milliseconds (e.g., 500ms for 2 times per second)
        // Adjust this value based on how often you want to poll the scale.
        loopTimer->start(500); // Trigger sendDataLoop every 500 milliseconds (0.5 seconds)
        isLoopActive = true;
        //ui->PushButton->setText("Stop Weight Loop"); // Change button text
        qDebug() << "Weight reading loop started.";
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
    QRegularExpression regex("^([+-]?\\d*\\.?\\d+)\\s*(.*)$");
    QRegularExpressionMatch match = regex.match(SerialRecvInfo);

    double numericValue = 0.0;
    QString unitText = "";

    if (match.hasMatch()) {
        numericValue = match.captured(1).toDouble();
        unitText = match.captured(2).trimmed();
        qDebug() << "Parsed: Numeric=" << numericValue << ", Unit='" << unitText << "'";
    } else {
        qDebug() << "DisplayInfo: Format of string not recognized: '" << SerialRecvInfo << "'";
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
    const QByteArray receivedBytes = ScaleSerial->readAll();

    // Convert the received QByteArray to QString.
    QString receivedStringChunk = QString::fromUtf8(receivedBytes);

    // Append to the buffer to handle incomplete messages (if data comes in chunks).
    serialBuffer += receivedStringChunk;

    // Check for the message terminator (e.g., '\r' for "0.084 kg\r").
    while (serialBuffer.contains('\r')) { // Use a while loop to process multiple messages if they arrive in one go
        int endIndex = serialBuffer.indexOf('\r');
        QString completeMessage = serialBuffer.left(endIndex).trimmed(); // Extract and trim the message

        // Process the complete message
        qDebug() << "Complete message received (from readyRead): '" << completeMessage << "'";

        // Pass the complete, parsed message to DisplayInfo
        DisplayInfo(completeMessage);

        // Remove the processed message (and its terminator) from the buffer
        serialBuffer.remove(0, endIndex + 1); // +1 to remove the '\r' itself
    }
}


void MainWindow::sendDataLoop()
{
    // This function is called by the QTimer periodically
    if (ScaleSerial->isOpen()) {
        QByteArray dataToSend = "P";
        qint64 bytesWritten = ScaleSerial->write(dataToSend);

        if (bytesWritten == -1) {
            qCritical() << "Failed to write to serial port during loop:" << ScaleSerial->errorString();
            // Optionally, stop the timer if there's a persistent write error
            // loopTimer->stop();
            // isLoopActive = false;
            // ui->PushButton->setText("Start Weight Loop");
            QMessageBox::critical(this, "Write Error in Loop", ScaleSerial->errorString());
        } else if (bytesWritten != dataToSend.size()) {
            qWarning() << "Warning: Not all data written to serial port during loop.";
        } else {
            qDebug() << "Sent 'P' command in loop.";
            // IMPORTANT: Do NOT call readAll() here.
            // The readSerialData slot (connected to readyRead) will handle incoming data.
        }
    } else {
        qDebug() << "Serial port closed, stopping weight reading loop.";
        loopTimer->stop();
        isLoopActive = false;
        //ui->PushButton->setText("Start Weight Loop");
        QMessageBox::warning(this, "Serial Port Closed", "The serial port was closed. Stopping weight reading loop.");
    }
}

void MainWindow::on_pushButtonSerialConfig_clicked()
{
    // Clear any existing items in the QComboBox before adding new ones
    // Assuming your QComboBox is named 'serialPortComboBox' in mainwindow.ui
    ui->comboBoxSerialPorts->clear();

    qDebug() << "Scanning for serial ports...";

    // Get a list of all available serial ports
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    if (serialPortInfos.isEmpty()) {
        ui->comboBoxSerialPorts->addItem("No serial ports found");
        qDebug() << "No serial ports found.";
    } else {
        // Iterate through the list of found ports
        for (const QSerialPortInfo &portInfo : serialPortInfos) {
            QString portDetails;
            portDetails += portInfo.portName(); // e.g., "COM3", "/dev/ttyUSB0"

            if (!portInfo.description().isEmpty()) {
                portDetails += " - " + portInfo.description(); // e.g., "USB Serial Device"
            }
            // You can add more details if you like:
            // if (!portInfo.manufacturer().isEmpty()) {
            //     portDetails += " (" + portInfo.manufacturer() + ")";
            // }
            // if (!portInfo.serialNumber().isEmpty()) {
            //     portDetails += " [SN: " + portInfo.serialNumber() + "]";
            // }

            // Add the descriptive string to the QComboBox.
            // We also store the actual port name (e.g., "COM3") as the item's data (Qt::UserRole).
            // This makes it easy to retrieve the actual port name when the user selects an item.
            ui->comboBoxSerialPorts->addItem(portDetails, QVariant(portInfo.portName()));

            qDebug() << "Found Port: " << portDetails;
            qDebug() << "  System Location: " << portInfo.systemLocation();
            qDebug() << "  Vendor ID: " << (portInfo.hasVendorIdentifier() ? QString::number(portInfo.vendorIdentifier(), 16) : "N/A");
            qDebug() << "  Product ID: " << (portInfo.hasProductIdentifier() ? QString::number(portInfo.productIdentifier(), 16) : "N/A");
        }
    }
}


void MainWindow::on_pushButtonAsciiHex_clicked()
{

}

