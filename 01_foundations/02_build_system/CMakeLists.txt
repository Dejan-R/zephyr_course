
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(build_system)

target_sources(app PRIVATE src/main.c)

target_compile_definitions(app PRIVATE
    APP_VERSION=\"1.0\"
)
