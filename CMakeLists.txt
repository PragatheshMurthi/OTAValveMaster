cmake_minimum_required(VERSION 3.16)

project(OTAValveMaster LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(ota_valve_core STATIC
    comm.c
    netintr.c
    motor.c
    lora.c
    gsm.c
    ipc.c
)

target_include_directories(ota_valve_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(ota_valve_master
    main.c
)

target_link_libraries(ota_valve_master PRIVATE ota_valve_core)

add_executable(demo_user
    demo_user.c
)

target_link_libraries(demo_user PRIVATE ota_valve_core)

message(STATUS "Configured OTAValveMaster build for the production application")
