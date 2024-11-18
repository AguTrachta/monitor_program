cmake_minimum_required(VERSION 3.14)
project(Monitor VERSION 1.0 LANGUAGES C)

# Include the Conan toolchain for dependency management
include(${CMAKE_BINARY_DIR}/Release/generators/conan_toolchain.cmake)

# Configure CMAKE_PREFIX_PATH for Conan dependencies
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/Release/generators" ${CMAKE_PREFIX_PATH})

# Find dependencies managed by Conan
find_package(cJSON REQUIRED)

# Include directories for header files
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(
    ../../../lib/memory/include
    # Add any additional include directories here
)

# Add the Monitoring project library
add_library(monitoring_project STATIC
    src/main.c
    src/metrics.c
    src/expose_metrics.c
    src/json_metrics.c
    ../../../lib/memory/src/memory.c
    ../../../lib/memory/src/stats_memory.c
)

# Link necessary libraries to the Monitoring project
target_link_libraries(monitoring_project
    /usr/local/lib/libprom.so
    /usr/local/lib/libpromhttp.so
    cjson::cjson
    m
    pthread
)

# Set the output directory for the library
set_target_properties(monitoring_project PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

