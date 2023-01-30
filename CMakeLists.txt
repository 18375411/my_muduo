cmake_minimum_required(VERSION 2.5)
project(mymuduo)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#设置调试信息 -g在编译后的程序中保留调试符号信息 启动C++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

#定义参与编译的源代码文件
aux_source_directory(. SRC_LIST)
#编译动态库
add_library(mymuduo SHARED ${SRC_LIST})
