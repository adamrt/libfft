# libFFT - Final Fantasy Tactics Library

A C library for reading and extracting data from Final Fantasy Tactics PS1 binary files.

## Overview

libFFT provides a comprehensive API for accessing game data from Final Fantasy
Tactics (PS1), including maps, textures, events, character data, and more. The
library is designed as a single-header implementation that makes it easy to
integrate into C/C++ projects.

## Requirements

You need to provide your own FFT binary file ripped from a PSX disc. Only the US version is supported:
- **Serial**: SCUS-94221
- **SHA1**: `2b5d4db3229cdc7bbd0358b95fcba33dddae8bba`
- **MD5**: `b156ba386436d20fd5ed8d37bab6b624`

## Library Usage

```c
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

int main() {
    // Initialize with your FFT binary file
    fft_init("fft.bin");
    
    // Read game data
    fft_span_t battle_data = fft_io_open(F_BATTLE_BIN);
    // ... process data
    fft_io_close(battle_data);
    
    // Clean up
    fft_shutdown();
    return 0;
}
```

## Building

This library is intended to be used in your applications, but you can build the tools with: 

```bash
./build.sh
```

This creates two executables:
- `fft_export_images` - Tool for extracting game images
- `fft_debug` - Debug/testing tool not for general consumption


## Limitations

- **Not thread-safe** - Use in single-threaded applications only
- **Uses assertions** - Library will abort on errors rather than returning error codes
- **PS1 US version only** - Other versions/platforms not supported

## Acknowledgments

Special thanks to the [FFHacktics community](https://ffhacktics.com/wiki/) whose
research and documentation made this library possible.

