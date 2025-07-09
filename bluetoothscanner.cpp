#include "bluetoothscanner.h"

#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>


BluetoothScanner::BluetoothScanner(QObject *parent)
    : QObject{parent}
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    // Establece el tiempo de espera para el descubrimiento de BLE (en milisegundos)
    discoveryAgent->setLowEnergyDiscoveryTimeout(25000); // 25 segundos

    // Conecta las señales del agente de descubrimiento a nuestros slots
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothScanner::addDevice);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BluetoothScanner::deviceScanError);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothScanner::deviceScanFinished);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BluetoothScanner::deviceScanFinished); // Cancelado también finaliza
}

BluetoothScanner::~BluetoothScanner()
{
    // El QBluetoothDeviceDiscoveryAgent es un QObject y se eliminará
    // automáticamente cuando su padre (BluetoothScanner) sea eliminado,
    // o cuando el QObject padre se elimine.
    // No es estrictamente necesario llamar a delete discoveryAgent; aquí
    // si el padre está configurado correctamente.
}

void BluetoothScanner::startScan()
{
    qDebug() << "Iniciando escaneo de dispositivos Bluetooth...";
    discoveredDevices.clear(); // Limpiar la lista de dispositivos anteriores
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod); // Opcional: QBluetoothDeviceDiscoveryAgent::ClassicMethod o AllMethods
}

void BluetoothScanner::stopScan()
{
    if (discoveryAgent->isActive()) {
        discoveryAgent->stop();
        qDebug() << "Escaneo de dispositivos Bluetooth detenido.";
    }
}

void BluetoothScanner::addDevice(const QBluetoothDeviceInfo &info)
{
    // Filtrar dispositivos duplicados si es necesario (QBluetoothDeviceDiscoveryAgent
    // puede reportar el mismo dispositivo varias veces).
    // Aquí, simplemente agregamos si el nombre no está vacío y emitimos la señal.

    if (info.isValid() && !info.name().isEmpty()) {
        bool alreadyExists = false;
        for (const QBluetoothDeviceInfo &device : std::as_const(discoveredDevices)) {
            if (device.address() == info.address()) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            discoveredDevices.append(info);
            qDebug() << "Dispositivo encontrado: " << info.name() << " (" << info.address().toString() << ")";
            emit deviceFound(info.name(), info.address().toString());
        }
    }
}

void BluetoothScanner::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString errorString;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorString = "Bluetooth está apagado.";
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        errorString = "Error de I/O al escanear.";
        break;
    //case QBluetoothDeviceDiscoveryAgent::UnsupportedMethodError:
      //  errorString = "Método de escaneo no soportado.";
        //break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        errorString = "Plataforma no soporta descubrimiento de Bluetooth.";
        break;
    case QBluetoothDeviceDiscoveryAgent::MissingPermissionsError:
        errorString = "Permisos de Bluetooth faltantes.";
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        errorString = "Adaptador Bluetooth inválido o no disponible.";
        break;
    default:
        errorString = "Error desconocido durante el escaneo: " + QString::number(error);
        break;
    }
    qWarning() << "Error al escanear dispositivos: " << errorString;
    emit scanError(errorString);
}

void BluetoothScanner::deviceScanFinished()
{
    qDebug() << "Escaneo de dispositivos Bluetooth finalizado. Dispositivos encontrados: " << discoveredDevices.size();
    emit scanFinished();
}
