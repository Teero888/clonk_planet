import os
import struct
from c4_io import C4Group, unscramble

planet_data_path = "/home/teero/clonk_planet/planet_data"
files = ["Worlds.c4f", "Twonky.c4p"]

for f in files:
    path = os.path.join(planet_data_path, f)
    with open(path, 'rb') as fd:
        raw = fd.read(204)
        header = unscramble(raw)
        v1, v2, entries = struct.unpack('<iii', header[28:40])
        print(f"File: {f}, Ver: {v1}.{v2}, Entries: {entries}")
