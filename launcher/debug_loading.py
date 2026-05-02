import os
from c4_io import C4Group

planet_data_path = "/home/teero/clonk_planet/planet_data"
files = [f for f in os.listdir(planet_data_path) if f.lower().endswith(('.c4f', '.c4s', '.c4p', '.c4d', '.c4v'))]
print(f"Found {len(files)} potential group files.")

for f in files:
    path = os.path.join(planet_data_path, f)
    grp = C4Group(path)
    print(f"File: {f}, Entries: {len(grp.entries)}")
    if len(grp.entries) == 0:
        # Debug why it's empty
        with open(path, 'rb') as fd:
            raw = fd.read(204)
            from c4_io import unscramble
            header = unscramble(raw)
            print(f"  Header ID: {header[0:25]}")
            print(f"  Header Startswith ID: {header.startswith(b'RedWolf Design GrpFolder')}")
            import struct
            num_entries = struct.unpack('<i', header[36:40])[0]
            print(f"  Num entries in header: {num_entries}")
