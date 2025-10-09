cd animation

ffmpeg -framerate 30 -pattern_type glob -i 'frame_*.png' -c:v libwebp_anim -loop 0 -quality 95 out.webp
