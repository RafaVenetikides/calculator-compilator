cmake_minimum_required(VERSION 3.23)
project(Calculator C)

set(CMAKE_C_STANDARD 23)

add_executable(Calculator
        main.c
        src/expression_parser.c
        src/file_utils.c
        src/float_conversions.c
        src/generate_assembly.c
        src/stack_operations.c
        src/serial_operations.c
        src/operations/add_operation.c
        include/operations/add_operation.h
        src/operations/subtract_operation.c
        include/operations/subtract_operation.h
        src/operations/multiply_operation.c
        include/operations/multiply_operation.h
        src/operations/div_operation.c
        include/operations/div_operation.h
        src/operations/div_int_operation.c
        include/operations/div_int_operation.h
        src/operations/rem_operation.c
        include/operations/rem_operation.h
        src/operations/power_operation.c
        include/operations/power_operation.h)
