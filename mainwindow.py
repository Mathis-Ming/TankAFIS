from PyQt5.QtWidgets import QWidget, QPushButton, QVBoxLayout, QHBoxLayout
from PyQt5.QtCore import QThreadPool
from Bluetooth_test import BluetoothClient, ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.thread_pool = QThreadPool()
        self.notifying = True
        self.send_bluetooth_message("")

    def initUI(self):
        self.button_on = QPushButton('On')
        self.button_on.clicked.connect(self.on_click_on)

        self.button_off = QPushButton('Off')
        self.button_off.clicked.connect(self.on_click_off)

        self.button_stop_notify = QPushButton('Stop Notify')
        self.button_stop_notify.clicked.connect(self.on_click_stop_notify)

        self.button_start_notify = QPushButton('Start Notify')
        self.button_start_notify.clicked.connect(self.on_click_start_notify)

        layout = QHBoxLayout()
        layout.addWidget(self.button_on)
        layout.addWidget(self.button_off)
        layout.addWidget(self.button_stop_notify)
        layout.addWidget(self.button_start_notify)

        self.setLayout(layout)
        self.setWindowTitle('On/Off Motor Bluetooth')
        self.button_start_notify.setDisabled(True)
        self.button_on.setDisabled(True)
        self.button_off.setDisabled(True)

        self.show()

    def on_click_on(self):
        try:
            self.send_bluetooth_message("on")
            self.button_start_notify.setDisabled(True)
            self.button_stop_notify.setDisabled(True)
            self.button_on.setDisabled(True)
            self.button_off.setDisabled(False)
        except Exception as e:
            print(e)

    def on_click_stop_notify(self):
        try:
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
        try:
            self.notifying = True
            self.send_bluetooth_message("")
            self.button_start_notify.setDisabled(True)
            self.button_stop_notify.setDisabled(False)
            self.button_on.setDisabled(True)
            self.button_off.setDisabled(True)
        except Exception as e:
            print(e)

    def on_click_off(self):
        try:
            self.send_bluetooth_message("off")
            self.button_start_notify.setDisabled(False)
            self.button_stop_notify.setDisabled(True)
            self.button_on.setDisabled(False)
            self.button_off.setDisabled(True)
        except Exception as e:
            print(e)

    def send_bluetooth_message(self, message=""):
        self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, message)
        self.thread_pool.start(self.worker)
