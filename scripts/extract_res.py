import struct
import os

def extract_resources(pe_path, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    with open(pe_path, 'rb') as f:
        data = f.read()

    # PE Header
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    num_sections = struct.unpack_from("<H", data, pe_offset + 6)[0]
    opt_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    
    # Optional Header
    magic = struct.unpack_from("<H", data, pe_offset + 24)[0]
    if magic == 0x10b: # PE32
        res_dir_offset = pe_offset + 24 + 96 + 16 # DataDirectory[2] is Resource Table
    else: # PE32+
        res_dir_offset = pe_offset + 24 + 112 + 16

    res_vaddr = struct.unpack_from("<I", data, res_dir_offset)[0]
    res_vsize = struct.unpack_from("<I", data, res_dir_offset + 4)[0]

    if res_vaddr == 0:
        print("No resource directory found.")
        return

    # Find section containing the resource table
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

    if res_raw_ptr == 0:
        print("Resource directory not found in any section.")
        return

    def parse_dir(dir_raw_ptr, level=0, path=""):
        # Resource Directory Table
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

            new_path = f"{path}_{entry_name}" if path else entry_name
            
            if is_dir:
                parse_dir(res_raw_ptr + offset, level + 1, new_path)
            else:
                # Resource Data Entry
                data_vaddr = struct.unpack_from("<I", data, res_raw_ptr + offset)[0]
                data_size = struct.unpack_from("<I", data, res_raw_ptr + offset + 4)[0]
                
                # Find raw pointer for data_vaddr
                raw_data_ptr = 0
                for j in range(num_sections):
                    sec_off = sections_offset + j * 40
                    vaddr = struct.unpack_from("<I", data, sec_off + 12)[0]
                    vsize = struct.unpack_from("<I", data, sec_off + 8)[0]
                    raw_ptr = struct.unpack_from("<I", data, sec_off + 20)[0]
                    if vaddr <= data_vaddr < vaddr + vsize:
                        raw_data_ptr = raw_ptr + (data_vaddr - vaddr)
                        break
                
                if raw_data_ptr:
                    res_data = data[raw_data_ptr : raw_data_ptr + data_size]
                    if not res_data: continue
                    res_type = path.split('_')[0]
                    
                    filename = f"Planet_fixed.bin_{new_path}"
                    
                    # If it's a BITMAP (Type 2), add BITMAPFILEHEADER
                    if res_type == "2":
                        # BITMAP resource is just BITMAPINFOHEADER + data
                        # We need to prepend a 14-byte BITMAPFILEHEADER
                        file_header = b'BM' + struct.pack("<IHHIB", 14 + data_size, 0, 0, 14 + struct.unpack_from("<I", res_data, 0)[0], 0)
                        # Wait, the offset to bits is more complex if there's a palette
                        # Actually, for Type 2 resources, the data IS the BITMAPINFO and bits.
                        # The BITMAPFILEHEADER.bfOffBits is usually 14 + header_size + palette_size.
                        # header_size is in res_data[0:4]
                        hdr_size = struct.unpack_from("<I", res_data, 0)[0]
                        num_colors = struct.unpack_from("<I", res_data, 32)[0]
                        bit_count = struct.unpack_from("<H", res_data, 14)[0]
                        
                        palette_size = 0
                        if bit_count <= 8:
                            if num_colors == 0:
                                palette_size = (1 << bit_count) * 4
                            else:
                                palette_size = num_colors * 4
                        
                        off_bits = 14 + hdr_size + palette_size
                        file_header = b'BM' + struct.pack("<IHH I", 14 + data_size, 0, 0, off_bits)
                        
                        with open(os.path.join(output_dir, filename + ".bmp"), 'wb') as out:
                            out.write(file_header)
                            out.write(res_data)
                    else:
                        with open(os.path.join(output_dir, filename), 'wb') as out:
                            out.write(res_data)

    parse_dir(res_raw_ptr)

if __name__ == "__main__":
    extract_resources('dumps/Planet_dump_large.bin', 'launcher/res_dump')
