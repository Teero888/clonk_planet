#!/usr/bin/env python3
import sys
import os
import subprocess
from PIL import Image, ImageChops

def run_cmd(args):
    print(f"Running: {' '.join(args)}")
    res = subprocess.run(args, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error running command: {res.stderr}")
        return False
    return True

def create_diff_image(img1, img2):
    diff = ImageChops.difference(img1, img2)
    diff_data = diff.load()
    w, h = diff.size
    for x in range(w):
        for y in range(h):
            p = diff_data[x, y]
            diff_data[x, y] = tuple(min(255, c * 5) for c in p)
    return diff

def main():
    os.makedirs("build", exist_ok=True)
    
    # 1. Build launcher
    print("Rebuilding clonk launcher...")
    if not run_cmd(["cmake", "--build", "build", "-j4"]):
        sys.exit(1)
        
    # 2. Run Main Page screenshot
    print("Capturing Main Page screenshot...")
    main_screenshot_path = "build/temp_main.png"
    if os.path.exists(main_screenshot_path):
        os.remove(main_screenshot_path)
    run_cmd(["./build/clonk_launcher", "--screenshot-main", main_screenshot_path])
    
    # 3. Run Options Page screenshot
    print("Capturing Options Page screenshot...")
    options_screenshot_path = "build/temp_options.png"
    if os.path.exists(options_screenshot_path):
        os.remove(options_screenshot_path)
    run_cmd(["./build/clonk_launcher", "--screenshot-options", options_screenshot_path])
    
    if not os.path.exists(main_screenshot_path) or not os.path.exists(options_screenshot_path):
        print("Failed to capture screenshots. Make sure the binary runs successfully.")
        sys.exit(1)
        
    # 4. Compare Main Page
    ref_main_full = Image.open("todo/winxp/MainPage.png")
    ref_main = ref_main_full.crop((10, 54, 10 + 578, 55 + 392))
    our_main = Image.open(main_screenshot_path)
    
    ref_main = ref_main.convert("RGB")
    our_main = our_main.convert("RGB")
    
    w_main = min(ref_main.size[0], our_main.size[0])
    h_main = min(ref_main.size[1], our_main.size[1])
    ref_main = ref_main.crop((0, 0, w_main, h_main))
    our_main = our_main.crop((0, 0, w_main, h_main))
    
    diff_main = create_diff_image(ref_main, our_main)
    
    # Create side-by-side main page
    comparison_main = Image.new("RGB", (w_main * 3, h_main))
    comparison_main.paste(ref_main, (0, 0))
    comparison_main.paste(our_main, (w_main, 0))
    comparison_main.paste(diff_main, (w_main * 2, 0))
    comparison_main.save("build/comparison_main.png")
    
    # 5. Compare Options Page
    ref_opt_full = Image.open("todo/winxp/OptionsProgram.png")
    ref_opt = ref_opt_full.crop((15, 40, 15 + 380, 40 + 410))
    our_opt = Image.open(options_screenshot_path)
    
    ref_opt = ref_opt.convert("RGB")
    our_opt = our_opt.convert("RGB")
    
    w_opt = min(ref_opt.size[0], our_opt.size[0])
    h_opt = min(ref_opt.size[1], our_opt.size[1])
    ref_opt = ref_opt.crop((0, 0, w_opt, h_opt))
    our_opt = our_opt.crop((0, 0, w_opt, h_opt))
    
    diff_opt = create_diff_image(ref_opt, our_opt)
    
    # Create side-by-side options page
    comparison_opt = Image.new("RGB", (w_opt * 3, h_opt))
    comparison_opt.paste(ref_opt, (0, 0))
    comparison_opt.paste(our_opt, (w_opt, 0))
    comparison_opt.paste(diff_opt, (w_opt * 2, 0))
    comparison_opt.save("build/comparison_options.png")
    
    print("Visual comparison completed successfully!")
    print("Main comparison saved to: build/comparison_main.png")
    print("Options comparison saved to: build/comparison_options.png")

if __name__ == "__main__":
    main()
