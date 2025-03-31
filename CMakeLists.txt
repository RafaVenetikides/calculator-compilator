cmake_minimum_required(VERSION 3.23)
project(Calculator C)

set(CMAKE_C_STANDARD 23)

add_executable(Calculator
        main.c
        src/expression_parser.c
        src/file_utils.c
        src/float_conversions.c
        src/operations.c
        src/stack_operations.c
        src/serial_operations.c)
