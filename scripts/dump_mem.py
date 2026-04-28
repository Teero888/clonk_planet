import sys

pid = sys.argv[1]
mem_path = f"/proc/{pid}/mem"

with open(mem_path, 'rb') as f:
    f.seek(0x00400000)
    data = f.read(0x00200000)

with open('Planet_dump.bin', 'wb') as f:
    f.write(data)
print(f"Dumped {len(data)} bytes")
