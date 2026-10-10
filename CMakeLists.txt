cmake_minimum_required(VERSION 3.16)

project(OTAValveMaster LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ota_valve_core STATIC
    src/comm.c
    src/netintr.c
    src/motor.c
    src/ipc.c
    src/lora.cpp
    src/gsm.c
)

target_include_directories(ota_valve_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(ota_valve_master
    src/main.cpp
)

target_link_libraries(ota_valve_master PRIVATE ota_valve_core)

add_executable(demo_user
    demo_user.c
)

target_include_directories(demo_user
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(demo_user PRIVATE ota_valve_core)

message(STATUS "Configured OTAValveMaster as a C++ project")
