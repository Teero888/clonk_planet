import sys
import os
from PyQt5.QtWidgets import QApplication, QDialog, QLabel
from PyQt5.QtCore import Qt
from clonk_ui import ClonkButton, ClonkTextArea, ClonkTexturedWidget

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

if __name__ == "__main__":
    app = QApplication(sys.argv)
    # Test text
    test_text = "This is a reproduction of the Clonk Planet popup design. It features the tiled background texture, the textured OK button with offset (124, 13), and the sunken border encasing this message."
    dialog = ClonkPopupDialog(text=test_text)
    dialog.show()
    sys.exit(app.exec_())
