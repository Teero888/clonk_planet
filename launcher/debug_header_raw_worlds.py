import os
import struct
from c4_io import unscramble

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Worlds.c4f"
path = os.path.join(planet_data_path, f)

with open(path, 'rb') as fd:
    raw_header = fd.read(204)
    # Header IS scrambled
    header = raw_header # NO unscramble
    v1, v2, entries = struct.unpack('<iii', header[32:44])
    print(f"Header raw: {header[0:20]}")
    print(f"Header raw (ints): {v1}, {v2}, {entries}")
