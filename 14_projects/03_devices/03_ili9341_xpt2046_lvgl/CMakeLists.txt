cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ili9341_xpt2046_lvgl)

target_sources(app PRIVATE
    src/main.c
    src/logo.c
)