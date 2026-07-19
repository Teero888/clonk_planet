import sys
import os
import subprocess
import socket
import struct
import zlib
import gzip
import signal
import tempfile
import re
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                             QLabel, QTreeView, QTextEdit, QRadioButton, QButtonGroup,
                             QFrame, QDialog, QTabWidget, QVBoxLayout, 
                             QStackedWidget, QHBoxLayout, QGroupBox, QComboBox, 
                             QSpinBox, QCheckBox, QGridLayout, QSlider, QLineEdit, QMessageBox, QListWidget, QStyledItemDelegate, QStyle, QStyleOptionViewItem)
from PyQt5.QtGui import QPixmap, QPalette, QBrush, QStandardItemModel, QStandardItem, QIcon, QPainter, QColor, QFont, QKeySequence, QImage, QFontDatabase
from PyQt5.QtCore import Qt, QRect, QSize, QTimer, QUrl, QPoint, pyqtSignal, QProcess, QEvent
from PyQt5.QtMultimedia import QSoundEffect, QMediaPlayer, QMediaContent
from clonk_ui import ClonkButton, ClonkArea, ClonkTexturedWidget
from clonk_popup import ClonkPopupDialog, ClonkPlayerPropertiesDialog
from win3d import Win3DGroupBox, Win3DButton, Win3DTabWidget
from c4_io import C4Group, C4GroupWriter, unscramble, parse_c4_text, update_c4_text, get_group_title

class PixelDelegate(QStyledItemDelegate):
    def __init__(self, parent=None, check_on=None, check_off=None, check_locked_on=None, check_locked_off=None):
        super().__init__(parent)
        self.check_on = check_on
        self.check_off = check_off
        self.check_locked_on = check_locked_on
        self.check_locked_off = check_locked_off

    def paint(self, painter, option, index):
        self.initStyleOption(option, index)
        painter.save()
        painter.setRenderHint(QPainter.Antialiasing, False)
        painter.setRenderHint(QPainter.SmoothPixmapTransform, False)
        
        widget = option.widget
        style = widget.style() if widget else QApplication.style()

        # 0. Draw background (without full-row selection highlight)
        opts = QStyleOptionViewItem(option)
        opts.state &= ~QStyle.State_Selected
        style.drawPrimitive(QStyle.PE_PanelItemViewItem, opts, painter, widget)

        # 1. Draw Checkbox
        if option.features & QStyleOptionViewItem.HasCheckIndicator:
            check_rect = style.subElementRect(QStyle.SE_ItemViewItemCheckIndicator, option, widget)
            check_rect.translate(-1, 0)
            
            data = index.data(Qt.UserRole + 1)
            is_locked = data.get('is_locked', False) if data else False
            
            if is_locked:
                pix = self.check_locked_on if option.checkState == Qt.Checked else self.check_locked_off
            else:
                pix = self.check_on if option.checkState == Qt.Checked else self.check_off
                
            if pix and not pix.isNull():
                tx = check_rect.x() + (check_rect.width() - pix.width()) // 2
                ty = check_rect.y() + (check_rect.height() - pix.height()) // 2
                painter.drawPixmap(tx, ty, pix)

        # 2. Draw Icon (Decoration)
        if option.features & QStyleOptionViewItem.HasDecoration:
            decor_rect = style.subElementRect(QStyle.SE_ItemViewItemDecoration, option, widget)
            if option.features & QStyleOptionViewItem.HasCheckIndicator:
                decor_rect.translate(-6, 0)
            else:
                decor_rect.translate(-3, 0)
            icon = index.data(Qt.DecorationRole)
            if isinstance(icon, QIcon):
                state = QIcon.On if option.state & QStyle.State_Open else QIcon.Off
                pix = icon.pixmap(option.decorationSize, QIcon.Normal, state)
                if not pix.isNull():
                    tx = decor_rect.x() + (decor_rect.width() - pix.width()) // 2
                    ty = decor_rect.y() + (decor_rect.height() - pix.height()) // 2
                    painter.drawPixmap(tx, ty, pix)

        # 3. Draw Text (Display) with label-only highlight
        display_rect = style.subElementRect(QStyle.SE_ItemViewItemText, option, widget)
        if option.features & QStyleOptionViewItem.HasCheckIndicator:
            display_rect.translate(-7, 0)
        else:
            display_rect.translate(-3, 0)
        text = index.data(Qt.DisplayRole)
        if text:
            painter.setFont(option.font)
            fm = option.fontMetrics
            text_w = fm.horizontalAdvance(text) if hasattr(fm, 'horizontalAdvance') else fm.width(text)
            
            if option.state & QStyle.State_Selected:
                # Highlight only the text area
                highlight_rect = QRect(display_rect.x() - 2, option.rect.y(), text_w + 6, option.rect.height())
                group = QPalette.Active if option.state & QStyle.State_HasFocus else QPalette.Inactive
                painter.fillRect(highlight_rect, option.palette.brush(group, QPalette.Highlight))
                painter.setPen(option.palette.color(group, QPalette.HighlightedText))
            else:
                painter.setPen(option.palette.text().color())
            
            painter.drawText(display_rect, Qt.AlignVCenter | Qt.AlignLeft, text)

        painter.restore()

    def editorEvent(self, event, model, option, index):
        if event.type() in (QEvent.MouseButtonPress, QEvent.MouseButtonRelease, QEvent.MouseButtonDblClick):
            data = index.data(Qt.UserRole + 1)
            if data and data.get('is_locked', False):
                return False # Block interaction
        return super().editorEvent(event, model, option, index)

