import sys
import os
import subprocess
import socket
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                             QLabel, QTreeView, QTextEdit, QRadioButton, QButtonGroup,
                             QFrame, QDialog, QTabWidget, QVBoxLayout, 
                             QStackedWidget, QHBoxLayout, QGroupBox, QComboBox, 
                             QSpinBox, QCheckBox, QGridLayout, QSlider, QLineEdit, QMessageBox, QListWidget)
from PyQt5.QtGui import QPixmap, QPalette, QBrush, QStandardItemModel, QStandardItem, QIcon, QPainter, QColor, QFont, QKeySequence
from PyQt5.QtCore import Qt, QRect, QSize, QTimer, QUrl, QPoint
from PyQt5.QtMultimedia import QSoundEffect

class HostDialog(QDialog):
    def __init__(self, parent=None, title="New host address", initial_text=""):
        super().__init__(parent)
        self.setWindowTitle(title)
        self.setFixedSize(280, 110)
        
        # Inherit the exact 90s styling from the main Options menu
        if parent:
            self.setStyleSheet(parent.styleSheet())
            
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        
        self.label = QLabel(f"{title}:")
        layout.addWidget(self.label)
        
        self.input = QLineEdit(initial_text)
        layout.addWidget(self.input)
        
        layout.addSpacing(10)
        
        btn_layout = QHBoxLayout()
        btn_layout.setContentsMargins(0, 0, 0, 0)
        btn_layout.addStretch()
        
        self.btn_ok = Win3DButton("OK")
        self.btn_ok.setFixedSize(75, 23)
        self.btn_ok.setDefault(True)
        self.btn_ok.clicked.connect(self.accept)
        
        self.btn_cancel = Win3DButton("Cancel")
        self.btn_cancel.setFixedSize(75, 23)
        self.btn_cancel.clicked.connect(self.reject)
        
        btn_layout.addWidget(self.btn_ok)
        btn_layout.addWidget(self.btn_cancel)
        
        layout.addLayout(btn_layout)
        
    def get_address(self):
        return self.input.text()
    
class KeyBindInput(QLineEdit):
    def __init__(self, action_name, default_key="", parent=None):
        super().__init__(default_key, parent)
        self.action_name = action_name # Store the name so we know what this binds to
        
        # Make it look like a text box, but behave like a button
        self.setReadOnly(True) 
        self.setFixedSize(65, 20)
        self.setCursor(Qt.ArrowCursor)

    def keyPressEvent(self, event):
        # Ignore if the user just presses a modifier key by itself (Shift, Ctrl, etc.)
        if event.key() in (Qt.Key_Shift, Qt.Key_Control, Qt.Key_Alt, Qt.Key_Meta):
            return
            
        # Convert the keycode into a readable string (e.g., "Space", "Return", "A")
        key_str = QKeySequence(event.key()).toString()
        self.setText(key_str)

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

        font = self.font()
        painter.setFont(font)
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
        menu_style = "QMenu { background-color: #c0c0c0; font-size: 11px; border: 2px outset #ffffff; color: black; } QMenu::item:selected { background-color: #000080; color: white; }"
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

        self.btn_new = create_btn("New", 482, 10); self.btn_activate = create_btn("Activate", 482, 34); self.btn_rename = create_btn("Rename", 482, 59); self.btn_delete = create_btn("Delete", 482, 83); self.btn_props = create_btn("Properties", 482, 107);
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

class Win3DFrame(QFrame):
    def __init__(self, parent=None, colors=None, bg_color="#f5f5f5"):
        super().__init__(parent)
        # Default to classic Win9x raised border: 
        # Outer Top/Left, Inner Top/Left, Inner Bottom/Right, Outer Bottom/Right
        self.colors = colors or ("#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a")
        self.bg_color = bg_color

    def paintEvent(self, event):
        painter = QPainter(self)
        w, h = self.width(), self.height()
        
        # Fill background (leaves room for the 2px border)
        if self.bg_color:
            painter.fillRect(2, 2, w-4, h-4, QColor(self.bg_color))
        
        # 1. Outer Top/Left
        painter.setPen(QColor(self.colors[0]))
        painter.drawLine(0, 0, w-1, 0)
        painter.drawLine(0, 0, 0, h-1)
        
        # 2. Inner Top/Left
        painter.setPen(QColor(self.colors[1]))
        painter.drawLine(1, 1, w-2, 1)
        painter.drawLine(1, 1, 1, h-2)
        
        # 3. Inner Bottom/Right
        painter.setPen(QColor(self.colors[2]))
        painter.drawLine(1, h-2, w-2, h-2)
        painter.drawLine(w-2, 1, w-2, h-2)
        
        # 4. Outer Bottom/Right
        painter.setPen(QColor(self.colors[3]))
        painter.drawLine(0, h-1, w-1, h-1)
        painter.drawLine(w-1, 0, w-1, h-1)
        
