#include <QCoreApplication>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>

const QBluetoothUuid serviceUUID(QBluetoothUuid::fromString("00001101-0000-1000-8000-00805F9B34FB")); // SerialPort UUID

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Create a device discovery agent
    QBluetoothDeviceDiscoveryAgent discoveryAgent;

    discoveryAgent.setLowEnergyDiscoveryTimeout(10000); // Set discovery timeout to 10 seconds

    // Discover Bluetooth devices
    discoveryAgent.start();

    QObject::connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, [](const QBluetoothDeviceInfo &device) {
        // Check if the device is an Arduino
        if (device.name() == "Arduino") {
            // Create a Bluetooth socket
            QBluetoothSocket socket;

            // Find the Arduino's service UUID
            // This can be obtained by checking the Arduino's documentation or using a Bluetooth scanner
            const QBluetoothUuid serviceUUID(QBluetoothUuid::SerialPort);

            // Connect to the Arduino's service
            socket.connectToService(device, serviceUUID);

            QObject::connect(&socket, &QBluetoothSocket::connected, []() {
                qDebug("Connected to Arduino!");
            });

            QObject::connect(&socket, &QBluetoothSocket::disconnected, []() {
                qDebug("Disconnected from Arduino!");
            });

            QObject::connect(&socket, &QBluetoothSocket::readyRead, []() {
                // Read data from the Arduino
                QByteArray data = socket.readAll();

                // Process the data
                qDebug() << data.data();
            });
        }
    });

    return a.exec();
}
