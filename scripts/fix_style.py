#!/usr/bin/env python3

import subprocess
import glob
import os

def format_files():
    files_to_format = []
    extensions = ["**/*.cpp", "**/*.c", "**/*.h", "**/*.glsl"]
    excluded_dirs = ["standard/inc/external","standard/inc/miniaudio.h", "build", "original_source"]

    for ext in extensions:
        files_to_format.extend(glob.glob(ext, recursive=True))

    for file in files_to_format:
        path_parts = os.path.normpath(file).split(os.sep)
        if any(excluded_dir in path_parts for excluded_dir in excluded_dirs):
            continue
        print(f"Formatting {file}...")
        subprocess.run(["clang-format", "--style=LLVM", "-i", file])

if __name__ == "__main__":
    format_files()
