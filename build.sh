#!/bin/bash

# Build script for libfft - single header library with development tools
set -e

CC=${CC:-clang}
CFLAGS="-std=c11 -Wall -Wextra -Werror -Wpedantic -Wshadow -Wformat=2 -Wnull-dereference -Wdouble-promotion -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla -Wno-unused-parameter -Wno-unused-function -g -O0 -DDEBUG"

mkdir -p build

show_usage() {
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  all           Build all tools (default)"
    echo "  test          Build and run tests"
    echo "  debug         Build fft_debug tool"
    echo "  export        Build fft_export_images tool"
    echo "  clean         Clean build directory"
    echo ""
    echo "Environment variables:"
    echo "  CC            Compiler to use (default: clang)"
}

compile_tool() {
    local tool_name=$1
    local source_file=$2
    $CC $CFLAGS -o "build/$tool_name" "$source_file" -I.
    echo "Built: build/$tool_name"
}

case "${1:-all}" in
    "test")
        echo "Building and running tests..."
        $CC $CFLAGS -o build/test test.c -I.
        build/test
        ;;
    
    "debug")
        compile_tool "fft_debug" "tools/fft_debug.c"
        ;;
    
    "export")
        compile_tool "fft_export_images" "tools/fft_export_images.c"
        ;;
    
    "all")
        compile_tool "fft_debug" "tools/fft_debug.c"
        compile_tool "fft_export_images" "tools/fft_export_images.c"
        ;;
    
    "clean")
        rm -rf build/*
        ;;
    
    "help"|"-h"|"--help")
        show_usage
        ;;
    
    *)
        echo "Unknown command: $1"
        echo ""
        show_usage
        exit 1
        ;;
esac