class SplashWindow(QWidget):
    finished = pyqtSignal()
    def __init__(self, intro_frames, sound_start):
        super().__init__()
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setFixedSize(550, 412)
        self.label = QLabel(self)
        self.label.setGeometry(0, 0, 550, 412)
        self.intro_frames = intro_frames
        self.intro_index = 0
        self.sound_start = sound_start
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.next_frame)
        
        # Center on screen
        screen = QApplication.primaryScreen().geometry()
        self.move((screen.width() - self.width()) // 2, (screen.height() - self.height()) // 2)

    def start(self):
        if self.sound_start: self.sound_start.play()
        self.timer.start(33)
        self.show()

    def next_frame(self):
        if self.intro_index < len(self.intro_frames):
            path = os.path.join(os.path.dirname(__file__), 'res_splash', self.intro_frames[self.intro_index])
            if os.path.exists(path):
                self.label.setPixmap(QPixmap(path))
            self.intro_index += 1
        elif self.intro_index < len(self.intro_frames) + 15:
            # Stay at the last frame
            self.intro_index += 1
        else:
            self.timer.stop()
            self.finished.emit()
            self.close()

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
    def __init__(self, action_name, key_code=0, dialog=None):
        # Convert numeric key code to string
        key_str = QKeySequence(key_code).toString() if key_code else ""
        super().__init__(key_str)
        self.action_name = action_name
        self.key_code = key_code
        self.dialog = dialog

        self.setReadOnly(True)
        self.setFixedSize(65, 20)
        self.setCursor(Qt.ArrowCursor)

    def keyPressEvent(self, event):
        if event.key() in (Qt.Key_Shift, Qt.Key_Control, Qt.Key_Alt, Qt.Key_Meta):
            return

        # Clonk 4 (Windows) expects Virtual Key codes.
        # Map common non-ASCII Qt keys to Windows VK codes.
        qt_to_vk = {
            Qt.Key_Left: 37, Qt.Key_Up: 38, Qt.Key_Right: 39, Qt.Key_Down: 40,
            Qt.Key_Insert: 45, Qt.Key_Delete: 46, Qt.Key_Home: 36, Qt.Key_End: 35,
            Qt.Key_PageUp: 33, Qt.Key_PageDown: 34, Qt.Key_Return: 13, Qt.Key_Enter: 13,
            Qt.Key_Backspace: 8, Qt.Key_Space: 32, Qt.Key_Tab: 9, Qt.Key_Escape: 27
        }
        
        raw_key = event.key()
        self.key_code = qt_to_vk.get(raw_key, raw_key)
        
        # If it's a letter, Qt uses ASCII but sometimes we need to ensure it's uppercase
        if 0x41 <= self.key_code <= 0x5A: # A-Z
             pass
        elif 0x61 <= self.key_code <= 0x7A: # a-z
             self.key_code -= 0x20
             
        key_str = QKeySequence(raw_key).toString()
        self.setText(key_str)
        # Update the dialog's cache
        if self.dialog and hasattr(self.dialog, 'on_key_changed'):
            self.dialog.on_key_changed(self.action_name, self.key_code)
class ClonkLauncher(QMainWindow):
    def __init__(self):
        super().__init__()
        self.base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
        self.planet_data_path = os.path.join(self.base_path, 'planet_data')
        self.res_path = os.path.join(self.base_path, 'launcher/res')
        self.res_music_path = os.path.join(self.base_path, 'launcher/res_music')
        self.wav_path = os.path.join(self.base_path, 'launcher/res_wav')
        self.dump_path = os.path.join(self.base_path, 'launcher/res_dump')
        self.comic_font_family = "Comic Sans MS"
        
        # Load Atlases
        icons_path = os.path.join(self.dump_path, 'Planet_fixed.bin_2_1015_1031.bmp')
        self.icons_atlas = QPixmap(icons_path)
        if not self.icons_atlas.isNull():
            mask = self.icons_atlas.createMaskFromColor(QColor("#ff00ff"), Qt.MaskInColor)
            self.icons_atlas.setMask(mask)

        check_path = os.path.join(self.dump_path, 'Planet_fixed.bin_2_1016_1031.bmp')
        self.check_atlas = QPixmap(check_path)
        if not self.check_atlas.isNull():
            mask = self.check_atlas.createMaskFromColor(QColor("#ff00ff"), Qt.MaskInColor)
            self.check_atlas.setMask(mask)

        self.temp_dir = tempfile.TemporaryDirectory()
        self.check_on_pix = QPixmap()
        self.check_off_pix = QPixmap()
        self.check_locked_on_pix = QPixmap()
        self.check_locked_off_pix = QPixmap()
        if not self.check_atlas.isNull():
            self.check_on_pix = self.check_atlas.copy(QRect(16, 0, 16, 16))
            self.check_off_pix = self.check_atlas.copy(QRect(32, 0, 16, 16))
            self.check_locked_on_pix = self.check_atlas.copy(QRect(112, 0, 16, 16))
            self.check_locked_off_pix = self.check_atlas.copy(QRect(128, 0, 16, 16))

        self.sound_start = self.load_sound('sound_7008.wav')
        self.sound_click = self.load_sound('sound_7002.wav')
        
        # Music Player
        self.music_player = QMediaPlayer(self)
        self.midi_process = QProcess(self)
        self.current_music_path = None
        self.extract_frontend_music()

        # Global access for UI components
        ClonkButton.click_sound = self.sound_click
        ClonkButton.font_family = self.comic_font_family
        Win3DButton.click_sound = self.sound_click
        Win3DButton.font_family = self.comic_font_family

        self.config_path = os.path.join(self.planet_data_path, 'clonk.ini')
        self.language = "US"
        self.load_config()
        self.init_ui()
        self.refresh_resources()

    def load_config(self):
        self.config_data = {}
        self.other_sections = {}
        if os.path.exists(self.config_path):
            try:
                with open(self.config_path, 'r', encoding='latin-1') as f:
                    current_section = None
                    for line in f:
                        line = line.strip()
                        if not line or line.startswith(';'): continue
                        if line.startswith('[') and line.endswith(']'):
                            current_section = line[1:-1]
                            if current_section.lower() != 'software':
                                self.other_sections[current_section] = []
                            continue
                        
                        if current_section.lower() == 'software' and '=' in line:
                            key, val = line.split('=', 1)
                            self.config_data[key.strip()] = val.strip()
                        elif current_section is not None and current_section.lower() != 'software':
                            self.other_sections[current_section].append(line)
            except Exception as e:
                print(f"Error loading config: {e}")
        
        self.language = self.get_cfg("General\\Language", "US")

    def save_config(self):
        # Update language
        self.config_data["RedWolf Design\\Clonk 4\\General\\Language"] = self.language
        
        abs_path = os.path.abspath(self.config_path)
        print(f"DEBUG: Saving config to: {abs_path}")
        try:
            with open(self.config_path, 'wb') as f:
                # Write [Software] section
                f.write(b"[Software]\r\n")
                # Case-insensitive sort for keys
                keys = sorted(self.config_data.keys(), key=lambda x: x.lower())
                # print(f"DEBUG: Total keys in [Software]: {len(keys)}")
                for key in keys:
                    val = self.config_data[key]
                    line = f"{key}={val}"
                    #if "Controls" in key:
                        # print(f"DEBUG: Writing keybind to file: {line}")
                    f.write(f"{line}\r\n".encode('latin-1'))
                
                # Write other sections
                for section, lines in self.other_sections.items():
                    f.write(f"[{section}]\r\n".encode('latin-1'))
                    for line in lines:
                        f.write(f"{line}\r\n".encode('latin-1'))
            # print(f"DEBUG: Save complete. Size: {os.path.getsize(self.config_path)} bytes")
        except Exception as e:
            print(f"ERROR: {e}")
            QMessageBox.critical(None, "Error", f"Could not save config: {e}")

    def get_atlas_icon(self, index):
        if self.icons_atlas.isNull(): return QIcon()
        return QIcon(self.icons_atlas.copy(QRect(index * 16, 0, 16, 16)))

    def get_group_title(self, grp, default_name):
        from c4_io import get_group_title as gt
        title = gt(grp, self.language)
        if title == "Unknown": return default_name
        return title

    def apply_clonk_transparency(self, pix):
        if pix.isNull(): return pix
        img = pix.toImage().convertToFormat(QPixmap.fromImage(QImage()).toImage().Format_ARGB32)
        # We use a bitwise approach for speed on larger Title.bmp images
        # Magic colors that Clonk uses for transparency
        magic_colors = [
            QColor("#ff00ff").rgb() & 0xFFFFFF, # Magenta
            QColor("#c0c4fc").rgb() & 0xFFFFFF, # Clonk Blue
            QColor("#000000").rgb() & 0xFFFFFF, # Black (often used in Title.bmp/Icon.bmp)
            QColor("#008080").rgb() & 0xFFFFFF, # Teal
            QColor("#ffff00").rgb() & 0xFFFFFF  # Yellow
        ]
        
        bg_rgb = img.pixelColor(0, 0).rgb() & 0xFFFFFF
        
        # Always mask magenta and clonk-blue
        to_mask = {magic_colors[0], magic_colors[1]}
        # Only mask others if they are at the corner (0,0)
        if bg_rgb in magic_colors:
            to_mask.add(bg_rgb)
            
        modified = False
        for y in range(img.height()):
            for x in range(img.width()):
                if (img.pixel(x, y) & 0xFFFFFF) in to_mask:
                    img.setPixel(x, y, 0) # Set to fully transparent
                    modified = True
        
        return QPixmap.fromImage(img) if modified else pix

    def refresh_resources(self):
        self.tree_model.clear()
        if not os.path.exists(self.planet_data_path): return

        bold_font = QFont(self.comic_font_family)
        bold_font.setBold(True)

        def get_item_info(name, grp):
            ext = name.lower()[-4:]
            default_name = name
            if ext in ('.c4p', '.c4f', '.c4s', '.c4d', '.c4v'):
                default_name = name[:-4]
            title = self.get_group_title(grp, default_name)
            cat = 9
            if ext == '.c4p': cat = 0
            elif ext == '.c4f': cat = 1
            elif ext == '.c4s': cat = 2
            elif ext == '.c4d': cat = 3
            return {'name': name, 'grp': grp, 'title': title, 'cat': cat}

        def set_item_icon(item, grp, default_atlas_idx):
            icon_bmp = grp.get_file('Icon.bmp')
            if icon_bmp:
                pix = QPixmap()
                pix.loadFromData(icon_bmp)
                item.setIcon(QIcon(self.apply_clonk_transparency(pix)))
            else:
                if default_atlas_idx is not None:
                    item.setIcon(self.get_atlas_icon(default_atlas_idx))

        def add_item(parent_item, name, grp, path, current_subs):
            info = get_item_info(name, grp)
            item = QStandardItem(info['title'])
            item_type = 'unknown'
            if info['cat'] == 0: item_type = 'clonk'
            elif info['cat'] == 1: item_type = 'folder'
            elif info['cat'] == 2: item_type = 'scenario'
            elif info['cat'] == 3: item_type = 'package'
            
            item.setData({'path': path, 'sub': current_subs, 'type': item_type})
            
            if item_type == 'folder':
                item.setFont(bold_font)
                set_item_icon(item, grp, 4)
                subs = []
                for e in grp.entries:
                    if e['name'].lower()[-4:] in ('.c4f', '.c4s', '.c4d', '.c4p'):
                        sub_data = grp.get_file(e['name'])
                        if sub_data:
                            s_grp = C4Group(raw_data=sub_data)
                            subs.append(get_item_info(e['name'], s_grp))
                
                def tutorial_sort_key(s):
                    keyboard_order = [
                        "A Clonk", "More Clonks", "Air Travel", "Production Line",
                        "Gold Mine", "Underground", "Acid Lake", "Wipf Rescue",
                        "Arctic Ocean", "Volcanic"
                    ]
                    mouse_order = [
                        "Settlement", "Goldmine", "Production", "Objects"
                    ]
                    
                    t = s['title']
                    if info['title'] == "Keyboard Control" and t in keyboard_order:
                        return (0, keyboard_order.index(t))
                    if info['title'] == "Mouse Control" and t in mouse_order:
                        return (0, mouse_order.index(t))
                    return (s['cat'], t.lower())

                for s in sorted(subs, key=tutorial_sort_key):
                    add_item(item, s['name'], s['grp'], path, current_subs + [s['name']])
                    
            elif item_type == 'scenario':
                icon_bmp = grp.get_file('Icon.bmp')
                if icon_bmp:
                    set_item_icon(item, grp, None)
                else:
                    scen_data = grp.get_file('Scenario.txt')
                    icon_id = 0
                    if scen_data:
                        m = re.search(r'^Icon=(\d+)', scen_data.decode('latin-1', errors='ignore'), re.MULTILINE)
                        if m: icon_id = int(m.group(1))
                    item.setIcon(self.get_atlas_icon(icon_id + 24))
            elif item_type == 'package':
                set_item_icon(item, grp, 12)
                if not current_subs: # Root node
                    item.setCheckable(True)
                    is_objects = (name.lower() == 'objects.c4d' or info['title'] == 'Objects')
                    item.setCheckState(Qt.Checked if is_objects else Qt.Unchecked)
                subs = []
                for e in grp.entries:
                    if e['name'].lower().endswith('.c4d'):
                        sub_data = grp.get_file(e['name'])
                        if sub_data:
                            s_grp = C4Group(raw_data=sub_data)
                            subs.append(get_item_info(e['name'], s_grp))
                def tutorial_sort_key(s):
                    keyboard_order = [
                        "A Clonk", "More Clonks", "Air Travel", "Production Line",
                        "Gold Mine", "Underground", "Acid Lake", "Wipf Rescue",
                        "Arctic Ocean", "Volcanic"
                    ]
                    mouse_order = [
                        "Settlement", "Goldmine", "Production", "Objects"
                    ]
                    
                    t = s['title']
                    if info['title'] == "Keyboard Control" and t in keyboard_order:
                        return (0, keyboard_order.index(t))
                    if info['title'] == "Mouse Control" and t in mouse_order:
                        return (0, mouse_order.index(t))
                    return (s['cat'], t.lower())

                for s in sorted(subs, key=tutorial_sort_key):
                    add_item(item, s['name'], s['grp'], path, current_subs + [s['name']])
            elif item_type == 'clonk':
                item.setCheckable(True)
                item.setCheckState(Qt.Checked)
                player_data = grp.get_file('Player.txt')
                color = 1
                if player_data:
                    m = re.search(r'^Color=(\d+)', player_data.decode('latin-1', errors='ignore'), re.MULTILINE)
                    if m: color = int(m.group(1))
                item.setIcon(self.get_atlas_icon(99 + color))

            if parent_item:
                parent_item.appendRow(item)
            else:
                self.tree_model.appendRow(item)

        roots = []
        for f in os.listdir(self.planet_data_path):
            if f.lower()[-4:] in ('.c4f', '.c4s', '.c4p', '.c4d'):
                path = os.path.join(self.planet_data_path, f)
                roots.append(get_item_info(f, C4Group(path)))
        
        for r in sorted(roots, key=lambda x: (x['cat'], x['title'].lower())):
            add_item(None, r['name'], r['grp'], os.path.join(self.planet_data_path, r['name']), [])

        self.tree.collapseAll()

    def on_item_expanded(self, index):
        item = self.tree_model.itemFromIndex(index)
        if not item: return
        data = item.data()
        if data and data.get('type') == 'folder':
            item.setIcon(self.get_atlas_icon(22))

    def on_item_collapsed(self, index):
        item = self.tree_model.itemFromIndex(index)
        if not item: return
        data = item.data()
        if data and data.get('type') == 'folder':
            item.setIcon(self.get_atlas_icon(4))

    def on_tree_selection(self, selected, deselected):
        indexes = selected.indexes()
        if not indexes: return
        # Always use column 0 item
        idx = indexes[0]
        if idx.column() != 0:
            idx = idx.siblingAtColumn(0)
        item = self.tree_model.itemFromIndex(idx)
        data = item.data()
        if not data: 
            self.desc.setText("Select an item to see its description.")
            self.preview.setPixmap(QPixmap())
            return

        path = data.get('path')
        sub = data.get('sub')

        grp = C4Group(path)
        target_grp = grp
        if sub:
            # It's a group inside a group
            sub_list = sub if isinstance(sub, list) else [sub]
            for s in sub_list:
                sub_data = target_grp.get_file(s)
                if sub_data:
                    target_grp = C4Group(raw_data=sub_data)
                else:
                    break

        # Description
        desc_data = None
        lang_order = [self.language, 'US' if self.language == 'DE' else 'DE']
        for l in lang_order:
            for ext in ['.rtf', '.txt']:
                desc_data = target_grp.get_file(f'Desc{l}{ext}')
                if desc_data: break
            if desc_data: break
        
        if not desc_data:
            for ini_name in ['Scenario.txt', 'Player.txt']:
                ini_data = target_grp.get_file(ini_name)
                if ini_data:
                    ini_text = ini_data.decode('latin-1', errors='ignore')
                    import re
                    if ini_name == 'Player.txt':
                        def get_field(name, default="0"):
                            m = re.search(rf'^{name}=(.*)$', ini_text, re.MULTILINE)
                            return m.group(1).strip() if m else default
                        
                        rank_name = get_field('RankName', 'Neuling')
                        p_name = get_field('Name', 'Clonk')
                        score = get_field('Score', '0')
                        rounds = get_field('Rounds', '0')
                        rounds_won = get_field('RoundsWon', '0')
                        rounds_lost = get_field('RoundsLost', '0')
                        
                        play_time = int(get_field('TotalPlayingTime', '0'))
                        secs = play_time // 36 if play_time > 1000 else play_time
                        
                        h = secs // 3600
                        m_ = (secs % 3600) // 60
                        s_ = secs % 60
                        
                        comment = get_field('Comment', '')
                        
                        desc_str = f"<b><span style='font-size: 10pt'>{rank_name} {p_name}</span></b><br><br>Score: {score}<br>Rounds: {rounds} ({rounds_won} won {rounds_lost} lost)<br>Playing time: {h:02d}:{m_:02d}:{s_:02d}<br>Comment: {comment}"
                        desc_data = desc_str.encode('latin-1')
                        break
                    else:
                        m = re.search(r'^Comment=(.*)$', ini_text, re.MULTILINE)
                        if m:
                            desc_data = m.group(1).encode('latin-1')
                            break

        if desc_data:
            text = desc_data.decode('latin-1', errors='ignore').rstrip('\x00')
            if text.startswith(r'{\rtf'):
                import re
                skip_level = 0
                html = ''
                i = 0
                bold = False
                italic = False
                strike = False
                underline = False
                curr_size = None
                
                while i < len(text):
                    c = text[i]
                    if c == '{':
                        i += 1
                        if skip_level > 0:
                            skip_level += 1
                        else:
                            j = i
                            while j < len(text) and text[j] in ('\n', '\r', ' '): j += 1
                            if j < len(text) and text[j] == '\\':
                                j += 1
                                tag = ''
                                if j < len(text) and text[j] == '*':
                                    tag = '*'
                                else:
                                    while j < len(text) and text[j].isalpha():
                                        tag += text[j]
                                        j += 1
                                if tag in ('fonttbl', 'colortbl', 'stylesheet', 'info', '*'):
                                    skip_level = 1
                    elif c == '}':
                        i += 1
                        if skip_level > 0: skip_level -= 1
                    elif c == '\\':
                        i += 1
                        tag = ''
                        if i < len(text) and not text[i].isalpha():
                            tag = text[i]
                            i += 1
                            if tag == "'":
                                if i + 2 <= len(text):
                                    try:
                                        if skip_level == 0: html += chr(int(text[i:i+2], 16))
                                    except: pass
                                    i += 2
                                continue
                        else:
                            while i < len(text) and text[i].isalpha():
                                tag += text[i]
                                i += 1
                            arg = ''
                            if i < len(text) and text[i] == '-':
                                arg += '-'
                                i += 1
                            while i < len(text) and text[i].isdigit():
                                arg += text[i]
                                i += 1
                            if i < len(text) and text[i] == ' ': i += 1
                        
                        if skip_level == 0:
                            if tag == 'par' or tag == 'line': html += '<br>'
                            elif tag == 'tab': html += '&nbsp;&nbsp;&nbsp;&nbsp;'
                            elif tag == 'b':
                                if arg == '0':
                                    if bold: html += '</b>'; bold = False
                                else:
                                    if not bold: html += '<b>'; bold = True
                            elif tag == 'i':
                                if arg == '0':
                                    if italic: html += '</i>'; italic = False
                                else:
                                    if not italic: html += '<i>'; italic = True
                            elif tag == 'strike':
                                if arg == '0':
                                    if strike: html += '</s>'; strike = False
                                else:
                                    if not strike: html += '<s>'; strike = True
                            elif tag == 'ul' or tag == 'ulnone':
                                if tag == 'ulnone' or arg == '0':
                                    if underline: html += '</u>'; underline = False
                                else:
                                    if not underline: html += '<u>'; underline = True
                            elif tag == 'fs' and arg:
                                try:
                                    size_pt = int(arg) // 2
                                    if curr_size is not None:
                                        html += '</span>'
                                    html += f'<span style="font-size: {size_pt}pt">'
                                    curr_size = size_pt
                                except ValueError:
                                    pass
                    elif c in ('\r', '\n'):
                        i += 1
                    else:
                        if skip_level == 0:
                            if c == '<': html += '&lt;'
                            elif c == '>': html += '&gt;'
                            else: html += c
                        i += 1
                
                if curr_size is not None: html += '</span>'
                if bold: html += '</b>'
                if italic: html += '</i>'
                if strike: html += '</s>'
                if underline: html += '</u>'
                
                html = re.sub(r'(<br>\s*){3,}', '<br><br>', html).strip()
                self.desc.setHtml(html)
            else:
                if text.startswith('<b>'):
                    self.desc.setHtml(text)
                else:
                    title_text = ""
                    title_data = target_grp.get_file('Title.txt') or target_grp.get_file('Names.txt')
                    if title_data:
                        lines = title_data.decode('latin-1', errors='ignore').splitlines()
                        for l in lines:
                            if l.startswith(self.language + ':'): title_text = l[3:]; break
                            elif ':' in l and not any(l.startswith(p+':') for p in ['DE', 'US']): title_text = l.split(':', 1)[1]
                            elif l and not ':' in l: title_text = l
                            
                    if not title_text:
                        sub_v = data.get('sub')
                        title_text = (sub_v[-1] if isinstance(sub_v, list) and sub_v else sub_v) or os.path.basename(data.get('path', ''))
                        if isinstance(title_text, list): title_text = os.path.basename(data.get('path', ''))
                        if '.' in title_text: title_text = title_text.rsplit('.', 1)[0]
                        
                    safe_text = text.replace('<', '&lt;').replace('>', '&gt;').replace('\n', '<br>')
                    html_text = f"<b><span style='font-size: 10pt'>{title_text}</span></b><br><br>{safe_text}"
                    self.desc.setHtml(html_text)
        else:
            self.desc.setPlainText("No description available.")

        # Image
        is_title = False
        img_data = target_grp.get_file('Title.bmp')
        if img_data:
            is_title = True
        else:
            img_data = target_grp.get_file('Icon.bmp')

        needs_crop = False
        if not img_data:
            img_data = target_grp.get_file('Graphics.bmp') or target_grp.get_file('Picture.bmp') or target_grp.get_file('Portrait.bmp')
            needs_crop = True

        if img_data:
            pix = QPixmap()
            pix.loadFromData(img_data)

            if not is_title:
                pix = self.apply_clonk_transparency(pix)            
            if needs_crop:
                defcore_data = target_grp.get_file('DefCore.txt')
                if defcore_data:
                    import re
                    defcore_text = defcore_data.decode('latin-1', errors='ignore')
                    m_pic = re.search(r'^Picture=\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)', defcore_text, re.MULTILINE)
                    if m_pic:
                        px, py, pw, ph = map(int, m_pic.groups())
                        pix = pix.copy(px, py, pw, ph)
                    else:
                        m_w = re.search(r'^Width=\s*(\d+)', defcore_text, re.MULTILINE)
                        m_h = re.search(r'^Height=\s*(\d+)', defcore_text, re.MULTILINE)
                        if m_w and m_h:
                            pw, ph = int(m_w.group(1)), int(m_h.group(1))
                            pix = pix.copy(0, 0, pw, ph)
                            
            if pix.width() > 0 and pix.height() > 0:
                is_scenario = data.get('type') in ('scenario', 'folder')
                is_root_package = (data.get('type') in ('package', 'clonk') and not data.get('sub'))

                if is_scenario or is_root_package:
                    pix = pix.scaled(self.preview.size(), Qt.IgnoreAspectRatio, Qt.FastTransformation)
                    self.preview.setGraphicsEffect(None)
                else:
                    orig_w = pix.width()
                    offset = QSize(20,20)
                    pix = pix.scaled(self.preview.size() - offset, Qt.KeepAspectRatio, Qt.FastTransformation)
                    f = pix.width() / orig_w if orig_w > 0 else 1.0

                    from PyQt5.QtWidgets import QGraphicsDropShadowEffect
                    effect = QGraphicsDropShadowEffect()
                    effect.setBlurRadius(0)
                    effect.setColor(QColor(0, 0, 0, 141))
                    # Shadow offset is half a pixel of the scaled texture
                    off = 3
                    effect.setOffset(off, off)
                    self.preview.setGraphicsEffect(effect)

            self.preview.setPixmap(pix)

        # Handle Package Locking
        root = self.tree_model.invisibleRootItem()
        if item.data().get('type') == 'scenario':
            scen_title = item.text()
            scen_path = item.data().get('path', '')
            scen_file = os.path.basename(scen_path).lower()
            scen_sub = item.data().get('sub', [])
            if scen_sub:
                scen_file = scen_sub[-1].lower() if isinstance(scen_sub, list) else scen_sub.lower()

            locks = {}
            # Robust matching for special scenarios
            if "research" in scen_title.lower() or scen_file == "research.c4s":
                locks = {"Hazard": True, "Knights": False, "Objects": False}
            elif "dunkelfels" in scen_title.lower() or scen_file == "dunkelfels.c4s":
                locks = {"Hazard": False, "Knights": True, "Objects": True}
            
            for row in range(root.rowCount()):
                it = root.child(row)
                it_data = it.data()
                if it_data and it_data.get('type') == 'package':
                    pkg_title = it.text().lower()
                    pkg_path = it_data.get('path', '').lower()
                    pkg_file = os.path.basename(pkg_path)
                    if pkg_file.endswith('.c4d'): pkg_file = pkg_file[:-4]

                    # Reset to default state first (Objects only)
                    is_obj = (pkg_title == "objects" or pkg_file == "objects")
                    it.setCheckState(Qt.Checked if is_obj else Qt.Unchecked)

                    # Check if this package should be locked
                    matched_lock = None
                    for lock_name, lock_state in locks.items():
                        if pkg_title == lock_name.lower() or pkg_file == lock_name.lower():
                            matched_lock = lock_state
                            break

                    d = it.data() or {}
                    if matched_lock is not None:
                        d['is_locked'] = True
                        it.setCheckState(Qt.Checked if matched_lock else Qt.Unchecked)
                    else:
                        d['is_locked'] = False
                    it.setData(d)
        else:
            # Unlock everything if not a scenario
            for row in range(root.rowCount()):
                it = root.child(row)
                it_data = it.data()
                if it_data and it_data.get('type') == 'package':
                    d = it.data() or {}
                    d['is_locked'] = False
                    it.setData(d)
        
        # Reset effect if no image
        if not img_data:
            self.preview.setGraphicsEffect(None)
            self.preview.setPixmap(QPixmap())

    def load_sound(self, name):
        path = os.path.join(self.wav_path, name)
        if os.path.exists(path):
            effect = QSoundEffect(); effect.setSource(QUrl.fromLocalFile(os.path.abspath(path))); effect.setVolume(1.0)
            return effect
        return None

    def play_sound(self, effect):
        if effect: effect.play()

    def extract_frontend_music(self):
        # 1. Check if already dumped in res_music
        for name in ('Frontend.mid', 'Frontend Old.mid'):
            path = os.path.join(self.res_music_path, name)
            if os.path.exists(path):
                self.current_music_path = path
                return

        # 2. Extract from Music.c4g
        music_grp_path = os.path.join(self.planet_data_path, 'Music.c4g')
        if os.path.exists(music_grp_path):
            try:
                grp = C4Group(music_grp_path)
                # Try Frontend.mid or Frontend Old.mid
                song_data = grp.get_file('Frontend.mid') or grp.get_file('Frontend Old.mid')
                if song_data:
                    temp_path = os.path.join(self.temp_dir.name, 'frontend.mid')
                    with open(temp_path, 'wb') as f:
                        f.write(song_data)
                    self.current_music_path = temp_path
            except Exception as e:
                print(f"Error extracting music: {e}")

    def get_cfg(self, sub_key, default):
        search_key = f"RedWolf Design\\Clonk 4\\{sub_key}".lower()
        for k, v in self.config_data.items():
            if k.lower() == search_key:
                return v
        return str(default)

    def start_music(self):
        if self.current_music_path and os.path.exists(self.current_music_path):
            if self.get_cfg("Sound\\FEMusic", "1") == "1":
                # Check for SoundFont and our custom C++ player
                sf_path = os.path.join(self.planet_data_path, "FluidR3_GM_GS.sf2")
                midi_player_bin = os.path.abspath(os.path.join(self.base_path, 'build', 'clonk_midi'))

                # print(f"DEBUG: Starting music from {self.current_music_path}")
                if os.path.exists(sf_path) and os.path.exists(midi_player_bin):
                    # Play via clonk_midi process
                    self.stop_music()
                    args = [midi_player_bin, self.current_music_path, sf_path]
                    self.midi_process.start(midi_player_bin, args[1:])
                else:
                    # Fallback to QMediaPlayer (may not work on all systems)
                    self.music_player.setMedia(QMediaContent(QUrl.fromLocalFile(os.path.abspath(self.current_music_path))))
                    self.music_player.play()

    def stop_music(self):
        # print("DEBUG: Stopping music")
        if self.midi_process.state() != QProcess.NotRunning:
            self.midi_process.terminate()
            if not self.midi_process.waitForFinished(1000):
                self.midi_process.kill()
        self.music_player.stop()

    def update_music(self):
        if self.get_cfg("Sound\\FEMusic", "1") == "1":
            # Check if MIDI process is starting or running, or if QMediaPlayer is playing
            is_playing = (self.music_player.state() == QMediaPlayer.PlayingState or 
                          self.midi_process.state() != QProcess.NotRunning)
            # print(f"DEBUG: update_music - is_playing={is_playing}, player_state={self.music_player.state()}, midi_state={self.midi_process.state()}")
            if not is_playing:
                self.start_music()
        else:
            self.stop_music()
    def init_ui(self):

        self.client_w, self.client_h = 578, 393
        self.setWindowTitle("Clonk Planet")
        self.central = QWidget(); self.setCentralWidget(self.central); self.central.setFixedSize(self.client_w, self.client_h)
        self.init_menu(); self.setFixedSize(self.sizeHint())
        self.bg_label = QLabel(self.central); self.bg_label.setGeometry(0, 0, self.client_w, self.client_h)
        self.set_background('Planet_fixed.bin_2_1019_1031.bmp', is_raw=True)
        self.ui_container = QWidget(self.central); self.ui_container.setGeometry(0, 0, self.client_w, self.client_h)
        self.setup_main_ui()

    def init_menu(self):
        import webbrowser
        menubar = self.menuBar(); menubar.setStyleSheet("")
        menu_style = "QMenu { background-color: #c0c0c0; font-size: 11px; border: 2px outset #ffffff; color: black; } QMenu::item:selected { background-color: #000080; color: white; }"
        options_menu = menubar.addMenu("Options"); options_menu.setStyleSheet(menu_style); options_menu.addAction("Options...").triggered.connect(self.show_options); options_menu.addAction("Registration...")
        help_menu = menubar.addMenu("Help"); help_menu.setStyleSheet(menu_style); help_menu.addAction("Contents..."); help_menu.addAction("Credits...").triggered.connect(self.show_credits); help_menu.addAction("Clonk on the web").triggered.connect(lambda: webbrowser.open("http://www.clonk.de/")); help_menu.addSeparator(); help_menu.addAction("About Clonk Planet...").triggered.connect(self.show_about)

    def set_background(self, name, is_raw=False):
        dir_p = self.dump_path if is_raw else self.res_path
        path = os.path.join(dir_p, name)
        if os.path.exists(path):
            pix = QPixmap(path)
            if pix.width() < 500:
                palette = self.bg_label.palette(); palette.setBrush(QPalette.Window, QBrush(pix)); self.bg_label.setPalette(palette); self.bg_label.setAutoFillBackground(True); self.bg_label.setPixmap(QPixmap()) 
            else:
                scaled = pix.scaled(self.client_w, self.client_h, Qt.KeepAspectRatioByExpanding, Qt.FastTransformation)
                crop_rect = QRect((scaled.width() - self.client_w) // 2, (scaled.height() - self.client_h) // 2, self.client_w, self.client_h)
                self.bg_label.setAutoFillBackground(False); self.bg_label.setPixmap(scaled.copy(crop_rect))
        else:
            self.bg_label.setAutoFillBackground(False); self.bg_label.setStyleSheet("background-color: #c0c0c0;")

    def setup_main_ui(self):
        font_style = f"'{self.comic_font_family}', 'Chilanka', 'cursive'"
        self.ui_container.setStyleSheet(f"* {{ font-family: {font_style}; font-size: 11px; color: black; }}")

        # Tree View Area
        self.tree_frame = ClonkArea(self.ui_container, 9, 10, 243, 346, bg_color="white")
        self.tree = QTreeView(self.tree_frame)
        self.tree.setGeometry(2, 2, 239, 342)
        self.tree.setHeaderHidden(True); self.tree.setFrameShape(QFrame.NoFrame)
        self.tree_model = QStandardItemModel(); self.tree.setModel(self.tree_model)
        self.tree.setItemDelegate(PixelDelegate(self.tree, self.check_on_pix, self.check_off_pix, self.check_locked_on_pix, self.check_locked_off_pix))
        self.tree.selectionModel().selectionChanged.connect(self.on_tree_selection)
        self.tree.expanded.connect(self.on_item_expanded)
        self.tree.collapsed.connect(self.on_item_collapsed)
        self.tree.setRootIsDecorated(False)
        self.tree.setIndentation(20)
        self.tree.setExpandsOnDoubleClick(True)
        self.tree.setEditTriggers(QTreeView.NoEditTriggers)
        self.tree.setIconSize(QSize(16, 16))
        self.tree.setUniformRowHeights(True)
        self.tree.setStyleSheet(f"""
            QTreeView {{
                background-color: white;
                border: none;
            }}
            QTreeView::branch {{
                image: none;
            }}
            QTreeView::branch:has-children {{
                image: none;
            }}
        """)
        
        p = self.tree.palette()
        p.setColor(QPalette.Highlight, QColor("#3096fa"))
        p.setColor(QPalette.Inactive, QPalette.Highlight, QColor("#3096fa")) # Keep color when unfocused
        self.tree.setPalette(p)

        
        # Preview Area
        self.preview_frame = ClonkArea(self.ui_container, 261, 10, 212, 151, bg_color=None)
        self.preview = QLabel(self.preview_frame); self.preview.setGeometry(2, 2, 208, 147); self.preview.setAlignment(Qt.AlignCenter)

        # Description Area
        self.desc_frame = ClonkArea(self.ui_container, 261, 166, 212, 171, bg_color="white")
        self.desc = QTextEdit(self.desc_frame); self.desc.setGeometry(2, 2, 208, 167); self.desc.setReadOnly(True); self.desc.setFrameShape(QFrame.NoFrame)
        self.desc.setText("Select a scenario or player to begin.")

        # Buttons
        btn_bg_raw = os.path.join(self.dump_path, 'Planet_fixed.bin_2_1006_1031.bmp')
        global_offset = (117, 14)
        def create_btn(text, x, y):
            btn = ClonkButton(text, self.ui_container, btn_bg_raw, global_offset); btn.move(x, y); btn.clicked.connect(lambda: self.play_sound(self.sound_click))
            return btn

        self.btn_new = create_btn("New", 482, 10); self.btn_activate = create_btn("Activate", 482, 34); self.btn_rename = create_btn("Rename", 482, 59); self.btn_delete = create_btn("Delete", 482, 83); self.btn_props = create_btn("Properties", 482, 107);
        self.btn_props.clicked.connect(self.show_props)
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

    def show_options(self):
        if OptionsDialog(self).exec_() == QDialog.Accepted:
            # Try to refresh preview or other states without clearing the whole tree
            self.update_music()
            indexes = self.tree.selectedIndexes()
            if indexes:
                self.on_tree_selection(self.tree.selectionModel().selection(), None)

    def show_about(self): AboutDialog(self).exec_()
    def show_credits(self): CreditsDialog(self).exec_()

    def show_props(self):
        indexes = self.tree.selectedIndexes()
        if not indexes: return
        item = self.tree_model.itemFromIndex(indexes[0])
        data = item.data()
        if not data or data.get('type') != 'clonk': return

        path = data.get('path')
        sub = data.get('sub')
        
        grp = C4Group(path)
        target_grp = grp
        # If nested, we need to drill down
        if sub:
            for s in sub:
                sub_data = target_grp.get_file(s)
                if sub_data:
                    target_grp = C4Group(raw_data=sub_data)
                else:
                    break

        info_data = target_grp.get_file('Player.txt')
        if not info_data: return
        
        content = info_data.decode('latin-1', errors='ignore')
        sections = parse_c4_text(content)
        pref = sections.get('Preferences', {})
        
        try:
            pref_color = int(pref.get('Color', 0))
            pref_control = int(pref.get('Control', 0))
            pref_mouse = int(pref.get('Mouse', 0))
        except:
            pref_color = pref_control = pref_mouse = 0

        dlg = ClonkPlayerPropertiesDialog(self)
        dlg.load_settings(pref_color, pref_control, pref_mouse)
        
        if dlg.exec_() == QDialog.Accepted:
            new_settings = dlg.get_settings()
            final_content = update_c4_text(content, 'Preferences', new_settings)
            final_data = final_content.encode('latin-1')

            # Build updated group
            writer = C4GroupWriter()
            writer.pack = target_grp.is_packed # Keep packing status
            
            # Add all existing files except Player.txt
            writer.add_from_group(target_grp, exclude=['Player.txt'])
            # Add updated Player.txt
            writer.add_file('Player.txt', final_data)
            
            # If it's a top-level group, we can write directly to disk
            if not sub:
                try:
                    writer.write_to_file(path)
                    
                    # Instead of full refresh, just update the icon
                    item = self.tree_model.itemFromIndex(indexes[0])
                    color = 1
                    m = re.search(r'^Color=(\d+)', final_content, re.MULTILINE)
                    if m: color = int(m.group(1))
                    item.setIcon(self.get_atlas_icon(99 + color))
                    
                    # Also update description if it was changed
                    self.on_tree_selection_changed()
                except Exception as e:
                    print(f"Error saving player: {e}")
                    #ClonkPopupDialog(self, text=f"Error saving player: {e}").exec_()
            else:
                # Nested saving is complex (requires recursive group rebuilding)
                # For this prototype, we inform the user
                print(f"Updated Player.txt for nested group {path}/{'/'.join(sub)}:\n{final_content}")
                #ClonkPopupDialog(self, text="Updated (nested groups simulated in prototype)").exec_()
    def launch_game(self):
        self.stop_music()
        clonk_bin = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'clonk'))
        if not os.path.exists(clonk_bin):
            # Try just 'clonk' in the same directory as well, or from PATH
            if not os.path.exists(os.path.join(self.planet_data_path, 'clonk')):
                 QMessageBox.critical(self, "Error", f"Executable not found at {clonk_bin}")
                 return
            clonk_bin = "./clonk"

        args = [clonk_bin]
        
        # 1. Add selected scenario
        scenario_selected = False
        indexes = self.tree.selectedIndexes()
        if indexes:
            item = self.tree_model.itemFromIndex(indexes[0])
            data = item.data()
            if data and data.get('type') == 'scenario':
                scenario_selected = True
                path = data.get('path')
                sub = data.get('sub')
                if sub:
                    # Path is the root group (e.g. Worlds.c4f), sub is the internal path
                    rel_group = os.path.relpath(path, self.planet_data_path)
                    if isinstance(sub, list):
                        args.append(os.path.join(rel_group, *sub))
                    else:
                        args.append(os.path.join(rel_group, sub))
                else:
                    args.append(os.path.relpath(path, self.planet_data_path))

        if not scenario_selected:
            msg = "To start a round you need to select a scenario first. To do this, open a scenario folder (book) by double clicking on it and select the desired scenario. Then click 'start'."
            ClonkPopupDialog(self, text=msg).exec_()
            return

        # 2. Add checked players and definition packages
        def collect_checked(parent_item):
            for row in range(parent_item.rowCount()):
                item = parent_item.child(row)
                if item.isCheckable() and item.checkState() == Qt.Checked:
                    data = item.data()
                    if data:
                        path = data.get('path')
                        sub = data.get('sub')
                        if path:
                            rel_path = os.path.relpath(path, self.planet_data_path)
                            if sub:
                                if isinstance(sub, list):
                                    args.append(os.path.join(rel_path, *sub))
                                else:
                                    args.append(os.path.join(rel_path, sub))
                            else:
                                args.append(rel_path)
                
                if item.hasChildren():
                    collect_checked(item)

        collect_checked(self.tree_model.invisibleRootItem())

        print(f"Launching: {' '.join(args)}")
        subprocess.Popen(args, cwd=self.planet_data_path)


class OptionsDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Properties")
        self.setFixedSize(380, 411)
        
        self.launcher = parent
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

        # Keyboard Setup
        self.key_codes = {}
        for b in range(1, 5):
            for i in range(1, 13):
                key_name = f"Kbd{b}Key{i}"
                cfg_val = self.launcher.get_cfg(f"Controls\\{key_name}", "0")
                try:
                    self.key_codes[key_name] = int(cfg_val)
                except ValueError:
                    self.key_codes[key_name] = 0

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

    def on_key_changed(self, action_name, code):
        key_id = f"Kbd{self.current_block}{action_name}"
        #print(f"DEBUG: Key changed - {key_id} = {code}")
        self.key_codes[key_id] = code

    def load_key_profile(self, block_num):
        self.current_block = block_num
        is_hardcoded = (block_num == 4)
        
        hardcoded_map = {
            "Key1": Qt.Key_Insert, "Key2": Qt.Key_Home,   "Key3": Qt.Key_PageUp,
            "Key4": Qt.Key_Delete, "Key5": Qt.Key_Up,     "Key6": Qt.Key_PageDown,
            "Key7": Qt.Key_Left,   "Key8": Qt.Key_Down,   "Key9": Qt.Key_Right,
            "Key10": Qt.Key_End,   "Key11": Qt.Key_Return, "Key12": Qt.Key_Backspace
        }

        for key_id, inp in self.key_inputs.items():
            if is_hardcoded:
                code = hardcoded_map.get(key_id, 0)
            else:
                code = self.key_codes.get(f"Kbd{block_num}{key_id}", 0)
            
            inp.setEnabled(not is_hardcoded)
            inp.key_code = code
            inp.setText(QKeySequence(code).toString() if code else "")
        
        if hasattr(self, 'btn_reset'):
            self.btn_reset.setEnabled(not is_hardcoded)

    def prompt_reset_controls(self):
        reply = QMessageBox.warning(
            self, 
            "Clonk Planet", 
            "Reset all controls?", 
            QMessageBox.Ok | QMessageBox.Cancel
        )

        if reply == QMessageBox.Ok:
            # Revert the current block back to default state (logic can be expanded here)
            print("Resetting controls!")

    def accept(self):
        #print(f"Applying settings... Current key_codes: {self.key_codes}")
        def set_cfg(sub_key, val):
            full_key = f"RedWolf Design\\Clonk 4\\{sub_key}"
            # Case-insensitive update: find existing key to avoid duplicates
            target_key = full_key
            for k in list(self.launcher.config_data.keys()):
                if k.lower() == full_key.lower():
                    target_key = k
                    break
            #print(f"DEBUG: set_cfg - {target_key} = {val}")
            self.launcher.config_data[target_key] = str(val)

        # Program page
        self.launcher.language = "DE" if self.radio_german.isChecked() else "US"
        set_cfg("General\\Language", self.launcher.language)
        set_cfg("General\\FEFontName", self.combo_font.currentText())
        set_cfg("General\\FEFontSize", self.spin_menus.value())
        set_cfg("General\\RXFontName", self.combo_font.currentText())
        set_cfg("General\\RXFontSize", self.spin_game.value())
        set_cfg("Explorer\\ShowQuickStart", 1 if self.check_quick.isChecked() else 0)
        set_cfg("Developer\\Active", 1 if self.check_dev.isChecked() else 0)

        # Graphics page
        res = 0
        if self.radio_800.isChecked(): res = 1
        elif self.radio_1024.isChecked(): res = 2
        set_cfg("Graphics\\Resolution", res)
        set_cfg("Graphics\\SmokeLevel", self.slider_smoke.value() * 2)
        set_cfg("Graphics\\SplitscreenDividers", 1 if self.chk_viewport.isChecked() else 0)
        set_cfg("Graphics\\ShowPlayerInfoAlways", 1 if self.chk_ext_info.isChecked() else 0)
        set_cfg("Graphics\\ColorAnimation", 1 if self.chk_color_anim.isChecked() else 0)
        set_cfg("Graphics\\ShowStartupMessages", 1 if self.chk_startup.isChecked() else 0)
        set_cfg("Graphics\\ShowCommands", 1 if self.chk_obj_cmds.isChecked() else 0)
        set_cfg("Graphics\\ShowPortraits", 1 if self.chk_portraits.isChecked() else 0)
        set_cfg("Graphics\\DDrawAccel", 1 if self.chk_ddraw.isChecked() else 0)

        # Sound page
        set_cfg("Sound\\RXSound", 1 if self.chk_game_sfx.isChecked() else 0)
        set_cfg("Sound\\RXSoundLoops", 1 if self.chk_game_loops.isChecked() else 0)
        set_cfg("Sound\\RXMusic", 1 if self.chk_game_music.isChecked() else 0)
        set_cfg("Sound\\FESamples", 1 if self.chk_front_sfx.isChecked() else 0)
        set_cfg("Sound\\FEMusic", 1 if self.chk_front_music.isChecked() else 0)

        # Keyboard page
        #print(f"DEBUG: Saving keys... current keys in config: {len(self.launcher.config_data)}")
        for b in range(1, 4): # Only save blocks 1-3
            for i in range(1, 13):
                key = f"Kbd{b}Key{i}"
                if key in self.key_codes:
                    val = self.key_codes[key]
                    set_cfg(f"Controls\\{key}", val)
        #print(f"DEBUG: After saving keys... current keys in config: {len(self.launcher.config_data)}")

        # Network page
        set_cfg("Network\\Active", 1 if self.chk_net_active.isChecked() else 0)
        set_cfg("Network\\LocalName", self.inp_hostname.text())
        set_cfg("Network\\MasterServerAddress", self.inp_master.text())
        
        hosts = []
        for i in range(self.list_hosts.count()):
            hosts.append(self.list_hosts.item(i).text())
        set_cfg("Network\\Hosts", ";".join(hosts))

        self.launcher.save_config()
        super().accept()

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
        
        if self.launcher.language == "DE":
            self.radio_german.setChecked(True)
        else:
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
        font_name = self.launcher.get_cfg("General\\FEFontName", self.launcher.comic_font_family)
        self.combo_font.addItem(font_name)
        self.combo_font.setFixedWidth(220) 
        font_layout.addWidget(self.combo_font)
        
        # Sub-layout for spinboxes
        spin_container = QHBoxLayout()
        spin_container.setSpacing(15)
        
        menus_layout = QVBoxLayout()
        menus_layout.setSpacing(2)
        menus_layout.addWidget(QLabel("Menus"))
        self.spin_menus = QSpinBox()
        self.spin_menus.setValue(int(self.launcher.get_cfg("General\\FEFontSize", 9)))
        self.spin_menus.setFixedWidth(45) 
        menus_layout.addWidget(self.spin_menus)
        
        game_layout = QVBoxLayout()
        game_layout.setSpacing(2)
        game_layout.addWidget(QLabel("Game"))
        self.spin_game = QSpinBox()
        self.spin_game.setValue(int(self.launcher.get_cfg("General\\RXFontSize", 10)))
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
        self.check_quick.setChecked(self.launcher.get_cfg("Explorer\\ShowQuickStart", "1") == "1")
        front_layout.addWidget(self.check_quick)
        layout.addWidget(front_group)

        # Mode Group
        mode_group = Win3DGroupBox("Mode")
        mode_layout = QVBoxLayout(mode_group)
        mode_layout.setContentsMargins(8, 16, 8, 8)
        self.check_dev = QCheckBox("Enable developer mode")
        self.check_dev.setChecked(self.launcher.get_cfg("Developer\\Active", "0") == "1")
        mode_layout.addWidget(self.check_dev)
        layout.addWidget(mode_group)
        
        layout.addStretch()
        return page
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
        
        res_val = int(self.launcher.get_cfg("Graphics\\Resolution", "1"))
        if res_val == 0: self.radio_640.setChecked(True)
        elif res_val == 1: self.radio_800.setChecked(True)
        else: self.radio_1024.setChecked(True)
        
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
        self.slider_smoke.setValue(int(self.launcher.get_cfg("Graphics\\SmokeLevel", "200")) // 2)
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
        
        self.chk_viewport = QCheckBox("Viewport delimiters")
        self.chk_viewport.setChecked(self.launcher.get_cfg("Graphics\\SplitscreenDividers", "1") == "1")
        self.chk_ext_info = QCheckBox("Extended player info")
        self.chk_ext_info.setChecked(self.launcher.get_cfg("Graphics\\ShowPlayerInfoAlways", "1") == "1")
        self.chk_color_anim = QCheckBox("Color animation")
        self.chk_color_anim.setChecked(self.launcher.get_cfg("Graphics\\ColorAnimation", "1") == "1")
        self.chk_startup = QCheckBox("Startup messages")
        self.chk_startup.setChecked(self.launcher.get_cfg("Graphics\\ShowStartupMessages", "1") == "1")
        self.chk_obj_cmds = QCheckBox("Object commands")
        self.chk_obj_cmds.setChecked(self.launcher.get_cfg("Graphics\\ShowCommands", "1") == "1")
        self.chk_portraits = QCheckBox("Portraits")
        self.chk_portraits.setChecked(self.launcher.get_cfg("Graphics\\ShowPortraits", "0") == "1")
        
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
        self.chk_ddraw.setChecked(self.launcher.get_cfg("Graphics\\DDrawAccel", "0") == "1")
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
        self.chk_game_sfx.setChecked(self.launcher.get_cfg("Sound\\RXSound", "1") == "1")
        self.chk_game_loops = QCheckBox("Sound loops")
        self.chk_game_loops.setChecked(self.launcher.get_cfg("Sound\\RXSoundLoops", "1") == "1")
        self.chk_game_music = QCheckBox("Music")
        self.chk_game_music.setChecked(self.launcher.get_cfg("Sound\\RXMusic", "1") == "1")
        
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
        self.chk_front_sfx.setChecked(self.launcher.get_cfg("Sound\\FESamples", "1") == "1")
        self.chk_front_music = QCheckBox("Music")
        self.chk_front_music.setChecked(self.launcher.get_cfg("Sound\\FEMusic", "1") == "1")
        
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

        self.current_block = 1
        self.key_inputs = {} # ActionName -> Widget

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

        # Map display labels to "KeyN" IDs
        keybinds_info = [
            ("Select left",   "Key1",  0, 0), ("Select toggle", "Key2",  0, 1), ("Select right", "Key3",  0, 2),
            ("Throw",         "Key4",  1, 0), ("Up / jump",     "Key5",  1, 1), ("Dig",           "Key6",  1, 2),
            ("Left",          "Key7",  2, 0), ("Down / stop",   "Key8",  2, 1), ("Right",         "Key9",  2, 2),
            ("Player menu",   "Key10", 3, 0), ("Special 1",    "Key11", 3, 1), ("Special 2",    "Key12", 3, 2),
        ]

        for label_text, key_id, row, col in keybinds_info:
            item_layout = QHBoxLayout()
            item_layout.setSpacing(6)
            
            icon_label = QLabel()
            icon_label.setPixmap(get_kb_icon(int(key_id[3:])-1))
            item_layout.addWidget(icon_label)
            
            text_layout = QVBoxLayout()
            text_layout.setSpacing(1)
            
            lbl = QLabel(label_text)
            
            # Load initial code for block 1
            code = self.key_codes.get(f"Kbd1{key_id}", 0)
            inp = KeyBindInput(key_id, code, self)
            self.key_inputs[key_id] = inp
            
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
        self.btn_reset.clicked.connect(self.prompt_reset_controls)
        
        reset_layout.addStretch()
        reset_layout.addWidget(self.btn_reset)
        reset_layout.addStretch()
        
        layout.addLayout(reset_layout)

        return page

    def create_network_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(8, 14, 8, 8)
        layout.setSpacing(7)

        # Top Row (Active & IP Config)
        top_row = QHBoxLayout()
        self.chk_net_active = QCheckBox("Active")
        self.chk_net_active.setChecked(self.launcher.get_cfg("Network\\Active", "0") == "1")
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
        self.inp_hostname = QLineEdit(self.launcher.get_cfg("Network\\LocalName", sys_hostname.upper()))
        local_layout.addWidget(self.inp_hostname)
        local_layout.addWidget(QLabel(f"Address: {sys_hostname.lower()}"))
        
        layout.addWidget(local_group)

        # Network Hosts Group
        hosts_group = Win3DGroupBox("Network hosts")
        hosts_layout = QHBoxLayout(hosts_group)
        hosts_layout.setContentsMargins(8, 16, 8, 8)
        
        self.list_hosts = QListWidget()
        hosts_str = self.launcher.get_cfg("Network\\Hosts", "")
        if hosts_str:
            for host in hosts_str.split(";"):
                if host: self.list_hosts.addItem(host)
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
        
        self.inp_master = QLineEdit(self.launcher.get_cfg("Network\\MasterServerAddress", "www.clonk.de"))
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
    import signal
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    
    app = QApplication(sys.argv)
    app.setStyle("Windows") 

    # Load custom font
    font_path = os.path.join(os.path.dirname(__file__), '..', 'planet_data', 'Comic.ttf')
    comic_font_family = "Comic Sans MS"
    if os.path.exists(font_path):
        font_id = QFontDatabase.addApplicationFont(font_path)
        if font_id != -1:
            comic_font_family = QFontDatabase.applicationFontFamilies(font_id)[0]
    
    app_font = QFont(comic_font_family, 8)
    app.setFont(app_font)

    launcher = ClonkLauncher()
    launcher.comic_font_family = comic_font_family
    
    # Setup Splash
    splash_frames = [f'splash_{i:03d}.bmp' for i in range(1, 52)]
    splash = SplashWindow(splash_frames, launcher.sound_start)
    splash.finished.connect(launcher.show)
    splash.finished.connect(launcher.start_music)
    splash.start()

    sys.exit(app.exec_())
