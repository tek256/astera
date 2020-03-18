#!/bin/bash

# NOTE: This script requires the library `zip`

# Create list of files from user arguments
file_list=("${file_list[@]}")

# For each argument check if it's a file or a folder
for file in "$@"
do
	if [ -f "$file" ]; then
		file_list+=("${file}")
	fi
	if [ -d "$file" ]; then
		for subfile in $(find "$file" -type f)
		do
			if [ -f "$subfile" ]; then
				file_list+=("${subfile}")
			fi	
		done
	fi	
done

# Prompt for the compression level
read -rp "Compression Level [0-9] (default 0): " compression_level

# Make sure the compression level is valid
if [[ "$compression_level" -lt 0 ]] || [[ "$compression_level" -gt 9 ]]; then
  compression_level=0
fi

# Prompt for the output file
read -rp "Output File (default output.zip): " output_file

# Make sure the output file variable has something, otherwise go to default
if [ -z "$output_file" ]; then
  output_file="output.zip"
fi

# Zip is all together!
zip "$output_file -$compression_level ${file_list[*]}"
