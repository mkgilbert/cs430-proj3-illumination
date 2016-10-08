cmake_minimum_required(VERSION 3.6)
project(cs430_proj3_illumination)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES src/main.c src/raycaster.c include/raycaster.h src/ppmrw.c include/ppmrw.h include/vector_math.h src/json.c include/json.h include/base.h)
add_executable(cs430_proj3_illumination ${SOURCE_FILES})