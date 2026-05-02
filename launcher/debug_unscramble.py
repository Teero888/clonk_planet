import os
from c4_io import C4Group

planet_data_path = "/home/teero/clonk_planet/planet_data"
f = "Twonky.c4p"
path = os.path.join(planet_data_path, f)
grp = C4Group(path)
print(f"File: {f}, Entries: {len(grp.entries)}")
for e in grp.entries:
    print(f"  Entry: {e['name']}, Size: {e['size']}")
    if e['name'] == 'Player.txt':
        data = grp.get_file(e['name'])
        print(f"    Content (first 50): {data[:50]}")
