import os
import struct
import sys
import glob

def parse_string(data, offset):
    s = ""
    while offset + 1 < len(data):
        ch = struct.unpack_from("<H", data, offset)[0]
        offset += 2
        if ch == 0:
            break
        s += chr(ch) if ch < 128 else '?'
    return s, offset

def parse_dlg(filename, print_items=False):
    with open(filename, 'rb') as f:
        data = f.read()

    offset = 0
    try:
        sig = struct.unpack_from("<H", data, offset)[0]
        is_ex = (sig == 1)

        if is_ex:
            dlgVer, signature, helpID, exStyle, style, cItems, x, y, cx, cy = struct.unpack_from("<HHIIIIhhhh", data, offset)
            offset += 26
        else:
            style, exStyle, cItems, x, y, cx, cy = struct.unpack_from("<IIHhhhh", data, offset)
            offset += 18

        # Menu, Class, Title
        if is_ex:
            m = struct.unpack_from("<H", data, offset)[0]
            if m == 0: offset += 2
            elif m == 0xFFFF: offset += 4
            else: _, offset = parse_string(data, offset)
            c = struct.unpack_from("<H", data, offset)[0]
            if c == 0: offset += 2
            elif c == 0xFFFF: offset += 4
            else: _, offset = parse_string(data, offset)
            title, offset = parse_string(data, offset)
        else:
            m = struct.unpack_from("<H", data, offset)[0]
            if m == 0: offset += 2
            elif m == 0xFFFF: offset += 4
            else: _, offset = parse_string(data, offset)
            c = struct.unpack_from("<H", data, offset)[0]
            if c == 0: offset += 2
            elif c == 0xFFFF: offset += 4
            else: _, offset = parse_string(data, offset)
            title, offset = parse_string(data, offset)
        
        if style & 0x40: # DS_SETFONT
            if is_ex: offset += 6
            else: offset += 2
            _, offset = parse_string(data, offset)

        print(f"--- {os.path.basename(filename)}: Title='{title}' Rect=({x},{y},{cx},{cy}) Items={cItems} ---")

        if print_items:
            for i in range(cItems):
                offset = (offset + 3) & ~3
                if is_ex:
                    helpID, exStyle, itemStyle, ix, iy, icx, icy, id = struct.unpack_from("<IIIIhhhh", data, offset)
                    offset += 24
                else:
                    itemStyle, exStyle, ix, iy, icx, icy, id = struct.unpack_from("<IIhhhhH", data, offset)
                    offset += 18
                
                # Class
                c = struct.unpack_from("<H", data, offset)[0]
                c_str = ""
                if c == 0xFFFF:
                    c_val = struct.unpack_from("<H", data, offset+2)[0]
                    offset += 4
                    classes = {0x0080: "Button", 0x0081: "Edit", 0x0082: "Static", 0x0083: "Listbox", 0x0084: "Scrollbar", 0x0085: "Combobox"}
                    c_str = classes.get(c_val, f"0x{c_val:04x}")
                else:
                    c_str, offset = parse_string(data, offset)
                
                # Title
                t = struct.unpack_from("<H", data, offset)[0]
                t_str = ""
                if t == 0xFFFF:
                    t_val = struct.unpack_from("<H", data, offset+2)[0]
                    offset += 4
                    t_str = f"ResID:{t_val}"
                else:
                    t_str, offset = parse_string(data, offset)
                
                # Extra data
                extra = struct.unpack_from("<H", data, offset)[0]
                offset += 2 + extra
                
                print(f"  Item {id}: {c_str} '{t_str}' @ {ix}, {iy}, {icx}, {icy} (Style:{hex(itemStyle)})")

    except Exception as e:
        print(f"Error parsing {filename}: {e}")

if len(sys.argv) > 1:
    parse_dlg(sys.argv[1], True)
else:
    for f in sorted(glob.glob("/tmp/dlgs_fixed/*")):
        parse_dlg(f)
