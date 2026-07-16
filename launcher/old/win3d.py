from PyQt5.QtWidgets import QWidget, QPushButton, QFrame, QVBoxLayout, QStackedWidget
from PyQt5.QtGui import QPainter, QColor, QFont
from PyQt5.QtCore import Qt, QRect

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
    click_sound = None
    font_family = "Comic Sans MS, Chilanka, cursive"
    
    def __init__(self, *args, **kwargs):
        # Custom properties
        raised_colors = kwargs.pop('raised_colors', None)
        sunken_colors = kwargs.pop('sunken_colors', None)
        bg_color = kwargs.pop('bg_color', "#f5f5f5")
        self.arrow = kwargs.pop('arrow', None)
        
        # Support both (text, parent) and (parent, x, y, w, h)
        if len(args) > 0 and not isinstance(args[0], str):
            parent = args[0]
            super().__init__("", parent)
            if len(args) >= 5:
                self.setGeometry(args[1], args[2], args[3], args[4])
        else:
            super().__init__(*args, **kwargs)

        self.raised_colors = raised_colors or ("#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a")
        self.sunken_colors = sunken_colors or ("#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff")
        self.bg_color = bg_color
        
        self.setFocusPolicy(Qt.NoFocus)
        
        # Clonk style font
        self.setFont(QFont(self.font_family, 9))
        
        # Audio feedback
        self.clicked.connect(self._play_click)

    def _play_click(self):
        if Win3DButton.click_sound:
            Win3DButton.click_sound.play()

    def paintEvent(self, event):
        painter = QPainter(self)
        rect = self.rect()
        w, h = rect.width(), rect.height()

        is_pressed = self.isDown() or self.isChecked()
        colors = self.sunken_colors if is_pressed else self.raised_colors

        # The Default Button check
        offset = 0
        if self.isDefault():
            painter.setPen(QColor("#9e9e9e"))
            painter.drawRect(0, 0, w-1, h-1)
            offset = 1 

        # Fill background
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
        painter.drawLine(offset+1, h-2-offset, w-1-offset, h-2-offset)
        painter.drawLine(w-2-offset, offset+1, w-2-offset, h-1-offset)
        
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
            
        # Draw Arrow
        if self.arrow:
            painter.setPen(QColor("black"))
            
            mid_x = w // 2
            mid_y = h // 2
            
            if is_pressed:
                mid_x += 1
                mid_y += 1
                
            if self.arrow == "left":
                # Col 1 (Base): x = mid_x + 1, height 5
                painter.drawLine(mid_x + 1, mid_y - 2, mid_x + 1, mid_y + 2)
                # Col 2 (Mid): x = mid_x, height 3
                painter.drawLine(mid_x, mid_y - 1, mid_x, mid_y + 1)
                # Col 3 (Tip): x = mid_x - 1, height 1
                painter.drawPoint(mid_x - 1, mid_y)
            elif self.arrow == "right":
                # Col 1 (Base): x = mid_x - 1, height 5
                painter.drawLine(mid_x - 1, mid_y - 2, mid_x - 1, mid_y + 2)
                # Col 2 (Mid): x = mid_x, height 3
                painter.drawLine(mid_x, mid_y - 1, mid_x, mid_y + 1)
                # Col 3 (Tip): x = mid_x + 1, height 1
                painter.drawPoint(mid_x + 1, mid_y)

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