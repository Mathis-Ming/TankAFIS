from PyQt5.QtCore import QThread, pyqtSignal

class VerifyConn(QThread):
    error_conn = pyqtSignal()

    def __init__(self, client):
        super().__init__()
        self.client = client

    def run(self):
        while self.client.is_connected:
            self.wait(1000)

        self.error_conn.emit()
        return

class VerifyNotConn(QThread):
    reconn = pyqtSignal()

    def __init__(self, client):
        super().__init__()
        self.client = client

    def run(self):
        while not self.client.is_connected:
            self.wait(1000)

        self.reconn.emit()
        return