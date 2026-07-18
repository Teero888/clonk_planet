import sys
from PySide6.QtCore import Qt, QRect
from PySide6.QtGui import QFont, QFontDatabase, QPainter, QImage, QColor
from PySide6.QtWidgets import QApplication

def get_text_pixels(img):
    # Convert QImage to black/white character grid
    w, h = img.width(), img.height()
    grid = []
    for y in range(h):
        row = []
        for x in range(w):
            c = QColor(img.pixel(x, y))
            # Black if dark
            row.append(1 if c.red() < 100 else 0)
        grid.append(row)
    
    # Crop empty borders
    min_x, max_x = w, 0
    min_y, max_y = h, 0
    for y in range(h):
        for x in range(w):
            if grid[y][x]:
                if x < min_x: min_x = x
                if x > max_x: max_x = x
                if y < min_y: min_y = y
                if y > max_y: max_y = y
                
    if min_x > max_x or min_y > max_y:
        return []
        
    cropped = []
    for y in range(min_y, max_y + 1):
        row = grid[y][min_x : max_x + 1]
        cropped.append(row)
    return cropped

def grid_to_str(grid):
    return '\n'.join(''.join('#' if x else '.' for x in row) for row in grid)

def main():
    app = QApplication([])
    
    # Load custom font
    font_id = QFontDatabase.addApplicationFont("build/Comic.ttf")
    if font_id == -1:
        print("Failed to load build/Comic.ttf")
        return
    family = QFontDatabase.applicationFontFamilies(font_id)[0]
    print(f"Loaded family: {family}")
    
    # Target grid for "New" from Windows XP MainPage.png
    target_str = """
.#......#................
.##.....#................
.#.#....#................
.#..#...#..###..#..#..#..
.#..#...#.#...#.#..#..#..
.#...#..#.#..#..#.##.#...
.#....#.#.###...#.##.#...
.#.....##.#...#..#.#.#...
.#......#..###...#..#....
"""
    target = []
    for line in target_str.strip().split('\n'):
        target.append([1 if c == '#' else 0 for c in line])
        
    target_h = len(target)
    target_w = len(target[0])
    print(f"Target dimensions: {target_w}x{target_h}")
    print("Target grid:")
    print(target_str.strip())
    print("\nSearching for match...")
    
    hinting_modes = {
        "Default": QFont.PreferDefaultHinting,
        "None": QFont.PreferNoHinting,
        "Vertical": QFont.PreferVerticalHinting,
        "Full": QFont.PreferFullHinting
    }
    
    weights = {
        "Normal": QFont.Normal,
        "Medium": QFont.Medium,
        "DemiBold": QFont.DemiBold,
        "Bold": QFont.Bold
    }
    
    best_score = 0
    best_params = None
    best_grid = None
    
    # Grid search
    for size in range(8, 16):
        for w_name, weight in weights.items():
            for h_name, hinting in hinting_modes.items():
                # Render text
                img = QImage(100, 30, QImage.Format_ARGB32)
                img.fill(Qt.white)
                
                painter = QPainter(img)
                font = QFont(family, size)
                font.setWeight(weight)
                font.setStyleStrategy(QFont.NoAntialias)
                font.setHintingPreference(hinting)
                painter.setFont(font)
                
                # Draw text centered vertically
                painter.setPen(QColor("black"))
                painter.drawText(0, 0, 100, 30, Qt.AlignVCenter | Qt.AlignLeft, "New")
                painter.end()
                
                grid = get_text_pixels(img)
                if not grid:
                    continue
                    
                grid_h = len(grid)
                grid_w = len(grid[0])
                
                # Compute match score (simple pixel similarity)
                # Align top-left and count matching pixels
                score = 0
                for y in range(min(target_h, grid_h)):
                    for x in range(min(target_w, grid_w)):
                        if target[y][x] == grid[y][x]:
                            score += 1
                            
                # Penalty for size mismatch
                penalty = abs(target_h - grid_h) * 5 + abs(target_w - grid_w) * 5
                final_score = score - penalty
                
                if final_score > best_score:
                    best_score = final_score
                    best_params = (size, w_name, h_name, grid_w, grid_h)
                    best_grid = grid
                    
                if grid == target:
                    print(f"\n*** EXACT MATCH FOUND! ***")
                    print(f"Size: {size}")
                    print(f"Weight: {w_name}")
                    print(f"Hinting: {h_name}")
                    return

    print(f"\nNo exact match. Best score: {best_score}")
    if best_params:
        print(f"Best parameters: Size={best_params[0]}, Weight={best_params[1]}, Hinting={best_params[2]} (rendered size: {best_params[3]}x{best_params[4]})")
        print("Best rendered grid:")
        print(grid_to_str(best_grid))

if __name__ == "__main__":
    main()
