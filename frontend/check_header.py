import gzip
import struct

def unscramble(data_raw):
    data = bytearray(data_raw)
    for i in range(0, len(data) - 2, 3):
        data[i], data[i+2] = data[i+2], data[i]
    for i in range(len(data)):
        data[i] ^= 237
    return bytes(data)

with open('ClonkPlanetData/Graphics.c4g', 'rb') as f:
    data = f.read()
    fixed_data = bytearray(data)
    fixed_data[0] ^= 1
    fixed_data[1] ^= 7
    decomp = gzip.decompress(fixed_data)
    header = unscramble(decomp[0:204])
    print("Header Hex:", header.hex(' '))
    print("Header ASCII:", header[:50])
    # Try common offsets for Entries
    for off in range(24, 60, 4):
        val = struct.unpack('<I', header[off:off+4])[0]
        print(f"Offset {off}: {val}")
