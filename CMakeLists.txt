cmake_minimum_required(VERSION 3.24)

project(b-cad VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)            # REQ-TEC-001
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

include(CTest)  # aktiviert enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
