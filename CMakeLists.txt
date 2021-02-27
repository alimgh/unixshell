cmake_minimum_required(VERSION 3.17)
project(unixshell C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS -pthread)

add_executable(unixshell main.c)
target_link_libraries(unixshell readline)
