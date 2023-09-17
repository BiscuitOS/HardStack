#!/bin/ash
# 
# Stack
STACK=`ulimit -s`
ulimit -s 1024
# Running program
BiscuitOS-PAGING-PF-ANON-STACK-default
ulimit -s ${STACK}
