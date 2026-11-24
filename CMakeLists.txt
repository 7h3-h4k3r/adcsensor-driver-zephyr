cmake_minimum_required(VERSION 3.20.0)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(RenvSensors)
target_include_directories(app PRIVATE drivers/sensor/adcs)
target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_SENSORADC app PRIVATE drivers/sensor/adcs/adcsensor.c)