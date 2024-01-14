from PyQt5.QtWidgets import QWidget, QVBoxLayout, QPushButton
from PyQt5.QtGui import QPainter, QColor

class CircularIndicator(QWidget):
    def __init__(self):
        super().__init__()

        self.indicator_color = QColor("red")

        self.init_ui()

    def init_ui(self):
        # Créer un bouton pour déclencher le changement de couleur du voyant
        self.button = QPushButton("Toggle Indicator Color")
        self.button.clicked.connect(self.toggle_indicator_color)
        self.button.hide()

        # Disposition des widgets dans la fenêtre
        layout = QVBoxLayout(self)
        layout.addWidget(self.button)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Définir la couleur du voyant
        painter.setBrush(self.indicator_color)

        # Dessiner un cercle
        size = min(self.width(), self.height())
        painter.drawEllipse((self.width() - size) // 2, (self.height() - size) // 2, size, size)

    def toggle_indicator_color(self):
        # Fonction qui bascule la couleur du voyant lorsque le bouton est cliqué
        if self.indicator_color == QColor("red"):
            self.indicator_color = QColor("green")
        else:
            self.indicator_color = QColor("red")

        # Redessiner le voyant après le changement de couleur
        self.update()