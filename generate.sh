#!/bin/bash
for path in data/trim/*.mp4
do
    
file=${path##*/}
name=${file%%.*}
echo $name
dur=`ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 $path`
rate=`echo "scale=4; (65536-2)/$dur" | bc`
echo $dur $rate
out=data/rgb/$name.16x16.rgb
ffmpeg -i $path -vf "crop=in_h:in_h,scale=-2:16" -r $rate  -c:v rawvideo -pix_fmt rgb24 -y $out
mkdir -p maps/$name
./hilbert $out maps/$name
python ./generate_map.py maps/$name

done
