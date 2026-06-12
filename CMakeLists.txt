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

# devices フォルダのヘッダファイルから自動的にデバイス名を抽出
file(GLOB DEVICE_HEADERS "${CMAKE_CURRENT_LIST_DIR}/devices/*.h")
set(IIDX_AVAILABLE_DEVICES "")

foreach(HEADER ${DEVICE_HEADERS})
    # ファイル名から拡張子を除去し、大文字に変換
    get_filename_component(HEADER_NAME ${HEADER} NAME_WE)
    string(TOUPPER ${HEADER_NAME} DEVICE_NAME)
    list(APPEND IIDX_AVAILABLE_DEVICES ${DEVICE_NAME})
endforeach()

# Device selection (set via -DIIDX_DEVICE=...)
if(NOT DEFINED IIDX_DEVICE)
    set(IIDX_DEVICE "IIDX_PS2" CACHE STRING "Target device")
    message(WARNING "IIDX_DEVICE not specified; defaulting to ${IIDX_DEVICE}")
else()
    set(IIDX_DEVICE "${IIDX_DEVICE}" CACHE STRING "Target device")
endif()
set_property(CACHE IIDX_DEVICE PROPERTY STRINGS ${IIDX_AVAILABLE_DEVICES})

# Validate device selection
list(FIND IIDX_AVAILABLE_DEVICES "${IIDX_DEVICE}" device_index)
if(device_index EQUAL -1)
    message(FATAL_ERROR "Invalid IIDX_DEVICE='${IIDX_DEVICE}'. Valid options: ${IIDX_AVAILABLE_DEVICES}")
endif()

add_executable(iidxcon_pipico
  iidxcon_pipico.c
  usb_descriptors.c
)

# Add compile definition for the selected device
# IIDX_DEVICE を定義（例：RAINBOW2PLUS）
target_compile_definitions(iidxcon_pipico PRIVATE ${IIDX_DEVICE})
# ヘッダファイル名を文字列として定義（例：devices/rainbow2plus.h）
# ファイル名を小文字に変換してパスを構築
string(TOLOWER "${IIDX_DEVICE}" IIDX_DEVICE_LOWER)
target_compile_definitions(iidxcon_pipico PRIVATE IIDX_DEVICE_HEADER="devices/${IIDX_DEVICE_LOWER}.h")
pico_enable_stdio_uart(iidxcon_pipico 1)
pico_add_extra_outputs(iidxcon_pipico)

target_include_directories(iidxcon_pipico PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(iidxcon_pipico PRIVATE pico_stdlib pico_multicore pico_unique_id tinyusb_device tinyusb_board)
