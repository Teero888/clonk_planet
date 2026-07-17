#!/usr/bin/env bash

# Make sure ffmpeg is installed
if ! command -v ffmpeg &> /dev/null; then
    echo "Error: ffmpeg is required but not installed. Run: sudo pacman -S ffmpeg"
    exit 1
fi

echo "Processing WAV files in place..."

for file in *.wav; do
    # Skip if no wav files found
    [ -e "$file" ] || continue

    echo "Fixing and cleaning: $file"
    
    # Create a unique temporary filename
    tmp_file=$(mktemp --suffix=.wav)

    # 1. Force 16-bit PCM 44.1kHz
    # 2. Fade in 5ms -> Reverse -> Fade in 5ms -> Reverse back
    ffmpeg -y -hide_banner -loglevel error -i "$file" \
        -acodec pcm_s16le -ar 44100 \
        -af "afade=t=in:st=0:d=0.005, areverse, afade=t=in:st=0:d=0.005, areverse" \
        "$tmp_file"

    # Check if ffmpeg succeeded before replacing the original file
    if [ $? -eq 0 ]; then
        mv "$tmp_file" "$file"
    else
        echo "Warning: Failed to process $file, leaving original untouched."
        rm -f "$tmp_file"
    fi
done

echo "done"