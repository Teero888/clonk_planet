import os
import struct

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Worlds.c4f"
path = os.path.join(planet_data_path, f)
with open(path, 'rb') as fd:
    raw = fd.read(204)
    # NO unscramble
    v1, v2, entries = struct.unpack('<iii', raw[32:44])
    print(f"File: {f}, Ver: {v1}.{v2}, Entries: {entries}")
    print(f"Header ID: {raw[0:25]}")
