# ESP-IDF CMake 项目配置

cmake_minimum_required(VERSION 3.16)

# 设置项目名称
set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)

# 包含 ESP-IDF 构建系统
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 项目名称
project(sg90_servo_control)
