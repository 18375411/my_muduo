cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
 
project(testUnit LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../)

#库
add_library(InetAdress ../InetAdress.cpp)

#test
add_executable(testInetAdress testInetAdress.cpp)
target_link_libraries(testInetAdress InetAdress)

enable_testing()
 
#注意这里
add_test(
	NAME catch_test 
	COMMAND $<TARGET_FILE:testInetAdress> --success
	)


