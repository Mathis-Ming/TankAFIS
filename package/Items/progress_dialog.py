import time
from PyQt5.QtCore import QThread, pyqtSignal, Qt
from PyQt5.QtWidgets import QDialog, QLabel, QVBoxLayout, QProgressBar


class ProgressBarThread(QThread):
    startSig = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.stop_upgrade = False
        self.dlg_action = ProgressDialog()
        self.dlg_action.show()

    def __del__(self):
        print("Finished")

    def run(self):
        # Simuler une action longue (remplacez cela par votre propre logique)
        self.startSig.emit()
        count = 0
        while self.stop_upgrade == False:
            time.sleep(0.05)  # ajustez selon vos besoins
            self.dlg_action.updateProgress(count)
            count += 1

        self.dlg_action.close()
        self.quit()
        self.wait()
        return


    def connectFinished(self):
        self.stop_upgrade = True


class ProgressDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.setWindowFlags(Qt.FramelessWindowHint)
        self.progress_bar = QProgressBar(self)
        self.progress_bar.setRange(0, 0)  # Progression indéterminée
        self.progress_bar.setAlignment(Qt.AlignCenter)  # Alignement au centre
        self.progress_bar.setMinimumWidth(300)
        self.lbl_connect = QLabel("Connecting...")

        layout = QVBoxLayout(self)
        layout.addWidget(self.lbl_connect)
        layout.addWidget(self.progress_bar)

    def updateProgress(self, value):
        self.progress_bar.setValue(value)