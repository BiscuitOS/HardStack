#!/bin/bash

#
# Text attribute 

printf $(tput setaf 2; tput bold)'Color show\n\n'$(tput sgr0)

for ((i = 0; i <= 7; i++)); do
	echo $(tput setaf $i)"Show me the money"$(tput sgr0)
done

printf '\n'$(tput setaf 2; tput setab 0; tput bold)'backgrond color show'$(tput sgr0)'\n'

for ((i = 0, j = 7; i <= 7; i++, j--)); do
	echo $(tput setaf $i; tput setab $j; tput bold)"Show me the money"$(tput sgr0)
done
