#!/bin/bash

file_list=`find ./`
echo $file_list > file_list.txt

for file in $file_list
do
	#echo "$file"
	if [ -d $file ]; then
	    echo "skip $file"
	else
	    sed -i -e '/BU5D[0-9]\{5\}/d' $file
	    sed -i -e '/BK4D[0-9]\{5\}/d' $file
	    sed -i -e '/DTS[0-9]\{13\}/d' $file
	    #modify for update qualcom 2110 baseline begin
	    #modify for wifi baseline 20091126 begin
	    #sed -i -e 's/lxy: //' $file
	fi
done
