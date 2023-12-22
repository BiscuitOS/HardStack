#!/bin/bash

# 指定搜索的目录
search_dir=`pwd`

# 遍历指定目录下所有名为 Makefile 的文件
find "$search_dir" -name 'Makefile' | while read makefile; do
    # 检查文件内容中是否包含字符串 "APP"
    if grep -q 'Application Project' "$makefile"; then
        # 如果包含，则在 "LCFLAGS" 首次出现之后添加特定的多行文本
        awk '{
            print;
            if ($0 ~ /LCFLAGS/ && !added) {
                print "# MEMORY";
                print "MEMORY_FLUID            := $(PWD)/../../../rootfs/rootfs/usr/share/BiscuitOS_memory_fluid.h";
                print "ifneq ($(wildcard $(MEMORY_FLUID)),)";
                print "LCFLAGS                 += -include $(MEMORY_FLUID)";
                print "endif";
                added = 1;
            }
        }' "$makefile" > tmpfile && mv tmpfile "$makefile"
        echo "Modified $makefile to add custom text after LCFLAGS"
    fi
done

