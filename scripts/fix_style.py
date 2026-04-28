#!/usr/bin/env python3

import subprocess
from pathlib import Path

def format_files():
    extensions = ["*.cpp", "*.c", "*.h", "*.glsl"]
    excluded_dirs = [
        Path("standard/inc/external"),
        Path("standard/zlib"),
        Path("standard/unibase"),
        Path("build"),
        Path("original_source")
    ]

    for ext in extensions:
        for file_path in Path(".").rglob(ext):
            if any(excl in file_path.parents for excl in excluded_dirs):
                continue

            print(f"Formatting {file_path}...")
            subprocess.run(["clang-format", "--style=LLVM", "-i", str(file_path)])

if __name__ == "__main__":
    format_files()