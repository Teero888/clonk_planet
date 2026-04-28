import sys
import os
import subprocess
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                             QLabel, QTreeView, QTextEdit, QRadioButton, QButtonGroup,
                             QFrame, QDialog, QTabWidget, QVBoxLayout)
from PyQt5.QtGui import QPixmap, QPalette, QBrush, QStandardItemModel, QStandardItem, QIcon, QPainter, QColor, QFont
from PyQt5.QtCore import Qt, QRect, QSize, QTimer, QUrl, QPoint
from PyQt5.QtMultimedia import QSoundEffect

class ClonkButton(QPushButton):
    def __init__(self, text, parent=None, bg_path=None, bg_offset=(0, 0)):
        super().__init__(text, parent)
        self.bg_pix = QPixmap(bg_path) if bg_path and os.path.exists(bg_path) else None
        self.bg_offset = bg_offset
        self.setFixedSize(86, 20)

    def paintEvent(self, event):
        painter = QPainter(self)
        rect = self.rect()
        w = rect.width()
        h = rect.height()

        if self.bg_pix:
            painter.drawTiledPixmap(rect, self.bg_pix, QPoint(self.bg_offset[0], self.bg_offset[1]))
        else:
            painter.fillRect(rect, QColor("#c0c0c0"))

        font = QFont("Comic Sans MS", 8)
        font.setBold(False)
        painter.setFont(font)

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
            # Normal state (Raised)
            painter.setPen(QColor("#e3e3e3")); painter.drawLine(0, 0, w-1, 0); painter.drawLine(0, 0, 0, h-1)
            painter.setPen(QColor("#ffffff")); painter.drawLine(1, 1, w-2, 1); painter.drawLine(1, 1, 1, h-2)
            painter.setPen(QColor("#a6a6a6")); painter.drawLine(1, h-2, w-2, h-2); painter.drawLine(w-2, 1, w-2, h-2)
            painter.setPen(QColor("#6a6a6a")); painter.drawLine(0, h-1, w-1, h-1); painter.drawLine(w-1, 0, w-1, h-1)

        painter.setPen(Qt.white); painter.drawText(t_rect.adjusted(1, 1, 1, 1), Qt.AlignCenter, self.text())
        painter.setPen(Qt.black); painter.drawText(t_rect, Qt.AlignCenter, self.text())

class ClonkArea(QFrame):
    def __init__(self, parent, x, y, w, h, bg_color=None):
        super().__init__(parent)
        self.setGeometry(x, y, w, h)
        self.bg_color = bg_color

    def paintEvent(self, event):
        painter = QPainter(self)
        w, h = self.width(), self.height()
        
        # Fill background inside border
        if self.bg_color:
            painter.fillRect(2, 2, w-4, h-4, QColor(self.bg_color))
        
        # Sunken Border
        # Top-Left Outer: a6a6a6
        painter.setPen(QColor("#a6a6a6"))
        painter.drawLine(0, 0, w-1, 0)
        painter.drawLine(0, 0, 0, h-1)
        # Top-Left Inner: 6a6a6a
        painter.setPen(QColor("#6a6a6a"))
        painter.drawLine(1, 1, w-2, 1)
        painter.drawLine(1, 1, 1, h-2)
        # Bottom-Right Inner: e3e3e3
        painter.setPen(QColor("#e3e3e3"))
        painter.drawLine(1, h-2, w-2, h-2)
        painter.drawLine(w-2, 1, w-2, h-2)
        # Bottom-Right Outer: ffffff
        painter.setPen(QColor("#ffffff"))
        painter.drawLine(0, h-1, w-1, h-1)
        painter.drawLine(w-1, 0, w-1, h-1)

