import os
from launcher.c4_io import C4Group

def dump_music():
    music_grp_path = 'planet_data/Music.c4g'
    output_dir = 'launcher/res_music'
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    if not os.path.exists(music_grp_path):
        print(f"Error: {music_grp_path} not found.")
        return

    try:
        grp = C4Group(music_grp_path)
        print(f"Extracting {len(grp.entries)} entries from {music_grp_path}...")
        for e in grp.entries:
            name = e['name']
            if name.lower().endswith('.mid'):
                data = grp.get_file(name)
                if data:
                    with open(os.path.join(output_dir, name), 'wb') as f:
                        f.write(data)
                    print(f"  Extracted: {name}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    dump_music()
