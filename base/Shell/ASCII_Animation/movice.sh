#!/bin/bash

###
# Argument list 
# 

ROOT=`pwd`
COLOR=0
INPUT_FILE=
OUTPUT_DIR=
FREQUEN=10
WIDTH=280
HEIGH=180
NAME=ASCII
TIME=1
START=00:00:00

help_func() {
	echo "Welcome to Image to ASCII"
	echo "./movice.sh [-rwh] <-i INPUT_FILE> <-o OUTPUT_DIR>" 
	echo ""
	echo "  -i <...>   : Input image file path"
	echo "  -o <...>   : Output directory"
	echo "  -r <...>   : Movie frequency"
	echo "  -w <...>   : Movie width"
	echo "  -h <...>   : Movie heigh"
	echo "  -t <...>   : Movie time"
	echo "  -s <...>   : Movie start time"
	echo ""
	echo "e.g."
	echo "./movice.sh -i temp.mp4 -o temp"
}

while getopts ":i:o:t:s:r:w:h:" opt
do
	case $opt in
		o)
			OUTPUT_DIR=$OPTARG	
		;;
		i)
			INPUT_FILE=$OPTARG
		;;
		r)
			FREQUEN=$OPTARG
		;;
		w)
			WIDTH=$OPTARG
		;;
		h)
			HEIGH=$OPTARG
		;;
		t)
			TIME=$OPTARG
		;;
		s)
			START=$OPTARG
		;;
		?)
			help_func
			exit 1;;
	esac
done

if [ ! -f /usr/bin/jp2a ]; then
	sudo apt-get install -y jp2a ffmpeg
fi

## check argument
if [ -z $INPUT_FILE ]; then
	echo "ERROR: need special input file.."
	echo ""
	help_func
	exit 1
else
	if [ ! -f $INPUT_FILE ]; then
		echo "Input file doesn't exist!"
		exit 1
	fi
fi
if [ -z $OUTPUT_DIR ]; then
	echo "ERROR: need special output dir"
        echo ""
        help_func
        exit 1
else
	if [ -d $ROOT/$OUTPUT_DIR ]; then
		rm -rf $ROOT/$OUTPUT_DIR 
	fi
	mkdir -p $ROOT/$OUTPUT_DIR
fi

# Capture image from movie
if [ -d ${ROOT}/.tmp ]; then
	rm -rf ${ROOT}/.tmp
fi
mkdir -p ${ROOT}/.tmp

ffmpeg -i $INPUT_FILE -f image2 -r $FREQUEN -ss $START -t $TIME -s ${WIDTH}X${HEIGH} ${ROOT}/.tmp/${NAME}1%5d.jpeg

# Cover image to ASCII
COUNT_IMAGE=`ls -l ${ROOT}/.tmp | grep "^-"|wc -l`
INDEX=0

while [ $INDEX -lt ${COUNT_IMAGE} ]
do
	INDEX=`expr ${INDEX} + 1`
	SUB_NAME=`expr 100000 + $INDEX`
	filename=${ROOT}/.tmp/${NAME}${SUB_NAME}.jpeg
	outfile=$ROOT/$OUTPUT_DIR/${NAME}${SUB_NAME}.batB
	jp2a $filename > $outfile
done

INDEX=0
while true
do
	if [ $INDEX -ge ${COUNT_IMAGE} ]; then
		INDEX=0
	fi
	INDEX=`expr ${INDEX} + 1`
	SUB_NAME=`expr 100000 + $INDEX`
	filename=$ROOT/$OUTPUT_DIR/${NAME}${SUB_NAME}.batB
	cat $filename
	sleep 0.06
done















