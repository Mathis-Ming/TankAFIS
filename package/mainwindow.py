from datetime import datetime
from PyQt5.QtWidgets import QWidget, QListWidget, QPushButton, QGridLayout, QListWidgetItem, QSizePolicy, QLabel, QLCDNumber
from PyQt5.QtCore import QThreadPool, Qt, QTimer, pyqtSignal
from PyQt5.Qt import QFont, QStyle
from PyQt5.QtGui import QCursor, QColor
from package.API.Bluetooth_API import BluetoothClient, ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID
from package.Items.circular_indication import CircularIndicator
from package.Items.state_indication import StateIndicator
from package.Items.progress_dialog import ProgressBarThread
from package.Items import custom_icon
from package.Items.verify_connection import VerifyConn, VerifyNotConn


class MainWindow(QWidget):
    connect_finished = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.thread_pool = QThreadPool()
        self.setWindowTitle("TankAFIS")
        self.last_data = ""
        self.notifying = False
        self.stopped_program = False
        self.client_in_memory = None
        self.reading = False
        self.writing = False
        self.moving = False
        self.timer = QTimer(self)
        self.timer_value = 0
        self.setup_ui()

    def setup_ui(self):
        #Méthode qui appellera d'autres méthodes
        self.create_widgets()
        self.modify_widgets()
        self.create_layouts()
        self.add_widgets_to_layouts()
        self.setup_connections()
        """Pour créer toutes ces méthodes :
            Faire click mollette juste après le self et rester
            appuyé et descendre jusqu'au dernier self
            Ensuite ctrl+W puis ctrl+C
            Ensuite refaire la première étape sur les méthodes
            sans le def, puis écrire def
            Ensuite faire ctrl+droite et mettre le parenthèses
            et :
            Enfin mettre pass pour toutes les fonctions
        """

    def create_widgets(self):
        self.btn_connect = QPushButton("Connect")
        self.btn_disconnect = QPushButton("Disconnect")
        self.btn_start = QPushButton("START")
        self.btn_pause = QPushButton("PAUSE")
        self.btn_stop = QPushButton("STOP")
        self.lw_instructions = QListWidget()
        self.circular_indic = CircularIndicator()
        self.btn_clear_notif = QPushButton("Clear")
        self.reading_indic = StateIndicator()
        self.writing_indic = StateIndicator()
        self.moving_indic = StateIndicator()
        self.lbl_reading = QLabel("Reading")
        self.lbl_writing = QLabel("Writing")
        self.lbl_moving = QLabel("Moving")
        self.lcd_timer = QLCDNumber()


    def modify_widgets(self):
        self.lw_instructions.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.lw_instructions.setAlternatingRowColors(True)
        self.lw_instructions.setStyleSheet("alternate-background-color: #303030;")
        self.btn_pause.setCursor(QCursor(Qt.PointingHandCursor))
        self.btn_stop.setCursor(QCursor(Qt.PointingHandCursor))
        self.btn_start.setCursor(QCursor(Qt.PointingHandCursor))
        self.btn_connect.setCursor(QCursor(Qt.PointingHandCursor))
        self.btn_disconnect.setCursor(QCursor(Qt.PointingHandCursor))
        self.btn_clear_notif.setCursor(QCursor(Qt.PointingHandCursor))
        icon_color = QColor(0, 255, 0)
        standard_icon = self.style().standardIcon(QStyle.SP_MediaPlay)
        icon = custom_icon.create_custom_icon(icon_color=icon_color, standard_icon=standard_icon)
        self.btn_start.setIcon(icon)
        icon_color = QColor(255, 255, 0, 200)
        standard_icon = self.style().standardIcon(QStyle.SP_MediaPause)
        icon = custom_icon.create_custom_icon(icon_color, standard_icon)
        self.btn_pause.setIcon(icon)
        icon_color = QColor(255, 0, 0, 200)
        standard_icon = self.style().standardIcon(QStyle.SP_MediaStop)
        icon = custom_icon.create_custom_icon(icon_color, standard_icon)
        self.btn_stop.setIcon(icon)
        self.lcd_timer.setSegmentStyle(QLCDNumber.Filled)
        self.lcd_timer.display(f"0:00")
        self.btn_disconnect.setDisabled(True)
        self.btn_start.setDisabled(True)
        self.btn_pause.setDisabled(True)
        self.btn_stop.setDisabled(True)

    def create_layouts(self):
        self.main_layout = QGridLayout(self)

    def add_widgets_to_layouts(self):
        self.main_layout.addWidget(self.lw_instructions, 10, 3, 5, 10)
        self.main_layout.addWidget(self.btn_connect, 0, 12, 1, 1)
        self.main_layout.addWidget(self.btn_disconnect, 1, 12, 1, 1)
        self.main_layout.addWidget(self.btn_start, 0, 0, 1, 2)
        self.main_layout.addWidget(self.btn_pause, 1, 0, 1, 2)
        self.main_layout.addWidget(self.btn_stop, 2, 0, 1, 2)
        self.main_layout.addWidget(self.circular_indic, 0, 11, 1, 1)
        self.main_layout.addWidget(self.btn_clear_notif, 15, 12, 1, 1)
        self.main_layout.addWidget(self.reading_indic, 11, 0, 1, 1)
        self.main_layout.addWidget(self.writing_indic, 12, 0, 1, 1)
        self.main_layout.addWidget(self.moving_indic, 13, 0, 1, 1)
        self.main_layout.addWidget(self.lbl_reading, 11, 1, 1, 2)
        self.main_layout.addWidget(self.lbl_writing, 12, 1, 1, 2)
        self.main_layout.addWidget(self.lbl_moving, 13, 1, 1, 2)
        self.main_layout.addWidget(self.lcd_timer, 15, 0, 1, 2)


    def setup_connections(self):
        self.btn_connect.clicked.connect(self.on_click_upgrade_connect)
        self.btn_disconnect.clicked.connect(self.on_click_disconnect)
        self.btn_start.clicked.connect(self.on_click_start)
        self.btn_pause.clicked.connect(self.on_click_pause)
        self.btn_stop.clicked.connect(self.on_click_stop)
        self.btn_clear_notif.clicked.connect(self.on_click_clear_notif)
        self.timer.timeout.connect(self.update_timer)

    def update_timer(self):
        self.timer_value += 1
        minutes = self.timer_value // 60
        seconds = self.timer_value % 60
        self.lcd_timer.display(f"{minutes}:{seconds:02d}")

    def pause_resume_timer(self):
        if self.timer.isActive():
            self.timer.stop()
        else:
            if self.stopped_program == True:
                self.lcd_timer.display(f"0:00")
                self.timer_value = 0
                self.stopped_program = False
            self.timer.start(1000)

    def on_click_upgrade_connect(self):
        self.progress_bar_thread = ProgressBarThread()
        self.connect_finished.connect(self.progress_bar_thread.connectFinished)
        self.progress_bar_thread.startSig.connect(self.on_click_connect)
        self.progress_bar_thread.start()

    def error_connect(self):
        try:
            self.btn_connect.setDisabled(False)
            self.circular_indic.toggle_indicator_color()
            self.btn_disconnect.setDisabled(True)
            self.conn_monitor.error_conn.disconnect()
            self.conn_monitor.quit()
            self.conn_monitor.terminate()
            del self.conn_monitor
            self.not_conn_monitor = VerifyNotConn(self.client_in_memory)
            self.not_conn_monitor.reconn.connect(self.reconnect)
            self.not_conn_monitor.start()
        except Exception as e:
            print(e)

    def reconnect(self):
        try:
            self.btn_connect.setDisabled(True)
            self.circular_indic.toggle_indicator_color()
            self.not_conn_monitor.reconn.disconnect()
            self.not_conn_monitor.quit()
            self.not_conn_monitor.terminate()
            del self.not_conn_monitor

            self.conn_monitor = VerifyConn(self.client_in_memory)
            self.conn_monitor.error_conn.connect(self.error_connect)
            self.conn_monitor.start()
        except Exception as e:
            print(e)


    def on_click_connect(self):
        try:
            # Création du client Bluetooth (sans se connecter)
            self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, "")
            self.worker.connect_to = True
            self.bluetooth_action()
            while self.worker.connected == False:
                continue
            self.client_in_memory = self.worker.client
            self.worker.connect_to = False
            self.circular_indic.toggle_indicator_color()
            self.btn_disconnect.setDisabled(False)
            self.btn_connect.setDisabled(True)
            self.btn_start.setDisabled(False)
            self.btn_pause.setDisabled(True)
            self.connect_finished.emit()
            self.conn_monitor = VerifyConn(self.client_in_memory)
            self.conn_monitor.error_conn.connect(self.error_connect)
            self.conn_monitor.start()
        except Exception as e:
            print(e)

    def on_click_disconnect(self):
        try:
            if self.worker:
                self.conn_monitor.quit()
                self.conn_monitor.terminate()
                del self.conn_monitor
                # Création du client Bluetooth (sans se connecter)
                self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, "")
                self.worker.connect_to = False
                self.worker.disconnect_from = True
                self.worker.client = self.client_in_memory
                self.bluetooth_action()
                while self.worker.connected == True:
                    continue
                self.worker.disconnect_from = False
                self.circular_indic.toggle_indicator_color()
                self.btn_disconnect.setDisabled(True)
                self.btn_connect.setDisabled(False)
                self.btn_start.setDisabled(True)
                self.btn_pause.setDisabled(True)
            else:
                print("No client created")
        except Exception as e:
            print(e)

    def add_data(self, data):
        if data == self.last_data:
            return
        self.last_data = data
        data = f'{datetime.now().strftime("%H:%M:%S")} | {data}'
        if "Warning" in data:
            lw_item = QListWidgetItem(data)
            lw_item.setFont(QFont('Arial', 10))
            lw_item.setForeground(Qt.darkRed)
            self.lw_instructions.addItem(lw_item)
            last_item = self.lw_instructions.item(self.lw_instructions.count()-1)
            self.lw_instructions.scrollToItem(last_item)
        elif "Notification" in data:
            lw_item = QListWidgetItem(data)
            lw_item.setFont(QFont('Arial', 10))
            lw_item.setForeground(Qt.darkYellow)
            self.lw_instructions.addItem(lw_item)
            last_item = self.lw_instructions.item(self.lw_instructions.count() - 1)
            self.lw_instructions.scrollToItem(last_item)
        elif "Sent" in data:
            lw_item = QListWidgetItem(data)
            lw_item.setFont(QFont('Arial', 10))
            lw_item.setForeground(Qt.white)
            self.lw_instructions.addItem(lw_item)
            last_item = self.lw_instructions.item(self.lw_instructions.count() - 1)
            self.lw_instructions.scrollToItem(last_item)
        elif "Reading" in data:
            lw_item = QListWidgetItem(data)
            lw_item.setFont(QFont('Arial', 10))
            lw_item.setForeground(Qt.white)
            self.lw_instructions.addItem(lw_item)
            last_item = self.lw_instructions.item(self.lw_instructions.count() - 1)
            self.lw_instructions.scrollToItem(last_item)
            if self.reading != True:
                self.reading_indic.toggle_indicator_color()
                self.reading = True
            if self.writing == True:
                self.writing_indic.toggle_indicator_color()
                self.writing = False
            if self.moving == True:
                self.moving_indic.toggle_indicator_color()
                self.moving = False
        elif "Writing" in data:
            lw_item = QListWidgetItem(data)
            lw_item.setFont(QFont('Arial', 10))
            lw_item.setForeground(Qt.white)
            self.lw_instructions.addItem(lw_item)
            last_item = self.lw_instructions.item(self.lw_instructions.count() - 1)
            self.lw_instructions.scrollToItem(last_item)
            if self.writing != True:
                self.writing_indic.toggle_indicator_color()
                self.writing = True
            if self.reading == True:
                self.reading_indic.toggle_indicator_color()
                self.reading = False
            if self.moving == True:
                self.moving_indic.toggle_indicator_color()
                self.moving = False
        elif "Moving" in data:
            lw_item = QListWidgetItem(data)
            lw_item.setFont(QFont('Arial', 10))
            lw_item.setForeground(Qt.white)
            self.lw_instructions.addItem(lw_item)
            last_item = self.lw_instructions.item(self.lw_instructions.count() - 1)
            self.lw_instructions.scrollToItem(last_item)
            if self.moving != True:
                self.moving_indic.toggle_indicator_color()
                self.moving = True
            if self.reading == True:
                self.reading_indic.toggle_indicator_color()
                self.reading = False
            if self.writing == True:
                self.writing_indic.toggle_indicator_color()
                self.writing = False

    def on_click_start(self):
        if self.stopped_program == True:
            self.on_click_clear_notif()
            if self.reading == True:
                self.reading_indic.toggle_indicator_color()
                self.reading = False
            if self.writing == True:
                self.writing_indic.toggle_indicator_color()
                self.writing = False
            if self.moving == True:
                self.moving_indic.toggle_indicator_color()
                self.moving = False
        try:
            # Création du client Bluetooth (sans se connecter)
            self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, "")
            self.worker.client = self.client_in_memory
            self.worker.message = "on"
            self.bluetooth_action()  # Envoi du message "on" au module Bluetooth pour activer le moteur
            while self.worker.finished_operation == False:
                continue
            self.add_data("Sent : Start instructions")
            self.pause_resume_timer()
            self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, "")
            self.worker.worker_signal.result.connect(self.add_data)
            self.worker.client = self.client_in_memory
            self.worker.message = ""
            #Activation de la réception des message à l'aide de l'attribut étant un booleen et de la méthode send_bluetooth_message
            self.notifying = True
            self.worker.notifying = True
            self.bluetooth_action()
            while self.worker.started_notify == False:
                continue
            self.btn_disconnect.setDisabled(True)
            self.btn_connect.setDisabled(True)
            self.btn_start.setDisabled(True)
            self.btn_pause.setDisabled(False)
            self.btn_stop.setDisabled(True)
        except Exception as e:
            print(e)

    def on_click_pause(self):
        try:
            #Arrêt de la réception des messages à l'aide des attributs correspondant à des booleens
            self.notifying = False
            self.worker.notifying = False
            while self.worker.finished_operation == False:
                continue

            self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, "")
            self.worker.client = self.client_in_memory
            self.worker.message = "pause"
            self.bluetooth_action()  # Envoi du message "on" au module Bluetooth pour activer le moteur
            while self.worker.finished_operation == False:
                continue
            self.add_data("Sent : Pause instructions")
            self.pause_resume_timer()
            self.btn_disconnect.setDisabled(True)
            self.btn_connect.setDisabled(True)
            self.btn_start.setDisabled(False)
            self.btn_pause.setDisabled(True)
            self.btn_stop.setDisabled(False)
        except Exception as e:
            print(e)

    def on_click_stop(self):
        self.stopped_program = True
        self.worker = BluetoothClient(ADDRESS, WRITE_CHAR_UUID, NOTIFY_CHAR_UUID, self.notifying, "")
        self.worker.client = self.client_in_memory
        self.worker.message = "off"
        self.bluetooth_action()  # Envoi du message "on" au module Bluetooth pour activer le moteur
        while self.worker.finished_operation == False:
            continue
        self.add_data("Sent : Stop instructions")
        self.btn_disconnect.setDisabled(False)
        self.btn_connect.setDisabled(True)
        self.btn_start.setDisabled(False)
        self.btn_pause.setDisabled(True)
        self.btn_stop.setDisabled(True)

    def on_click_clear_notif(self):
        if self.lw_instructions.count() > 0:
            self.lw_instructions.clear()
            return
        else:
            return

    def bluetooth_action(self):
        """
        Méthode permettant de lancer la réception et l'envoi de message en bluetooth dans un autre thread
        :param message: string; Message à envoyer au module Bluetooth
        :return:
        """
        self.thread_pool.start(self.worker)



