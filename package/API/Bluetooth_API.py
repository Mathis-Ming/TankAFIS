import asyncio
from bleak import BleakClient
from PyQt5.QtCore import  QObject, QRunnable, pyqtSignal
import time

#Adresse MAC du module Bluetooth
ADDRESS = "70:3E:97:E3:65:1B"

#UUID d'un service du module Bluetooth permettant de lui écrire des messages
WRITE_CHAR_UUID = "0000FFE3-0000-1000-8000-00805F9B34FB"

#UUID d'un service du module Bluetooth permettant de recevoir des messages
NOTIFY_CHAR_UUID = "0000FFE2-0000-1000-8000-00805F9B34FB"

class WorkerSignals(QObject):
    result = pyqtSignal(str)

class BluetoothClient(QRunnable):
    """
    Classe permettant d'écrire ou de recevoir des messages du module Bluetooth
    """


    def __init__(self, device_address, characteristic_uuid, charachteristic_notify_uuid, notifying, message=""):
        """
        :param device_address: string; Adresse MAC du module Bluetooth
        :param characteristic_uuid: string; UUID d'un service WRITE du module Bluetooth
        :param charachteristic_notify_uuid: string; UUID d'un service NOTIFY du module Bluetooth
        :param notifying: bool; Permet de savoir si on fait appel à une instance pour recevoir ou envoyer des messages au module Bluetooth
        :param message: string; Message envoyé au module Bluetooth
        """
        super().__init__()
        self.device_address = device_address
        self.characteristic_uuid = characteristic_uuid
        self.characteristic_notify_uuid = charachteristic_notify_uuid
        self.message = message
        self.notifying = notifying
        self.finished_operation = False
        self.started_notify = False
        self.connected = False
        self.connect_to = False
        self.disconnect_from = False
        self.worker_signal = WorkerSignals()
        self.notification = ""
        #self.signals = pyqtSignal()


    async def connect_to_device(self):
        self.client = BleakClient(self.device_address)
        try:
            print("Here")
            await self.client.connect()
            print("Connecté.")
            self.connected = True
        except Exception as e:
            print(e)

    async def disconnect_from_device(self):
        if not self.client:
            print("Client doesn't exist")
        else:
            try:
                #if self.client.is_connected:
                await self.client.disconnect()
                print("Déconnecté. ")
                self.connected = False
                #else:
                #    print("Not connected")
            except Exception as e:
                print(e)
            except RuntimeError as e:
                print("Déconnecté... ")


    async def write_to_device(self):
        """
        Méthode permettant d'écrire au module Bluetooth
        :return:
        """

        #client = BleakClient(self.device_address)                                                   #On créé un client (ordinateur) pouvant se connecter au module Bluetooth (serveur)
        message_encoded = self.message.encode('utf-8')                                              #Encodage du message en utf-8
        try:
            if not self.client.is_connected:
                print("Connecting ...")
                self.client = BleakClient(self.device_address)
                await self.client.connect()                                                                  #Connexion au serveur de manière asynchrone
                print("Connecté.")
            else:
                #await client.write_gatt_char(self.characteristic_uuid, message_encoded, response=True)  #Ecriture du message au module Bluetooth
                await self.client.write_gatt_char(self.characteristic_uuid, message_encoded)
                print(f"Message '{self.message}' envoyé.")
                #await self.client.disconnect()                                                               #Deconnexion du module Bluetooth
                self.finished_operation = True
                #print("Déconnecté.")
        except Exception as e:
            print(e)


    def asyncio_run(self, coroutine):
        """
        Méthode statique permettant de lancer une fonction de manière asynchrone
        :param coroutine: function; Qui sera soit la fonction de notification ou soit la fonction d'écriture
        :return: None;
        """
        #loop = asyncio.get_event_loop()
        #asyncio.set_event_loop(loop)
        #result = loop.run_until_complete(coroutine)
        #return result
        try:
            asyncio.run(coroutine)
        except Exception as e:
            print(e)
        return

    def notification_handler(self, sender, data):
        """
        Méthode permettant de recevoir un message de la part du module Bluetooth et le décoder
        :param sender:
        :param data: string; Donnée envoyée par le module Bluetooth
        :return:
        """
        try:
            if all(bit == 0 for bit in bytearray(data)):
                return
            else:
                #print(data.decode('utf-8'))
                self.notification = data.decode('utf-8')
                time.sleep(0.05)
                self.worker_signal.result.emit(self.notification)
        except Exception as e:
            print(e)


    async def notify_from_device(self):
        """
        Méthode permettant la réception des données du module Bluetooth
        :return:
        """
        #client = BleakClient(self.device_address)                                                       #On créé un client (ordinateur) pouvant se connecter au module Bluetooth
        try:
            if not self.client.is_connected:
                print("Connecting ...")
                self.client = BleakClient(self.device_address)
                await self.client.connect()  # Connexion au serveur de manière asynchrone
                print("Connecté.")
            await self.client.start_notify(self.characteristic_notify_uuid, self.notification_handler)       #Réception des messages venant du serveur
            print("Notification activée. En attente de données...")
            self.started_notify = True
            while self.notifying:
                await asyncio.sleep(0.2)                                                                #Réception tant que self.notifying est à True
            await self.client.stop_notify(self.characteristic_notify_uuid)                                   #Arrêt de la réception dès que self.notifying est à False
            print("Notification désactivée.")
            #await client.disconnect()                                                                   #Déconnexion du client
            self.finished_operation = True
            #print("Déconnecté.")
        except Exception as e:
            print(e)


    def run(self):
        """
        Méthode virtuelle de QRunnable qui est possible d'être lancée par un objet QThreadPool (situé dans le fichier mainwindow.py)
        Cela permet de lancer la fonction de lancement de message ou de notification dans un thread pour ne pas déranger une interface graphique
        :return:
        """
        try:
            print(self.disconnect_from)
            print(self.connect_to)
            if self.notifying == True and (self.connect_to == self.disconnect_from == False):                          #A True, on lance la réception des messages en provenance du module Bluetooth
                self.asyncio_run(self.notify_from_device())
            elif self.notifying == False and (self.connect_to == self.disconnect_from == False) and not self.message == "":                                               #A False, on lance l'écriture d'un message au module Bluetooth
                self.asyncio_run(self.write_to_device())
            elif self.connect_to == True:
                self.asyncio_run(self.connect_to_device())
            elif self.disconnect_from == True or self.notifying == False and (self.connect_to == self.disconnect_from == False) and self.message == "":
                self.asyncio_run(self.disconnect_from_device())
        except asyncio.CancelledError:
            pass
        except Exception as e:
            print(e)