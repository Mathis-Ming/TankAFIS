import asyncio
from bleak import BleakClient
from PyQt5.QtCore import QThread, pyqtSignal, QRunnable, QThreadPool
import time

# Remplacez ceci par l'adresse MAC de votre périphérique BLE
ADDRESS = "70:3E:97:E3:65:1B"
# Remplacez ceci par l'UUID de la caractéristique Write
WRITE_CHAR_UUID = "0000FFE3-0000-1000-8000-00805F9B34FB"
NOTIFY_CHAR_UUID = "0000FFE2-0000-1000-8000-00805F9B34FB"

class BluetoothClient(QRunnable):

    def __init__(self, device_address, characteristic_uuid, charachteristic_notify_uuid, notifying, message=""):
        super().__init__()
        self.device_address = device_address
        self.characteristic_uuid = characteristic_uuid
        self.characteristic_notify_uuid = charachteristic_notify_uuid
        self.message = message
        self.notifying = notifying
        #self.signals = pyqtSignal()

    async def write_to_device(self):
        client = BleakClient(self.device_address)
        message_encoded = self.message.encode('utf-8')
        try:
            await client.connect()
            print("Connecté.")
            await client.write_gatt_char(self.characteristic_uuid, message_encoded, response=True)
            print(f"Message '{self.message}' envoyé.")
            await client.disconnect()
            print("Déconnecté.")
        except Exception as e:
            print(e)

    @staticmethod
    def asyncio_run(coroutine):
        #loop = asyncio.get_event_loop()
        #asyncio.set_event_loop(loop)
        #result = loop.run_until_complete(coroutine)
        #return result
        asyncio.run(coroutine)
        return

    def notification_handler(self, sender, data):
        print("Données reçues: ", data.decode('utf-8'))

    async def notify_from_device(self):
        client = BleakClient(self.device_address)
        try:
            await client.connect()
            print("Connecté.")
            await client.start_notify(self.characteristic_notify_uuid, self.notification_handler)
            print("Notification activée. En attente de données...")
            while self.notifying:
                await asyncio.sleep(0.2)  # Modifier si nécessaire pour attendre plus ou moins longtemps
            await client.stop_notify(self.characteristic_notify_uuid)
            print("Notification désactivée.")
            await client.disconnect()
            print("Déconnecté.")
        except Exception as e:
            print(e)


    def run(self):
        if self.notifying == True:
            self.asyncio_run(self.notify_from_device())
        else:
            self.asyncio_run(self.write_to_device())
        #self.signals.completed.emit()


if __name__ == "__main__":
    bluetooth_client = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, False, "off")
    bluetooth_client.run()

    print("HEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEERE")



    #bluetooth_client.notifying = False
    #bluetooth_client.run()
    #bluetooth_client_2 = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, "off")
    #bluetooth_client_2.run()

    #asyncio.run(write_to_device(ADDRESS, WRITE_CHAR_UUID, "off"))