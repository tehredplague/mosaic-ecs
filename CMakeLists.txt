cmake_minimum_required(VERSION 3.28)
project(mosaic_ecs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_executable(mosaic_ecs main.cpp
        compiler.cpp
        compiler.h
        scanner.cpp
        scanner.h
        parser.cpp
        parser.h
        stmt.cpp
        stmt.h
        expr.cpp
        expr.h
        token.cpp
        token.h
        debug.h
        object.cpp
        object.h
        vm.h
        vm.cpp
        value.cpp
        value.h
        debug.cpp
        chunk.h
        ffi.cpp
        ffi.h)
