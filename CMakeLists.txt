cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gps_beacon)

target_sources(app PRIVATE src/main.c src/bluetooth.c src/serial.c src/utils.c)
