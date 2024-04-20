# Copyright (c) 2024 BiologIC Technologies Ltd
# Mass Gas Flow module


cmake_minimum_required(VERSION 3.20.0)


set(BOARD pico_plasma)
set(DBOARD_FLASH_RUNNER jlink)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

find_package(Zephyr REQUIRED HINTS ${ZEPHYR_BASE})
project(dotstar)

target_include_directories(app PRIVATE ${CMAKE_BINARY_DIR}/dotstar/include src)

target_sources(app PRIVATE src/main.c)