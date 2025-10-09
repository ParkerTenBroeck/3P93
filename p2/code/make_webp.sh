cd animation

ffmpeg -framerate 30 -pattern_type glob -i 'frame_*.png' -vf "scale='if(gt(iw/ih,1),min(480,iw),-1)':'if(gt(iw/ih,1),-1,min(480,ih))'"  -c:v libwebp_anim -loop 0 -quality 95 out.webp
