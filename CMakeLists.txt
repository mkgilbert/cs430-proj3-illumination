cmake_minimum_required(VERSION 3.6)
project(cs430_proj3_illumination)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99")

set(SOURCE_FILES main.c)
add_executable(cs430_proj3_illumination ${SOURCE_FILES})