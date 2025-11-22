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

# Device selection (set via -DIIDX_DEVICE=...)
# Valid values: IIDX_PS2 (default), RAINBOW2, RAINBOW2PLUS
if(NOT DEFINED IIDX_DEVICE)
    set(IIDX_DEVICE "IIDX_PS2" CACHE STRING "Target device: IIDX_PS2, RAINBOW2, RAINBOW2PLUS")
    # Warn when falling back to default
    message(WARNING "IIDX_DEVICE not specified; defaulting to IIDX_PS2")
else()
    set(IIDX_DEVICE "${IIDX_DEVICE}" CACHE STRING "Target device: IIDX_PS2, RAINBOW2, RAINBOW2PLUS")
endif()
set_property(CACHE IIDX_DEVICE PROPERTY STRINGS IIDX_PS2 RAINBOW2 RAINBOW2PLUS)
add_executable(iidxcon_pipico
  iidxcon_pipico.c
  usb_descriptors.c
)

# Map IIDX_DEVICE to preprocessor definitions used in the source
if(IIDX_DEVICE STREQUAL "IIDX_PS2")
    target_compile_definitions(iidxcon_pipico PRIVATE IIDX_PS2)
elseif(IIDX_DEVICE STREQUAL "RAINBOW2")
    target_compile_definitions(iidxcon_pipico PRIVATE RAINBOW2)
elseif(IIDX_DEVICE STREQUAL "RAINBOW2PLUS")
    target_compile_definitions(iidxcon_pipico PRIVATE RAINBOW2PLUS)
else()
    message(FATAL_ERROR "Invalid IIDX_DEVICE='${IIDX_DEVICE}'. Valid options: IIDX_PS2, RAINBOW2, RAINBOW2PLUS")
endif()
pico_enable_stdio_uart(iidxcon_pipico 1)
pico_add_extra_outputs(iidxcon_pipico)

target_include_directories(iidxcon_pipico PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(iidxcon_pipico PRIVATE pico_stdlib pico_multicore pico_unique_id tinyusb_device tinyusb_board)
