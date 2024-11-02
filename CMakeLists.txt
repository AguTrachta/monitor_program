cmake_minimum_required(VERSION 3.14)
project(Monitor VERSION 1.0 LANGUAGES C)

# Incluir el toolchain de Conan
include(${CMAKE_BINARY_DIR}/Release/generators/conan_toolchain.cmake)

# Configurar CMAKE_PREFIX_PATH para encontrar dependencias en Release/generators
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/Release/generators" ${CMAKE_PREFIX_PATH})

# Encontrar las dependencias gestionadas por Conan
find_package(cJSON REQUIRED)

# Incluir directorios de encabezados propios
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)

# Añadir el ejecutable
add_executable(monitoring_project
    src/main.c
    src/metrics.c
    src/expose_metrics.c
    src/json_metrics.c
)

# Vincular las librerías libprom, libpromhttp y cJSON
target_link_libraries(monitoring_project
    /usr/local/lib/libprom.so
    /usr/local/lib/libpromhttp.so
    cjson::cjson
    m
    pthread
)

# Establecer el directorio de salida del ejecutable a la carpeta raíz del proyecto
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")




