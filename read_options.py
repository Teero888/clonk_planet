import struct, os

def get_string(data, offset):
    s = ""
    while offset + 1 < len(data):
        ch = struct.unpack_from("<H", data, offset)[0]
        offset += 2
        if ch == 0: break
        s += chr(ch) if ch < 128 else '?'
    return s, offset

def parse_from_data(data, name):
    try:
        style, ex, count, x, y, w, h = struct.unpack_from("<IIHhhhh", data, 0)
        print(f"\n=== Resource {name} ({count} items) ===")
        print(f"Base Dimensions: {x}, {y}, {w}, {h} DLUs")
        
        offset = 18
        for _ in range(3):
            val = struct.unpack_from("<H", data, offset)[0]
            if val == 0xFFFF: offset += 4
            else: _, offset = get_string(data, offset)

        if style & 0x40: # DS_SETFONT
            offset += 2 # point size
            _, offset = get_string(data, offset)

        for i in range(count):
            offset = (offset + 3) & ~3
            it = struct.unpack_from("<IIhhhhH", data, offset)
            offset += 18
            c = struct.unpack_from("<H", data, offset)[0]
            if c == 0xFFFF:
                c_type = {0x80:"Btn", 0x81:"Edit", 0x82:"Static", 0x83:"LBox", 0x84:"SBar", 0x85:"CBox"}.get(struct.unpack_from("<H", data, offset+2)[0], "Unk")
                offset += 4
            else: c_type, offset = get_string(data, offset)
            t = struct.unpack_from("<H", data, offset)[0]
            if t == 0xFFFF: t_str = f"ResID:{struct.unpack_from('<H', data, offset+2)[0]}"; offset += 4
            else: t_str, offset = get_string(data, offset)
            extra = struct.unpack_from("<H", data, offset)[0]
            offset += 2 + extra
            print(f"  ID {it[6]:4} | {c_type:6} | '{t_str:20}' | Rect: {it[2]:3}, {it[3]:3}, {it[4]:3}, {it[5]:3}")
    except Exception as e:
        print(f"Error parsing {name}: {e}")

for i in range(3011, 3023):
    path = f"/tmp/dlgs_fixed/Planet_fixed.bin_5_{i}_1031"
    if os.path.exists(path):
        with open(path, 'rb') as f:
            parse_from_data(f.read(), i)
