#!/bin/bash

# First picture.
index=0
# Total number of animation
total=299

# Original Image Size: 
#    Width:  280 Pixels
#    Height: 180 Pixels

# Display Image Size:
#    Width:  150
#    Height: 49
resize -s 49 150

while true
do
	filename=one${index}.datc
	if [ ${index} -gt ${total} ]; then
		index=0
	fi
	#sleep 0.032    # 10s
	sleep 0.06      # 20s
	cat ${filename}
	let "index+=1"
done
