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

#endif // FFT_IMPLEMENTATION
