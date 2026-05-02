import os
import struct
from c4_io import unscramble

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Twonky.c4p"
path = os.path.join(planet_data_path, f)

with open(path, 'rb') as fd:
    raw_header = fd.read(204)
    # Header IS scrambled
    header = unscramble(raw_header)
    num_entries = struct.unpack('<i', header[36:40])[0]
    print(f"Num entries: {num_entries}")
    
    for i in range(num_entries):
        e_raw = fd.read(316)
        # IS THIS SCRAMBLED?
        e_un = unscramble(e_raw)
        name_raw = e_raw[0:20].hex(' ')
        name_un = e_un[0:20].hex(' ')
        print(f"Entry {i} Name Raw: {name_raw}")
        print(f"Entry {i} Name Un: {name_un}")
        
        # Original name
        n_orig = e_raw[0:260].split(b'\x00')[0]
        n_un = e_un[0:260].split(b'\x00')[0]
        print(f"  Name (Raw): {n_orig}")
        print(f"  Name (Unscrambled): {n_un}")
