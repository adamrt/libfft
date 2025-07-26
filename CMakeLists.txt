cmake_minimum_required(VERSION 3.20)
project(libfft C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable compile_commands.json for tools like clangd/clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Debug build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, etc.)" FORCE)
endif()

# Common warning flags
set(COMMON_WARNINGS
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
    -Wno-unused-function # You can remove this if you want unused function warnings
)

# Apply warning flags to all targets
add_compile_options(${COMMON_WARNINGS})

# Add main executable
add_executable(main tools/main.c)
target_include_directories(main PRIVATE .)
