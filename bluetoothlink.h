#ifndef BLUETOOTHLINK_H
#define BLUETOOTHLINK_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QBluetoothLocalDevice>

class BluetoothLink : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothLink(QObject *parent = nullptr);
    ~BluetoothLink();

    void startDiscovery();
    void openConnection(const QBluetoothDeviceInfo& deviceInfo);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo& deviceInfo);
    void connected();
    void newData();

private:
    QBluetoothDeviceDiscoveryAgent* _discoveryAgent;
    QBluetoothSocket* _socket;
    QBluetoothDeviceInfo _deviceInfo;
    QBluetoothLocalDevice _localDevice;

    void setupConnections();
};

#endif // BLUETOOTHLINK_H
