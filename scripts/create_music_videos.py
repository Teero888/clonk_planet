import os
import subprocess
import sys

RENDER_DIR = 'launcher/res_music/render'

def upscale_cover(dir_path, bmp_name):
    base_name = os.path.splitext(bmp_name)[0]
    png_name = f"{base_name}.png"
    bmp_path = os.path.join(dir_path, bmp_name)
    png_path = os.path.join(dir_path, png_name)
    
    print(f"Upscaling cover losslessly (nearest-neighbor): {bmp_path} -> {png_path}...")
    
    # Scale to 1440x1080 (standard 4:3 1080p) using neighbor scaling for crisp pixel art
    cmd = [
        'ffmpeg', '-y',
        '-i', bmp_path,
        '-vf', 'scale=1440:1080:flags=neighbor',
        '-frames:v', '1',
        png_path
    ]
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        print(f"  --> Cover upscaled successfully.")
        # Keep the old BMP file
        return png_name
    else:
        print(f"  --> Error upscaling cover: {result.stderr.decode('utf-8', errors='ignore')}")
        return None

def create_videos():
    if not os.path.exists(RENDER_DIR):
        print(f"Error: {RENDER_DIR} not found.")
        sys.exit(1)

    for subdir in ['hazard', 'knights', 'main']:
        dir_path = os.path.join(RENDER_DIR, subdir)
        if not os.path.isdir(dir_path):
            continue

        # 1. Convert and upscale .bmp to .png if present
        bmp_files = [f for f in os.listdir(dir_path) if f.lower().endswith('.bmp')]
        if bmp_files:
            upscale_cover(dir_path, bmp_files[0])

        # 2. Find the .png cover
        png_files = [f for f in os.listdir(dir_path) if f.lower().endswith('.png')]
        if not png_files:
            print(f"Skipping {subdir}: No cover found.")
            continue

        cover_png = os.path.join(dir_path, png_files[0])
        print(f"Using upscaled cover {cover_png} for directory {subdir}")

        # 3. Find all .mp3 files
        mp3_files = [f for f in os.listdir(dir_path) if f.lower().endswith('.mp3')]
        for mp3 in mp3_files:
            mp3_path = os.path.join(dir_path, mp3)
            base_name = os.path.splitext(mp3)[0]
            mp4_path = os.path.join(dir_path, f"{base_name}.mp4")

            print(f"Creating video: {mp4_path}...")
            
            # Run ffmpeg with stillimage tuning for optimal speed and size
            cmd = [
                'ffmpeg', '-y',
                '-loop', '1', '-i', cover_png,
                '-i', mp3_path,
                '-c:v', 'libx264', '-tune', 'stillimage',
                '-c:a', 'aac', '-b:a', '192k',
                '-pix_fmt', 'yuv420p',
                '-shortest',
                mp4_path
            ]
            
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if result.returncode == 0:
                print(f"  --> Successfully created: {mp4_path}")
            else:
                print(f"  --> Error creating video for {mp3}: {result.stderr.decode('utf-8', errors='ignore')}")

if __name__ == "__main__":
    create_videos()
