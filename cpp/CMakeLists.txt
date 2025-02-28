cmake_minimum_required(VERSION 3.14)

if (NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "You must set VCPKG_ROOT environment variable to point to your vcpkg installation")
endif ()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

project(pyvframe_project LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(pybind11_DIR ${PYBIND11_CMAKE_DIR} CACHE PATH "Path to pybind11 CMake files")

find_package(pybind11 REQUIRED CONFIG HINTS ${PYBIND11_CMAKE_DIR})

find_package(OpenSSL REQUIRED)

# Add Python module
pybind11_add_module(pyvframe python_bindings.cpp)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/findFFmpeg.cmake)
if (TARGET ffmpeg_ep)
    add_dependencies(pyvframe ffmpeg_ep)
endif()

ensure_ffmpeg()

target_include_directories(pyvframe PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_sources(pyvframe PRIVATE video.cpp)
target_link_libraries(pyvframe PRIVATE ${FFMPEG_LIBRARIES_LIST} FFmpeg::FFmpeg OpenSSL::SSL OpenSSL::Crypto)

# Installation
install(TARGETS pyvframe DESTINATION ${CMAKE_INSTALL_PREFIX}/)

add_custom_command(TARGET pyvframe POST_BUILD
        COMMAND echo "include pyvframe/pyvframe${PYTHON_MODULE_EXTENSION}" > "${CMAKE_CURRENT_SOURCE_DIR}/../MANIFEST.in"
        COMMENT "Created ${CMAKE_CURRENT_SOURCE_DIR}/../MANIFEST.in"
    )

# Find the last occurrence of '.' in PYTHON_MODULE_EXTENSION
string(FIND "${PYTHON_MODULE_EXTENSION}" "." LAST_DOT REVERSE)

# Extract only the last extension (e.g., ".so", ".pyd", ".dylib")
if (LAST_DOT GREATER -1)
    string(SUBSTRING "${PYTHON_MODULE_EXTENSION}" ${LAST_DOT} -1 PYTHON_LIB_EXTENSION)
else()
    set(PYTHON_LIB_EXTENSION "${PYTHON_MODULE_EXTENSION}")  # Fallback case
endif()

# Define the symlink name dynamically
set(PYTHON_SYMLINK_NAME "pyvframe${PYTHON_LIB_EXTENSION}")

# Create a symbolic link
add_custom_command(TARGET pyvframe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
        "${CMAKE_CURRENT_SOURCE_DIR}/../pyvframe/pyvframe${PYTHON_MODULE_EXTENSION}" 
        "${CMAKE_CURRENT_SOURCE_DIR}/../pyvframe/${PYTHON_SYMLINK_NAME}"
    COMMENT "Created symbolic link ${PYTHON_SYMLINK_NAME} -> pyvframe${PYTHON_MODULE_EXTENSION}"
)