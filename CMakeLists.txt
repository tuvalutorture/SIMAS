cmake_minimum_required(VERSION 4.0)
project(SIMAS C)

set(CMAKE_C_STANDARD 90)

include_directories(src)

add_executable(SIMAS
        src/functions.c
        src/functions.h
        src/hashmap.c
        src/hashmap.h
        src/helpers.c
        src/helpers.h
        src/runtime.c
        src/runtime.h
        src/simas.c
        src/strings.c
        src/strings.h
        src/variables.c
        src/variables.h)
