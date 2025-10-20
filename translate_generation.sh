#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com

ts_list=(`ls translations/*.ts`)

# 优先使用Qt6 lrelease工具
if [ -f "/usr/lib/qt6/bin/lrelease" ]; then
    lrelease="/usr/lib/qt6/bin/lrelease" 
elif [ -f "/usr/lib/qt5/bin/lrelease" ]; then
    lrelease="/usr/lib/qt5/bin/lrelease"
else
    lrelease="lrelease"
fi

# 遍历所有ts文件并生成qm文件
for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    ${lrelease} "${ts}"
done
