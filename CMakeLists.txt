cmake_minimum_required(VERSION 3.24)

project(b-cad VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)            # REQ-TEC-001
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # für clang-tidy (make lint)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

include(CTest)  # aktiviert enable_testing()

option(BCAD_COVERAGE "Mit gcov-Coverage-Instrumentierung bauen (make coverage-gate)" OFF)
if(BCAD_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_subdirectory(src)
# Zur Laufzeit ladbare Plugins (ADR-0017, slice-026b) — eigener Baum
# außerhalb src/; die AK-Tests laden die hier gebauten MODULE-Targets.
add_subdirectory(plugins)
add_subdirectory(tests)
