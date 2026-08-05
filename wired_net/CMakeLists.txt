# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(wired_net C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(wired_net 
    src/main.c
    src/wizchip_spi.c
    lib/w5500/wizchip_conf.c
    lib/w5500/socket.c
    lib/w5500/w5500.c
    src/neopixel.c
    src/osc_parser.c   
)
pico_set_program_name(wired_net "wired_net")
pico_set_program_version(wired_net "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(wired_net 0)
pico_enable_stdio_usb(wired_net 1)

# Add the standard library to the build
target_link_libraries(wired_net
        pico_stdlib
        hardware_spi
        hardware_pio)

# Add the standard include files to the build
target_include_directories(wired_net PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        src
        lib/w5500
)

#led controll pulls
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2815gen.pio)


pico_add_extra_outputs(wired_net)

