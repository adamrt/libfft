// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Adam Patterson

// For the latest version: - https://github.com/adamrt/libfft

#ifndef FFT_H
#define FFT_H

// Thank you to the FFHacktics community and their wiki. This library would not
// be possible without their amazing work (https://ffhacktics.com/wiki/).

// ============================================================================
//            ___       ___  ________  ________ ________ _________
//           |\  \     |\  \|\   __  \|\  _____\\  _____\\___   ___\
//           \ \  \    \ \  \ \  \|\ /\ \  \__/\ \  \__/\|___ \  \_|
//            \ \  \    \ \  \ \   __  \ \   __\\ \   __\    \ \  \
//             \ \  \____\ \  \ \  \|\  \ \  \_| \ \  \_|     \ \  \
//              \ \_______\ \__\ \_______\ \__\   \ \__\       \ \__\
//               \|_______|\|__|\|_______|\|__|    \|__|        \|__|
//
//                   libFFT - A Final Fantasy Tactics Library
//
// ============================================================================

// This library provides a way to read data from the Final Fantasy Tactics PS1
// binary file. It allows reading maps, textures, events, etc. It is designed to
// be used in C/C++ projects and provides a simple API for reading the data in a
// structured way.
//
// The purpose of this library is to codify the knowledge known about the FFT
// data and to make it easy to use the data in other applications.
//
// This library can read:
//   - Maps: meshes, textures, terrain, lighting, etc.
//   - Events: event instructions, messages, etc.
//
// The only supported file is the PSX US Final Fantasy Tactics BIN file:
//   - Serial: SCUS-94221
//   - SHA1:   2b5d4db3229cdc7bbd0358b95fcba33dddae8bba
//   - MD5:    b156ba386436d20fd5ed8d37bab6b624
//
// You can check your file with the following commands:
//     ```bash
//     $ md5sum fft.bin
//     $ sha1sum fft.bin
//     ```
// ============================================================================
//
// Usage:
//
// To use this library, you need to include the header file in your project and
// in a single translation unit (C file), define `FFT_IMPLEMENTATION` before
// including the header. You can include the file without defining
// `FFT_IMPLEMENTATION` in multiple translation units to use the library.
//
//     ```c
//     #include <stdio.h>
//
//     #define FFT_IMPLEMENTATION
//     #include "fft.h"
//     ```
//
// Then you can initialize the library by calling `fft_init` with the path to the
// FFT binary file. After that, you can read maps and other data.
//
//     ```c
//     int main() {
//         fft_init("fft.bin");
//         map_data_t* map = fft_read_map(1); // Read map with ID 1
//         if (map) {
//             // Do something with the map data
//             fft_destroy_map(map); // Clean up when done
//         }
//
//         fft_shutdown(); // Clean up the library
//     }
//     ```
// ============================================================================
//
// Data structures:
//
// The data structures are intentionally simple and try not to make assumptions
// about the users intentions. For instance, we keep the original types were
// possible, like using i16 for vertex position data instead of casting to the
// more common f32. We use custom types for fixed-point math to inform the user
// of the intended use in the game. We store that data in its original format
// but provide helper functions and macros to convert to commonly desired types.
//
// One exception where we don't stick to the original types is when there are
// multiple values in a single byte. We typically split these into separate fields
// for ease of use. For example:
//
//     Polygon Tile Locations:
//     +--------------+------+---------+
//     | Width (bits) | Type | Purpose |
//     |--------------+------+---------+
//     | 7            | uint | Z coord |
//     | 1            | N/A  | Height  |
//     | 8            | uint | X coord |
//     +--------------+------+---------+
//
// Our data structure would be something that split the values from the first
// byte into two separate fields:
//
//     ```c
//     typedef struct {
//         u8 x;
//         u8 z;
//         u8 height;
//     } polygon_tile_location_t;
//     ```
//
//   This makes accessing fields a little easier, more readable at the minor
//   expense of a little space.
//
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // FFT_H

// *===========================================================================
// *                             IMPLEMENTATION
// *===========================================================================

#ifdef FFT_IMPLEMENTATION

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Usage:
//   assert(condition, "Simple message");
//   assert(condition, "Formatted message: %d, %s", value1, value2);
#define FFT_ASSERT(cond, ...)                                   \
    do {                                                        \
        if (!(cond)) {                                          \
            fprintf(stderr, "Assertion failed: " __VA_ARGS__);  \
            fprintf(stderr, " at %s:%d\n", __FILE__, __LINE__); \
            exit(EXIT_FAILURE);                                 \
        }                                                       \
    } while (0)

//
// Span
//
// Span allows reading specific types from a byte array.
//

enum {
    // This is the size of a map texture, which is the largest file size we read.
    FFT_SPAN_MAX_BYTES = 131072,
};

typedef struct {
    const uint8_t* data;
    size_t size;
    size_t offset;
} fft_span_t;

static void fft_span_read_bytes(fft_span_t* f, size_t size, uint8_t* out_bytes) {
    FFT_ASSERT(size <= FFT_SPAN_MAX_BYTES, "Too many bytes requested.");
    memcpy(out_bytes, &f->data[f->offset], size);
    f->offset += size;
    return;
}

// FN_SPAN_READ is a macro that generates a read function for a specific type. It
// reads the value, returns it and increments the offset.
#define FFT_FN_SPAN_READ(name, type)                                                  \
    static type fft_span_read_##name(fft_span_t* span) {                              \
        FFT_ASSERT(span->offset + sizeof(type) <= span->size, "Out of bounds read."); \
        type value;                                                                   \
        memcpy(&value, &span->data[span->offset], sizeof(type));                      \
        span->offset += sizeof(type);                                                 \
        return value;                                                                 \
    }

FFT_FN_SPAN_READ(u8, uint8_t)
FFT_FN_SPAN_READ(u16, uint16_t)
FFT_FN_SPAN_READ(u32, uint32_t)
FFT_FN_SPAN_READ(i8, int8_t)
FFT_FN_SPAN_READ(i16, int16_t)
FFT_FN_SPAN_READ(i32, int32_t)

#endif // FFT_IMPLEMENTATION
