import gzip

with open('ClonkPlanetData/Graphics.c4g', 'rb') as f:
    data = f.read()
    fixed_data = bytearray(data)
    fixed_data[0] ^= 1
    fixed_data[1] ^= 7
    decomp = gzip.decompress(fixed_data)
    pos = decomp.find(b'BM', 204)
    print("Found 'BM' at:", pos)
    print("Data at pos:", decomp[pos:pos+20].hex(' '))
