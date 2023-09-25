cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(relay_pico C CXX ASM)
set(APP_NAME relay_pico)
set(PICO_BOARD pico_w)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Wno-unused-value
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping Pico W examples as support is not available")
    else()

    message("Running makefsdata python script")
    
    execute_process(COMMAND
        py makefsdata.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

    add_executable(${APP_NAME} 
        main.cpp
        SonoffMiniR2Detector.cpp
        tcpclient.c
        ssi.cpp)

    target_link_libraries(${APP_NAME}
        pico_stdlib              # for core functionality
        hardware_adc
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
    )

    target_include_directories(${APP_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
    )

        # create map/bin/hex file etc.
        pico_add_extra_outputs(${APP_NAME})
    endif()
endif()

# Enable usb output, disable uart output
pico_enable_stdio_usb(${APP_NAME} 1)
pico_enable_stdio_uart(${APP_NAME} 0)