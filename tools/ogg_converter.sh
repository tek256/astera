#!/bin/bash
# NOTE: this script isn't strictly posix compliant for use of dynamic arrays
# NOTE: this script also relies on ffmpeg

# Create a list from the arguments passed
file_list=("${file_list[@]}")

# For each file in the argument array
for file in "$@"; 
do
	# If it's a directory, add all the sub files
	if [ -d "$file" ]; then
		for subfile in $(find "$file" -type f ! -name "*.ogg")
		do
			if [ -f "$subfile" ]; then
				file_list+=("${subfile}")
			fi	
		done
	fi	
done

# For each file in the file list 
for f in "${file_list[@]}"; do
	echo "${f%.*}.ogg"
	# Convert to pure OGG stream
	ffmpeg -i "$f" -map 0:a -acodec libvorbis -b:a 64k -map_metadata 0 "${f%.*}.ogg"
done
