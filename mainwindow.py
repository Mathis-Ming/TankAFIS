from PyQt5.QtWidgets import QWidget, QPushButton, QHBoxLayout
from PyQt5.QtCore import QThreadPool
from Bluetooth_test import BluetoothClient, ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID

class MainWindow(QWidget):
    """
    Classe spéciale de PyQt5 permettant la création d'une fenêtre
    """
    def __init__(self):
        """
        self.thread_pool : est couplé avec l'utilisation d'un QRunnable dans le fichier Bluetooth_test.py
        """
        super().__init__()
        self.initUI()                           #Méthode qui initialise les boutons et autres composantes graphiques du logiciel
        self.thread_pool = QThreadPool()
        self.notifying = True
        self.send_bluetooth_message("")         #On lance automatiquement la réception de données du module Bluetooth au lancement de l'application

    def initUI(self):
        """
        Méthode qui initialise les composantes graphiques de l'interface graphique
        :return:
        """
        self.button_on = QPushButton('On')                                      #Bouton ON
        self.button_on.clicked.connect(self.on_click_on)                        #Méthode qui se lance quand on clique sur le bouton ON

        self.button_off = QPushButton('Off')                                    #Bouton OFF
        self.button_off.clicked.connect(self.on_click_off)                      #Méthode qui se lance quand on clique sur le bouton OFF

        self.button_stop_notify = QPushButton('Stop Notify')                    #Bouton STOP NOTIFY
        self.button_stop_notify.clicked.connect(self.on_click_stop_notify)      #Méthode qui se lance quand on clique sur le bouton STOP NOTIFY

        self.button_start_notify = QPushButton('Start Notify')                  #Bouton START NOTIFY
        self.button_start_notify.clicked.connect(self.on_click_start_notify)    #Méthode qui se lance quand on clique sur le bouton START NOTIFY

        #Arrangement horizontal des boutons
        layout = QHBoxLayout()
        layout.addWidget(self.button_on)
        layout.addWidget(self.button_off)
        layout.addWidget(self.button_stop_notify)
        layout.addWidget(self.button_start_notify)

        self.setLayout(layout)
        self.setWindowTitle('On/Off Motor Bluetooth')

        #Initialement, ces 3 boutons ne sont pas clickables
        self.button_start_notify.setDisabled(True)
        self.button_on.setDisabled(True)
        self.button_off.setDisabled(True)

        self.show()                                                             #Affichage à l'écran de l'interface graphique

    def on_click_on(self):
        """
        Méthode s'activant via un signal, lorsqu'on clique sur le bouton ON
        Il permet d'activer un moteur
        :return:
        """
        try:
            self.send_bluetooth_message("on")                   #Envoi du message "on" au module Bluetooth pour activer le moteur
            self.button_start_notify.setDisabled(True)
            self.button_stop_notify.setDisabled(True)
            self.button_on.setDisabled(True)
            self.button_off.setDisabled(False)
        except Exception as e:
            print(e)

    def on_click_stop_notify(self):
        """
        Méthode s'activant via un signal, lorsqu'on clique sur le bouton STOP NOTIFY
        Il permet d'arrêter la réception des messages
        :return:
        """
        try:
            #Arrêt de la réception des messages à l'aide des attributs correspondant à des booleens
            self.notifying = False
            self.worker.notifying = False
            del self.worker
            self.button_start_notify.setDisabled(False)
            self.button_stop_notify.setDisabled(True)
            self.button_on.setDisabled(False)
            self.button_off.setDisabled(True)
        except Exception as e:
            print(e)

    def on_click_start_notify(self):
        """
        Méthode s'activant via un signal, lorsqu'on clique sur le bouton START NOTIFY
        Il permet de lancer la réception des messages
        :return:
        """
        try:
            #Activation de la réception des message à l'aide de l'attribut étant un booleen et de la méthode send_bluetooth_message
            self.notifying = True
            self.send_bluetooth_message("")
            self.button_start_notify.setDisabled(True)
            self.button_stop_notify.setDisabled(False)
            self.button_on.setDisabled(True)
            self.button_off.setDisabled(True)
        except Exception as e:
            print(e)

    def on_click_off(self):
        """
        Méthode s'activant via un signal, lorsqu'on clique sur le bouton OFF
        Il permet de stopper le moteur
        :return:
        """
        try:
            self.send_bluetooth_message("off")                  #Envoi du message "off" au module Bluetooth pour arrêter le moteur
            self.button_start_notify.setDisabled(False)
            self.button_stop_notify.setDisabled(True)
            self.button_on.setDisabled(False)
            self.button_off.setDisabled(True)
        except Exception as e:
            print(e)

    def send_bluetooth_message(self, message=""):
        """
        Méthode permettant de lancer la réception et l'envoi de message en bluetooth dans un autre thread
        :param message: string; Message à envoyer au module Bluetooth
        :return:
        """

        #Création du client Bluetooth (sans se connecter)
        self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, message)

        #Lancement de la méthode "run" de la variable self.worker
        self.thread_pool.start(self.worker)
