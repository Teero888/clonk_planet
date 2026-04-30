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
from PyQt5.QtGui import QPixmap, QPalette, QBrush, QStandardItemModel, QStandardItem, QIcon, QPainter, QColor, QFont, QKeySequence, QImage
from PyQt5.QtCore import Qt, QRect, QSize, QTimer, QUrl, QPoint, pyqtSignal
from PyQt5.QtMultimedia import QSoundEffect
from clonk_ui import ClonkButton, ClonkArea, ClonkTexturedWidget
from clonk_popup import ClonkPopupDialog

class PixelDelegate(QStyledItemDelegate):
    def __init__(self, parent=None, check_on=None, check_off=None):
        super().__init__(parent)
        self.check_on = check_on
        self.check_off = check_off

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
    def __init__(self, action_name, key_code=0, parent=None):
        # Convert numeric key code to string
        key_str = QKeySequence(key_code).toString() if key_code else ""
        super().__init__(key_str, parent)
        self.action_name = action_name 
        self.key_code = key_code
        
        self.setReadOnly(True) 
        self.setFixedSize(65, 20)
        self.setCursor(Qt.ArrowCursor)

    def keyPressEvent(self, event):
        if event.key() in (Qt.Key_Shift, Qt.Key_Control, Qt.Key_Alt, Qt.Key_Meta):
            return
            
        self.key_code = event.key()
        key_str = QKeySequence(self.key_code).toString()
        self.setText(key_str)
        # Update the parent dialog's cache
        if hasattr(self.parent(), 'on_key_changed'):
            self.parent().on_key_changed(self.action_name, self.key_code)

def unscramble(data_raw):
    data = bytearray(data_raw)
    # Inverse of (XOR then SWAP) is (SWAP then XOR)
    for i in range(0, len(data) - 2, 3):
        data[i], data[i+2] = data[i+2], data[i]
    for i in range(len(data)):
        data[i] ^= 237
    return bytes(data)

class C4Group:
    def __init__(self, path=None, raw_data=None):
        self.path = path
        self.entries = []
        self.data = b''
        if raw_data is not None:
            self.load_from_data(raw_data)
        elif path is not None:
            self.load()

    def load(self):
        try:
            with open(self.path, 'rb') as f:
                raw_data = f.read()
            self.load_from_data(raw_data)
        except Exception as e:
            print(f"Error loading {self.path}: {e}")

    def load_from_data(self, raw_data):
        try:
            if not raw_data: return

            if raw_data[0:2] in (b'\x1f\x8b', b'\x1e\x8c'):
                if raw_data[0:2] == b'\x1e\x8c':
                    fixed = bytearray(raw_data)
                    fixed[0] ^= 1; fixed[1] ^= 7
                    self.data = gzip.decompress(fixed)
                else:
                    self.data = gzip.decompress(raw_data)
            else:
                self.data = raw_data

            if len(self.data) < 204: return
            header = unscramble(self.data[0:204])
            if not header.startswith(b'RedWolf Design GrpFolder'):
                return

            num_entries = struct.unpack('<i', header[36:40])[0]
            entry_base = 204
            file_base = 204 + num_entries * 316

            for i in range(num_entries):
                e_data = self.data[entry_base + i*316 : entry_base + (i+1)*316]
                name = e_data[0:260].split(b'\x00')[0].decode('ascii', errors='ignore')
                packed, child = struct.unpack('<ii', e_data[260:268])
                size, esize, offset = struct.unpack('<iii', e_data[268:280])

                self.entries.append({
                    'name': name,
                    'size': size,
                    'offset': file_base + offset,
                    'is_group': bool(child)
                })
        except Exception as e:
            print(f"Error parsing group data: {e}")

    def get_file(self, name):
        for e in self.entries:
            if e['name'].lower() == name.lower():
                return self.data[e['offset'] : e['offset'] + e['size']]
        return None