class Win3DButton(QPushButton):
    def __init__(self, text, parent=None, raised_colors=None, sunken_colors=None, bg_color="#f5f5f5"):
        super().__init__(text, parent)
        self.raised_colors = raised_colors or ("#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a")
        
        self.sunken_colors = sunken_colors or ("#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff")
        
        self.bg_color = bg_color

    def paintEvent(self, event):
        painter = QPainter(self)
        rect = self.rect()
        w, h = rect.width(), rect.height()

        is_pressed = self.isDown() or self.isChecked()
        colors = self.sunken_colors if is_pressed else self.raised_colors

        # The Default Button check
        # If it's the default button, draw a black outline and shift the 3D border inside by 1px
        offset = 0
        if self.isDefault():
            painter.setPen(QColor("#9e9e9e"))
            # Draw the outer black ring
            painter.drawRect(0, 0, w-1, h-1)
            offset = 1 # Shift the rest of the drawing inwards

        # Fill background (respecting the offset)
        painter.fillRect(offset, offset, w - (offset*2), h - (offset*2), QColor(self.bg_color))

        # 1. Outer Top/Left
        painter.setPen(QColor(colors[0]))
        painter.drawLine(offset, offset, w-1-offset, offset)
        painter.drawLine(offset, offset, offset, h-1-offset)
        
        # 2. Inner Top/Left
        painter.setPen(QColor(colors[1]))
        painter.drawLine(offset+1, offset+1, w-2-offset, offset+1)
        painter.drawLine(offset+1, offset+1, offset+1, h-2-offset)
        
        # 3. Inner Bottom/Right
        painter.setPen(QColor(colors[2]))
        painter.drawLine(offset+1, h-2-offset, w-2-offset, h-2-offset)
        painter.drawLine(w-2-offset, offset+1, w-2-offset, h-2-offset)
        
        # 4. Outer Bottom/Right
        painter.setPen(QColor(colors[3]))
        painter.drawLine(offset, h-1-offset, w-1-offset, h-1-offset)
        painter.drawLine(w-1-offset, offset, w-1-offset, h-1-offset)

        # Draw the text
        painter.setFont(self.font())
        painter.setPen(QColor("#808080") if not self.isEnabled() else QColor("black"))
        
        # Shift the text 1 pixel down and right if pressed
        text_rect = rect.adjusted(1, 1, 0, 0) if is_pressed else rect
        
        painter.drawText(text_rect, Qt.AlignCenter, self.text())

