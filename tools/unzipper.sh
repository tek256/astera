#!/bin/bash
# NOTE: This script requires the library `unzip`

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

# Prompt for the output directory 
read -rp "Where would you like it output to? (Default: Current Directory): " output_directory

# Make sure the output directory exists
if [[ ! -d "$output_directory" ]]; then
  mkdir "$output_directory"
fi

# Unzip all the things!
unzip -d "$output_directory ${file_list[*]}"
