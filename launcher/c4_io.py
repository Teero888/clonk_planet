import struct
import gzip
import os
import re
import time

def unscramble(data_raw):
    data = bytearray(data_raw)
    # Inverse of (XOR then SWAP) is (SWAP then XOR)
    for i in range(0, len(data) - 2, 3):
        data[i], data[i+2] = data[i+2], data[i]
    for i in range(len(data)):
        data[i] ^= 237
    return bytes(data)

def scramble(data_raw):
    """The inverse of unscramble: XOR then SWAP."""
    data = bytearray(data_raw)
    # XOR deface
    for i in range(len(data)):
        data[i] ^= 237
    # BYTE swap
    for i in range(0, len(data) - 2, 3):
        data[i], data[i+2] = data[i+2], data[i]
    return bytes(data)

class C4Group:
    def __init__(self, path=None, raw_data=None):
        self.path = path
        self.entries = []
        self.data = b''
        if raw_data is not None:
            self.load_from_data(raw_data)
        elif path is not None:
            self.load()

    def load(self):
        try:
            with open(self.path, 'rb') as f:
                raw_data = f.read()
            self.load_from_data(raw_data)
        except Exception as e:
            print(f"Error loading {self.path}: {e}")

    def load_from_data(self, raw_data):
        try:
            if not raw_data: return

            if raw_data[0:2] in (b'\x1f\x8b', b'\x1e\x8c'):
                if raw_data[0:2] == b'\x1e\x8c':
                    fixed = bytearray(raw_data)
                    fixed[0] ^= 1; fixed[1] ^= 7
                    self.data = gzip.decompress(fixed)
                else:
                    self.data = gzip.decompress(raw_data)
            else:
                self.data = raw_data

            if len(self.data) < 204: return
            header = unscramble(self.data[0:204])
            if not header.startswith(b'RedWolf Design GrpFolder'):
                return

            num_entries = struct.unpack('<i', header[36:40])[0]
            entry_base = 204
            file_base = 204 + num_entries * 316

            self.entries = []
            for i in range(num_entries):
                e_data_raw = self.data[entry_base + i*316 : entry_base + (i+1)*316]
                e_data = unscramble(e_data_raw)
                name = e_data[0:260].split(b'\x00')[0].decode('ascii', errors='ignore')
                packed, child = struct.unpack('<ii', e_data[260:268])
                size, esize, offset = struct.unpack('<iii', e_data[268:280])

                self.entries.append({
                    'name': name,
                    'size': size,
                    'entry_size': esize,
                    'offset': file_base + offset,
                    'packed': bool(packed),
                    'is_group': bool(child)
                })
        except Exception as e:
            print(f"Error parsing group data: {e}")

    def get_file(self, name):
        for e in self.entries:
            if e['name'].lower() == name.lower():
                return self.data[e['offset'] : e['offset'] + e['size']]
        return None

class C4GroupWriter:
    def __init__(self):
        self.entries = []
        self.maker = "Gemini Launcher"

    def add_file(self, name, data, packed=False):
        self.entries.append({
            'name': name,
            'data': data,
            'packed': packed,
            'time': int(time.time())
        })

    def add_from_group(self, source_grp, exclude=None):
        if exclude is None: exclude = []
        for e in source_grp.entries:
            if e['name'].lower() not in [x.lower() for x in exclude]:
                data = source_grp.get_file(e['name'])
                if data:
                    # Keep packed status from source
                    self.add_file(e['name'], data, packed=e['packed'])

    def _make_header(self):
        # Header is 204 bytes
        header = bytearray(204)
        # id (24+1 bytes)
        id_str = b"RedWolf Design GrpFolder\x00"
        header[0:len(id_str)] = id_str
        # Ver1 (1), Ver2 (2) at offset 28 and 32? No, Ver1 at 28, Ver2 at 32, Entries at 36.
        # C4Group.cpp: num_entries = struct.unpack('<i', header[36:40])[0]
        struct.pack_into('<iii', header, 28, 1, 2, len(self.entries))
        # Maker (30+1 bytes) at offset 44
        m_bytes = self.maker.encode('ascii', errors='ignore')[:30]
        header[44:44+len(m_bytes)] = m_bytes
        # Creation, Original at offset 108
        struct.pack_into('<ii', header, 108, int(time.time()), 1)
        
        return scramble(header)

    def _make_entry_core(self, entry, offset):
        # Entry core is 316 bytes
        core = bytearray(316)
        # FileName (260 bytes)
        name_bytes = entry['name'].encode('ascii', errors='ignore')[:259]
        core[0:len(name_bytes)] = name_bytes
        
        entry_size = len(entry['data'])
        data = entry['data']
        packed = 1 if entry['packed'] else 0
        
        size = len(data)
        # In C4Group, ChildGroup is true if the entry is another group
        child_group = 1 if entry['name'].lower().endswith(('.c4p', '.c4f', '.c4s', '.c4d', '.c4v')) else 0
        
        # struct C4GroupEntryCore: FileName[261], Packed, ChildGroup, Size, EntrySize, Offset, Time, fbuf[30]
        # BUT header reading suggests different offsets. 
        # header[36:40] is num_entries. 
        # C4Group.cpp: name = e_data[0:260], packed/child = struct.unpack('<ii', e_data[260:268])
        # size, esize, offset = struct.unpack('<iii', e_data[268:280])
        
        struct.pack_into('<ii', core, 260, packed, child_group)
        struct.pack_into('<iii', core, 268, size, entry_size, offset)
        # Time is usually after offset (offset 280)
        struct.pack_into('<i', core, 280, entry['time'])
        
        return scramble(core), data

    def write_to_file(self, path):
        # 1. Prepare data and entry cores
        file_base = 204 + len(self.entries) * 316
        
        entry_cores = []
        total_data = []
        current_offset = 0 # Relative to file_base
        
        for entry in self.entries:
            core, data = self._make_entry_core(entry, current_offset)
            entry_cores.append(core)
            total_data.append(data)
            current_offset += len(data)

        # 2. Write to disk
        with open(path, 'wb') as f:
            # Header
            header = self._make_header()
            f.write(header)
            # Index
            for core in entry_cores:
                f.write(core)
            # Data blocks
            for data in total_data:
                f.write(data)

