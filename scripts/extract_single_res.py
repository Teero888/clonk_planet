import struct
import os

def extract_res(pe_path, type_name, res_id, lang_id, output_path):
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

    def get_dir(ptr, name):
        num_named = struct.unpack_from("<H", data, ptr + 12)[0]
        num_id = struct.unpack_from("<H", data, ptr + 14)[0]
        for i in range(num_named + num_id):
            entry_ptr = ptr + 16 + i * 8
            id_or_name = struct.unpack_from("<I", data, entry_ptr)[0]
            off = struct.unpack_from("<I", data, entry_ptr + 4)[0]
            if id_or_name & 0x80000000:
                name_off = res_raw_ptr + (id_or_name & 0x7FFFFFFF)
                name_len = struct.unpack_from("<H", data, name_off)[0]
                n = data[name_off+2 : name_off+2+name_len*2].decode('utf-16le')
                if n == name: return res_raw_ptr + (off & 0x7FFFFFFF)
            elif str(id_or_name) == str(name):
                return res_raw_ptr + (off & 0x7FFFFFFF)
        return None

    t_ptr = get_dir(res_raw_ptr, type_name)
    if not t_ptr: return False
    r_ptr = get_dir(t_ptr, res_id)
    if not r_ptr: return False
    l_ptr = get_dir(r_ptr, lang_id)
    if not l_ptr:
        # Just take the first one if lang doesn't match
        l_ptr = r_ptr + 16
        # Data entry is not a dir
        data_vaddr = struct.unpack_from("<I", data, l_ptr)[0]
        data_size = struct.unpack_from("<I", data, l_ptr + 4)[0]
    else:
        # l_ptr is a dir, but for leaf it's a data entry?
        # No, the level 3 is always Data Entry.
        # Wait, get_dir returns the address of the next table.
        # For the last level, it returns the address of the Data Entry.
        # My get_dir is slightly wrong for the last level.
        pass

    # Correcting for the last level
    num_named = struct.unpack_from("<H", data, r_ptr + 12)[0]
    num_id = struct.unpack_from("<H", data, r_ptr + 14)[0]
    entry_ptr = r_ptr + 16 # First entry
    off = struct.unpack_from("<I", data, entry_ptr + 4)[0]
    # Data Entry
    de_ptr = res_raw_ptr + off
    data_vaddr = struct.unpack_from("<I", data, de_ptr)[0]
    data_size = struct.unpack_from("<I", data, de_ptr + 4)[0]

    raw_data_ptr = 0
    for i in range(num_sections):
        sec_off = sections_offset + i * 40
        vaddr = struct.unpack_from("<I", data, sec_off + 12)[0]
        vsize = struct.unpack_from("<I", data, sec_off + 8)[0]
        raw_ptr = struct.unpack_from("<I", data, sec_off + 20)[0]
        if vaddr <= data_vaddr < vaddr + vsize:
            raw_data_ptr = raw_ptr + (data_vaddr - vaddr)
            break
    
    if raw_data_ptr:
        with open(output_path, 'wb') as out:
            out.write(data[raw_data_ptr : raw_data_ptr + data_size])
        return True
    return False

if __name__ == "__main__":
    import sys
    extract_res(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
