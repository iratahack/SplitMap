cmake_minimum_required(VERSION 3.13.4)
project(splitmap)

#set(CMAKE_C_FLAGS "-Wall -O0 -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_C_FLAGS "-Wall")

add_executable(splitmap splitmap.c optimize.c compress.c memory.c)
