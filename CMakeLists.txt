cmake_minimum_required(VERSION 3.20)
project(libfft C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable compile_commands.json for tools like clangd/clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, etc.)" FORCE)

add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wnull-dereference
    -Wdouble-promotion
    -Wconversion
    -Wsign-conversion
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wvla
    -Wno-unused-parameter
    -Wno-unused-function)

add_executable(fft_debug tools/fft_debug.c)
target_include_directories(fft_debug PRIVATE .)

add_executable(fft_export_images tools/fft_export_images.c)
target_include_directories(fft_export_images PRIVATE .)