class ClonkLauncher(QMainWindow):
    def __init__(self):
        super().__init__()
        self.res_path = os.path.join(os.path.dirname(__file__), 'res')
        self.wav_path = os.path.join(os.path.dirname(__file__), 'res_wav_fixed')
        self.dump_path = os.path.join(os.path.dirname(__file__), 'res_dump')
        self.sound_start = self.load_sound('sound_7008.wav')
        self.sound_click = self.load_sound('sound_7002.wav')
        self.init_ui()

    def load_sound(self, name):
        path = os.path.join(self.wav_path, name)
        if os.path.exists(path):
            effect = QSoundEffect(); effect.setSource(QUrl.fromLocalFile(os.path.abspath(path))); effect.setVolume(1.0)
            return effect
        return None

    def play_sound(self, effect):
        if effect: effect.play()

    def init_ui(self):
        self.client_w, self.client_h = 578, 393
        self.setWindowTitle("Clonk Planet")
        self.central = QWidget(); self.setCentralWidget(self.central); self.central.setFixedSize(self.client_w, self.client_h)
        self.init_menu(); self.setFixedSize(self.sizeHint())
        self.bg_label = QLabel(self.central); self.bg_label.setGeometry(0, 0, self.client_w, self.client_h)
        self.is_intro = True
        self.intro_frames = ['Planet_fixed.bin_2_1000_1031.bmp', 'Planet_fixed.bin_2_1010_1031.bmp', 'Planet_fixed.bin_2_1021_1031.bmp', 'Planet_fixed.bin_2_1036_1031.bmp']
        self.intro_index = 0
        self.ui_container = QWidget(self.central); self.ui_container.setGeometry(0, 0, self.client_w, self.client_h); self.ui_container.hide()
        self.setup_main_ui()
        self.play_sound(self.sound_start)
        self.intro_timer = QTimer(); self.intro_timer.timeout.connect(self.next_intro_frame); self.intro_timer.start(1000); self.next_intro_frame()

    def init_menu(self):
        import webbrowser
        menubar = self.menuBar(); menubar.setStyleSheet("")
        menu_style = "QMenu { background-color: #c0c0c0; font-family: 'Comic Sans MS'; font-size: 11px; border: 2px outset #ffffff; color: black; } QMenu::item:selected { background-color: #000080; color: white; }"
        options_menu = menubar.addMenu("Options"); options_menu.setStyleSheet(menu_style); options_menu.addAction("Options...").triggered.connect(self.show_options); options_menu.addAction("Registration...")
        help_menu = menubar.addMenu("Help"); help_menu.setStyleSheet(menu_style); help_menu.addAction("Contents..."); help_menu.addAction("Credits...").triggered.connect(self.show_credits); help_menu.addAction("Clonk on the web").triggered.connect(lambda: webbrowser.open("http://www.clonk.de/")); help_menu.addSeparator(); help_menu.addAction("About Clonk Planet...").triggered.connect(self.show_about)

    def next_intro_frame(self):
        if self.intro_index < len(self.intro_frames):
            self.set_background(self.intro_frames[self.intro_index], is_raw=True); self.intro_index += 1
        else:
            self.intro_timer.stop(); self.end_intro()

    def set_background(self, name, is_raw=False):
        dir_p = self.dump_path if is_raw else self.res_path
        path = os.path.join(dir_p, name)
        if os.path.exists(path):
            pix = QPixmap(path)
            if pix.width() < 500:
                palette = self.bg_label.palette(); palette.setBrush(QPalette.Window, QBrush(pix)); self.bg_label.setPalette(palette); self.bg_label.setAutoFillBackground(True); self.bg_label.setPixmap(QPixmap()) 
            else:
                self.bg_label.setAutoFillBackground(False); self.bg_label.setPixmap(pix.scaled(self.client_w, self.client_h, Qt.IgnoreAspectRatio, Qt.SmoothTransformation))
        else:
            self.bg_label.setAutoFillBackground(False); self.bg_label.setStyleSheet("background-color: #c0c0c0;")

    def end_intro(self):
        self.is_intro = False; self.set_background('Planet_fixed.bin_2_1019_1031.bmp', is_raw=True); self.ui_container.show()

    def setup_main_ui(self):
        font_style = "'Comic Sans MS', 'Chilanka', 'cursive'"
        self.ui_container.setStyleSheet(f"* {{ font-family: {font_style}; font-size: 11px; color: black; }}")

        # Tree View Area
        self.tree_frame = ClonkArea(self.ui_container, 9, 10, 243, 346, bg_color="white")
        self.tree = QTreeView(self.tree_frame)
        self.tree.setGeometry(2, 2, 239, 342)
        self.tree.setHeaderHidden(True); self.tree.setFrameShape(QFrame.NoFrame)
        self.tree_model = QStandardItemModel(); self.tree.setModel(self.tree_model)
        
        # Preview Area
        self.preview_frame = ClonkArea(self.ui_container, 261, 10, 212, 151, bg_color="black")
        self.preview = QLabel(self.preview_frame); self.preview.setGeometry(2, 2, 208, 147); self.preview.setAlignment(Qt.AlignCenter)

        # Description Area
        self.desc_frame = ClonkArea(self.ui_container, 261, 166, 212, 171, bg_color="white")
        self.desc = QTextEdit(self.desc_frame); self.desc.setGeometry(2, 2, 208, 167); self.desc.setReadOnly(True); self.desc.setFrameShape(QFrame.NoFrame)
        self.desc.setText("Select a scenario or player to begin.")

        # Buttons
        btn_bg_raw = os.path.join(self.dump_path, 'Planet_fixed.bin_2_1006_1031.bmp')
        global_offset = (0, 0)
        def create_btn(text, x, y):
            btn = ClonkButton(text, self.ui_container, btn_bg_raw, global_offset); btn.move(x, y); btn.clicked.connect(lambda: self.play_sound(self.sound_click))
            return btn

        self.btn_new = create_btn("New", 482, 10); self.btn_activate = create_btn("Activate", 482, 34); self.btn_rename = create_btn("Rename", 482, 59); self.btn_delete = create_btn("Delete", 482, 83); self.btn_props = create_btn("Properties", 482, 107); self.btn_props.clicked.connect(self.show_options)
        self.btn_start = create_btn("Start", 482, 310); self.btn_start.clicked.connect(self.launch_game); self.btn_quit = create_btn("Quit", 482, 335); self.btn_quit.clicked.connect(self.close)

        self.view_label = QLabel("View:", self.ui_container); self.view_label.setGeometry(482, 161, 72, 15); self.view_label.setStyleSheet("background: transparent;")
        self.radio_player = QRadioButton("Player", self.ui_container); self.radio_player.setGeometry(482, 180, 90, 15);  #self.radio_player.toggled.connect(lambda c: self.set_background('Planet_fixed.bin_2_1010_1031.bmp', True) if c else None)
        self.radio_dev = QRadioButton("Developer", self.ui_container); self.radio_dev.setGeometry(482, 195, 90, 15);  self.radio_dev.setChecked(True) #self.radio_dev.toggled.connect(lambda c: self.set_background('Planet_fixed.bin_2_1019_1031.bmp', True) if c else None);
        self.view_group = QButtonGroup(self); self.view_group.addButton(self.radio_player); self.view_group.addButton(self.radio_dev)
        self.author_label = QLabel("Author: RedWolf Design", self.ui_container); self.author_label.setGeometry(261, 341, 212, 15); self.author_label.setStyleSheet("background: transparent;")

        # Status Bar / Animation
        self.status_frame = ClonkArea(self.ui_container, 47, 364, 521, 24)
        self.status_bar = QLabel(" Welcome to Clonk Planet", self.status_frame); self.status_bar.setGeometry(2, 2, 517, 20); self.status_bar.setStyleSheet("background: transparent;")
        self.anim_frame = ClonkArea(self.ui_container, 9, 364, 33, 24)
        self.animation = QLabel(self.anim_frame); self.animation.setGeometry(2, 2, 29, 20); self.animation.setStyleSheet("background: transparent;")
        icon_path = os.path.join(self.res_path, 'MenuSymbol.bmp')
        if os.path.exists(icon_path): self.animation.setPixmap(QPixmap(icon_path).scaled(24, 20, Qt.KeepAspectRatio)); self.animation.setAlignment(Qt.AlignCenter)

        self.separator = QFrame(self.ui_container); self.separator.setGeometry(0, 0, 580, 2); self.separator.setFrameShape(QFrame.HLine); self.separator.setFrameShadow(QFrame.Sunken)

    def show_options(self): OptionsDialog(self).exec_()
    def show_about(self): AboutDialog(self).exec_()
    def show_credits(self): CreditsDialog(self).exec_()
    def launch_game(self):
        clonk_bin = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'clonk'))
        if os.path.exists(clonk_bin): subprocess.Popen([clonk_bin], cwd=os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

class OptionsDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent); self.setWindowTitle("Properties"); self.setFixedSize(400, 300); self.setStyleSheet("* { font-family: 'Comic Sans MS'; font-size: 11px; }")
        layout = QVBoxLayout(self); self.tabs = QTabWidget(); self.tabs.addTab(QWidget(), "Program"); self.tabs.addTab(QWidget(), "Graphics"); self.tabs.addTab(QWidget(), "Sound"); self.tabs.addTab(QWidget(), "Keyboard"); self.tabs.addTab(QWidget(), "Network"); layout.addWidget(self.tabs)
        self.btn_ok = QPushButton("OK", self); self.btn_ok.clicked.connect(self.accept); layout.addWidget(self.btn_ok)

class AboutDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent); self.setFixedSize(550, 412); self.layout = QVBoxLayout(self); self.layout.setContentsMargins(0, 0, 0, 0)
        self.label = QLabel(self); path = os.path.join(parent.dump_path, 'Planet_fixed.bin_2_1000_1031.bmp')
        if os.path.exists(path): self.label.setPixmap(QPixmap(path))
        self.layout.addWidget(self.label); self.label.mousePressEvent = lambda e: self.accept()

class CreditsDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent); self.setFixedSize(550, 412); self.layout = QVBoxLayout(self); self.layout.setContentsMargins(0, 0, 0, 0)
        self.label = QLabel(self); path = os.path.join(parent.dump_path, 'Planet_fixed.bin_2_1010_1031.bmp')
        if os.path.exists(path): self.label.setPixmap(QPixmap(path))
        self.layout.addWidget(self.label); self.label.mousePressEvent = lambda e: self.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv); launcher = ClonkLauncher(); launcher.show(); sys.exit(app.exec_())
