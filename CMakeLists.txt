# 要求 CMake 最低版本不低于 3.21
cmake_minimum_required(VERSION 3.21)
project(my_shell)

# 设置 C 标准为 17
set(CMAKE_C_STANDARD 17)
# 强制要求编译器支持该标准（若不支持则报错，而不是“衰减”到旧标准）
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(my_shell main.c)