def parse_c4_text(content):
    """Parses C4-style .txt files with sections like [SectionName]"""
    sections = {}
    current_section = None
    for line in content.splitlines():
        line = line.strip()
        if not line or line.startswith(';'): continue
        if line.startswith('[') and line.endswith(']'):
            current_section = line[1:-1]
            sections[current_section] = {}
            continue
        if current_section and '=' in line:
            key, val = line.split('=', 1)
            sections[current_section][key.strip()] = val.strip()
    return sections

def update_c4_text(content, section_name, new_values):
    """Updates specific keys in a C4-style .txt file, preserving other content."""
    lines = content.splitlines()
    new_lines = []
    in_section = False
    found_keys = set()
    
    # Normalize keys to lower for matching but preserve case for writing
    new_values_norm = {k.lower(): (k, v) for k, v in new_values.items()}

    for line in lines:
        s_line = line.strip()
        if s_line == f'[{section_name}]': 
            in_section = True
            new_lines.append(line)
            continue
        if s_line.startswith('[') and s_line.endswith(']'):
            if in_section: # Close out target section if we missed keys
                for k_lower, (k_orig, v) in new_values_norm.items():
                    if k_lower not in found_keys:
                        new_lines.append(f"{k_orig}={v}")
            in_section = False
            new_lines.append(line)
            continue
        
        if in_section and '=' in s_line:
            key, val = s_line.split('=', 1)
            key_clean = key.strip()
            key_lower = key_clean.lower()
            if key_lower in new_values_norm:
                orig_key, new_val = new_values_norm[key_lower]
                new_lines.append(f"{orig_key}={new_val}")
                found_keys.add(key_lower)
            else:
                new_lines.append(line)
        else:
            new_lines.append(line)
    
    # If we were in target section at end of file
    if in_section:
        for k_lower, (k_orig, v) in new_values_norm.items():
            if k_lower not in found_keys:
                new_lines.append(f"{k_orig}={v}")
                
    # If section was never found, add it
    if not any(l.strip() == f'[{section_name}]' for l in lines):
        if new_lines and new_lines[-1].strip():
            new_lines.append("")
        new_lines.append(f"[{section_name}]")
        for k_orig, v in new_values.items():
            new_lines.append(f"{k_orig}={v}")

    return '\r\n'.join(new_lines) + '\r\n'

def get_group_title(grp, language="US"):
    title = "Unknown"
    title_data = grp.get_file('Title.txt') or grp.get_file('Names.txt')
    if title_data:
        lines = title_data.decode('latin-1', errors='ignore').splitlines()
        for l in lines:
            if l.startswith(language + ':'): title = l[3:]; break
            elif ':' in l and not any(l.startswith(p+':') for p in ['DE', 'US']): title = l.split(':', 1)[1]
            elif l and not ':' in l: title = l
    if '.' in title: title = title.rsplit('.', 1)[0]
    return title
