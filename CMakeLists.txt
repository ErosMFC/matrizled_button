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
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(botao_matrizled C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(botao_matrizled botao_matrizled.c )

pico_set_program_name(botao_matrizled "botao_matrizled")
pico_set_program_version(botao_matrizled "0.1")

# Generate PIO header
pico_generate_pio_header(botao_matrizled ${CMAKE_CURRENT_LIST_DIR}/ws2812b.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(botao_matrizled 0)
pico_enable_stdio_usb(botao_matrizled 1)

# Add the standard library to the build
target_link_libraries(botao_matrizled
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(botao_matrizled PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_SDK_PATH}/src/common/pico_base/include
)

# Add any user requested libraries
target_link_libraries(botao_matrizled 
         hardware_pio
         hardware_clocks
        
        )

pico_add_extra_outputs(botao_matrizled)

