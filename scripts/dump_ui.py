import ctypes
from ctypes import wintypes

user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32

def get_text(hwnd):
    length = user32.GetWindowTextLengthW(hwnd)
    if length == 0: return ""
    buf = ctypes.create_unicode_buffer(length + 1)
    user32.GetWindowTextW(hwnd, buf, length + 1)
    return buf.value

def get_class(hwnd):
    buf = ctypes.create_unicode_buffer(256)
    user32.GetClassNameW(hwnd, buf, 256)
    return buf.value

def callback(hwnd, lparam):
    if not user32.IsWindowVisible(hwnd): return True
    
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    
    # Get relative position if it has a parent
    parent = user32.GetParent(hwnd)
    prect = wintypes.RECT()
    if parent:
        user32.GetWindowRect(parent, ctypes.byref(prect))
        x = rect.left - prect.left
        y = rect.top - prect.top
    else:
        x, y = rect.left, rect.top
        
    w = rect.right - rect.left
    h = rect.bottom - rect.top
    
    text = get_text(hwnd)
    cls = get_class(hwnd)
    cid = user32.GetWindowLongW(hwnd, -12) # GWL_ID
    
    print(f"HWND: {hex(hwnd)} | Class: {cls} | Text: '{text}' | ID: {cid} | Rect: {x}, {y}, {w}, {h}")
    
    # Recurse into children
    user32.EnumChildWindows(hwnd, WNDENUMPROC(callback), 0)
    return True

WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)

# Find the main launcher window. It might have "Clonk Planet" in title.
def find_main():
    def find_cb(hwnd, lparam):
        text = get_text(hwnd)
        if "Clonk" in text and user32.IsWindowVisible(hwnd) and not user32.GetParent(hwnd):
            print(f"\n--- Found Main Window: '{text}' ({hex(hwnd)}) ---")
            callback(hwnd, 0)
        return True
    user32.EnumWindows(WNDENUMPROC(find_cb), 0)

if __name__ == "__main__":
    find_main()
