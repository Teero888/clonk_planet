import os
import struct

with open('Planet_fixed.bin', 'rb') as f:
    data = f.read()

os.makedirs('frontend/res_wav_fixed', exist_ok=True)

# Locations from wrestool: 
# 7000: 0x23ed30, 4748
# 7008: 0x23ffc0, 193144
offsets = [
    (0x23ed30, 4748, "7000"),
    (0x23d7c8, 5476, "7001"),
    (0x239e10, 970, "7002"),
    (0x23a1e0, 776, "7003"),
    (0x23a4e8, 776, "7004"),
    (0x23c928, 3738, "7005"),
    (0x23ac78, 5902, "7006"),
    (0x23c388, 1436, "7007"),
    (0x23ffc0, 193144, "7008"),
    (0x23a7f0, 1157, "7009")
]

for off, size, name in offsets:
    # WAV header: RIFF, size, WAVE, fmt , fmt_size(16), tag(1), channels(1), freq(22050), ...
    header = b'RIFF' + struct.pack("<I", size + 36) + b'WAVEfmt '
    header += struct.pack("<IHHIIHH", 16, 1, 1, 22050, 22050, 1, 8) # PCM, 1ch, 22kHz, 8bit
    header += b'data' + struct.pack("<I", size)
    
    with open(f'frontend/res_wav_fixed/sound_{name}.wav', 'wb') as out:
        out.write(header)
        out.write(data[off : off + size])
