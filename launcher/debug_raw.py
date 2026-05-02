import os
import struct
from c4_io import unscramble

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Worlds.c4f"
path = os.path.join(planet_data_path, f)

with open(path, 'rb') as fd:
    raw_header = fd.read(204)
    header = unscramble(raw_header)
    num_entries = struct.unpack('<i', header[36:40])[0]
    print(f"Num entries: {num_entries}")
    
    for i in range(num_entries):
        e_raw = fd.read(316)
        e_un = unscramble(e_raw)
        print(f"Entry {i} raw (first 32): {e_raw[:32].hex(' ')}")
        print(f"Entry {i} unscrambled (first 32): {e_un[:32].hex(' ')}")
        name = e_un[0:260].split(b'\x00')[0].decode('ascii', errors='ignore')
        print(f"  Name: '{name}'")
        if i > 2: break
