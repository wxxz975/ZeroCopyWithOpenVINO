# ==============================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================
project("ZeroCopyResnet50")
cmake_minimum_required(VERSION 3.12)

set (TARGET_NAME "ZeroCopyResnet50")
set (CMAKE_BUILD_TYPE Debug)

## use C11/C++17
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV libavformat libavcodec libswscale libavutil)
find_package(OpenVINO COMPONENTS Runtime)
find_package(OpenCL)
find_package(OpenCV)


if (LIBAV_FOUND AND OpenVINO_FOUND AND OpenCL_FOUND)

    #file (GLOB MAIN_SRC *.cpp)
    #file (GLOB MAIN_HEADERS *.h)

    add_executable(${TARGET_NAME} 
        ${CMAKE_SOURCE_DIR}/main.cpp
        ${CMAKE_SOURCE_DIR}/Resnet50.cpp
        ${CMAKE_SOURCE_DIR}/VideoDecoder.cpp
        ${CMAKE_SOURCE_DIR}/ISession.cpp
        #${MAIN_SRC} ${MAIN_HEADERS}
    )

    include_directories(
        ${OpenCV_INCLUDE_DIRS}
    )
    target_link_libraries(
        ${TARGET_NAME} PRIVATE
        ${LIBAV_LIBRARIES}
        openvino::runtime
        ${OpenCL_LIBRARY}
        ${OpenCV_LIBRARIES}
        )


endif()
