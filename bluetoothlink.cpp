#include "bluetoothlink.h"
#include <QDebug>

BluetoothLink::BluetoothLink(QObject *parent)
    : QObject(parent),
    _discoveryAgent(new QBluetoothDeviceDiscoveryAgent),
    _socket(new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this))
{
    setupConnections();
}

BluetoothLink::~BluetoothLink() {
    if (_socket->state() == QBluetoothSocket::ConnectedState) {
        _socket->disconnectFromService();
        _socket->waitForDisconnected();
    }
    delete _discoveryAgent;
    delete _socket;
}
void BluetoothLink::setupConnections() {
    connect(_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothLink::deviceDiscovered);
    connect(_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, _discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deleteLater);
    connect(_socket, &QBluetoothSocket::connected, this, &BluetoothLink::connected);
    connect(_socket, &QBluetoothSocket::readyRead, this, &BluetoothLink::newData);
}

void BluetoothLink::startDiscovery() {
    _discoveryAgent->start();
}

void BluetoothLink::deviceDiscovered(const QBluetoothDeviceInfo& deviceInfo) {
    qDebug() << "Discovered device:" << deviceInfo.name() << deviceInfo.address().toString();
    // Choose a device to connect to, or implement your own logic
}

void BluetoothLink::openConnection(const QBluetoothDeviceInfo& deviceInfo) {
    _deviceInfo = deviceInfo;

    // Disconnect existing socket if any
    if (_socket->state() == QBluetoothSocket::ConnectedState) {
        _socket->disconnectFromService();
        _socket->waitForDisconnected();
    }

    // Connect to the Bluetooth device
    _socket->connectToService(deviceInfo.address(), QBluetoothUuid(QBluetoothUuid::SerialPort));
}

void BluetoothLink::connected() {
    qDebug() << "Connected to device:" << _deviceInfo.name() << _deviceInfo.address().toString();
}

void BluetoothLink::newData() {
    qDebug() << "Received data:" << _socket->readAll();
}
