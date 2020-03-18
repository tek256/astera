#!/bin/bash

file_list=("${file_list[@]}")

for file in "$@"; 
do
	if [ -d "$file" ]; then
		for subfile in $(find "$file" -type f ! -name "*.ogg")
		do
			if [ -f "$subfile" ]; then
				file_list+=("${subfile}")
			fi	
		done
	fi	
done

for f in "${file_list[@]}"; do
	echo "${f%.*}.ogg"
	ffmpeg -i "$f" -map 0:a -acodec libvorbis -b:a 64k -map_metadata 0 "${f%.*}.ogg"
done
