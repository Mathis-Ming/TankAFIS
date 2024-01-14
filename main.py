import sys
import os
import setproctitle
from PyQt5.QtWidgets import QApplication
from package.mainwindow import MainWindow

script_directory = os.path.dirname(os.path.abspath(__file__))
style_file_path = os.path.join(script_directory, "resources\style.css")


if __name__ == "__main__":
    setproctitle.setproctitle("TankAFIS_IHM")
    print(style_file_path)
    app = QApplication(sys.argv)

    with open(style_file_path, 'r') as file:
        style_sheet = file.read()
        app.setStyleSheet(style_sheet)
    window = MainWindow()
    window.resize(750,550)
    window.show()
    sys.exit(app.exec_())


