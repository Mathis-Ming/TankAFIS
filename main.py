from mainwindow import MainWindow
from PyQt5.QtWidgets import QApplication
import sys

def main():
    """
    Lancement de l'application créée dans le fichier mainwindow.py
    :return:
    """
    app = QApplication(sys.argv)
    window = MainWindow()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()