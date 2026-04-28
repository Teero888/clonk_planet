import os
import struct
import zlib
import gzip

def unscramble(data_raw):
    data = bytearray(data_raw)
    for i in range(0, len(data) - 2, 3):
        data[i], data[i+2] = data[i+2], data[i]
    for i in range(len(data)):
        data[i] ^= 237
    return bytes(data)

def extract_group(group_path, target_dir):
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)
        
    with open(group_path, 'rb') as f:
        data = f.read()

    # Check for Scrambled Gzip
    if data[0:2] == b'\x1e\x8c':
        print(f"Decompressing scrambled GZIP {group_path}...")
        fixed_data = bytearray(data)
        fixed_data[0] ^= 1
        fixed_data[1] ^= 7
        data = gzip.decompress(fixed_data)
    elif data[0:2] == b'\x1f\x8b':
        print(f"Decompressing standard GZIP {group_path}...")
        data = gzip.decompress(data)

    header = unscramble(data[0:204])
    if not header.startswith(b'RedWolf Design GrpFolder'):
        print(f"Invalid group header in {group_path}")
        return

    entries_count = struct.unpack('<I', header[36:40])[0]
    print(f"Extracting {entries_count} entries from {group_path}...")
    
    # Files start after header and all entry cores
    file_base = 204 + entries_count * 316
    
    for i in range(entries_count):
        entry = data[204 + i*316 : 204 + (i+1)*316]
        # In Scrambled Gzip, entries are NOT scrambled
        name = entry[0:260].strip(b'\x00').decode('ascii', errors='ignore')
        size = struct.unpack('<I', entry[268:272])[0]
        # offset is relative to file_base
        rel_offset = struct.unpack('<I', entry[276:280])[0]
        
        if not name: continue
        print(f"  Saving {name} ({size} bytes)...")
        
        file_data = data[file_base + rel_offset : file_base + rel_offset + size]
        with open(os.path.join(target_dir, name), 'wb') as out_f:
            out_f.write(file_data)

if __name__ == "__main__":
    extract_group('ClonkPlanetData/Graphics.c4g', 'frontend/res')
