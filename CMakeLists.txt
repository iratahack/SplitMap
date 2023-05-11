cmake_minimum_required(VERSION 3.13.4)
project(splitmap)

#set(CMAKE_C_FLAGS "-O3 -Wall -fsanitize=address -static-libasan -g")
set(CMAKE_C_FLAGS "-Wall -O3")

add_executable(splitmap splitmap.c optimize.c compress.c memory.c)
