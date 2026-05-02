import os
import struct
from c4_io import C4Group, unscramble

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Worlds.c4f"
path = os.path.join(planet_data_path, f)

grp = C4Group(path)
print(f"File: {f}, Entries: {len(grp.entries)}")

# Try reading raw entry data from C4Group.data
entry_base = 204
num_entries = len(grp.entries)

for i in range(num_entries):
    e_data = grp.data[entry_base + i*316 : entry_base + (i+1)*316]
    # Try WITHOUT unscramble
    name = e_data[0:260].split(b'\x00')[0].decode('ascii', errors='ignore')
    print(f"Entry {i} (NO unscramble): Name='{name}'")
    if i > 2: break
