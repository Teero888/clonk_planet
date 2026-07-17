import re
import os
import codecs

def cpp_escape(s):
    res = []
    for c in s:
        if c == '\n':
            res.append('\\n')
        elif c == '\r':
            res.append('\\r')
        elif c == '\t':
            res.append('\\t')
        elif c == '"':
            res.append('\\"')
        elif c == '\\':
            res.append('\\\\')
        elif ord(c) < 32 or ord(c) > 126:
            res.append(f'\\{ord(c):03o}')
        else:
            res.append(c)
    return ''.join(res)

def generate():
    # 1. Parse resource.h
    define_re = re.compile(r'#define\s+([A-Za-z0-9_]+)\s+(\d+)')
    id_map = {}
    with open('engine/res/resource.h', 'r', encoding='latin-1') as f:
        for line in f:
            m = define_re.search(line)
            if m:
                id_map[m.group(1)] = int(m.group(2))
                
    # Parse engine.rc
    in_stringtable = False
    string_table = {}
    
    # regex for string table entry
    entry_re = re.compile(r'^\s*([A-Za-z0-9_]+)\s+"(.*)"\s*$')
    
    with open('engine/res/engine.rc', 'r', encoding='latin-1') as f:
        for line in f:
            line_strip = line.strip()
            if line_strip.startswith('STRINGTABLE'):
                in_stringtable = True
                continue
            if in_stringtable:
                if line_strip == 'BEGIN':
                    continue
                if line_strip == 'END':
                    in_stringtable = False
                    continue
                m = entry_re.match(line_strip)
                if m:
                    name = m.group(1)
                    val = m.group(2)
                    if name in id_map:
                        try:
                            # Decode unicode escape sequences in raw string (e.g. \r, \n)
                            val = codecs.escape_decode(bytes(val, "utf-8"))[0].decode("utf-8")
                        except Exception:
                            pass
                        string_table[id_map[name]] = val
                    else:
                        print(f"Warning: constant {name} not in resource.h")
                        
    # Generate C++ source
    out_path = 'standard/src/LoadStringFallback.cpp'
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write('// Auto-generated string table fallback for Linux\n')
        f.write('#include <Compat.h>\n')
        f.write('#include <unordered_map>\n')
        f.write('#include <string>\n')
        f.write('#include <cstring>\n\n')
        f.write('static const std::unordered_map<uint32_t, const char*> g_fallback_strings = {\n')
        for i in sorted(string_table.keys()):
            escaped = cpp_escape(string_table[i])
            f.write(f'  {{ {i}, "{escaped}" }},\n')
        f.write('};\n\n')
        
        f.write('int LoadStringFallback(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int nBufferMax) {\n')
        f.write('  auto it = g_fallback_strings.find(uID);\n')
        f.write('  if (it != g_fallback_strings.end()) {\n')
        f.write('    strncpy(lpBuffer, it->second, nBufferMax);\n')
        f.write('    lpBuffer[nBufferMax - 1] = \'\\0\';\n')
        f.write('    return strlen(lpBuffer);\n')
        f.write('  }\n')
        f.write('  if (nBufferMax > 0) lpBuffer[0] = \'\\0\';\n')
        f.write('  return 0;\n')
        f.write('}\n')

if __name__ == "__main__":
    generate()