class Win3DGroupBox(QWidget):
    def __init__(self, title, parent=None, colors=None):
        super().__init__(parent)
        self._title = title
        # Default to sunken look for GroupBoxes: Outer TL, Inner TL, Inner BR, Outer BR
        self.colors = colors or ("#a6a6a6", "#ffffff", "#a6a6a6", "#ffffff")

    def title(self):
        return self._title

    def paintEvent(self, event):
        painter = QPainter(self)
        fm = self.fontMetrics()
        title = self.title()
        
        # Get width using horizontalAdvance (fallback to width for older PyQt versions)
        text_w = fm.horizontalAdvance(title) if hasattr(fm, 'horizontalAdvance') else fm.width(title)
        text_h = fm.height()
        
        # Customization
        text_x = 8  # Shifts text a little to the left
        
        # Moves the top line higher up. (text_h // 2 is center; -1 pushes it higher)
        line_y = (text_h // 2) - 1 
        
        w = self.width()
        h = self.height()
        
        # Calculate the gap needed for the title text so the line doesn't strike through it
        gap_start = text_x - 2
        gap_end = text_x + text_w + 4
        if not title:
            gap_start = gap_end = 0

        # 1. Outer Top/Left
        painter.setPen(QColor(self.colors[0]))
        painter.drawLine(0, line_y, gap_start, line_y)
        if gap_end < w: painter.drawLine(gap_end, line_y, w-1, line_y)
        painter.drawLine(0, line_y, 0, h-1)
        
        # 2. Inner Top/Left
        painter.setPen(QColor(self.colors[1]))
        painter.drawLine(1, line_y+1, gap_start, line_y+1)
        if gap_end < w: painter.drawLine(gap_end, line_y+1, w-2, line_y+1)
        painter.drawLine(1, line_y+1, 1, h-2)
        
        # 3. Inner Bottom/Right
        painter.setPen(QColor(self.colors[2]))
        painter.drawLine(1, h-2, w-2, h-2)
        painter.drawLine(w-2, line_y+1, w-2, h-2)
        
        # 4. Outer Bottom/Right
        painter.setPen(QColor(self.colors[3]))
        painter.drawLine(0, h-1, w-1, h-1)
        painter.drawLine(w-1, line_y, w-1, h-1)
        
        # Draw Title
        painter.setPen(Qt.black)
        painter.drawText(text_x, fm.ascent() + 1, title)

class Win3DTabWidget(QWidget):
    def __init__(self, parent=None, colors=None):
        super().__init__(parent)
        self.colors = colors or ("#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a")
        self.tabs = []
        self.active_index = 0

        self.stacked = QStackedWidget(self)
        self.main_layout = QVBoxLayout(self)
        
        # We set top margin heavily to make room for the 2 rows of tabs (approx 44px)
        # plus the frame border, and give it authentic inner padding
        self.main_layout.setContentsMargins(12, 50, 14, 14)
        self.main_layout.addWidget(self.stacked)
        
        self.setMouseTracking(True)

    def addTab(self, widget, text, icon=None): 
        self.tabs.append({
            'text': text,
            'icon': icon,
            'widget': widget
        })
        self.stacked.addWidget(widget)
        self.update()

    def set_active_index(self, index):
        if 0 <= index < len(self.tabs):
            self.active_index = index
            self.stacked.setCurrentIndex(index)
            self.update()

    def mousePressEvent(self, event):
        w = self.width()
        cols = 3
        spacing = 0
        start_x = 2 # Base start is 2, so active tab (x-2) hits 0 perfectly!
        
        # Dynamically calculate width so they span the exact width of the frame
        base_tab_w = (w - 4) // cols 
        remainder = (w - 4) % cols # Catch any leftover pixels if width isn't perfectly divisible
        
        tab_h = 22
        num_tabs = len(self.tabs)
        num_rows = (num_tabs + cols - 1) // cols
        active_row = self.active_index // cols if num_tabs > 0 else 0

        visual_row_counter = 0
        for r in range(num_rows):
            v_row = num_rows - 1 if r == active_row else visual_row_counter
            if r != active_row: visual_row_counter += 1

            start_idx = r * cols
            end_idx = min(start_idx + cols, num_tabs)
            for i in range(start_idx, end_idx):
                col = i % cols
                
                extra_x = 0
                current_w = base_tab_w
                
                # Apply stagger and width adjustments
                if v_row == num_rows - 1:
                    if col == 0: extra_x = 0; current_w = base_tab_w + 2
                    elif col == 1: extra_x = 2; current_w = base_tab_w + 2
                    elif col == 2: extra_x = 4; current_w = base_tab_w - 4 + remainder # Shrink to absorb stagger
                elif col == 2:
                    current_w = base_tab_w + remainder # Add remainder to last tab of inactive rows
                    
                x = start_x + col * (base_tab_w + spacing) + extra_x
                y = 2 + v_row * tab_h
                
                is_active = (i == self.active_index)
                draw_x = x - 2 if is_active else x
                draw_y = y - 2 if is_active else y
                draw_w = current_w + 4 if is_active else current_w
                draw_h = tab_h + 3 if is_active else tab_h
                
                rect = QRect(draw_x, draw_y, draw_w, draw_h)
                if rect.contains(event.pos()):
                    self.set_active_index(i)
                    return

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.fillRect(self.rect(), QColor("#f5f5f5"))

        if not self.tabs: return

        w = self.width()
        h = self.height()
        
        cols = 3
        spacing = 0
        start_x = 2 
        
        base_tab_w = (w - 4) // cols
        remainder = (w - 4) % cols
        
        tab_h = 20
        num_tabs = len(self.tabs)
        num_rows = (num_tabs + cols - 1) // cols
        active_row = self.active_index // cols

        frame_y = num_rows * tab_h + 2 
        
        def draw_tab(i, visual_row):
            tab_data = self.tabs[i]
            col = i % cols
            
            extra_x = 0
            current_w = base_tab_w
            
            if visual_row == num_rows - 1:
                if col == 0: extra_x = 0; current_w = base_tab_w + 2
                elif col == 1: extra_x = 2; current_w = base_tab_w + 2
                elif col == 2: extra_x = 4; current_w = base_tab_w - 4 + remainder
            elif col == 2:
                current_w = base_tab_w + remainder
                
            x = start_x + col * (base_tab_w + spacing) + extra_x
            y = 2 + visual_row * tab_h
            
            is_active = (i == self.active_index)
            
            draw_x = x - 2 if is_active else x
            draw_y = y - 2 if is_active else y
            draw_w = current_w + 4 if is_active else current_w
            draw_h = (frame_y + 2) - draw_y if is_active else tab_h
            
            painter.fillRect(draw_x, draw_y, draw_w, draw_h, QColor("#f5f5f5"))
            
            # White Outer Top/Left
            painter.setPen(QColor(self.colors[0]))
            painter.drawLine(draw_x+2, draw_y, draw_x+draw_w-3, draw_y) 
            painter.drawLine(draw_x, draw_y+2, draw_x, draw_y+draw_h-1) 
            painter.drawPoint(draw_x+1, draw_y+1)
            
            # Light Gray Inner Top/Left
            painter.setPen(QColor(self.colors[1]))
            painter.drawLine(draw_x+2, draw_y+1, draw_x+draw_w-3, draw_y+1)
            painter.drawLine(draw_x+1, draw_y+2, draw_x+1, draw_y+draw_h-1)
            
            # Dark Gray Inner Right
            painter.setPen(QColor(self.colors[2]))
            painter.drawLine(draw_x+draw_w-2, draw_y+2, draw_x+draw_w-2, draw_y+draw_h-1)
            
            # Black Outer Right
            painter.setPen(QColor(self.colors[3]))
            painter.drawLine(draw_x+draw_w-1, draw_y+2, draw_x+draw_w-1, draw_y+draw_h-1)
            painter.drawPoint(draw_x+draw_w-2, draw_y+1)
            
            if is_active:
                painter.setPen(QColor("#f5f5f5"))
                if col == 0:
                    painter.drawLine(draw_x+draw_w - 1, draw_y+draw_h-1, draw_x+draw_w-2, draw_y+draw_h-1)            
                elif col == 1:
                    painter.drawLine(draw_x, draw_y+draw_h-1, draw_x+draw_w-1, draw_y+draw_h-1)
                elif col == 2:
                    painter.drawLine(draw_x, draw_y+draw_h-1, draw_x+1, draw_y+draw_h-1)
            
            # Draw Icon and Text
            painter.setPen(Qt.black)
            
            fm = painter.fontMetrics()
            text_w = fm.horizontalAdvance(tab_data['text']) if hasattr(fm, 'horizontalAdvance') else fm.width(tab_data['text'])
            icon_size = 16
            content_spacing = 4
            
            icon = tab_data['icon']
            total_w = (icon_size + content_spacing + text_w) if icon else text_w
            
            content_x = draw_x + (draw_w - total_w) // 2
            
            if icon:
                icon_y = draw_y + (draw_h - icon_size) // 2
                painter.drawPixmap(content_x, icon_y, icon.pixmap(icon_size, icon_size))
                content_x += icon_size + content_spacing
                
            text_rect = QRect(content_x, draw_y, text_w, draw_h)
            painter.drawText(text_rect, Qt.AlignVCenter | Qt.AlignLeft, tab_data['text'])

        visual_row_counter = 0
        for r in range(num_rows):
            v_row = num_rows - 1 if r == active_row else visual_row_counter
            if r != active_row: 
                visual_row_counter += 1
                
            start_idx = r * cols
            end_idx = min(start_idx + cols, num_tabs)
            
            for i in reversed(range(start_idx, end_idx)):
                if i != self.active_index:
                    draw_tab(i, v_row)
            
        # Draw Main Page Frame 
        painter.setPen(QColor(self.colors[0])) 
        painter.drawLine(0, frame_y, w-1, frame_y)
        painter.drawLine(0, frame_y, 0, h-1)
        
        painter.setPen(QColor(self.colors[1])) 
        painter.drawLine(1, frame_y+1, w-2, frame_y+1)
        painter.drawLine(1, frame_y+1, 1, h-2)
        
        painter.setPen(QColor(self.colors[2])) 
        painter.drawLine(1, h-2, w-2, h-2)
        painter.drawLine(w-2, frame_y+1, w-2, h-2)
        
        painter.setPen(QColor(self.colors[3])) 
        painter.drawLine(0, h-1, w-1, h-1)
        painter.drawLine(w-1, frame_y, w-1, h-1)
        
        if 0 <= self.active_index < num_tabs:
            draw_tab(self.active_index, num_rows - 1)

class OptionsDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Properties")
        self.setFixedSize(380, 411)
        
        self.icon_path = parent.res_path if parent else ""

# Accurate Win9x styling
        self.setStyleSheet("""
            /* The DOT before QWidget is crucial! It stops the background color 
               from infecting the QComboBox and QSpinBox subclasses */
            QDialog, .QWidget {
                background-color: #f5f5f5;
                font-size: 11px;
                color: black;
            }
            QPushButton.TabBtn {
                background-color: #f5f5f5;
                border-top: 1px solid #ffffff;
                border-left: 1px solid #ffffff;
                border-right: 1px solid #404040;
                border-bottom: 1px solid #404040;
                padding: 3px 2px;
                margin: 0px 1px 0px 0px;
                text-align: center;
            }
            QPushButton.TabBtn:checked {
                background-color: #f5f5f5;
                border-bottom: 1px solid #f5f5f5; 
                margin-top: -2px;
                margin-bottom: -1px;
                padding-bottom: 4px;
            }
        """)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(6, 7, 6, 6)
        layout.setSpacing(0)

        # NEW TAB SETUP WITH TEXTURE ATLAS
        self.tab_widget = Win3DTabWidget(self)
        
        # Load the texture atlas
        atlas_path = os.path.join(parent.dump_path if parent else "", 'Planet_fixed.bin_2_1037_1031.bmp')
        atlas_pixmap = QPixmap(atlas_path)
        
        # If the atlas loaded successfully, strip out the magenta background
        if not atlas_pixmap.isNull():
            # Qt.MaskOutColor makes the specified color (magenta) transparent
            mask = atlas_pixmap.createMaskFromColor(QColor("#ff00ff"), Qt.MaskInColor)
            atlas_pixmap.setMask(mask)
            
        # Helper to extract a 16x16 icon from the atlas
        def get_icon(index):
            if atlas_pixmap.isNull(): return None
            # copy(x, y, width, height)
            rect = QRect(index * 16, 0, 16, 16)
            return QIcon(atlas_pixmap.copy(rect))

        # Replaced file names with your texture atlas indices!
        tabs_info = [
            ("Keyboard", 10),
            ("Game Pad", 12),
            ("Network", 13),
            ("Program", 6),
            ("Graphics", 2),
            ("Sound", 5)
        ]

        for name, atlas_index in tabs_info:
            icon = get_icon(atlas_index)
            
            if name == "Program":
                self.tab_widget.addTab(self.create_program_page(), name, icon)
            elif name == "Graphics":
                self.tab_widget.addTab(self.create_graphics_page(), name, icon)
            elif name == "Sound":
                self.tab_widget.addTab(self.create_sound_page(), name, icon)
            elif name == "Keyboard":
                self.tab_widget.addTab(self.create_keyboard_page(), name, icon)
            elif name == "Network":
                self.tab_widget.addTab(self.create_network_page(), name, icon)
            else:
                page = QWidget()
                page_layout = QVBoxLayout(page)
                page_layout.addWidget(QLabel(f"{name} Settings", alignment=Qt.AlignCenter))
                self.tab_widget.addTab(page, name, icon)

        layout.addWidget(self.tab_widget, 1)

        # Set "Program" (index 3) as the default active tab
        self.tab_widget.set_active_index(3)

        # Bottom buttons
        btn_layout = QHBoxLayout()
        btn_layout.setContentsMargins(0, 8, 0, 0)
        btn_layout.setSpacing(6)
        btn_layout.addStretch()
        
        self.btn_ok = Win3DButton("OK")
        self.btn_ok.setFixedSize(75, 23)
        self.btn_ok.setDefault(True)
        self.btn_ok.clicked.connect(self.accept)
        
        self.btn_cancel = Win3DButton("Cancel")
        self.btn_cancel.setFixedSize(75, 23)
        self.btn_cancel.clicked.connect(self.reject)
        
        btn_layout.addWidget(self.btn_ok)
        btn_layout.addWidget(self.btn_cancel)
        
        layout.addLayout(btn_layout)

    def create_program_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(7)

        # Language Group
        lang_group = Win3DGroupBox("Language")
        lang_layout = QVBoxLayout(lang_group)
        lang_layout.setContentsMargins(8, 23, 8, 8) 
        lang_layout.setSpacing(4)
        self.radio_german = QRadioButton("German")
        self.radio_english = QRadioButton("English")
        self.radio_english.setChecked(True)
        lang_layout.addWidget(self.radio_german)
        lang_layout.addWidget(self.radio_english)
        layout.addWidget(lang_group)

        # Font Group
        font_group = Win3DGroupBox("Font")
        font_layout = QVBoxLayout(font_group)
        font_layout.setContentsMargins(8, 23, 8, 8) 
        font_layout.setSpacing(8)
        
        self.combo_font = QComboBox()
        self.combo_font.addItem("Comic Sans MS")
        self.combo_font.setFixedWidth(220) 
        font_layout.addWidget(self.combo_font)
        
        # Sub-layout for spinboxes
        spin_container = QHBoxLayout()
        spin_container.setSpacing(15)
        
        menus_layout = QVBoxLayout()
        menus_layout.setSpacing(2)
        menus_layout.addWidget(QLabel("Menus"))
        self.spin_menus = QSpinBox()
        self.spin_menus.setValue(9)
        self.spin_menus.setFixedWidth(45) 
        menus_layout.addWidget(self.spin_menus)
        
        game_layout = QVBoxLayout()
        game_layout.setSpacing(2)
        game_layout.addWidget(QLabel("Game"))
        self.spin_game = QSpinBox()
        self.spin_game.setValue(10)
        self.spin_game.setFixedWidth(45) 
        game_layout.addWidget(self.spin_game)
        
        spin_container.addLayout(menus_layout)
        spin_container.addLayout(game_layout)
        spin_container.addStretch()
        
        font_layout.addLayout(spin_container)
        layout.addWidget(font_group)

        # Frontend Group
        front_group = Win3DGroupBox("Frontend")
        front_layout = QVBoxLayout(front_group)
        front_layout.setContentsMargins(8, 16, 8, 8)
        self.check_quick = QCheckBox("Display quick start screen")
        self.check_quick.setChecked(True)
        front_layout.addWidget(self.check_quick)
        layout.addWidget(front_group)

        # Mode Group
        mode_group = Win3DGroupBox("Mode")
        mode_layout = QVBoxLayout(mode_group)
        mode_layout.setContentsMargins(8, 16, 8, 8)
        self.check_dev = QCheckBox("Enable developer mode")
        mode_layout.addWidget(self.check_dev)
        layout.addWidget(mode_group)

        layout.addStretch()
        return page
    
    def create_graphics_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(7)

        # Top Row (Resolution & Smoke level)
        top_row = QHBoxLayout()
        top_row.setSpacing(8)

        # Resolution Group
        res_group = Win3DGroupBox("Resolution")
        res_layout = QVBoxLayout(res_group)
        res_layout.setContentsMargins(8, 16, 8, 8)
        res_layout.setSpacing(2)
        
        self.radio_640 = QRadioButton("640x480")
        self.radio_800 = QRadioButton("800x600")
        self.radio_1024 = QRadioButton("1024x768")
        self.radio_1024.setChecked(True) # Set default
        
        res_layout.addWidget(self.radio_640)
        res_layout.addWidget(self.radio_800)
        res_layout.addWidget(self.radio_1024)
        top_row.addWidget(res_group)

        # Smoke level Group
        smoke_group = Win3DGroupBox("Smoke level")
        smoke_layout = QVBoxLayout(smoke_group)
        smoke_layout.setContentsMargins(8, 16, 8, 8)
        smoke_layout.setSpacing(2)
        
        self.slider_smoke = QSlider(Qt.Horizontal)
        self.slider_smoke.setRange(0, 100)
        self.slider_smoke.setValue(75) # Match the screenshot position
        smoke_layout.addWidget(self.slider_smoke)
        
        labels_layout = QHBoxLayout()
        labels_layout.addWidget(QLabel("low"))
        labels_layout.addStretch()
        labels_layout.addWidget(QLabel("high"))
        smoke_layout.addLayout(labels_layout)
        
        # Add a stretch so the slider pushes to the top and aligns with Resolution buttons
        smoke_layout.addStretch() 
        
        # Give the smoke group a stretch factor of 1 so it takes up the extra width
        top_row.addWidget(smoke_group, 1) 
        
        layout.addLayout(top_row)

        # Display Group
        disp_group = Win3DGroupBox("Display")
        disp_layout = QVBoxLayout(disp_group)
        disp_layout.setContentsMargins(8, 16, 8, 8)
        disp_layout.setSpacing(2)
        
        self.chk_viewport = QCheckBox("Viewport delimiters"); self.chk_viewport.setChecked(True)
        self.chk_ext_info = QCheckBox("Extended player info"); self.chk_ext_info.setChecked(True)
        self.chk_color_anim = QCheckBox("Color animation"); self.chk_color_anim.setChecked(True)
        self.chk_startup = QCheckBox("Startup messages"); self.chk_startup.setChecked(True)
        self.chk_obj_cmds = QCheckBox("Object commands"); self.chk_obj_cmds.setChecked(True)
        self.chk_portraits = QCheckBox("Portraits")
        
        disp_layout.addWidget(self.chk_viewport)
        disp_layout.addWidget(self.chk_ext_info)
        disp_layout.addWidget(self.chk_color_anim)
        disp_layout.addWidget(self.chk_startup)
        disp_layout.addWidget(self.chk_obj_cmds)
        disp_layout.addWidget(self.chk_portraits)
        
        layout.addWidget(disp_group)

        # Troubleshooting Group
        trouble_group = Win3DGroupBox("Troubleshooting")
        trouble_layout = QVBoxLayout(trouble_group)
        trouble_layout.setContentsMargins(8, 16, 8, 8)
        
        self.chk_ddraw = QCheckBox("DirectDraw software emulation")
        trouble_layout.addWidget(self.chk_ddraw)
        
        layout.addWidget(trouble_group)

        # Push everything to the top
        layout.addStretch()
        return page
    
    def create_sound_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(7)

        # Game Group
        game_group = Win3DGroupBox("Game")
        game_layout = QVBoxLayout(game_group)
        game_layout.setContentsMargins(8, 16, 8, 8) 
        game_layout.setSpacing(4)
        
        self.chk_game_sfx = QCheckBox("Sound effects")
        self.chk_game_sfx.setChecked(True)
        self.chk_game_loops = QCheckBox("Sound loops")
        self.chk_game_loops.setChecked(True)
        self.chk_game_music = QCheckBox("Music")
        self.chk_game_music.setChecked(True)
        
        game_layout.addWidget(self.chk_game_sfx)
        game_layout.addWidget(self.chk_game_loops)
        game_layout.addWidget(self.chk_game_music)
        
        layout.addWidget(game_group)

        # Frontend Group
        front_group = Win3DGroupBox("Frontend")
        front_layout = QVBoxLayout(front_group)
        front_layout.setContentsMargins(8, 16, 8, 8)
        front_layout.setSpacing(4)
        
        self.chk_front_sfx = QCheckBox("Sound effects")
        self.chk_front_sfx.setChecked(True)
        self.chk_front_music = QCheckBox("Music")
        self.chk_front_music.setChecked(True)
        
        front_layout.addWidget(self.chk_front_sfx)
        front_layout.addWidget(self.chk_front_music)
        
        layout.addWidget(front_group)

        # Volume Group
        vol_group = Win3DGroupBox("Volume")
        # We use a horizontal layout here so we can easily center the button with stretches
        vol_layout = QHBoxLayout(vol_group)
        vol_layout.setContentsMargins(8, 16, 8, 12)
        
        self.btn_volume = Win3DButton("Volume control")
        self.btn_volume.setFixedSize(160, 23) # Make it wide like in the screenshot
        
        # Adding stretches on both sides forces the button to the exact center
        vol_layout.addStretch()
        vol_layout.addWidget(self.btn_volume)
        vol_layout.addStretch()
        
        layout.addWidget(vol_group)

        # Push everything to the top
        layout.addStretch()
        return page
    
    def create_keyboard_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(8, 14, 8, 8) 
        layout.setSpacing(12)

        # Keybind Profiles Data # fill them up later with defaults
        self.key_profiles = {
            1: {"Select left": "Q", "Select toggle": "W", "Select right": "E", "Throw": "A", "Up / jump": "S", "Dig": "D", "Left": "Y", "Down / stop": "X", "Right": "C", "Player menu": "<", "Special 1": "V", "Special 2": "F"},
            2: {"Select left": "", "Select toggle": "", "Select right": "", "Throw": "", "Up / jump": "", "Dig": "", "Left": "", "Down / stop": "", "Right": "", "Player menu": "", "Special 1": "", "Special 2": ""},
            3: {"Select left": "", "Select toggle": "", "Select right": "", "Throw": "", "Up / jump": "", "Dig": "", "Left": "", "Down / stop": "", "Right": "", "Player menu": "", "Special 1": "", "Special 2": ""},
            4: {"Select left": "", "Select toggle": "", "Select right": "", "Throw": "", "Up / jump": "", "Dig": "", "Left": "", "Down / stop": "", "Right": "", "Player menu": "", "Special 1": "", "Special 2": ""}
        }
        self.current_block = 1
        self.key_inputs = {} # We will store our KeyBindInput widgets here

        # Top Row (Block Selection)
        top_row = QHBoxLayout()
        top_row.addWidget(QLabel("Keyboard"))
        top_row.addSpacing(15) 
        
        self.radio_block1 = QRadioButton("Block 1")
        self.radio_block2 = QRadioButton("Block 2")
        self.radio_block3 = QRadioButton("Block 3")
        self.radio_block4 = QRadioButton("Block 4")
        
        self.radio_block1.setChecked(True)
        
        # Connect the radio buttons to the profile loader
        self.radio_block1.toggled.connect(lambda checked: self.load_key_profile(1) if checked else None)
        self.radio_block2.toggled.connect(lambda checked: self.load_key_profile(2) if checked else None)
        self.radio_block3.toggled.connect(lambda checked: self.load_key_profile(3) if checked else None)
        self.radio_block4.toggled.connect(lambda checked: self.load_key_profile(4) if checked else None)
        
        top_row.addWidget(self.radio_block1)
        top_row.addWidget(self.radio_block2)
        top_row.addWidget(self.radio_block3)
        top_row.addWidget(self.radio_block4)
        top_row.addStretch()
        
        layout.addLayout(top_row)

        # Separator Line
        separator = QFrame()
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        layout.addWidget(separator)

        # Keyboard Atlas Loading
        res_dump_path = os.path.join(os.path.dirname(self.icon_path), 'res_dump') if self.icon_path else ""
        atlas_path = os.path.join(res_dump_path, 'Planet_fixed.bin_2_1008_1031.bmp')
        
        kb_atlas = QPixmap(atlas_path)
        if not kb_atlas.isNull():
            mask = kb_atlas.createMaskFromColor(QColor("#ff00ff"), Qt.MaskInColor)
            kb_atlas.setMask(mask)

        def get_kb_icon(index):
            if kb_atlas.isNull(): return QPixmap(32, 32)
            return kb_atlas.copy(QRect(index * 32, 0, 32, 32))

        # Grid Layout for Keybinds
        grid = QGridLayout()
        grid.setHorizontalSpacing(15)
        grid.setVerticalSpacing(6)

        keybinds_info = [
            ("Select left", 0, 0, 0), ("Select toggle", 1, 0, 1), ("Select right", 2, 0, 2),
            ("Throw", 3, 1, 0),       ("Up / jump", 4, 1, 1),     ("Dig", 5, 1, 2),
            ("Left", 6, 2, 0),        ("Down / stop", 7, 2, 1),   ("Right", 8, 2, 2),
            ("Player menu", 9, 3, 0), ("Special 1", 10, 3, 1),    ("Special 2", 11, 3, 2),
        ]

        for label_text, atlas_idx, row, col in keybinds_info:
            item_layout = QHBoxLayout()
            item_layout.setSpacing(6)
            
            icon_label = QLabel()
            icon_label.setPixmap(get_kb_icon(atlas_idx))
            item_layout.addWidget(icon_label)
            
            text_layout = QVBoxLayout()
            text_layout.setSpacing(1)
            
            lbl = QLabel(label_text)
            
            # Using our new Key Catcher!
            inp = KeyBindInput(label_text, self.key_profiles[1][label_text])
            self.key_inputs[label_text] = inp
            
            text_layout.addWidget(lbl)
            text_layout.addWidget(inp)
            text_layout.addStretch() 
            
            item_layout.addLayout(text_layout)
            grid.addLayout(item_layout, row, col)

        layout.addLayout(grid)
        layout.addStretch()

        # Bottom Reset Button
        reset_layout = QHBoxLayout()
        self.btn_reset = Win3DButton("Reset all")
        self.btn_reset.setFixedSize(100, 23)
        self.btn_reset.clicked.connect(self.prompt_reset_controls) # Connect to the warning dialog!
        
        reset_layout.addStretch()
        reset_layout.addWidget(self.btn_reset)
        reset_layout.addStretch()
        
        layout.addLayout(reset_layout)

        return page
    
    def load_key_profile(self, block_num):
        # First, save whatever the user typed into the current block
        for action, inp_widget in self.key_inputs.items():
            self.key_profiles[self.current_block][action] = inp_widget.text()
            
        # Change the active block
        self.current_block = block_num
        
        # Load the new block's keys into the UI
        for action, inp_widget in self.key_inputs.items():
            inp_widget.setText(self.key_profiles[block_num][action])

    def prompt_reset_controls(self):
        # Because we globally set the app style to "Windows", this will automatically 
        # spawn with the classic 90s warning icon and retro buttons!
        reply = QMessageBox.warning(
            self, 
            "Clonk Planet", # Title of the window
            "Reset all controls?", 
            QMessageBox.Ok | QMessageBox.Cancel
        )
        
        if reply == QMessageBox.Ok:
            # Revert the current block back to default state (implement your actual default reset logic here)
            print("Resetting controls!")
    
    def create_network_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(8, 14, 8, 8)
        layout.setSpacing(7)

        # Top Row (Active & IP Config)
        top_row = QHBoxLayout()
        self.chk_net_active = QCheckBox("Active")
        self.chk_net_active.setChecked(False)
        top_row.addWidget(self.chk_net_active)
        
        top_row.addStretch()
        
        self.btn_ip_config = Win3DButton("IP configuration")
        self.btn_ip_config.setFixedSize(110, 23)
        top_row.addWidget(self.btn_ip_config)
        
        layout.addLayout(top_row)

        # Local Computer Group
        local_group = Win3DGroupBox("Local computer")
        local_layout = QVBoxLayout(local_group)
        local_layout.setContentsMargins(8, 16, 8, 8)
        local_layout.setSpacing(4)
        
        sys_hostname = socket.gethostname()
        
        local_layout.addWidget(QLabel("Name"))
        self.inp_hostname = QLineEdit(sys_hostname.upper())
        local_layout.addWidget(self.inp_hostname)
        local_layout.addWidget(QLabel(f"Address: {sys_hostname.lower()}"))
        
        layout.addWidget(local_group)

        # Network Hosts Group
        hosts_group = Win3DGroupBox("Network hosts")
        hosts_layout = QHBoxLayout(hosts_group)
        hosts_layout.setContentsMargins(8, 16, 8, 8)
        
        self.list_hosts = QListWidget()
        hosts_layout.addWidget(self.list_hosts)
        
        hosts_btn_layout = QVBoxLayout()
        hosts_btn_layout.setSpacing(4)
        
        self.btn_host_new = Win3DButton("New")
        self.btn_host_del = Win3DButton("Delete")
        self.btn_host_edit = Win3DButton("Edit")
        
        self.btn_host_new.setFixedSize(65, 23)
        self.btn_host_del.setFixedSize(65, 23)
        self.btn_host_edit.setFixedSize(65, 23)
        
        self.btn_host_new.clicked.connect(self.add_host)
        self.btn_host_del.clicked.connect(self.delete_host)
        self.btn_host_edit.clicked.connect(self.edit_host)
        
        hosts_btn_layout.addWidget(self.btn_host_new)
        hosts_btn_layout.addWidget(self.btn_host_del)
        hosts_btn_layout.addWidget(self.btn_host_edit)
        hosts_btn_layout.addStretch() 
        
        hosts_layout.addLayout(hosts_btn_layout)
        layout.addWidget(hosts_group)

        # Internet Master Server Group
        master_group = Win3DGroupBox("Internet master server")
        master_layout = QVBoxLayout(master_group)
        master_layout.setContentsMargins(8, 16, 8, 8)
        
        self.inp_master = QLineEdit("www.clonk.de")
        master_layout.addWidget(self.inp_master)
        
        layout.addWidget(master_group)
        
        layout.addStretch()

        # TOGGLE LOGIC
        def update_network_state(is_active):
            self.btn_ip_config.setEnabled(is_active)
            local_group.setEnabled(is_active)
            hosts_group.setEnabled(is_active)
            master_group.setEnabled(is_active)

        # Connect the checkbox to the toggle function
        self.chk_net_active.toggled.connect(update_network_state)
        
        # Run it once right away so it matches the initial checked state
        update_network_state(self.chk_net_active.isChecked())

        return page

    # Network List Helpers 

    def add_host(self):
        dlg = HostDialog(self, "New host address")
        if dlg.exec_() == QDialog.Accepted:
            addr = dlg.get_address().strip()
            if addr:
                self.list_hosts.addItem(addr)

    def edit_host(self):
        curr = self.list_hosts.currentItem()
        if curr:
            dlg = HostDialog(self, "Edit host address", curr.text())
            if dlg.exec_() == QDialog.Accepted:
                addr = dlg.get_address().strip()
                if addr:
                    curr.setText(addr)

    def delete_host(self):
        curr_row = self.list_hosts.currentRow()
        if curr_row >= 0:
            self.list_hosts.takeItem(curr_row)
    
    def on_tab_clicked(self, id):
        self.buttons[id].raise_()

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
    app = QApplication(sys.argv)
    
    # Turn on the classic Win9x rendering engine globally!
    app.setStyle("Windows") 
    
    app_font = QFont("MS Sans Serif", 8)
    app.setFont(app_font)
    launcher = ClonkLauncher()
    launcher.show()
    sys.exit(app.exec_())
