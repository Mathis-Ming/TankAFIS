from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPixmap, QPainter, QIcon


def create_custom_icon(icon_color, standard_icon):
    # Créez une icône personnalisée en utilisant une QPixmap
    custom_pixmap = QPixmap(32, 32)
    custom_pixmap.fill(Qt.transparent)

    # Dessinez l'icône standard avec une palette de couleurs modifiée
    painter = QPainter(custom_pixmap)
    standard_icon.paint(painter, custom_pixmap.rect(), Qt.AlignCenter, QIcon.Normal, QIcon.Off)

    # Modifiez la couleur de l'icône
    painter.setCompositionMode(QPainter.CompositionMode_SourceIn)
    painter.fillRect(custom_pixmap.rect(), icon_color)
    painter.end()

    # Créez une QIcon avec la QPixmap modifiée
    custom_icon = QIcon(custom_pixmap)
    return custom_icon