class ClonkLauncher(QMainWindow):
    def __init__(self):
        super().__init__()
        self.base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
        self.planet_data_path = os.path.join(self.base_path, 'planet_data')
        self.res_path = os.path.join(os.path.dirname(__file__), 'res')
        self.wav_path = os.path.join(os.path.dirname(__file__), 'res_wav')
        self.dump_path = os.path.join(os.path.dirname(__file__), 'res_dump')
        
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
        if not self.check_atlas.isNull():
            self.check_on_pix = self.check_atlas.copy(QRect(16, 0, 16, 16))
            self.check_off_pix = self.check_atlas.copy(QRect(32, 0, 16, 16))

        self.sound_start = self.load_sound('sound_7008.wav')
        self.sound_click = self.load_sound('sound_7002.wav')
        self.config_path = os.path.join(self.planet_data_path, 'clonk.ini')
        self.language = "US"
        self.load_config()
        self.init_ui()
        self.refresh_resources()

    def load_config(self):
        self.config_data = {}
        if os.path.exists(self.config_path):
            try:
                with open(self.config_path, 'r', encoding='latin-1') as f:
                    current_section = None
                    for line in f:
                        line = line.strip()
                        if not line or line.startswith(';'): continue
                        if line.startswith('[') and line.endswith(']'):
                            current_section = line[1:-1]
                            continue
                        if '=' in line:
                            key, val = line.split('=', 1)
                            self.config_data[key.strip()] = val.strip()
            except Exception as e:
                print(f"Error loading config: {e}")
        
        # Helper to get config values with defaults
        def get_cfg(sub_key, default):
            return self.config_data.get(f"RedWolf Design\\Clonk 4\\{sub_key}", default)

        self.language = get_cfg("General\\Language", "US")

    def save_config(self):
        # Update specific keys that might have changed outside the options dialog
        self.config_data["RedWolf Design\\Clonk 4\\General\\Language"] = self.language
        
        try:
            # We want to preserve other sections if they exist, but mostly we care about [Software]
            with open(self.config_path, 'w', encoding='latin-1') as f:
                f.write("[Software]\n")
                for key in sorted(self.config_data.keys()):
                    f.write(f"{key}={self.config_data[key]}\n")
        except Exception as e:
            print(f"Error saving config: {e}")

    def get_atlas_icon(self, index):
        if self.icons_atlas.isNull(): return QIcon()
        return QIcon(self.icons_atlas.copy(QRect(index * 16, 0, 16, 16)))

    def get_group_title(self, grp, default_name):
        title = default_name
        title_data = grp.get_file('Title.txt') or grp.get_file('Names.txt')
        if title_data:
            lines = title_data.decode('latin-1', errors='ignore').splitlines()
            for l in lines:
                if l.startswith(self.language + ':'): title = l[3:]; break
                elif ':' in l and not any(l.startswith(p+':') for p in ['DE', 'US']): title = l.split(':', 1)[1]
                elif l and not ':' in l: title = l
        if '.' in title: title = title.rsplit('.', 1)[0]
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

        bold_font = QFont()
        bold_font.setBold(True)

        def get_item_info(name, grp):
            ext = name.lower()[-4:]
            title = self.get_group_title(grp, name)
            cat = 9
            if ext == '.c4p': cat = 0
            elif ext == '.c4f': cat = 1
            elif ext == '.c4s': cat = 2
            elif ext == '.c4d': cat = 3
            elif ext == '.c4v': cat = 1 # Treat Video groups as folders
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
                    if e['name'].lower()[-4:] in ('.c4f', '.c4s', '.c4d', '.c4p', '.c4v'):
                        sub_data = grp.get_file(e['name'])
                        if sub_data:
                            s_grp = C4Group(raw_data=sub_data)
                            subs.append(get_item_info(e['name'], s_grp))
                
                for s in sorted(subs, key=lambda x: (x['cat'], x['title'].lower())):
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
                    item.setIcon(self.get_atlas_icon(icon_id + 26))
            elif item_type == 'package':
                set_item_icon(item, grp, 12)
                if not current_subs: # Root node
                    item.setCheckable(True)
                    item.setCheckState(Qt.Checked)
                subs = []
                for e in grp.entries:
                    if e['name'].lower().endswith('.c4d'):
                        sub_data = grp.get_file(e['name'])
                        if sub_data:
                            s_grp = C4Group(raw_data=sub_data)
                            subs.append(get_item_info(e['name'], s_grp))
                for s in sorted(subs, key=lambda x: (x['cat'], x['title'].lower())):
                    add_item(item, s['name'], s['grp'], path, current_subs + [s['name']])
            elif item_type == 'clonk':
                item.setCheckable(True)
                item.setCheckState(Qt.Checked)
                player_data = grp.get_file('Player.txt')
                color = 1
                if player_data:
                    m = re.search(r'^Color=(\d+)', player_data.decode('latin-1', errors='ignore'), re.MULTILINE)
                    if m: color = int(m.group(1))
                item.setIcon(self.get_atlas_icon(99 + color - 1))

            if parent_item:
                parent_item.appendRow(item)
            else:
                self.tree_model.appendRow(item)

        roots = []
        for f in os.listdir(self.planet_data_path):
            if f.lower()[-4:] in ('.c4f', '.c4s', '.c4p', '.c4d', '.c4v'):
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
        item = self.tree_model.itemFromIndex(indexes[0])
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
        img_data = target_grp.get_file('Title.bmp') or target_grp.get_file('Icon.bmp')
        needs_crop = False
        if not img_data:
            img_data = target_grp.get_file('Graphics.bmp') or target_grp.get_file('Picture.bmp') or target_grp.get_file('Portrait.bmp')
            needs_crop = True

        if img_data:
            pix = QPixmap()
            pix.loadFromData(img_data)
            
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
        else:
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
        font_style = "'Comic Sans MS', 'Chilanka', 'cursive'"
        self.ui_container.setStyleSheet(f"* {{ font-family: {font_style}; font-size: 11px; color: black; }}")

        # Tree View Area
        self.tree_frame = ClonkArea(self.ui_container, 9, 10, 243, 346, bg_color="white")
        self.tree = QTreeView(self.tree_frame)
        self.tree.setGeometry(2, 2, 239, 342)
        self.tree.setHeaderHidden(True); self.tree.setFrameShape(QFrame.NoFrame)
        self.tree_model = QStandardItemModel(); self.tree.setModel(self.tree_model)
        self.tree.setItemDelegate(PixelDelegate(self.tree, self.check_on_pix, self.check_off_pix))
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
            self.refresh_resources()
            # Try to re-select or at least refresh preview
            indexes = self.tree.selectedIndexes()
            if indexes:
                self.on_tree_selection(self.tree.selectionModel().selection(), None)

    def show_about(self): AboutDialog(self).exec_()
    def show_credits(self): CreditsDialog(self).exec_()
    def launch_game(self):
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

        # Helper to get config values
        def get_cfg(sub_key, default):
            return self.launcher.config_data.get(f"RedWolf Design\\Clonk 4\\{sub_key}", str(default))

        # Keyboard Setup
        self.key_codes = {}
        for b in range(1, 5):
            for i in range(1, 13):
                key_name = f"Kbd{b}Key{i}"
                self.key_codes[key_name] = int(get_cfg(f"Controls\\{key_name}", 0))

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
        def set_cfg(sub_key, val):
            self.launcher.config_data[f"RedWolf Design\\Clonk 4\\{sub_key}"] = str(val)

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
        for b in range(1, 4): # Only save blocks 1-3
            for i in range(1, 13):
                key = f"Kbd{b}Key{i}"
                if key in self.key_codes:
                    set_cfg(f"Controls\\{key}", self.key_codes[key])

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
        def get_cfg(sub_key, default):
            return self.launcher.config_data.get(f"RedWolf Design\\Clonk 4\\{sub_key}", str(default))

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
        font_name = get_cfg("General\\FEFontName", "Comic Sans MS")
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
        self.spin_menus.setValue(int(get_cfg("General\\FEFontSize", 9)))
        self.spin_menus.setFixedWidth(45) 
        menus_layout.addWidget(self.spin_menus)
        
        game_layout = QVBoxLayout()
        game_layout.setSpacing(2)
        game_layout.addWidget(QLabel("Game"))
        self.spin_game = QSpinBox()
        self.spin_game.setValue(int(get_cfg("General\\RXFontSize", 10)))
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
        self.check_quick.setChecked(get_cfg("Explorer\\ShowQuickStart", "1") == "1")
        front_layout.addWidget(self.check_quick)
        layout.addWidget(front_group)

        # Mode Group
        mode_group = Win3DGroupBox("Mode")
        mode_layout = QVBoxLayout(mode_group)
        mode_layout.setContentsMargins(8, 16, 8, 8)
        self.check_dev = QCheckBox("Enable developer mode")
        self.check_dev.setChecked(get_cfg("Developer\\Active", "0") == "1")
        mode_layout.addWidget(self.check_dev)
        layout.addWidget(mode_group)

        layout.addStretch()
        return page
    
    def create_graphics_page(self):
        def get_cfg(sub_key, default):
            return self.launcher.config_data.get(f"RedWolf Design\\Clonk 4\\{sub_key}", str(default))

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
        
        res_val = int(get_cfg("Graphics\\Resolution", "1"))
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
        self.slider_smoke.setValue(int(get_cfg("Graphics\\SmokeLevel", "200")) // 2)
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
        self.chk_viewport.setChecked(get_cfg("Graphics\\SplitscreenDividers", "1") == "1")
        self.chk_ext_info = QCheckBox("Extended player info")
        self.chk_ext_info.setChecked(get_cfg("Graphics\\ShowPlayerInfoAlways", "1") == "1")
        self.chk_color_anim = QCheckBox("Color animation")
        self.chk_color_anim.setChecked(get_cfg("Graphics\\ColorAnimation", "1") == "1")
        self.chk_startup = QCheckBox("Startup messages")
        self.chk_startup.setChecked(get_cfg("Graphics\\ShowStartupMessages", "1") == "1")
        self.chk_obj_cmds = QCheckBox("Object commands")
        self.chk_obj_cmds.setChecked(get_cfg("Graphics\\ShowCommands", "1") == "1")
        self.chk_portraits = QCheckBox("Portraits")
        self.chk_portraits.setChecked(get_cfg("Graphics\\ShowPortraits", "0") == "1")
        
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
        self.chk_ddraw.setChecked(get_cfg("Graphics\\DDrawAccel", "0") == "1")
        trouble_layout.addWidget(self.chk_ddraw)
        
        layout.addWidget(trouble_group)

        # Push everything to the top
        layout.addStretch()
        return page
    
    def create_sound_page(self):
        def get_cfg(sub_key, default):
            return self.launcher.config_data.get(f"RedWolf Design\\Clonk 4\\{sub_key}", str(default))

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
        self.chk_game_sfx.setChecked(get_cfg("Sound\\RXSound", "1") == "1")
        self.chk_game_loops = QCheckBox("Sound loops")
        self.chk_game_loops.setChecked(get_cfg("Sound\\RXSoundLoops", "1") == "1")
        self.chk_game_music = QCheckBox("Music")
        self.chk_game_music.setChecked(get_cfg("Sound\\RXMusic", "1") == "1")
        
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
        self.chk_front_sfx.setChecked(get_cfg("Sound\\FESamples", "1") == "1")
        self.chk_front_music = QCheckBox("Music")
        self.chk_front_music.setChecked(get_cfg("Sound\\FEMusic", "1") == "1")
        
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
        def get_cfg(sub_key, default):
            return self.launcher.config_data.get(f"RedWolf Design\\Clonk 4\\{sub_key}", str(default))

        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(8, 14, 8, 8)
        layout.setSpacing(7)

        # Top Row (Active & IP Config)
        top_row = QHBoxLayout()
        self.chk_net_active = QCheckBox("Active")
        self.chk_net_active.setChecked(get_cfg("Network\\Active", "0") == "1")
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
        self.inp_hostname = QLineEdit(get_cfg("Network\\LocalName", sys_hostname.upper()))
        local_layout.addWidget(self.inp_hostname)
        local_layout.addWidget(QLabel(f"Address: {sys_hostname.lower()}"))
        
        layout.addWidget(local_group)

        # Network Hosts Group
        hosts_group = Win3DGroupBox("Network hosts")
        hosts_layout = QHBoxLayout(hosts_group)
        hosts_layout.setContentsMargins(8, 16, 8, 8)
        
        self.list_hosts = QListWidget()
        hosts_str = get_cfg("Network\\Hosts", "")
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
        
        self.inp_master = QLineEdit(get_cfg("Network\\MasterServerAddress", "www.clonk.de"))
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
    app_font = QFont("MS Sans Serif", 8)
    app.setFont(app_font)

    launcher = ClonkLauncher()
    
    # Setup Splash
    splash_frames = [f'splash_{i:03d}.bmp' for i in range(1, 52)]
    splash = SplashWindow(splash_frames, launcher.sound_start)
    splash.finished.connect(launcher.show)
    splash.start()

    sys.exit(app.exec_())
