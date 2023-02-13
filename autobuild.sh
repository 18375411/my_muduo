#!/bin/bash

#一个命令返回一个非0退出状态值(失败),就退出.
set -e

#-d表示目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make -j4

cd ..

#把头文件拷贝到/usr/include/mymuduo    so库拷贝到/usr/lib
if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

for header in `ls *.hpp`
do
    cp $header /usr/include/mymuduo
done

cp `pwd`/lib/libmymuduo.so /usr/lib

ldconfig


