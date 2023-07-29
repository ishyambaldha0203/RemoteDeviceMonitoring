################################################################################
#
# This is a CMake project file for Remote Device Monitoring application.
#
################################################################################
cmake_minimum_required(VERSION 3.19.0)

include_guard(GLOBAL)

project(RemoteDeviceMonitoring VERSION 0.0.0.1 LANGUAGES CXX)

################################################################################
# C++ definitions
################################################################################
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################################################################
# Configurations
################################################################
set(CMAKE_BUILD_TYPE Debug)

################################################################
# VARIABLES
################################################################
set(COMMON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")

set(DEVICE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Src")
set(DEVICE_TAGET_NAME device)

set(MONITORING_SERVER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Src")
set(MONITORING_SERVER_TAGET_NAME monitor)

################################################################################
# Output
################################################################################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/exe/${CMAKE_BUILD_TYPE})

################################################################
# TARGESTS
################################################################
add_executable(${DEVICE_TAGET_NAME}
                    ${DEVICE_SOURCE_DIR}/Device.cpp)

add_executable(${MONITORING_SERVER_TAGET_NAME}
                    ${MONITORING_SERVER_SOURCE_DIR}/MonitoringServer.cpp)

################################################################
# INCLUDES
################################################################
target_include_directories(${DEVICE_TAGET_NAME} PRIVATE
    ${COMMON_INCLUDE_DIR}
)

target_include_directories(${MONITORING_SERVER_TAGET_NAME} PRIVATE
    ${COMMON_INCLUDE_DIR}
)
