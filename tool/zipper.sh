#!/bin/bash

file_list=("${file_list[@]}")

for file in "$@"
do
	if [ -f $file ]; then
		file_list+=("${file}")
	fi
	if [ -d $file ]; then
		for subfile in $(find $file -type f)
		do
			if [ -f $subfile ]; then
				file_list+=("${subfile}")
			fi	
		done
	fi	
done

./zipper ${file_list[@]}
