cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(Serious-Engine)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/RuntimeOutput")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RuntimeOutput")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfigs/BisonFlex.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfigs/Ecc.cmake)

add_subdirectory(Ecc)
add_subdirectory(Engine)
