import os
from PyQt5.QtWidgets import QPushButton, QFrame, QWidget, QDialog, QLabel
from PyQt5.QtGui import QPixmap, QPainter, QColor
from PyQt5.QtCore import Qt, QPoint

class ClonkArea(QFrame):
    def __init__(self, parent, x=None, y=None, w=None, h=None, bg_color=None):
        super().__init__(parent)
        if all(v is not None for v in (x, y, w, h)):
            self.setGeometry(x, y, w, h)
        self.bg_color = bg_color

    def paintEvent(self, event):
        painter = QPainter(self)
        w, h = self.width(), self.height()
        
        if self.bg_color:
            painter.fillRect(2, 2, w-4, h-4, QColor(self.bg_color))
        
        # Sunken Border
        painter.setPen(QColor("#a6a6a6"))
        painter.drawLine(0, 0, w-1, 0)
        painter.drawLine(0, 0, 0, h-1)
        
        painter.setPen(QColor("#6a6a6a"))
        painter.drawLine(1, 1, w-2, 1)
        painter.drawLine(1, 1, 1, h-2)
        
        painter.setPen(QColor("#e3e3e3"))
        painter.drawLine(1, h-2, w-2, h-2)
        painter.drawLine(w-2, 1, w-2, h-2)
        
        painter.setPen(QColor("#ffffff"))
        painter.drawLine(0, h-1, w-1, h-1)
        painter.drawLine(w-1, 0, w-1, h-1)
        
class ClonkTextArea(QFrame):
    def __init__(self, parent, x=None, y=None, w=None, h=None, bg_color=None):
        super().__init__(parent)
        if all(v is not None for v in (x, y, w, h)):
            self.setGeometry(x, y, w, h)
        self.bg_color = bg_color

    def paintEvent(self, event):
        painter = QPainter(self)
        w, h = self.width(), self.height()
        
        if self.bg_color:
            painter.fillRect(2, 2, w-4, h-4, QColor(self.bg_color))
        
        # Sunken Border
        painter.setPen(QColor("#a6a6a6"))
        painter.drawLine(0, 0, w-1, 0)
        painter.drawLine(0, 0, 0, h-1)
        
        painter.setPen(QColor("#ffffff"))
        painter.drawLine(1, 1, w-2, 1)
        painter.drawLine(1, 1, 1, h-2)
        
        painter.setPen(QColor("#a6a6a6"))
        painter.drawLine(1, h-2, w-2, h-2)
        painter.drawLine(w-2, 1, w-2, h-2)
        
        painter.setPen(QColor("#ffffff"))
        painter.drawLine(0, h-1, w-1, h-1)
        painter.drawLine(w-1, 0, w-1, h-1)

class ClonkButton(QPushButton):
    def __init__(self, text, parent=None, bg_path=None, bg_offset=(0, 0), size=(86, 20)):
        super().__init__(text, parent)
        self.bg_pix = QPixmap(bg_path) if bg_path and os.path.exists(bg_path) else None
        self.bg_offset = bg_offset
        self.setFixedSize(*size)

    def paintEvent(self, event):
        painter = QPainter(self)
        rect = self.rect()
        w, h = rect.width(), rect.height()

        if self.bg_pix and not self.bg_pix.isNull():
            painter.drawTiledPixmap(rect, self.bg_pix, QPoint(self.bg_offset[0], self.bg_offset[1]))
        else:
            painter.fillRect(rect, QColor("#c0c0c0"))

        font = self.font()
        painter.setFont(font)

        # Shift text 1px if pressed
        t_rect = rect.adjusted(1, 1, 1, 1) if self.isDown() else rect

        if self.isDown():
            # Pressed state
            painter.setPen(QColor("#6a6a6a"))
            painter.drawLine(0, 0, w-1, 0)
            painter.drawLine(0, 0, 0, h-1)
            painter.setPen(QColor("#a6a6a6"))
            painter.drawLine(1, 1, w-2, 1)
            painter.drawLine(1, 1, 1, h-2)
            painter.setPen(QColor("#ffffff"))
            painter.drawLine(0, h-1, w-1, h-1)
            painter.drawLine(w-1, 0, w-1, h-1)
        else:
            # Raised state
            painter.setPen(QColor("#e3e3e3")); painter.drawLine(0, 0, w-1, 0); painter.drawLine(0, 0, 0, h-1)
            painter.setPen(QColor("#ffffff")); painter.drawLine(1, 1, w-2, 1); painter.drawLine(1, 1, 1, h-2)
            painter.setPen(QColor("#a6a6a6")); painter.drawLine(1, h-2, w-2, h-2); painter.drawLine(w-2, 1, w-2, h-2)
            painter.setPen(QColor("#6a6a6a")); painter.drawLine(0, h-1, w-1, h-1); painter.drawLine(w-1, 0, w-1, h-1)

        painter.setPen(Qt.white); painter.drawText(t_rect.adjusted(1, 1, 1, 1), Qt.AlignCenter, self.text())
        painter.setPen(Qt.black); painter.drawText(t_rect, Qt.AlignCenter, self.text())

class ClonkTexturedWidget(QWidget):
    def __init__(self, parent=None, bg_path=None):
        super().__init__(parent)
        self.bg_pix = QPixmap(bg_path) if bg_path and os.path.exists(bg_path) else None

    def paintEvent(self, event):
        painter = QPainter(self)
        if self.bg_pix and not self.bg_pix.isNull():
            painter.drawTiledPixmap(self.rect(), self.bg_pix)
        else:
            painter.fillRect(self.rect(), QColor("#c0c0c0"))
