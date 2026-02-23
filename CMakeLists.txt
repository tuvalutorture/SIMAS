cmake_minimum_required(VERSION 4.0)
project(SIMAS C)

set(CMAKE_C_STANDARD 90)

include_directories(src)

add_executable(SIMAS
        src/funcs.c
        src/funcs.h
        src/hashmap.c
        src/hashmap.h
        src/helpers.c
        src/helpers.h
        src/runtime.c
        src/runtime.h
        src/simas.c
        src/strings.c
        src/strings.h
        src/vars.c
        src/vars.h)
