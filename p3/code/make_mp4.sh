cd animation

ffmpeg -framerate 30 -pattern_type glob -i 'frame_*.png' -vcodec libx264 -crf 23 -preset medium -acodec aac -b:a 192k out.mp4
