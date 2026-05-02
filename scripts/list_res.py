import struct
import os

def list_resources(pe_path):
    with open(pe_path, 'rb') as f:
        data = f.read()

    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    num_sections = struct.unpack_from("<H", data, pe_offset + 6)[0]
    opt_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    
    magic = struct.unpack_from("<H", data, pe_offset + 24)[0]
    if magic == 0x10b: # PE32
        res_dir_offset = pe_offset + 24 + 96 + 16
    else: # PE32+
        res_dir_offset = pe_offset + 24 + 112 + 16

    res_vaddr = struct.unpack_from("<I", data, res_dir_offset)[0]
    if res_vaddr == 0: return

    sections_offset = pe_offset + 24 + opt_header_size
    res_raw_ptr = 0
    for i in range(num_sections):
        sec_off = sections_offset + i * 40
        vaddr = struct.unpack_from("<I", data, sec_off + 12)[0]
        vsize = struct.unpack_from("<I", data, sec_off + 8)[0]
        raw_ptr = struct.unpack_from("<I", data, sec_off + 20)[0]
        if vaddr <= res_vaddr < vaddr + vsize:
            res_raw_ptr = raw_ptr + (res_vaddr - vaddr)
            break

    if res_raw_ptr == 0: return

    def parse_dir(dir_raw_ptr, path=""):
        num_named = struct.unpack_from("<H", data, dir_raw_ptr + 12)[0]
        num_id = struct.unpack_from("<H", data, dir_raw_ptr + 14)[0]
        
        for i in range(num_named + num_id):
            entry_ptr = dir_raw_ptr + 16 + i * 8
            id_or_name = struct.unpack_from("<I", data, entry_ptr)[0]
            offset_or_data = struct.unpack_from("<I", data, entry_ptr + 4)[0]
            
            is_dir = bool(offset_or_data & 0x80000000)
            offset = offset_or_data & 0x7FFFFFFF
            
            if id_or_name & 0x80000000:
                name_off = res_raw_ptr + (id_or_name & 0x7FFFFFFF)
                name_len = struct.unpack_from("<H", data, name_off)[0]
                entry_name = data[name_off+2 : name_off+2+name_len*2].decode('utf-16le')
            else:
                entry_name = str(id_or_name)

            new_path = f"{path}/{entry_name}" if path else entry_name
            
            if is_dir:
                parse_dir(res_raw_ptr + offset, new_path)
            else:
                data_vaddr = struct.unpack_from("<I", data, res_raw_ptr + offset)[0]
                data_size = struct.unpack_from("<I", data, res_raw_ptr + offset + 4)[0]
                print(f"RES: {new_path} | Size: {data_size}")

    parse_dir(res_raw_ptr)

if __name__ == "__main__":
    list_resources('dumps/Planet_fixed.bin')
