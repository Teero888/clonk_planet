import os
from c4_io import C4Group

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Worlds.c4f"
path = os.path.join(planet_data_path, f)
grp = C4Group(path)
print(f"File: {f}, Entries: {len(grp.entries)}")
for e in grp.entries:
    print(f"  Entry: {e['name']}, Size: {e['size']}")
