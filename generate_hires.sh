#!/bin/bash
for path in data/trim/totoro.mp4
do
    
file=${path##*/}
name=${file%%.*}
echo $name
dur=`ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 $path`
rate=`echo "scale=4; (65536-2)/$dur" | bc`
echo $dur $rate
out_path=data/frames/$name
mkdir -p $out_path
out=$out_path/%06d.jpg
ffmpeg -i $path -r $rate -y $out
mkdir -p maps/${name}_hires
python ./generate_hires_map.py $name

done
