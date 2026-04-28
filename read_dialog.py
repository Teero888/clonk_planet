import struct, sys

def get_string(data, offset):
    s = ""
    while offset < len(data):
        ch = struct.unpack_from("<H", data, offset)[0]
        offset += 2
        if ch == 0: break
        s += chr(ch)
    return s, offset

def read_dialog(filename):
    with open(filename, 'rb') as f: data = f.read()
    # Header: style(4), exStyle(4), items(2), x(2), y(2), cx(2), cy(2)
    h = struct.unpack_from("<IIHhhhh", data, 0)
    offset = 18
    # Skip Menu, Class, Title
    for _ in range(3):
        val = struct.unpack_from("<H", data, offset)[0]
        if val == 0xFFFF: offset += 4
        else: _, offset = get_string(data, offset)
        if val == 0: offset += 0 # Already handled by loop
    
    print(f"Dialog Size: {h[5]}x{h[6]} DLUs ({h[2]} items)")
    
    for i in range(h[2]):
        offset = (offset + 3) & ~3 # Align to DWORD
        # Item: style(4), exstyle(4), x(2), y(2), cx(2), cy(2), id(2)
        it = struct.unpack_from("<IIhhhhH", data, offset)
        offset += 18
        # Read Class
        c = struct.unpack_from("<H", data, offset)[0]
        if c == 0xFFFF:
            c_type = {0x80:"Btn", 0x81:"Edit", 0x82:"Static", 0x83:"LBox", 0x84:"SBar", 0x85:"CBox"}.get(struct.unpack_from("<H", data, offset+2)[0], "Unk")
            offset += 4
        else: c_type, offset = get_string(data, offset)
        # Read Text
        t = struct.unpack_from("<H", data, offset)[0]
        if t == 0xFFFF: t_str = f"Res:{struct.unpack_from('<H', data, offset+2)[0]}"; offset += 4
        else: t_str, offset = get_string(data, offset)
        # Skip extra bytes
        offset += 2 + struct.unpack_from("<H", data, offset)[0]
        print(f"ID {it[6]:4}: {c_type:6} '{t_str:20}' @ {it[2]:3}, {it[3]:3}, {it[4]:3}, {it[5]:3}")

