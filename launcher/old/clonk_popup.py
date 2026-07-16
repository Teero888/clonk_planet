import sys
import os
from PyQt5.QtWidgets import QApplication, QDialog, QLabel
from PyQt5.QtCore import Qt
from clonk_ui import ClonkButton, ClonkTextArea, ClonkArea, ClonkTexturedWidget, ClonkAtlasWidget
from win3d import Win3DButton

class ClonkPopupDialog(QDialog):
    def __init__(self, parent=None, text="Sample Clonk Message"):
        super().__init__(parent)
        
        # Dimensions 338x156
        self.setFixedSize(338, 156)
        self.setWindowTitle("Clonk Planet")
        
        # Paths to textures
        dump_path = os.path.join(os.path.dirname(__file__), 'res_dump')
        bg_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1019_1031.bmp')
        btn_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1006_1031.bmp')
        
        # Background texture (same as launcher)
        self.bg = ClonkTexturedWidget(self, bg_path=bg_texture)
        self.bg.setGeometry(0, 0, 338, 156)
        
        # Button: 75x23 pixels, 4px margin left, 6px margin bottom
        # POPUP OFFSET: (124, 13)
        self.btn_ok = ClonkButton("OK", self.bg, bg_path=btn_texture, size=(75, 23), bg_offset=(124, 13))
        self.btn_ok.move(5, 127)
        self.btn_ok.clicked.connect(self.accept)
        
        # Border: encasing text
        # border left margin: 5px border right margin: 4px border top margin 5px border margin to button: 5px
        self.border = ClonkTextArea(self.bg, 5, 5, 329, 117)
        
        # Text inside border
        self.label = QLabel(text, self.border)
        self.label.setGeometry(8, 8, 329 - 16, 117 - 16)
        self.label.setWordWrap(True)
        self.label.setAlignment(Qt.AlignTop | Qt.AlignLeft)
        
        # Clonk style font (from launcher.py)
        font_family = "'Comic Sans MS', 'Chilanka', 'cursive'"
        self.label.setStyleSheet(f"background: transparent; color: black; font-family: {font_family}; font-size: 12px;")

class ClonkPlayerPropertiesDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # Dimensions 338x156
        self.setFixedSize(368, 180)
        self.setWindowTitle("Clonk Planet")
        
        # Paths to textures
        dump_path = os.path.join(os.path.dirname(__file__), 'res_dump')
        bg_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1019_1031.bmp')
        btn_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1006_1031.bmp')
        
        # 600x80 pixels, 6 textures, each 100 pixels wide, 0px margin
        flag_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1007_1031.bmp')
        # 660x90 pixels, 5 textures, each 132 pixels wide, 0px margin
        keyboard_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1009_1031.bmp')
        # 180x90 pixels, 2 textures, each 90 pixels wide, 0px margin
        mouse_texture = os.path.join(dump_path, 'Planet_fixed.bin_2_1017_1031.bmp')
        
        # Background texture (same as launcher)
        self.bg = ClonkTexturedWidget(self, bg_path=bg_texture)
        self.bg.setGeometry(0, 0, 368, 180)
        
        self.btn_ok = ClonkButton("OK", self.bg, bg_path=btn_texture, size=(77, 23), bg_offset=(121, 12))
        self.btn_ok.move(101, 143)
        self.btn_ok.clicked.connect(self.accept)
        
        self.btn_cancel = ClonkButton("Cancel", self.bg, bg_path=btn_texture, size=(77, 23), bg_offset=(121, 12))
        self.btn_cancel.move(186, 143)
        self.btn_cancel.clicked.connect(self.reject)
        
        border_colors = ("#a6a6a6", "#6a6a6a", "#6a6a6a", "#ffffff")
        self.color_border = ClonkArea(self.bg, 27, 26, 99, 78, border_colors=border_colors)
        self.keyboard_border = ClonkArea(self.bg, 144, 26, 101, 78, border_colors=border_colors)
        self.mouse_border = ClonkArea(self.bg, 249, 26, 83, 78, border_colors=border_colors)
        
        # Atlas widgets inside borders (filling the area minus 2px border)
        self.color_atlas = ClonkAtlasWidget(self.color_border, flag_texture, (100, 80))
        self.color_atlas.setGeometry(2, 2, 99 - 4, 78 - 4)
        
        self.keyboard_atlas = ClonkAtlasWidget(self.keyboard_border, keyboard_texture, (132, 90))
        self.keyboard_atlas.setGeometry(2, 2, 101 - 4, 78 - 4)
        
        self.mouse_atlas = ClonkAtlasWidget(self.mouse_border, mouse_texture, (90, 90))
        self.mouse_atlas.setGeometry(2, 2, 83 - 4, 78 - 4)
        
        raised_colors = ("#e3e3e3", "#ffffff", "#a6a6a6", "#6a6a6a")
        sunken_colors = ("#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff")
        
        self.btn_color_left = Win3DButton(self.bg, 27, 109, 49, 15, arrow="left", raised_colors=raised_colors, sunken_colors=sunken_colors)
        self.btn_color_right = Win3DButton(self.bg, 77, 109, 49, 15, arrow="right", raised_colors=raised_colors, sunken_colors=sunken_colors)
        self.btn_keyboard_left = Win3DButton(self.bg, 165, 109, 49, 15, arrow="left", raised_colors=raised_colors, sunken_colors=sunken_colors)
        self.btn_keyboard_right = Win3DButton(self.bg, 215, 109, 49, 15, arrow="right", raised_colors=raised_colors, sunken_colors=sunken_colors)
        self.btn_mouse_toggle = Win3DButton(self.bg, 266, 109, 45, 15)

        # Preferred mouse state (default to ON/0)
        self.pref_mouse_index = 0

        # Clonk style font (from launcher.py)
        font_family = "'Comic Sans MS', 'Chilanka', 'cursive'"
        label_style = f"background: transparent; color: black; font-family: {font_family}; font-size: 12px;"

        self.lbl_color = QLabel("Color", self.bg)
        self.lbl_color.setGeometry(27, 7, 99, 18)
        self.lbl_color.setAlignment(Qt.AlignCenter)
        self.lbl_color.setStyleSheet(label_style)
        
        self.lbl_controls = QLabel("Control", self.bg)
        self.lbl_controls.setGeometry(143, 7, 188, 18) # Span from 144 to 332
        self.lbl_controls.setAlignment(Qt.AlignCenter)
        self.lbl_controls.setStyleSheet(label_style)

        # Click handlers to cycle through options
        self.btn_color_left.clicked.connect(lambda: self.cycle_atlas(self.color_atlas, 6, -1))
        self.btn_color_right.clicked.connect(lambda: self.cycle_atlas(self.color_atlas, 6, 1))
        self.btn_keyboard_left.clicked.connect(lambda: self.cycle_atlas(self.keyboard_atlas, 5, -1))
        self.btn_keyboard_right.clicked.connect(lambda: self.cycle_atlas(self.keyboard_atlas, 5, 1))
        
        # Mouse toggle logic (button and image click)
        self.btn_mouse_toggle.clicked.connect(lambda: self.cycle_atlas(self.mouse_atlas, 2, 1))
        self.mouse_atlas.clicked.connect(lambda: self.cycle_atlas(self.mouse_atlas, 2, 1))
        
    def load_settings(self, color, control, mouse):
        self.color_atlas.setIndex(min(max(0, color), 5))
        self.keyboard_atlas.setIndex(min(max(0, control), 4))
        self.pref_mouse_index = 0 if mouse != 0 else 1 # 0: ON, 1: OFF in atlas
        
        if self.keyboard_atlas.index == 4:
            self.mouse_atlas.setIndex(1) # Forced OFF
        else:
            self.mouse_atlas.setIndex(self.pref_mouse_index)

    def get_settings(self):
        # Color: 0-5, Control: 0-4, Mouse: 1 for ON, 0 for OFF
        mouse_val = 1 if self.pref_mouse_index == 0 else 0
        return {
            'Color': self.color_atlas.index,
            'Control': self.keyboard_atlas.index,
            'Mouse': mouse_val
        }

    def cycle_atlas(self, widget, count, delta):
        if widget == self.mouse_atlas:
            # Mouse is a toggle (single button), so it MUST wrap
            self.pref_mouse_index = (self.pref_mouse_index + delta) % count
            # Only apply to widget if not on gamepad
            if self.keyboard_atlas.index != 4:
                widget.setIndex(self.pref_mouse_index)
        else:
            # Clamped index (no wrap) for keyboard/color which have left/right buttons
            new_index = widget.index + delta
            if 0 <= new_index < count:
                widget.setIndex(new_index)
                
                # Gamepad logic: if keyboard is gamepad (index 4), force mouse OFF (index 1)
                # but don't change pref_mouse_index
                if widget == self.keyboard_atlas:
                    if new_index == 4:
                        self.mouse_atlas.setIndex(1)
                    else:
                        # Restore preferred mouse state
                        self.mouse_atlas.setIndex(self.pref_mouse_index)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    # Test text
    test_text = "This is a reproduction of the Clonk Planet popup design. It features the tiled background texture, the textured OK button with offset (124, 13), and the sunken border encasing this message."
    dialog = ClonkPlayerPropertiesDialog()
    dialog.show()
    sys.exit(app.exec_())
