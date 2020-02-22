#
# BiscuitOS Memory Allocator Project
#
# (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

# Hello All:
#
#    This project is used to learn linux-mm and practice on usespance.
#    The project contains:
#
#      1) MEMBLOCK Memory Allocator
#      2) PERCPU-UP Memory Allocator
#      3) PERCPU-SMP Memory Allocator
#      4) Buddy-normal Memory Allocator
#      5) Buddy-highmem Memory Allocator
#      6) PCP Memory Allocator
#      7) Slub Memory Allocator
#      8) kmem_cache Memory Allocator
#      9) kmalloc Memory Allocator
#      a) Name Memory Allocator
#      b) VMALLOC Memory Allocator
#      c) KMAP Memory Allocator
#      d) Fixmap Memory Allocator
#
#    These memory allocator are from Linux-kernel space, and I discard some
#    not important part, contain core logical on project. So, friends, have
#    a good time :)

# Root Direct
ROOT=`pwd`
# Project NAME
PROJECT_NAME=
# SUB-director
SUB_DIR=
# GITEE
GITEE=https://gitee.com/BiscuitOS_team/HardStack/raw/Gitee/Memory-Allocator/

usage()
{
	echo ""
	echo " ____  _                _ _    ___  ____  "
	echo "| __ )(_)___  ___ _   _(_) |_ / _ \/ ___| "
	echo "|  _ \| / __|/ __| | | | | __| | | \___ \ "
	echo "| |_) | \__ \ (__| |_| | | |_| |_| |___) |"
	echo "|____/|_|___/\___|\__,_|_|\__|\___/|____/ "
	echo ""
	echo "Hello BiscuitOS Open-Memory Project"
	echo ""
	echo "1. MEMBLOCK Physical Memory Allocator."
	echo "2. PERCPU(UP) Memory Allocator."
	echo "3. PERCPU(SMP) Memory Allocator."
	echo "4. Buddy-normal Memory Allocator."
	echo "5. Buddy-highmem Memory Allocator."
	echo "6. PCP Memory Allocator."
	echo "7. Slub Memory Allocator."
	echo "8. kmem_cache Memory Allocator."
	echo "9. kmalloc Memory Allocator."
	echo "a. Name Memory Allocator."
	echo "b. VMALLOC Memory Allocator."
	echo "c. KMAP Memory Allocator."
	echo "d. Fixmap Memory Allocator."
	echo ""
	echo "0. Exit"
	echo ""
	echo "BiscuitOS HomePage: https://biscuitos.github.io"
	echo "BiscuitOS Blog: https://biscuitos.github.io/blog/BiscuitOS_Catalogue/"
	echo "Author: BuddyZhang1 <buddy.zhang@aliyun.com>"
	echo ":)"
}

### File Database
# MEMBLOCK
MEMBLOCK_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
               "include/linux/memblock.h" "include/linux/mm.h" \
               "mm/memblock.c")

# PERCPU-UP
PERCPU_UP_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
                "include/linux/bitmap.h"        "include/linux/getorder.h" \
                "include/linux/gfp.h"           "include/linux/list.h" \
                "include/linux/memblock.h"      "include/linux/mm.h" \
                "include/linux/percpu.h"        "mm/bitmap.c" \
                "mm/memblock.c"                 "mm/percpu.c")

# PERCPU-SMP
PERCPU_SMP_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
                "include/linux/bitmap.h"         "include/linux/getorder.h" \
                "include/linux/gfp.h"            "include/linux/list.h" \
                "include/linux/memblock.h"       "include/linux/mm.h" \
                "include/linux/percpu.h"         "mm/bitmap.c" \
                "mm/memblock.c"                  "mm/percpu.c")

# Buddy-Normal
BUDDY_NRM_FILE=("main.c" "README.md" "Makefile" "include/linux/list.h" \
                "include/linux/buddy.h"         "mm/buddy.c")

# Buddy-HighMem
BUDDY_HMM_FILE=("main.c" "README.md" "Makefile" "include/linux/list.h" \
                "include/linux/buddy.h"         "mm/buddy.c")

# PCP
PCP_FILE=("main.c" "README.md" "Makefile" "include/linux/list.h" \
          "include/linux/buddy.h"         "mm/buddy.c")

# Slub
SLUB_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
           "include/linux/bitmap.h"        "include/linux/buddy.h" \
           "include/linux/getorder.h"      "include/linux/gfp.h" \
           "include/linux/list.h"          "include/linux/slub.h" \
           "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c")

# Kmem_cache
KMEM_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
           "include/linux/bitmap.h"        "include/linux/buddy.h" \
           "include/linux/getorder.h"      "include/linux/gfp.h" \
           "include/linux/list.h"          "include/linux/slub.h" \
           "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c")

# Kmalloc
KMALL_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
            "include/linux/bitmap.h"        "include/linux/buddy.h" \
            "include/linux/getorder.h"      "include/linux/gfp.h" \
            "include/linux/list.h"          "include/linux/slub.h" \
            "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c")

# Name
NAME_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
           "include/linux/bitmap.h"        "include/linux/buddy.h" \
           "include/linux/getorder.h"      "include/linux/gfp.h" \
           "include/linux/list.h"          "include/linux/slub.h" \
           "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c")

