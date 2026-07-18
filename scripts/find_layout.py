import sys
from PIL import Image

def find_buttons(img, client_offset_x, client_offset_y):
    # Buttons have a dark grey/black bottom-right border and white top-left border.
    # Let's find rectangles of size ~86x20.
    w, h = img.size
    pixels = img.load()
    
    # We will scan for white top-left corner followed by a 3D bevel.
    # A standard Clonk button is drawn with a textured background (or grey) and a raised border.
    # Let's search for lines of length ~86 where the top border is white (255,255,255)
    # and the left border is white, and the bottom border is dark.
    buttons = []
    visited = set()
    
    for y in range(client_offset_y, h - client_offset_y - 20):
        for x in range(client_offset_x + 300, w - 10):
            # Check if this could be the top-left corner of a button
            # White highlight is at x, y
            if pixels[x, y] == (255, 255, 255) and pixels[x-1, y] != (255, 255, 255):
                # Let's check if there is a white line to the right of length ~80
                length_x = 0
                while x + length_x < w and pixels[x + length_x, y] == (255, 255, 255):
                    length_x += 1
                
                # Let's check if there is a white line going down
                length_y = 0
                while y + length_y < h and pixels[x, y + length_y] == (255, 255, 255):
                    length_y += 1
                
                if length_x > 50 and length_y > 10:
                    # Let's verify the bottom-right corner is dark (like 106,106,106 or 0,0,0)
                    bx = x + length_x
                    by = y + length_y
                    
                    # Check if we already found this button
                    already_found = False
                    for bx_found, by_found, bw, bh in buttons:
                        if abs(bx_found - x) < 5 and abs(by_found - y) < 5:
                            already_found = True
                            break
                    
                    if not already_found:
                        buttons.append((x - client_offset_x, y - client_offset_y, length_x, length_y))
                        
    # Sort buttons by y coordinate
    buttons.sort(key=lambda b: b[1])
    return buttons

def main():
    img = Image.open("todo/winxp/MainPage.png")
    # Client area offset:
    # x: 10, y: 55
    client_x = 10
    client_y = 55
    
    print("Scanning MainPage.png for button coordinates (relative to client area)...")
    buttons = find_buttons(img, client_x, client_y)
    for idx, (bx, by, bw, bh) in enumerate(buttons):
        print(f"Button {idx+1}: x={bx}, y={by}, w={bw}, h={bh}")
        
if __name__ == "__main__":
    main()
