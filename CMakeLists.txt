# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(iidxcon_pipico)
pico_sdk_init()
add_executable(iidxcon_pipico
  iidxcon_pipico.c
  usb_descriptors.c
)
pico_enable_stdio_uart(iidxcon_pipico 1)
pico_add_extra_outputs(iidxcon_pipico)

target_include_directories(iidxcon_pipico PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(iidxcon_pipico PRIVATE pico_stdlib pico_multicore pico_unique_id tinyusb_device tinyusb_board)