# VMALLOC
VMALL_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
            "include/linux/bitmap.h"        "include/linux/buddy.h" \
            "include/linux/getorder.h"      "include/linux/gfp.h" \
            "include/linux/list.h"          "include/linux/slub.h" \
            "include/linux/rbtree.h"        "include/linux/vmalloc.h" \
            "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c" "mm/rbtree.c" \
            "mm/vmalloc.c")

# KMAP
KMAP_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
           "include/linux/bitmap.h"        "include/linux/buddy.h" \
           "include/linux/getorder.h"      "include/linux/gfp.h" \
           "include/linux/list.h"          "include/linux/slub.h" \
           "include/linux/highmem.h"       "include/linux/pgtable.h" \
           "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c" "mm/pgtable.c" \
           "mm/highmem.c")

# FIXMAP
FIXMAP_FILE=("main.c" "README.md" "Makefile" "include/linux/biscuitos.h" \
             "include/linux/bitmap.h"        "include/linux/buddy.h" \
             "include/linux/getorder.h"      "include/linux/gfp.h" \
             "include/linux/list.h"          "include/linux/slub.h" \
             "include/linux/highmem.h"       "include/linux/pgtable.h" \
             "mm/buddy.c" "mm/kasprintf.c" "mm/slub.c" "mm/pgtable.c" \
             "mm/highmem.c")

##
# Establish Project
Establish_Project()
{
	_ARRAY=$1

	if [ -d ${ROOT}/${PROJECT_NAME} ]; then
		return
	fi

	mkdir -p ${ROOT}/${PROJECT_NAME}
	cd ${ROOT}/${PROJECT_NAME} >> /dev/null

	for file in ${_ARRAY[*]}; do
		__FILE=
		__LDIR=
		result=`echo ${file} | grep "/"`
		[ ! -z "${result}" ] && __FILE=${file##*/}
		[ ! -z "${__FILE}" ] && __LDIR=${file%%/${__FILE}}
		[ ! -z "${__LDIR}" ] && mkdir -p ${__LDIR}
		wget ${GITEE}/${SUBDIR}/${file}
		[ ! -z "${__LDIR}" ] && mv ${__FILE} ${__LDIR}
	done

	cd - >> /dev/null
}

# Read and timeout 60s
usage
read -p "Input >> " -t 60 OPT
echo ""

# Establish Project
case ${OPT} in
	1) # MEMBLOCK
		SUBDIR=Memblock-allocator/memblock_userspace
		PROJECT_NAME=MEMBLOCK
		Establish_Project "${MEMBLOCK_FILE[*]}"
	;;
	2) # PERCPU-UP
		SUBDIR=PERCPU/UP_PERCPU_userspace
		PROJECT_NAME=PERCPU-UP
		Establish_Project "${PERCPU_UP_FILE[*]}"
	;;
	3) # PERCPU-SMP
		SUBDIR=PERCPU/SMP_PERCPU_userspace
		PROJECT_NAME=PERCPU-SMP
		Establish_Project "${PERCPU_SMP_FILE[*]}"
	;;
	4) # Buddy-Normal
		SUBDIR=Buddy/Userspace
		PROJECT_NAME=Buddy-Normal
		Establish_Project "${BUDDY_NRM_FILE[*]}"
	;;
	5) # Buddy-HighMEM
		SUBDIR=Buddy/HighMem_usespace
		PROJECT_NAME=Buddy-HighMEM
		Establish_Project "${BUDDY_HMM_FILE[*]}"
	;;
	6) # PCP
		SUBDIR=Buddy/PCP_userspace
		PROJECT_NAME=PCP
		Establish_Project "${PCP_FILE[*]}"
	;;
	7) # SLUB
		SUBDIR=slab/slub_userspace
		PROJECT_NAME=Slub
		Establish_Project "${SLUB_FILE[*]}"
	;;
	8) # KMEM_CACHE
		SUBDIR=slab/kmem_cache_userspace
		PROJECT_NAME=Kmem_cache
		Establish_Project "${KMEM_FILE[*]}"
	;;
	9) # Kmalloc
		SUBDIR=slab/kmalloc_userspace
		PROJECT_NAME=Kmalloc
		Establish_Project "${KMALL_FILE[*]}"
	;;
	a) # Name Allocator
		SUBDIR=slab/name_userspace
		PROJECT_NAME=NAME_ALLOC
		Establish_Project "${NAME_FILE[*]}"
	;;
	b) # VMAALLOC
		SUBDIR=vmalloc/vmalloc_userspace
		PROJECT_NAME=VMALLOC
		Establish_Project "${VMALL_FILE[*]}"
	;;
	c) # KMAP
		SUBDIR=Kmap/kmap_userspace
		PROJECT_NAME=KMAP
		Establish_Project "${KMAP_FILE[*]}"
	;;
	d) # FIXMAP
		SUBDIR=Kmap/kmap_userspace
		PROJECT_NAME=FIXMAP
		Establish_Project "${FIXMAP_FILE[*]}"
	;;
esac

echo "Hello BiscuitOS"
echo "Target:"
echo ">>     ${ROOT}/${PROJECT_NAME}"
echo ""
