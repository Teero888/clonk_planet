import struct

with open('Planet_dump.bin', 'rb') as f:
    data = bytearray(f.read())

pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
num_sections = struct.unpack_from("<H", data, pe_offset + 6)[0]
opt_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
sections_offset = pe_offset + 24 + opt_header_size

for i in range(num_sections):
    sec_off = sections_offset + i * 40
    name = data[sec_off:sec_off+8].strip(b'\x00').decode('ascii', errors='ignore')
    vsize, vaddr, raw_size, raw_ptr = struct.unpack_from("<IIII", data, sec_off + 8)
    print(f"Fixing section {name}: VAddr {hex(vaddr)} VSize {hex(vsize)}")
    struct.pack_into("<II", data, sec_off + 16, vsize, vaddr)

with open('Planet_fixed.bin', 'wb') as f:
    f.write(data)
