#!/bin/bash

# Simple build script for libfft
set -e

# Create build directory if it doesn't exist
mkdir -p build

# Change to build directory
cd build

if [ "$1" = "test" ]; then
    gcc -o test ../test.c -I..
    ./test
    exit $?
fi

# Configure with cmake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build completed successfully!"
echo "Executable: build/main"
