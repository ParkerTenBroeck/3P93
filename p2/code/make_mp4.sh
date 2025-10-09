cd animation

ffmpeg -framerate 30 -pattern_type glob -i 'frame_*.png' -vf "scale='if(gt(iw/ih,1),min(480,iw),-1)':'if(gt(iw/ih,1),-1,min(480,ih))'" -vcodec libx264 -crf 23 -preset medium -acodec aac -b:a 192k out.mp4 
