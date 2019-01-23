#!/bin/bash

###
# Argument list 
# 

DOWNLOAD_DIR=~/image2ascii
INSTALL_DIR=/usr/local/share/
TOOLS=${INSTALL_DIR}/image2ascii/bin/image2ascii

while getopts ":x" opt
do
	case $opt in
		x)
			PreInstall=1	
		;;
		?)
			echo "??????"
			exit 1;;
	esac
done

if [ ! -d ${INSTALL_DIR}/image2ascii ]; then
	echo "Download Image2ASCII tools...."
	sudo apt-get install -y golang-go > /dev/null
	sudo apt-get install -y ffmpeg > /dev/null
	mkdir -p $DOWNLOAD_DIR
	echo -e "#!/bin/bash\n export GOPATH=$DOWNLOAD_DIR" > ./1.sh
	source ./1.sh
	rm ./1.sh
	go get github.com/qeesung/image2ascii
	sudo mkdir -p $INSTALL_DIR
	sudo cp -rf $DOWNLOAD_DIR ${INSTALL_DIR}/
	rm -rf $DOWNLOAD_DIR
	sudo ln -s ${TOOLS} /usr/bin/
	echo "Install finish....."
fi

image2ascii
