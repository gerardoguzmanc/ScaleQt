#ifndef BLUETOOTHSCANNER_H
#define BLUETOOTHSCANNER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QDebug>
#include <QList>
#ifdef Q_OS_ANDROID
#include <QPermission>
#include <QBluetoothPermission> // For Bluetooth-specific permissions
#include <QLocationPermission>  // Often needed for Bluetooth scanning, especially older Android
#endif

class BluetoothScanner : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothScanner(QObject *parent = nullptr);
    ~BluetoothScanner();

    void startScan();
    void stopScan();

signals:
    void deviceFound(const QString &deviceName, const QString &deviceAddress);
    void scanFinished();
    void scanError(const QString &errorString);

private slots:
    void addDevice(const QBluetoothDeviceInfo &info);
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void deviceScanFinished();

private:
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QList<QBluetoothDeviceInfo> discoveredDevices;
};

#endif // BLUETOOTHSCANNER_H
