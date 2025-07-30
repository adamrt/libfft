#ifndef FFT_H
#define FFT_H


/*
================================================================================
libFFT - A Final Fantasy Tactics Library
================================================================================
SPDX-License-Identifier: BSD-2-Clause
Copyright (c) 2025 Adam Patterson

For the latest version: - https://github.com/adamrt/libfft

❤️ Thank you to the FFHacktics community and their wiki. This library would not
be possible without their amazing work (https://ffhacktics.com/wiki/).

================================================================================
           ___       ___  ________  ________ ________ _________
          |\  \     |\  \|\   __  \|\  _____\\  _____\\___   ___\
          \ \  \    \ \  \ \  \|\ /\ \  \__/\ \  \__/\|___ \  \_|
           \ \  \    \ \  \ \   __  \ \   __\\ \   __\    \ \  \
            \ \  \____\ \  \ \  \|\  \ \  \_| \ \  \_|     \ \  \
             \ \_______\ \__\ \_______\ \__\   \ \__\       \ \__\
              \|_______|\|__|\|_______|\|__|    \|__|        \|__|

================================================================================

DESCRIPTION
    This library provides a way to read data from the Final Fantasy Tactics PS1
    binary file. It allows reading maps, textures, events, etc. It is designed
    to be used in C/C++ projects and provides a simple API for reading the data
    in a structured way.

PURPOSE
    The purpose of this library is to codify the knowledge known about the FFT
    data and to make it easy to use the data in other applications.

USAGE
    To use this library, you need to include the header file in your project and
    in a single translation unit (C file), define `FFT_IMPLEMENTATION` before
    including the header. You can include the file without defining
    `FFT_IMPLEMENTATION` in multiple translation units to use the library.

        ```c
        #include <stdio.h>

        #define FFT_IMPLEMENTATION
        #include "fft.h"

        int main() {
            fft_init("my_fft_file.bin");
            fft_do_thing(...);
            ffft_shutdown();
        }
        ```
WARNINGS
    - This lib is not thread-safe
    - This lib uses ASSERTs in many places instead of returning errors

    Both of these will probably be addressed in the future. This library was
    extracted from another project of mine where single-threaded and asserts
    were acceptable.

GAME DATA/ASSETS:
    You will need to rip your own BIN file from a PSX disc. The only supported
    file is the PSX US Final Fantasy Tactics BIN file:
      - Serial: SCUS-94221
      - SHA1:   2b5d4db3229cdc7bbd0358b95fcba33dddae8bba
      - MD5:    b156ba386436d20fd5ed8d37bab6b624

    You can check your file with the following commands:
        ```bash
        $ sha1sum fft.bin
        $ md5sum fft.bin
        ```

================================================================================
About libFFT Data Structures
================================================================================

The data structures are intentionally simple and try not to make assumptions
about the users intentions. For instance, we keep the original types were
possible, like using i16 for vertex position data instead of casting to the
more common f32. We use custom types for fixed-point math to inform the user
of the intended use in the game. We store that data in its original format
but provide helper functions and macros to convert to commonly desired types.

One exception where we don't stick to the original types is when there are
multiple values in a single byte. We typically split these into separate fields
for ease of use. For example:

    Polygon Tile Locations:
    +--------------+------+---------+
    | Width (bits) | Type | Purpose |
    |--------------+------+---------+
    | 7            | uint | Z coord |
    | 1            | N/A  | Height  |
    | 8            | uint | X coord |
    +--------------+------+---------+

Our data structure would be something that split the values from the first
byte into two separate fields:

    ```c
    typedef struct {
        u8 x;
        u8 z;
        u8 height;
    } polygon_tile_location_t;
    ```

  This makes accessing fields a little easier, more readable at the minor
  expense of a little space.

================================================================================

*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void fft_init(const char* filename);
void fft_shutdown(void);

/*
================================================================================
Fixed Point Types
================================================================================
*/

// Fixed-point types for FFT
typedef int16_t fft_fixed16_t;

/*
================================================================================
Map state
================================================================================

The map state represents the state of a map. A map typically has multiple meshes
and textures for use in different events and random battles. The state is used
to determine which resources to load based on the time of day, weather, and
layout. Each resource has a specified state.

The default state is:
  - Time: FFT_TIME_DAY
  - Weather: FFT_WEATHER_NONE
  - Layout: 0
================================================================================
*/

typedef enum {
    FFT_TIME_DAY = 0x0,
    FFT_TIME_NIGHT = 0x1,
} fft_time_e;

typedef enum {
    FFT_WEATHER_NONE = 0x0,
    FFT_WEATHER_NONE_ALT = 0x1,
    FFT_WEATHER_NORMAL = 0x2,
    FFT_WEATHER_STRONG = 0x3,
    FFT_WEATHER_VERY_STRONG = 0x4,
} fft_weather_e;

// Each record is related to a specific time, weather, and layout.
typedef struct {
    fft_time_e time;
    fft_weather_e weather;
    int32_t layout;
} fft_state_t;

// Comparative functions for fft_state_t
bool fft_state_is_equal(fft_state_t a, fft_state_t b);
bool fft_state_is_default(fft_state_t map_state);

// String representations of the enums
const char* fft_time_str(fft_time_e value);
const char* fft_weather_str(fft_weather_e value);

/*
================================================================================
Map/GNS records
================================================================================

Each map has a single GNS file. These files contains a varying number of 20-byte
records (max 40). These records describe the for the map. They have a type
(fft_recordtype_e), which specifies if its a texture, mesh data, etc. They also
have the weather/time/layout they are valid for. Then the location (sector) and
size in the BIN file for that data.

Each resource is a separate file in the original PSX binary if you mount the
disk. But, we just get the sector and length, and read them directly from the
binary, to simplify the process.

We don't know the number of records ahead of time, so we read each one until the
type is FFT_RECORDTYPE_END.

Format: AA BC DD EE FF GG HH HH II JJ
+------+---------+-------+--------------------------------------+
| Pos  | Size    | Index | Description                          |
+------+---------+-------+--------------------------------------+
| AA   | 2 bytes |   0-1 | unknown, always 0x22, 0x30 or 0x70   |
| B    | 1 bytes |     2 | room layout                          |
| C    | 1 bytes |     3 | fft_time_e and fft_weather_e         |
| DD   | 2 bytes |   4-5 | fft_recordtype_e                     |
| EE   | 2 bytes |   6-7 | unknown                              |
| FF   | 2 bytes |   8-9 | start sector                         |
| GG   | 2 bytes | 10-11 | unknown                              |
| HHHH | 4 bytes | 12-15 | resource size                        |
| II   | 2 bytes | 16-17 | unknown                              |
| JJ   | 2 bytes | 18-19 | unknown                              |
+------+---------+-------+--------------------------------------+

================================================================================
*/

enum {
    FFT_RECORD_MAX = 40,  // Maximum number of records per map
    FFT_RECORD_SIZE = 20, // Size of a record in bytes
};

typedef enum {
    FFT_RECORDTYPE_NONE = 0x0000,
    FFT_RECORDTYPE_TEXTURE = 0x1701,
    FFT_RECORDTYPE_MESH_PRIMARY = 0x2E01,
    FFT_RECORDTYPE_MESH_OVERRIDE = 0x2F01,
    FFT_RECORDTYPE_MESH_ALT = 0x3001,
    FFT_RECORDTYPE_END = 0x3101, // End of file marker
} fft_recordtype_e;

const char* fft_recordtype_str(fft_recordtype_e value);

typedef struct {
    fft_recordtype_e type;
    fft_state_t state;
    uint32_t sector;
    uint32_t length;

    // Padding or unknown fields
    uint16_t aa;
    uint16_t ee;
    uint16_t gg;
    uint16_t ii;
    uint16_t jj;
} fft_record_t;

/*
================================================================================
Span
================================================================================

Span is a structure that represents a contiguous block of data in memory. A span
is used to represent a file or part of a file in the FFT BIN filesystem.

The related functions allow simple reading of specific datatypes.

Example:
    ```c
    fft_span_t span = fft_io_open(F_BATTLE_BIN);
    uint32_t thing = fft_span_read_u32(&span);
    ```

================================================================================
*/

typedef struct {
    const uint8_t* data;
    size_t size;
    size_t offset;
} fft_span_t;

/*
================================================================================
IO/Filesystem
================================================================================

The io module provides access to the FFT BIN filesystem. This is not for general
purpose filesystem access.

================================================================================
*/

// FFT_IO_INDEX is a list of most files in the filesystem from the original PSX
// bin file. They are stored in this macro so we can generate the enum and the
// list without having to duplicate and manage all the entires in two places.
//
// NOTE: Some files are not included in this list to keep it more manageable:
//   - Non-GNS Map files: Sector and len can be determined from the GNS files.
//   - EFFECT/*: We don't use EFFECTS and the list is huge.
//   - SOUND/*: We don't use sound yet
//
#define FFT_IO_INDEX                                               \
    X(F_BATTLE_BIN, 1000, 1397096, "BATTLE.BIN")                   \
    X(F_BATTLE__10M_SPR, 59862, 37377, "BATTLE/10M.SPR")           \
    X(F_BATTLE__10W_SPR, 59881, 37377, "BATTLE/10W.SPR")           \
    X(F_BATTLE__20M_SPR, 59900, 37377, "BATTLE/20M.SPR")           \
    X(F_BATTLE__20W_SPR, 59919, 37377, "BATTLE/20W.SPR")           \
    X(F_BATTLE__40M_SPR, 59938, 37377, "BATTLE/40M.SPR")           \
    X(F_BATTLE__40W_SPR, 59957, 37377, "BATTLE/40W.SPR")           \
    X(F_BATTLE__60M_SPR, 59976, 37377, "BATTLE/60M.SPR")           \
    X(F_BATTLE__60W_SPR, 59995, 37377, "BATTLE/60W.SPR")           \
    X(F_BATTLE__ADORA_SPR, 57141, 47100, "BATTLE/ADORA.SPR")       \
    X(F_BATTLE__AGURI_SPR, 57164, 43309, "BATTLE/AGURI.SPR")       \
    X(F_BATTLE__AJORA_SPR, 57186, 43822, "BATTLE/AJORA.SPR")       \
    X(F_BATTLE__ARLI_SPR, 57208, 41475, "BATTLE/ARLI.SPR")         \
    X(F_BATTLE__ARLI2_SP2, 60113, 32768, "BATTLE/ARLI2.SP2")       \
    X(F_BATTLE__ARU_SPR, 57229, 43358, "BATTLE/ARU.SPR")           \
    X(F_BATTLE__ARUFU_SPR, 57251, 43325, "BATTLE/ARUFU.SPR")       \
    X(F_BATTLE__ARUMA_SPR, 57273, 43822, "BATTLE/ARUMA.SPR")       \
    X(F_BATTLE__ARUTE_SEQ, 57062, 2476, "BATTLE/ARUTE.SEQ")        \
    X(F_BATTLE__ARUTE_SHP, 57034, 1944, "BATTLE/ARUTE.SHP")        \
    X(F_BATTLE__ARUTE_SPR, 57295, 47888, "BATTLE/ARUTE.SPR")       \
    X(F_BATTLE__BARITEN_SPR, 57319, 43955, "BATTLE/BARITEN.SPR")   \
    X(F_BATTLE__BARU_SPR, 57341, 44632, "BATTLE/BARU.SPR")         \
    X(F_BATTLE__BARUNA_SPR, 57363, 44172, "BATTLE/BARUNA.SPR")     \
    X(F_BATTLE__BEHI_SPR, 57385, 46393, "BATTLE/BEHI.SPR")         \
    X(F_BATTLE__BEHI2_SP2, 60129, 32768, "BATTLE/BEHI2.SP2")       \
    X(F_BATTLE__BEIO_SPR, 57408, 43746, "BATTLE/BEIO.SPR")         \
    X(F_BATTLE__BIBU2_SP2, 60145, 32768, "BATTLE/BIBU2.SP2")       \
    X(F_BATTLE__BIBUROS_SPR, 57430, 44353, "BATTLE/BIBUROS.SPR")   \
    X(F_BATTLE__BOM_SPR, 57452, 42546, "BATTLE/BOM.SPR")           \
    X(F_BATTLE__BOM2_SP2, 60161, 32768, "BATTLE/BOM2.SP2")         \
    X(F_BATTLE__CLOUD_SPR, 57473, 42953, "BATTLE/CLOUD.SPR")       \
    X(F_BATTLE__CYOKO_SEQ, 57053, 3068, "BATTLE/CYOKO.SEQ")        \
    X(F_BATTLE__CYOKO_SHP, 57026, 7316, "BATTLE/CYOKO.SHP")        \
    X(F_BATTLE__CYOKO_SPR, 57494, 49572, "BATTLE/CYOKO.SPR")       \
    X(F_BATTLE__CYOMON1_SPR, 60014, 37377, "BATTLE/CYOMON1.SPR")   \
    X(F_BATTLE__CYOMON2_SPR, 60033, 37377, "BATTLE/CYOMON2.SPR")   \
    X(F_BATTLE__CYOMON3_SPR, 60052, 37377, "BATTLE/CYOMON3.SPR")   \
    X(F_BATTLE__CYOMON4_SPR, 60071, 37377, "BATTLE/CYOMON4.SPR")   \
    X(F_BATTLE__DAISU_SPR, 57519, 43648, "BATTLE/DAISU.SPR")       \
    X(F_BATTLE__DAMI_SPR, 57541, 44690, "BATTLE/DAMI.SPR")         \
    X(F_BATTLE__DEMON_SPR, 57563, 45648, "BATTLE/DEMON.SPR")       \
    X(F_BATTLE__DEMON2_SP2, 60177, 32768, "BATTLE/DEMON2.SP2")     \
    X(F_BATTLE__DILY_SPR, 57586, 43462, "BATTLE/DILY.SPR")         \
    X(F_BATTLE__DILY2_SPR, 57608, 43163, "BATTLE/DILY2.SPR")       \
    X(F_BATTLE__DILY3_SPR, 57630, 44422, "BATTLE/DILY3.SPR")       \
    X(F_BATTLE__DORA_SPR, 57652, 44442, "BATTLE/DORA.SPR")         \
    X(F_BATTLE__DORA1_SPR, 57674, 46754, "BATTLE/DORA1.SPR")       \
    X(F_BATTLE__DORA2_SPR, 57697, 46437, "BATTLE/DORA2.SPR")       \
    X(F_BATTLE__DORA22_SP2, 60193, 32768, "BATTLE/DORA22.SP2")     \
    X(F_BATTLE__EFC_FNT_TIM, 57000, 32832, "BATTLE/EFC_FNT.TIM")   \
    X(F_BATTLE__EFF1_SEQ, 57080, 1244, "BATTLE/EFF1.SEQ")          \
    X(F_BATTLE__EFF1_SHP, 57076, 3144, "BATTLE/EFF1.SHP")          \
    X(F_BATTLE__EFF2_SEQ, 57081, 1244, "BATTLE/EFF2.SEQ")          \
    X(F_BATTLE__EFF2_SHP, 57078, 3144, "BATTLE/EFF2.SHP")          \
    X(F_BATTLE__ENTD1_ENT, 60353, 81920, "BATTLE/ENTD1.ENT")       \
    X(F_BATTLE__ENTD2_ENT, 60393, 81920, "BATTLE/ENTD2.ENT")       \
    X(F_BATTLE__ENTD3_ENT, 60433, 81920, "BATTLE/ENTD3.ENT")       \
    X(F_BATTLE__ENTD4_ENT, 60473, 81920, "BATTLE/ENTD4.ENT")       \
    X(F_BATTLE__ERU_SPR, 57720, 43909, "BATTLE/ERU.SPR")           \
    X(F_BATTLE__FURAIA_SPR, 57742, 37377, "BATTLE/FURAIA.SPR")     \
    X(F_BATTLE__FUSUI_M_SPR, 57761, 43845, "BATTLE/FUSUI_M.SPR")   \
    X(F_BATTLE__FUSUI_W_SPR, 57783, 43812, "BATTLE/FUSUI_W.SPR")   \
    X(F_BATTLE__FYUNE_SPR, 57805, 44698, "BATTLE/FYUNE.SPR")       \
    X(F_BATTLE__GANDO_SPR, 57827, 42967, "BATTLE/GANDO.SPR")       \
    X(F_BATTLE__GARU_SPR, 57848, 43687, "BATTLE/GARU.SPR")         \
    X(F_BATTLE__GIN_M_SPR, 57870, 44623, "BATTLE/GIN_M.SPR")       \
    X(F_BATTLE__GOB_SPR, 57892, 41268, "BATTLE/GOB.SPR")           \
    X(F_BATTLE__GORU_SPR, 57913, 44734, "BATTLE/GORU.SPR")         \
    X(F_BATTLE__GYUMU_SPR, 57935, 43822, "BATTLE/GYUMU.SPR")       \
    X(F_BATTLE__H61_SPR, 57957, 44172, "BATTLE/H61.SPR")           \
    X(F_BATTLE__H75_SPR, 57979, 43476, "BATTLE/H75.SPR")           \
    X(F_BATTLE__H76_SPR, 58001, 43557, "BATTLE/H76.SPR")           \
    X(F_BATTLE__H77_SPR, 58023, 43560, "BATTLE/H77.SPR")           \
    X(F_BATTLE__H78_SPR, 58045, 43560, "BATTLE/H78.SPR")           \
    X(F_BATTLE__H79_SPR, 58067, 43207, "BATTLE/H79.SPR")           \
    X(F_BATTLE__H80_SPR, 58089, 43362, "BATTLE/H80.SPR")           \
    X(F_BATTLE__H81_SPR, 58111, 43462, "BATTLE/H81.SPR")           \
    X(F_BATTLE__H82_SPR, 58133, 43822, "BATTLE/H82.SPR")           \
    X(F_BATTLE__H83_SPR, 58155, 43332, "BATTLE/H83.SPR")           \
    X(F_BATTLE__H85_SPR, 58177, 43362, "BATTLE/H85.SPR")           \
    X(F_BATTLE__HASYU_SPR, 58199, 47430, "BATTLE/HASYU.SPR")       \
    X(F_BATTLE__HEBI_SPR, 58223, 48525, "BATTLE/HEBI.SPR")         \
    X(F_BATTLE__HIME_SPR, 58247, 44670, "BATTLE/HIME.SPR")         \
    X(F_BATTLE__HYOU_SPR, 58269, 43553, "BATTLE/HYOU.SPR")         \
    X(F_BATTLE__HYOU2_SP2, 60209, 32768, "BATTLE/HYOU2.SP2")       \
    X(F_BATTLE__IKA_SPR, 58291, 42126, "BATTLE/IKA.SPR")           \
    X(F_BATTLE__IRON2_SP2, 60225, 32768, "BATTLE/IRON2.SP2")       \
    X(F_BATTLE__IRON3_SP2, 60241, 32768, "BATTLE/IRON3.SP2")       \
    X(F_BATTLE__IRON4_SP2, 60257, 32768, "BATTLE/IRON4.SP2")       \
    X(F_BATTLE__IRON5_SP2, 60273, 32768, "BATTLE/IRON5.SP2")       \
    X(F_BATTLE__ITEM_M_SPR, 58312, 44438, "BATTLE/ITEM_M.SPR")     \
    X(F_BATTLE__ITEM_W_SPR, 58334, 43955, "BATTLE/ITEM_W.SPR")     \
    X(F_BATTLE__KANBA_SPR, 58356, 43309, "BATTLE/KANBA.SPR")       \
    X(F_BATTLE__KANZEN_SEQ, 57064, 2068, "BATTLE/KANZEN.SEQ")      \
    X(F_BATTLE__KANZEN_SHP, 57035, 2584, "BATTLE/KANZEN.SHP")      \
    X(F_BATTLE__KANZEN_SPR, 58378, 48194, "BATTLE/KANZEN.SPR")     \
    X(F_BATTLE__KASANEK_SPR, 58402, 40516, "BATTLE/KASANEK.SPR")   \
    X(F_BATTLE__KASANEM_SPR, 58422, 40516, "BATTLE/KASANEM.SPR")   \
    X(F_BATTLE__KI_SPR, 58442, 45205, "BATTLE/KI.SPR")             \
    X(F_BATTLE__KNIGHT_M_SPR, 58465, 44406, "BATTLE/KNIGHT_M.SPR") \
    X(F_BATTLE__KNIGHT_W_SPR, 58487, 44433, "BATTLE/KNIGHT_W.SPR") \
    X(F_BATTLE__KURO_M_SPR, 58509, 45623, "BATTLE/KURO_M.SPR")     \
    X(F_BATTLE__KURO_W_SPR, 58532, 44669, "BATTLE/KURO_W.SPR")     \
    X(F_BATTLE__KYUKU_SPR, 58554, 48094, "BATTLE/KYUKU.SPR")       \
    X(F_BATTLE__LEDY_SPR, 58578, 43325, "BATTLE/LEDY.SPR")         \
    X(F_BATTLE__MARA_SPR, 58600, 42967, "BATTLE/MARA.SPR")         \
    X(F_BATTLE__MINA_M_SPR, 58621, 43433, "BATTLE/MINA_M.SPR")     \
    X(F_BATTLE__MINA_W_SPR, 58643, 43529, "BATTLE/MINA_W.SPR")     \
    X(F_BATTLE__MINOTA_SPR, 58665, 47737, "BATTLE/MINOTA.SPR")     \
    X(F_BATTLE__MINOTA2_SP2, 60289, 32768, "BATTLE/MINOTA2.SP2")   \
    X(F_BATTLE__MOL_SPR, 58689, 47102, "BATTLE/MOL.SPR")           \
    X(F_BATTLE__MOL2_SP2, 60305, 32768, "BATTLE/MOL2.SP2")         \
    X(F_BATTLE__MON_SEQ, 57055, 5882, "BATTLE/MON.SEQ")            \
    X(F_BATTLE__MON_SHP, 57030, 2276, "BATTLE/MON.SHP")            \
    X(F_BATTLE__MONK_M_SPR, 58712, 43336, "BATTLE/MONK_M.SPR")     \
    X(F_BATTLE__MONK_W_SPR, 58734, 43195, "BATTLE/MONK_W.SPR")     \
    X(F_BATTLE__MONO_M_SPR, 58756, 44371, "BATTLE/MONO_M.SPR")     \
    X(F_BATTLE__MONO_W_SPR, 58778, 43478, "BATTLE/MONO_W.SPR")     \
    X(F_BATTLE__MUSU_SPR, 58800, 43687, "BATTLE/MUSU.SPR")         \
    X(F_BATTLE__NINJA_M_SPR, 58822, 43572, "BATTLE/NINJA_M.SPR")   \
    X(F_BATTLE__NINJA_W_SPR, 58844, 43622, "BATTLE/NINJA_W.SPR")   \
    X(F_BATTLE__ODORI_W_SPR, 58866, 43332, "BATTLE/ODORI_W.SPR")   \
    X(F_BATTLE__ONMYO_M_SPR, 58888, 43886, "BATTLE/ONMYO_M.SPR")   \
    X(F_BATTLE__ONMYO_W_SPR, 58910, 44626, "BATTLE/ONMYO_W.SPR")   \
    X(F_BATTLE__ORAN_SPR, 58932, 44368, "BATTLE/ORAN.SPR")         \
    X(F_BATTLE__ORU_SPR, 58954, 44593, "BATTLE/ORU.SPR")           \
    X(F_BATTLE__OTHER_SEQ, 57058, 2414, "BATTLE/OTHER.SEQ")        \
    X(F_BATTLE__OTHER_SHP, 57032, 2264, "BATTLE/OTHER.SHP")        \
    X(F_BATTLE__OTHER_SPR, 57124, 33792, "BATTLE/OTHER.SPR")       \
    X(F_BATTLE__RAFA_SPR, 58976, 43207, "BATTLE/RAFA.SPR")         \
    X(F_BATTLE__RAGU_SPR, 58998, 45379, "BATTLE/RAGU.SPR")         \
    X(F_BATTLE__RAMUZA_SPR, 59021, 43354, "BATTLE/RAMUZA.SPR")     \
    X(F_BATTLE__RAMUZA2_SPR, 59043, 43154, "BATTLE/RAMUZA2.SPR")   \
    X(F_BATTLE__RAMUZA3_SPR, 59065, 43009, "BATTLE/RAMUZA3.SPR")   \
    X(F_BATTLE__REZE_SPR, 59087, 44187, "BATTLE/REZE.SPR")         \
    X(F_BATTLE__REZE_D_SPR, 59109, 46744, "BATTLE/REZE_D.SPR")     \
    X(F_BATTLE__RUDO_SPR, 59132, 43817, "BATTLE/RUDO.SPR")         \
    X(F_BATTLE__RUKA_SEQ, 57060, 2482, "BATTLE/RUKA.SEQ")          \
    X(F_BATTLE__RYU_M_SPR, 59154, 44265, "BATTLE/RYU_M.SPR")       \
    X(F_BATTLE__RYU_W_SPR, 59176, 43599, "BATTLE/RYU_W.SPR")       \
    X(F_BATTLE__SAMU_M_SPR, 59198, 44235, "BATTLE/SAMU_M.SPR")     \
    X(F_BATTLE__SAMU_W_SPR, 59220, 44495, "BATTLE/SAMU_W.SPR")     \
    X(F_BATTLE__SAN_M_SPR, 59242, 44395, "BATTLE/SAN_M.SPR")       \
    X(F_BATTLE__SAN_W_SPR, 59264, 44741, "BATTLE/SAN_W.SPR")       \
    X(F_BATTLE__SERIA_SPR, 59286, 43332, "BATTLE/SERIA.SPR")       \
    X(F_BATTLE__SIMON_SPR, 59308, 45924, "BATTLE/SIMON.SPR")       \
    X(F_BATTLE__SIRO_M_SPR, 59331, 44378, "BATTLE/SIRO_M.SPR")     \
    X(F_BATTLE__SIRO_W_SPR, 59353, 47285, "BATTLE/SIRO_W.SPR")     \
    X(F_BATTLE__SOURYO_SPR, 60090, 45899, "BATTLE/SOURYO.SPR")     \
    X(F_BATTLE__SUKERU_SPR, 59377, 42442, "BATTLE/SUKERU.SPR")     \
    X(F_BATTLE__SYOU_M_SPR, 59398, 45741, "BATTLE/SYOU_M.SPR")     \
    X(F_BATTLE__SYOU_W_SPR, 59421, 44838, "BATTLE/SYOU_W.SPR")     \
    X(F_BATTLE__TETSU_SPR, 59443, 46001, "BATTLE/TETSU.SPR")       \
    X(F_BATTLE__THIEF_M_SPR, 59466, 43670, "BATTLE/THIEF_M.SPR")   \
    X(F_BATTLE__THIEF_W_SPR, 59488, 43442, "BATTLE/THIEF_W.SPR")   \
    X(F_BATTLE__TOKI_M_SPR, 59510, 44348, "BATTLE/TOKI_M.SPR")     \
    X(F_BATTLE__TOKI_W_SPR, 59532, 44543, "BATTLE/TOKI_W.SPR")     \
    X(F_BATTLE__TORI_SPR, 59554, 43332, "BATTLE/TORI.SPR")         \
    X(F_BATTLE__TORI2_SP2, 60321, 32768, "BATTLE/TORI2.SP2")       \
    X(F_BATTLE__TYPE1_SEQ, 57037, 6754, "BATTLE/TYPE1.SEQ")        \
    X(F_BATTLE__TYPE1_SHP, 57017, 8192, "BATTLE/TYPE1.SHP")        \
    X(F_BATTLE__TYPE2_SEQ, 57041, 6545, "BATTLE/TYPE2.SEQ")        \
    X(F_BATTLE__TYPE2_SHP, 57021, 8728, "BATTLE/TYPE2.SHP")        \
    X(F_BATTLE__TYPE3_SEQ, 57045, 6820, "BATTLE/TYPE3.SEQ")        \
    X(F_BATTLE__TYPE4_SEQ, 57049, 6634, "BATTLE/TYPE4.SEQ")        \
    X(F_BATTLE__URI_SPR, 59576, 40595, "BATTLE/URI.SPR")           \
    X(F_BATTLE__URI2_SP2, 60337, 32768, "BATTLE/URI2.SP2")         \
    X(F_BATTLE__VERI_SPR, 59596, 46848, "BATTLE/VERI.SPR")         \
    X(F_BATTLE__VORU_SPR, 59619, 43554, "BATTLE/VORU.SPR")         \
    X(F_BATTLE__WAJU_M_SPR, 59641, 44283, "BATTLE/WAJU_M.SPR")     \
    X(F_BATTLE__WAJU_W_SPR, 59663, 44062, "BATTLE/WAJU_W.SPR")     \
    X(F_BATTLE__WEP_SPR, 57082, 85504, "BATTLE/WEP.SPR")           \
    X(F_BATTLE__WEP1_SEQ, 57072, 2607, "BATTLE/WEP1.SEQ")          \
    X(F_BATTLE__WEP1_SHP, 57066, 5218, "BATTLE/WEP1.SHP")          \
    X(F_BATTLE__WEP2_SEQ, 57074, 2657, "BATTLE/WEP2.SEQ")          \
    X(F_BATTLE__WEP2_SHP, 57069, 5436, "BATTLE/WEP2.SHP")          \
    X(F_BATTLE__WIGU_SPR, 59685, 43748, "BATTLE/WIGU.SPR")         \
    X(F_BATTLE__YUMI_M_SPR, 59707, 43233, "BATTLE/YUMI_M.SPR")     \
    X(F_BATTLE__YUMI_W_SPR, 59729, 43107, "BATTLE/YUMI_W.SPR")     \
    X(F_BATTLE__YUREI_SPR, 59751, 41970, "BATTLE/YUREI.SPR")       \
    X(F_BATTLE__ZARU_SPR, 59772, 43521, "BATTLE/ZARU.SPR")         \
    X(F_BATTLE__ZARU2_SPR, 59794, 43521, "BATTLE/ZARU2.SPR")       \
    X(F_BATTLE__ZARUE_SPR, 59816, 47018, "BATTLE/ZARUE.SPR")       \
    X(F_BATTLE__ZARUMOU_SPR, 59839, 45897, "BATTLE/ZARUMOU.SPR")   \
    X(F_BATTLE__ZODIAC_BIN, 60513, 65536, "BATTLE/ZODIAC.BIN")     \
    X(F_EVENT__ATCHELP_LZW, 6714, 90325, "EVENT/ATCHELP.LZW")      \
    X(F_EVENT__ATTACK_OUT, 2448, 125956, "EVENT/ATTACK.OUT")       \
    X(F_EVENT__BONUS_BIN, 5824, 958464, "EVENT/BONUS.BIN")         \
    X(F_EVENT__BTLEVT_BIN, 5771, 8636, "EVENT/BTLEVT.BIN")         \
    X(F_EVENT__BUNIT_OUT, 2832, 187316, "EVENT/BUNIT.OUT")         \
    X(F_EVENT__CARD_OUT, 2768, 107260, "EVENT/CARD.OUT")           \
    X(F_EVENT__CHAPTER1_BIN, 5776, 8192, "EVENT/CHAPTER1.BIN")     \
    X(F_EVENT__CHAPTER2_BIN, 5780, 8192, "EVENT/CHAPTER2.BIN")     \
    X(F_EVENT__CHAPTER3_BIN, 5784, 8192, "EVENT/CHAPTER3.BIN")     \
    X(F_EVENT__CHAPTER4_BIN, 5788, 8192, "EVENT/CHAPTER4.BIN")     \
    X(F_EVENT__DEBUGCHR_OUT, 2512, 64756, "EVENT/DEBUGCHR.OUT")    \
    X(F_EVENT__DEBUGMAP_OUT, 2064, 0, "EVENT/DEBUGMAP.OUT")        \
    X(F_EVENT__END1_BIN, 6394, 131072, "EVENT/END1.BIN")           \
    X(F_EVENT__END2_BIN, 6458, 131072, "EVENT/END2.BIN")           \
    X(F_EVENT__END3_BIN, 6522, 131072, "EVENT/END3.BIN")           \
    X(F_EVENT__END4_BIN, 6586, 131072, "EVENT/END4.BIN")           \
    X(F_EVENT__END5_BIN, 6650, 131072, "EVENT/END5.BIN")           \
    X(F_EVENT__EQUIP_OUT, 2640, 172884, "EVENT/EQUIP.OUT")         \
    X(F_EVENT__ETC_OUT, 2576, 7548, "EVENT/ETC.OUT")               \
    X(F_EVENT__EVTCHR_BIN, 7500, 4208640, "EVENT/EVTCHR.BIN")      \
    X(F_EVENT__EVTFACE_BIN, 5707, 65536, "EVENT/EVTFACE.BIN")      \
    X(F_EVENT__EVTOOL_OUT, 2960, 0, "EVENT/EVTOOL.OUT")            \
    X(F_EVENT__FONT_BIN, 3650, 77000, "EVENT/FONT.BIN")            \
    X(F_EVENT__FRAME_BIN, 3688, 37568, "EVENT/FRAME.BIN")          \
    X(F_EVENT__GAMEOVER_BIN, 5792, 65536, "EVENT/GAMEOVER.BIN")    \
    X(F_EVENT__HELP_LZW, 7320, 92608, "EVENT/HELP.LZW")            \
    X(F_EVENT__HELPMENU_OUT, 2256, 99716, "EVENT/HELPMENU.OUT")    \
    X(F_EVENT__ITEM_BIN, 6297, 33280, "EVENT/ITEM.BIN")            \
    X(F_EVENT__JOBSTTS_OUT, 2384, 112732, "EVENT/JOBSTTS.OUT")     \
    X(F_EVENT__JOIN_LZW, 7256, 16886, "EVENT/JOIN.LZW")            \
    X(F_EVENT__MAPTITLE_BIN, 3500, 307200, "EVENT/MAPTITLE.BIN")   \
    X(F_EVENT__OPEN_LZW, 7192, 21881, "EVENT/OPEN.LZW")            \
    X(F_EVENT__OPTION_OUT, 2128, 54508, "EVENT/OPTION.OUT")        \
    X(F_EVENT__REQUIRE_OUT, 2192, 127684, "EVENT/REQUIRE.OUT")     \
    X(F_EVENT__SAMPLE_LZW, 7064, 19336, "EVENT/SAMPLE.LZW")        \
    X(F_EVENT__SMALL_OUT, 2000, 7891, "EVENT/SMALL.OUT")           \
    X(F_EVENT__SPELL_MES, 7000, 14085, "EVENT/SPELL.MES")          \
    X(F_EVENT__TEST_EVT, 3707, 4096000, "EVENT/TEST.EVT")          \
    X(F_EVENT__UNIT_BIN, 5739, 65536, "EVENT/UNIT.BIN")            \
    X(F_EVENT__WIN001_BIN, 6292, 10240, "EVENT/WIN001.BIN")        \
    X(F_EVENT__WLDFACE_BIN, 6330, 131072, "EVENT/WLDFACE.BIN")     \
    X(F_EVENT__WLDFACE4_BIN, 6314, 32768, "EVENT/WLDFACE4.BIN")    \
    X(F_EVENT__WLDHELP_LZW, 7416, 110052, "EVENT/WLDHELP.LZW")     \
    X(F_EVENT__WORLD_LZW, 7128, 58077, "EVENT/WORLD.LZW")          \
    X(F_MAP__MAP000_GNS, 10026, 208, "MAP/MAP000.GNS")             \
    X(F_MAP__MAP001_GNS, 11304, 2388, "MAP/MAP001.GNS")            \
    X(F_MAP__MAP002_GNS, 12656, 2288, "MAP/MAP002.GNS")            \
    X(F_MAP__MAP003_GNS, 12938, 568, "MAP/MAP003.GNS")             \
    X(F_MAP__MAP004_GNS, 13570, 1368, "MAP/MAP004.GNS")            \
    X(F_MAP__MAP005_GNS, 14239, 1068, "MAP/MAP005.GNS")            \
    X(F_MAP__MAP006_GNS, 14751, 1468, "MAP/MAP006.GNS")            \
    X(F_MAP__MAP007_GNS, 15030, 628, "MAP/MAP007.GNS")             \
    X(F_MAP__MAP008_GNS, 15595, 1028, "MAP/MAP008.GNS")            \
    X(F_MAP__MAP009_GNS, 16262, 1468, "MAP/MAP009.GNS")            \
    X(F_MAP__MAP010_GNS, 16347, 248, "MAP/MAP010.GNS")             \
    X(F_MAP__MAP011_GNS, 16852, 1548, "MAP/MAP011.GNS")            \
    X(F_MAP__MAP012_GNS, 17343, 1288, "MAP/MAP012.GNS")            \
    X(F_MAP__MAP013_GNS, 17627, 568, "MAP/MAP013.GNS")             \
    X(F_MAP__MAP014_GNS, 18175, 1268, "MAP/MAP014.GNS")            \
    X(F_MAP__MAP015_GNS, 19510, 1928, "MAP/MAP015.GNS")            \
    X(F_MAP__MAP016_GNS, 20075, 1128, "MAP/MAP016.GNS")            \
    X(F_MAP__MAP017_GNS, 20162, 592, "MAP/MAP017.GNS")             \
    X(F_MAP__MAP018_GNS, 20745, 1248, "MAP/MAP018.GNS")            \
    X(F_MAP__MAP019_GNS, 21411, 1148, "MAP/MAP019.GNS")            \
    X(F_MAP__MAP020_GNS, 21692, 548, "MAP/MAP020.GNS")             \
    X(F_MAP__MAP021_GNS, 22270, 1368, "MAP/MAP021.GNS")            \
    X(F_MAP__MAP022_GNS, 22938, 1368, "MAP/MAP022.GNS")            \
    X(F_MAP__MAP023_GNS, 23282, 708, "MAP/MAP023.GNS")             \
    X(F_MAP__MAP024_GNS, 23557, 528, "MAP/MAP024.GNS")             \
    X(F_MAP__MAP025_GNS, 23899, 708, "MAP/MAP025.GNS")             \
    X(F_MAP__MAP026_GNS, 23988, 248, "MAP/MAP026.GNS")             \
    X(F_MAP__MAP027_GNS, 24266, 628, "MAP/MAP027.GNS")             \
    X(F_MAP__MAP028_GNS, 24544, 528, "MAP/MAP028.GNS")             \
    X(F_MAP__MAP029_GNS, 24822, 628, "MAP/MAP029.GNS")             \
    X(F_MAP__MAP030_GNS, 25099, 588, "MAP/MAP030.GNS")             \
    X(F_MAP__MAP031_GNS, 25764, 1148, "MAP/MAP031.GNS")            \
    X(F_MAP__MAP032_GNS, 26042, 648, "MAP/MAP032.GNS")             \
    X(F_MAP__MAP033_GNS, 26229, 528, "MAP/MAP033.GNS")             \
    X(F_MAP__MAP034_GNS, 26362, 588, "MAP/MAP034.GNS")             \
    X(F_MAP__MAP035_GNS, 27028, 1148, "MAP/MAP035.GNS")            \
    X(F_MAP__MAP036_GNS, 27643, 1188, "MAP/MAP036.GNS")            \
    X(F_MAP__MAP037_GNS, 27793, 308, "MAP/MAP037.GNS")             \
    X(F_MAP__MAP038_GNS, 28467, 1228, "MAP/MAP038.GNS")            \
    X(F_MAP__MAP039_GNS, 28555, 268, "MAP/MAP039.GNS")             \
    X(F_MAP__MAP040_GNS, 29165, 988, "MAP/MAP040.GNS")             \
    X(F_MAP__MAP041_GNS, 29311, 568, "MAP/MAP041.GNS")             \
    X(F_MAP__MAP042_GNS, 29653, 668, "MAP/MAP042.GNS")             \
    X(F_MAP__MAP043_GNS, 29807, 368, "MAP/MAP043.GNS")             \
    X(F_MAP__MAP044_GNS, 30473, 1148, "MAP/MAP044.GNS")            \
    X(F_MAP__MAP045_GNS, 30622, 328, "MAP/MAP045.GNS")             \
    X(F_MAP__MAP046_GNS, 30966, 668, "MAP/MAP046.GNS")             \
    X(F_MAP__MAP047_GNS, 31697, 1488, "MAP/MAP047.GNS")            \
    X(F_MAP__MAP048_GNS, 32365, 1168, "MAP/MAP048.GNS")            \
    X(F_MAP__MAP049_GNS, 33032, 1128, "MAP/MAP049.GNS")            \
    X(F_MAP__MAP050_GNS, 33701, 1148, "MAP/MAP050.GNS")            \
    X(F_MAP__MAP051_GNS, 34349, 1328, "MAP/MAP051.GNS")            \
    X(F_MAP__MAP052_GNS, 34440, 288, "MAP/MAP052.GNS")             \
    X(F_MAP__MAP053_GNS, 34566, 648, "MAP/MAP053.GNS")             \
    X(F_MAP__MAP054_GNS, 34647, 228, "MAP/MAP054.GNS")             \
    X(F_MAP__MAP055_GNS, 34745, 468, "MAP/MAP055.GNS")             \
    X(F_MAP__MAP056_GNS, 35350, 1228, "MAP/MAP056.GNS")            \
    X(F_MAP__MAP057_GNS, 35436, 248, "MAP/MAP057.GNS")             \
    X(F_MAP__MAP058_GNS, 35519, 248, "MAP/MAP058.GNS")             \
    X(F_MAP__MAP059_GNS, 35603, 248, "MAP/MAP059.GNS")             \
    X(F_MAP__MAP060_GNS, 35683, 248, "MAP/MAP060.GNS")             \
    X(F_MAP__MAP061_GNS, 35765, 368, "MAP/MAP061.GNS")             \
    X(F_MAP__MAP062_GNS, 36052, 548, "MAP/MAP062.GNS")             \
    X(F_MAP__MAP063_GNS, 36394, 628, "MAP/MAP063.GNS")             \
    X(F_MAP__MAP064_GNS, 36530, 548, "MAP/MAP064.GNS")             \
    X(F_MAP__MAP065_GNS, 36612, 248, "MAP/MAP065.GNS")             \
    X(F_MAP__MAP066_GNS, 37214, 1108, "MAP/MAP066.GNS")            \
    X(F_MAP__MAP067_GNS, 37817, 1108, "MAP/MAP067.GNS")            \
    X(F_MAP__MAP068_GNS, 38386, 1088, "MAP/MAP068.GNS")            \
    X(F_MAP__MAP069_GNS, 38473, 228, "MAP/MAP069.GNS")             \
    X(F_MAP__MAP070_GNS, 38622, 328, "MAP/MAP070.GNS")             \
    X(F_MAP__MAP071_GNS, 39288, 1168, "MAP/MAP071.GNS")            \
    X(F_MAP__MAP072_GNS, 39826, 1088, "MAP/MAP072.GNS")            \
    X(F_MAP__MAP073_GNS, 40120, 608, "MAP/MAP073.GNS")             \
    X(F_MAP__MAP074_GNS, 40724, 968, "MAP/MAP074.GNS")             \
    X(F_MAP__MAP075_GNS, 41391, 1188, "MAP/MAP075.GNS")            \
    X(F_MAP__MAP076_GNS, 41865, 1068, "MAP/MAP076.GNS")            \
    X(F_MAP__MAP077_GNS, 42532, 1188, "MAP/MAP077.GNS")            \
    X(F_MAP__MAP078_GNS, 43200, 1228, "MAP/MAP078.GNS")            \
    X(F_MAP__MAP079_GNS, 43295, 768, "MAP/MAP079.GNS")             \
    X(F_MAP__MAP080_GNS, 43901, 1088, "MAP/MAP080.GNS")            \
    X(F_MAP__MAP081_GNS, 44569, 1128, "MAP/MAP081.GNS")            \
    X(F_MAP__MAP082_GNS, 45044, 1068, "MAP/MAP082.GNS")            \
    X(F_MAP__MAP083_GNS, 45164, 1316, "MAP/MAP083.GNS")            \
    X(F_MAP__MAP084_GNS, 45829, 1128, "MAP/MAP084.GNS")            \
    X(F_MAP__MAP085_GNS, 46498, 948, "MAP/MAP085.GNS")             \
    X(F_MAP__MAP086_GNS, 47167, 948, "MAP/MAP086.GNS")             \
    X(F_MAP__MAP087_GNS, 47260, 808, "MAP/MAP087.GNS")             \
    X(F_MAP__MAP088_GNS, 47928, 988, "MAP/MAP088.GNS")             \
    X(F_MAP__MAP089_GNS, 48595, 1128, "MAP/MAP089.GNS")            \
    X(F_MAP__MAP090_GNS, 49260, 1128, "MAP/MAP090.GNS")            \
    X(F_MAP__MAP091_GNS, 49538, 628, "MAP/MAP091.GNS")             \
    X(F_MAP__MAP092_GNS, 50108, 1088, "MAP/MAP092.GNS")            \
    X(F_MAP__MAP093_GNS, 50387, 528, "MAP/MAP093.GNS")             \
    X(F_MAP__MAP094_GNS, 50554, 448, "MAP/MAP094.GNS")             \
    X(F_MAP__MAP095_GNS, 51120, 1048, "MAP/MAP095.GNS")            \
    X(F_MAP__MAP096_GNS, 51416, 568, "MAP/MAP096.GNS")             \
    X(F_MAP__MAP097_GNS, 52082, 1108, "MAP/MAP097.GNS")            \
    X(F_MAP__MAP098_GNS, 52749, 1128, "MAP/MAP098.GNS")            \
    X(F_MAP__MAP099_GNS, 53414, 1128, "MAP/MAP099.GNS")            \
    X(F_MAP__MAP100_GNS, 53502, 228, "MAP/MAP100.GNS")             \
    X(F_MAP__MAP101_GNS, 53579, 268, "MAP/MAP101.GNS")             \
    X(F_MAP__MAP102_GNS, 53659, 228, "MAP/MAP102.GNS")             \
    X(F_MAP__MAP103_GNS, 54273, 1088, "MAP/MAP103.GNS")            \
    X(F_MAP__MAP104_GNS, 54359, 328, "MAP/MAP104.GNS")             \
    X(F_MAP__MAP105_GNS, 54528, 728, "MAP/MAP105.GNS")             \
    X(F_MAP__MAP106_GNS, 54621, 628, "MAP/MAP106.GNS")             \
    X(F_MAP__MAP107_GNS, 54716, 628, "MAP/MAP107.GNS")             \
    X(F_MAP__MAP108_GNS, 54812, 628, "MAP/MAP108.GNS")             \
    X(F_MAP__MAP109_GNS, 54909, 628, "MAP/MAP109.GNS")             \
    X(F_MAP__MAP110_GNS, 55004, 628, "MAP/MAP110.GNS")             \
    X(F_MAP__MAP111_GNS, 55097, 668, "MAP/MAP111.GNS")             \
    X(F_MAP__MAP112_GNS, 55192, 608, "MAP/MAP112.GNS")             \
    X(F_MAP__MAP113_GNS, 55286, 628, "MAP/MAP113.GNS")             \
    X(F_MAP__MAP114_GNS, 55383, 628, "MAP/MAP114.GNS")             \
    X(F_MAP__MAP115_GNS, 56051, 1128, "MAP/MAP115.GNS")            \
    X(F_MAP__MAP116_GNS, 56123, 208, "MAP/MAP116.GNS")             \
    X(F_MAP__MAP117_GNS, 56201, 208, "MAP/MAP117.GNS")             \
    X(F_MAP__MAP118_GNS, 56279, 208, "MAP/MAP118.GNS")             \
    X(F_MAP__MAP119_GNS, 56356, 208, "MAP/MAP119.GNS")             \
    X(F_MAP__MAP125_GNS, 56435, 208, "MAP/MAP125.GNS")             \
    X(F_MENU__BK_FITR_TIM, 72198, 65556, "MENU/BK_FITR.TIM")       \
    X(F_MENU__BK_FITR2_TIM, 72231, 65556, "MENU/BK_FITR2.TIM")     \
    X(F_MENU__BK_FITR3_TIM, 72264, 65556, "MENU/BK_FITR3.TIM")     \
    X(F_MENU__BK_HONE_TIM, 72099, 65556, "MENU/BK_HONE.TIM")       \
    X(F_MENU__BK_HONE2_TIM, 72132, 65556, "MENU/BK_HONE2.TIM")     \
    X(F_MENU__BK_HONE3_TIM, 72165, 65556, "MENU/BK_HONE3.TIM")     \
    X(F_MENU__BK_SHOP_TIM, 72000, 65556, "MENU/BK_SHOP.TIM")       \
    X(F_MENU__BK_SHOP2_TIM, 72033, 65556, "MENU/BK_SHOP2.TIM")     \
    X(F_MENU__BK_SHOP3_TIM, 72066, 65556, "MENU/BK_SHOP3.TIM")     \
    X(F_MENU__FFTSAVE_DAT, 72319, 8064, "MENU/FFTSAVE.DAT")        \
    X(F_MENU__TUTO1_MES, 72298, 2186, "MENU/TUTO1.MES")            \
    X(F_MENU__TUTO1_SCR, 72297, 286, "MENU/TUTO1.SCR")             \
    X(F_MENU__TUTO2_MES, 72301, 5052, "MENU/TUTO2.MES")            \
    X(F_MENU__TUTO2_SCR, 72300, 296, "MENU/TUTO2.SCR")             \
    X(F_MENU__TUTO3_MES, 72305, 5406, "MENU/TUTO3.MES")            \
    X(F_MENU__TUTO3_SCR, 72304, 723, "MENU/TUTO3.SCR")             \
    X(F_MENU__TUTO4_MES, 72309, 3103, "MENU/TUTO4.MES")            \
    X(F_MENU__TUTO4_SCR, 72308, 189, "MENU/TUTO4.SCR")             \
    X(F_MENU__TUTO5_MES, 72312, 4815, "MENU/TUTO5.MES")            \
    X(F_MENU__TUTO5_SCR, 72311, 327, "MENU/TUTO5.SCR")             \
    X(F_MENU__TUTO6_MES, 72316, 1924, "MENU/TUTO6.MES")            \
    X(F_MENU__TUTO6_SCR, 72315, 117, "MENU/TUTO6.SCR")             \
    X(F_MENU__TUTO7_MES, 72318, 725, "MENU/TUTO7.MES")             \
    X(F_MENU__TUTO7_SCR, 72317, 92, "MENU/TUTO7.SCR")              \
    X(F_OPEN__ENDING_XA, 193873, 51961856, "OPEN/ENDING.XA")       \
    X(F_OPEN__FFTEND_STR, 137480, 29540352, "OPEN/FFTEND.STR")     \
    X(F_OPEN__FFTOP_STR, 97120, 52871168, "OPEN/FFTOP.STR")        \
    X(F_OPEN__FFTPR_STR, 151904, 15400960, "OPEN/FFTPR.STR")       \
    X(F_OPEN__FFTPRE_STR, 122936, 29786112, "OPEN/FFTPRE.STR")     \
    X(F_OPEN__FFTST_STR, 86998, 20729856, "OPEN/FFTST.STR")        \
    X(F_OPEN__FFTUNIT_STR, 159424, 70551552, "OPEN/FFTUNIT.STR")   \
    X(F_OPEN__OPEN_BIN, 86000, 222832, "OPEN/OPEN.BIN")            \
    X(F_OPEN__OPNBK_BIN, 86595, 825344, "OPEN/OPNBK.BIN")          \
    X(F_OPEN__OPNTEX_BIN, 86109, 995328, "OPEN/OPNTEX.BIN")        \
    X(F_SCEAP_DAT, 198, 20480, "SCEAP.DAT")                        \
    X(F_SCUS_942_21, 24, 356352, "SCUS_942.21")                    \
    X(F_SYSTEM_CNF, 23, 68, "SYSTEM.CNF")                          \
    X(F_WORLD__SNPLBIN_BIN, 76025, 73728, "WORLD/SNPLBIN.BIN")     \
    X(F_WORLD__SNPLMES_BIN, 76061, 245760, "WORLD/SNPLMES.BIN")    \
    X(F_WORLD__WLDBK_BIN, 76181, 16097280, "WORLD/WLDBK.BIN")      \
    X(F_WORLD__WLDCORE_BIN, 84041, 448808, "WORLD/WLDCORE.BIN")    \
    X(F_WORLD__WLDMES_BIN, 73561, 5046272, "WORLD/WLDMES.BIN")     \
    X(F_WORLD__WLDPIC_BIN, 73134, 874496, "WORLD/WLDPIC.BIN")      \
    X(F_WORLD__WLDTEX_TM2, 73000, 274432, "WORLD/WLDTEX.TM2")      \
    X(F_WORLD__WORLD_BIN, 84261, 973144, "WORLD/WORLD.BIN")

// This is an enum of all files in the filesystem. This is useful for
// referencing files in the filesystem and allowing indexing into file_list
//
// Result:
//   typedef enum {
//     F_BATTLE_BIN,
//     F_EVENT__TEST_EVT,
//     F_MAP__MAP000_GNS,
//     ...
//   }
typedef enum {
#define X(name, sector, size, path) name,
    FFT_IO_INDEX
#undef X
        F_FILE_COUNT // Automatically represents the count of files
} fft_io_entry_e;

/*
================================================================================
Images
================================================================================
*/

enum {
    FFT_IMAGE_DESC_COUNT = 12, // Number of image descriptors
};

typedef enum {
    FFT_IMAGETYPE_4BPP,
    FFT_IMAGETYPE_4BPP_PAL,
    FFT_IMAGETYPE_8BPP,
    FFT_IMAGETYPE_16BPP,
} fft_image_type_e;

typedef struct {
    uint32_t width;
    uint32_t height;
    size_t size;
    uint8_t* data;
    bool valid;
} fft_image_t;

typedef struct {
    char* name;
    fft_io_entry_e entry;
    fft_image_type_e type;

    uint32_t width;
    uint32_t height;

    size_t data_offset;
    size_t data_length;

    size_t pal_offset;
    size_t pal_length;
    uint32_t pal_count;
    uint32_t pal_default;

    uint32_t repeat;
    uint32_t repeat_offset;
} fft_image_desc_t;

// clang-format off
const fft_image_desc_t image_desc_list[FFT_IMAGE_DESC_COUNT] = {
    { .name = "BONUS.BIN",    .entry = F_EVENT__BONUS_BIN,    .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 200, .pal_offset = 25600, .pal_count = 6,  .repeat = 36,  .repeat_offset = 26624 },
    { .name = "CHAPTER1.BIN", .entry = F_EVENT__CHAPTER1_BIN, .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "CHAPTER2.BIN", .entry = F_EVENT__CHAPTER2_BIN, .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "CHAPTER3.BIN", .entry = F_EVENT__CHAPTER3_BIN, .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "CHAPTER4.BIN", .entry = F_EVENT__CHAPTER4_BIN, .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "EVTCHR.BIN",   .entry = F_EVENT__EVTCHR_BIN,   .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 200, .pal_offset = 1920,  .pal_count = 7,  .repeat = 137, .repeat_offset = 30720, .data_offset = 2560 },
    { .name = "FRAME.BIN",    .entry = F_EVENT__FRAME_BIN,    .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 288, .pal_offset = 36864, .pal_count = 22, .pal_default = 5 },
    { .name = "ITEM.BIN",     .entry = F_EVENT__ITEM_BIN,     .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 256, .pal_offset = 32768, .pal_count = 16 },
    { .name = "UNIT.BIN",     .entry = F_EVENT__UNIT_BIN,     .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 480, .pal_offset = 61440, .pal_count = 128 },
    { .name = "WLDFACE.BIN",  .entry = F_EVENT__WLDFACE_BIN,  .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 240, .pal_offset = 30720, .pal_count = 64, .repeat = 4,   .repeat_offset = 32768 },
    { .name = "WLDFACE4.BIN", .entry = F_EVENT__WLDFACE4_BIN, .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 240, .pal_offset = 30720, .pal_count = 64 },

    { .name = "OTHER.SPR",    .entry = F_BATTLE__OTHER_SPR,   .type = FFT_IMAGETYPE_4BPP_PAL, .width = 256, .height = 256, .pal_offset = 0,     .pal_count = 32, .data_offset = 1024 },
};
// clang-format on

static fft_image_desc_t image_get_desc(fft_io_entry_e entry);

#ifdef __cplusplus
}
#endif

#endif // FFT_H

/*
================================================================================
                                 IMPLEMENTATION

                                 IMPLEMENTATION

                                 IMPLEMENTATION
================================================================================
*/

#ifdef FFT_IMPLEMENTATION

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
================================================================================
Global State
================================================================================
*/

// WARNING: This is not thread-safe and should be used in a single-threaded context.
static struct {
    struct {
        FILE* file; // The main file handle for the FFT binary.
    } io;

    struct {
        size_t usage_peak;
        size_t usage_total;
        size_t usage_current;
        size_t allocations_total;
        size_t allocations_current;
    } mem;
} _fft_state;

/*
================================================================================
Defines
================================================================================
*/

// You can define these to change the memory allocation functions used by the library.
//
// #define FFT_MEM_ALLOC(size) malloc(size)
// #define FFT_MEM_ALLOC_TAG(size, tag) malloc(size)
// #define FFT_MEM_FREE(ptr) free(ptr)
#ifndef FFT_MEM_ALLOC
#    define FFT_MEM_ALLOC(size)          fft_mem_alloc(size, __FILE__, __LINE__, NULL);
#    define FFT_MEM_ALLOC_TAG(size, tag) fft_mem_alloc(size, __FILE__, __LINE__, tag);
#    define FFT_MEM_FREE(ptr)            fft_mem_free(ptr)
#endif

// Macros for min/max operations.
#define FFT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define FFT_MIN(a, b) ((a) < (b) ? (a) : (b))

// Bytes to kilobytes and megabytes.
#define FFT_BYTES_TO_KB(x) ((double)(x) / 1024.0)
#define FFT_BYTES_TO_MB(x) ((double)(x) / (1024.0 * 1024.0))

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

/*
================================================================================
Span Implementation
================================================================================
*/

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

/*
================================================================================
Memory Implementation
================================================================================
*/

typedef struct fft_mem_alloc_header_t {
    size_t size;
    int32_t line;
    const char* file;
    const char* tag;
    struct fft_mem_alloc_header_t* next;
} fft_mem_alloc_header_t;

static fft_mem_alloc_header_t* allocations_head = NULL;

static void fft_mem_init(void) {
    _fft_state.mem.usage_peak = 0;
    _fft_state.mem.usage_total = 0;
    _fft_state.mem.usage_current = 0;
    _fft_state.mem.allocations_total = 0;
    _fft_state.mem.allocations_current = 0;
}

static void fft_mem_shutdown(void) {
    if (_fft_state.mem.allocations_current != 0) {
        printf("Memory leak detected: %zu allocations remaining\n", _fft_state.mem.allocations_current);
        fft_mem_alloc_header_t* current = allocations_head;
        while (current) {
            printf("Leaked %zu bytes allocated from %s:%d\n", current->size, current->file, current->line);
            if (current->tag != NULL) {
                printf("\tTag: %s\n", current->tag);
            }
            current = current->next;
        }
    }

    if (_fft_state.mem.usage_current != 0) {
        printf("Memory leak detected: %zu bytes remaining\n", _fft_state.mem.usage_current);
        printf("Memory usage peak: %0.2fMB\n", FFT_BYTES_TO_MB(_fft_state.mem.usage_peak));
        printf("Memory usage total: %0.2fMB\n", FFT_BYTES_TO_MB(_fft_state.mem.usage_total));
        printf("Memory allocations: %zu\n", _fft_state.mem.allocations_total);
    }
}

static void* fft_mem_alloc(size_t size, const char* file, int32_t line, const char* tag) {
    fft_mem_alloc_header_t* header = calloc(1, sizeof(fft_mem_alloc_header_t) + size);
    FFT_ASSERT(header != NULL, "Failed to allocate memory");

    header->size = size;
    header->file = file;
    header->line = line;
    header->tag = tag;

    header->next = allocations_head;
    allocations_head = header;

    _fft_state.mem.usage_current += size;
    _fft_state.mem.usage_peak = FFT_MAX(_fft_state.mem.usage_peak, _fft_state.mem.usage_current);
    _fft_state.mem.usage_total += size;
    _fft_state.mem.allocations_total++;
    _fft_state.mem.allocations_current++;

    return (void*)(header + 1);
}

static void fft_mem_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    fft_mem_alloc_header_t* header = ((fft_mem_alloc_header_t*)ptr) - 1;

    // Remove from the linked list
    fft_mem_alloc_header_t** current = &allocations_head;
    while (*current) {
        if (*current == header) {
            *current = header->next;
            break;
        }
        current = &((*current)->next);
    }

    _fft_state.mem.allocations_current--;
    _fft_state.mem.usage_current -= header->size;

    free(header);
}

/*
================================================================================
IO/Filesystem Implementation
================================================================================
*/

enum {
    FFT_IO_SECTOR_SIZE = 2048,
    FFT_IO_SECTOR_SIZE_RAW = 2352,
    FFT_IO_SECTOR_HEADER_SIZE = 24,
};

typedef struct {
    uint32_t sector;
    uint32_t size;
    char* name;
} fft_io_desc_t;

// This is a list of description for all files in the filesystem.
//
// Result:
//   const file_desc_t file_list[F_FILE_COUNT] = {
//     [F_BATTLE_BIN] =      { .sector = 1000,  .size = 1397096, .name = "BATTLE.BIN"},
//     [F_EVENT__TEST_EVT] = { .sector = 3707,  .size = 4096000, .name = "EVENT/TEST.EVT"},
//     [F_MAP__MAP001_GNS] = { .sector = 11304, .size = 2388,    .name = "MAP/MAP000.GNS"},
//     ...
//   }
const fft_io_desc_t fft_io_file_list[F_FILE_COUNT] = {
#define X(oname, osector, osize, opath) [oname] = { .sector = osector, .size = osize, .name = opath },
    FFT_IO_INDEX
#undef X
};

static void fft_io_init(const char* filename) {
    _fft_state.io.file = fopen(filename, "rb");
    FFT_ASSERT(_fft_state.io.file != NULL, "Failed to open fft.bin");
}

static void fft_io_shutdown(void) {
    fclose(_fft_state.io.file);
}

static fft_io_desc_t fft_io_get_file_desc(uint32_t sector_start) {
    for (size_t i = 0; i < F_FILE_COUNT; i++) {
        if (fft_io_file_list[i].sector == sector_start) {
            return fft_io_file_list[i];
        }
    }
    return (fft_io_desc_t) { .sector = 0, .size = 0, .name = NULL };
}

static fft_span_t fft_io_read(uint32_t sector_start, uint32_t size) {
    uint32_t offset = 0;
    uint32_t occupied_sectors = (uint32_t)ceil(size / (double)FFT_IO_SECTOR_SIZE);

    // Try to get the file descriptor to tag the allocation with a filename.
    fft_io_desc_t desc = fft_io_get_file_desc(sector_start);
    uint8_t* bytes = NULL;
    if (desc.sector == 0 && desc.size == 0 && desc.name == NULL) {
        bytes = FFT_MEM_ALLOC(size);
    } else {
        bytes = FFT_MEM_ALLOC_TAG(size, desc.name);
    }

    for (uint32_t i = 0; i < occupied_sectors; i++) {
        int32_t seek_to = (int32_t)((sector_start + i) * FFT_IO_SECTOR_SIZE_RAW) + FFT_IO_SECTOR_HEADER_SIZE;
        int32_t sn = fseek(_fft_state.io.file, seek_to, SEEK_SET);
        FFT_ASSERT(sn == 0, "Failed to seek to sector");

        uint8_t sector[FFT_IO_SECTOR_SIZE];
        size_t rn = fread(sector, sizeof(uint8_t), FFT_IO_SECTOR_SIZE, _fft_state.io.file);
        FFT_ASSERT(rn == FFT_IO_SECTOR_SIZE, "Failed to read correct number of bytes from sector");

        size_t remaining_size = size - (size_t)offset;
        size_t bytes_to_copy = (remaining_size < FFT_IO_SECTOR_SIZE) ? remaining_size : FFT_IO_SECTOR_SIZE;

        memcpy(bytes + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return (fft_span_t) {
        .data = bytes,
        .size = size,
    };
}

static fft_span_t fft_io_open(fft_io_entry_e file) {
    fft_io_desc_t desc = fft_io_file_list[file];
    return fft_io_read(desc.sector, desc.size);
}

static void fft_io_close(fft_span_t span) {
    FFT_MEM_FREE((void*)span.data);
    return;
}

/*
================================================================================
Map state Implementation
================================================================================
*/

static fft_state_t fft_default_state = (fft_state_t) {
    .time = FFT_TIME_DAY,
    .weather = FFT_WEATHER_NONE,
    .layout = 0,
};

const char* fft_time_str(fft_time_e value) {
    switch (value) {
    case FFT_TIME_DAY:
        return "Day";
    case FFT_TIME_NIGHT:
        return "Night";
    default:
        return "Unknown";
    }
}

const char* fft_weather_str(fft_weather_e value) {
    switch (value) {
    case FFT_WEATHER_NONE:
        return "None";
    case FFT_WEATHER_NONE_ALT:
        return "NoneAlt";
    case FFT_WEATHER_NORMAL:
        return "Normal";
    case FFT_WEATHER_STRONG:
        return "Strong";
    case FFT_WEATHER_VERY_STRONG:
        return "VeryStrong";
    default:
        return "Unknown";
    }
}

bool fft_state_is_equal(fft_state_t a, fft_state_t b) {
    return a.time == b.time && a.weather == b.weather && a.layout == b.layout;
}

bool fft_state_is_default(fft_state_t map_state) {
    return fft_state_is_equal(fft_default_state, map_state);
}

/*
================================================================================
GNS/Records Implementation
================================================================================
*/

static fft_record_t fft_record_read(fft_span_t* span) {
    uint16_t aa = fft_span_read_u16(span);             // 0
    uint8_t layout = fft_span_read_u8(span);           // 1-2
    uint8_t time_and_weather = fft_span_read_u8(span); // 3
    fft_recordtype_e type = fft_span_read_u16(span);   // 4-5
    uint16_t ee = fft_span_read_u16(span);             // 6-7
    uint32_t sector = fft_span_read_u16(span);         // 8-9
    uint16_t gg = fft_span_read_u16(span);             // 10-11
    uint32_t length = fft_span_read_u32(span);         // 12-15
    uint16_t ii = fft_span_read_u16(span);             // 16-17
    uint16_t jj = fft_span_read_u16(span);             // 18-19

    // Split time and weather from single byte.
    fft_time_e time = (fft_time_e)((time_and_weather >> 7) & 0x1);
    fft_weather_e weather = (fft_weather_e)((time_and_weather >> 4) & 0x7);

    fft_record_t record = {
        .type = type,
        .sector = sector,
        .length = length,
        .state = {
            .time = time,
            .weather = weather,
            .layout = layout,
        },
        .aa = aa,
        .ee = ee,
        .gg = gg,
        .ii = ii,
        .jj = jj,
    };
    return record;
}

static uint32_t fft_record_read_all(fft_span_t* span, fft_record_t* out_records) {
    uint32_t count = 0;
    while (span->offset + 20 < span->size) {
        fft_record_t record = fft_record_read(span);
        if (record.type == FFT_RECORDTYPE_END) {
            // End of records, stop reading.
            break;
        }
        out_records[count++] = record;
    }
    return count;
}

const char* fft_recordtype_str(fft_recordtype_e value) {
    switch (value) {
    case FFT_RECORDTYPE_MESH_PRIMARY:
        return "Primary";
    case FFT_RECORDTYPE_MESH_ALT:
        return "Alt";
    case FFT_RECORDTYPE_MESH_OVERRIDE:
        return "Override";
    case FFT_RECORDTYPE_TEXTURE:
        return "Texture";
    case FFT_RECORDTYPE_END:
        return "End";
    default:
        return "Unknown";
    }
}

/*
================================================================================
Images Implementation
================================================================================
*/

enum {
    FFT_IMAGE_PAL_COL_COUNT = 16,                         // Each palette entry is 16 bytes (RGBA)
    FFT_IMAGE_PAL_ROW_SIZE = FFT_IMAGE_PAL_COL_COUNT * 4, // 4 bytes per color
};

// This function reads the 4bpp data from the span and converts it to a 32bpp image.
//
// The resulting image will be grayscale, with each pixel represented by four bytes (RGBA).
// The pixel values will be used in a to look up the actual color in a palette (CLUT).
static fft_image_t fft_image_read_4bpp(fft_span_t* span, uint32_t width, uint32_t height) {
    const uint32_t dims = (width * height);
    const uint32_t size = dims * 4;
    const uint32_t size_on_disk = dims / 2; // two pixels per byte

    uint8_t* data = FFT_MEM_ALLOC(size);

    uint32_t write_idx = 0;
    for (uint32_t i = 0; i < size_on_disk; i++) {
        uint8_t raw_pixel = fft_span_read_u8(span);

        uint8_t right = (raw_pixel & 0x0F);
        uint8_t left = (raw_pixel & 0xF0) >> 4;

        // Repeat each pixel 4 times to convert from 4bpp to 32bpp.
        for (uint32_t j = 0; j < 4; j++) {
            data[write_idx++] = right;
        }

        for (uint32_t j = 0; j < 4; j++) {
            data[write_idx++] = left;
        }
    }

    fft_image_t image = { 0 };
    image.width = width;
    image.height = height;
    image.data = data;
    image.size = size;
    image.valid = true;

    return image;
}

static fft_image_t fft_image_read_16bpp(fft_span_t* span, uint32_t width, uint32_t height) {
    const uint32_t dims = width * height;
    const uint32_t size = dims * 4;

    uint8_t* data = FFT_MEM_ALLOC(size);

    uint32_t write_idx = 0;
    for (uint32_t i = 0; i < dims; i++) {
        uint16_t val = fft_span_read_u16(span);

        data[write_idx++] = (uint8_t)((val & 0x001F) << 3);      // R
        data[write_idx++] = (uint8_t)((val & 0x03E0) >> 2);      // G
        data[write_idx++] = (uint8_t)((val & 0x7C00) >> 7);      // B
        data[write_idx++] = (uint8_t)((val == 0) ? 0x00 : 0xFF); // A
    }

    fft_image_t image = { 0 };
    image.width = width;
    image.height = height;
    image.data = data;
    image.size = size;
    image.valid = true;

    return image;
}

// Take a 4bpp image and a 16bpp palette (CLUT) and convert the image to a
// palettized format.
static void fft_image_palettize(fft_image_t* image, const fft_image_t* palette, uint8_t pal_index) {
    const uint32_t pixel_count = image->width * image->height;
    const uint32_t pal_offset = (FFT_IMAGE_PAL_ROW_SIZE * pal_index);

    for (uint32_t i = 0; i < pixel_count * 4; i = i + 4) {
        uint8_t pixel = image->data[i];
        memcpy(&image->data[i], &palette->data[pal_offset + (pixel * 4)], 4);
    }
}

static fft_image_t fft_image_read_4bpp_palettized(fft_span_t* span, fft_image_desc_t desc, uint8_t pal_index) {
    // Read the 4bpp image data.
    fft_image_t image = fft_image_read_4bpp(span, desc.width, desc.height);

    // Read the palette (CLUT) data.
    span->offset = desc.pal_offset;
    fft_image_t palette = fft_image_read_16bpp(span, FFT_IMAGE_PAL_COL_COUNT, desc.pal_count);

    fft_image_palettize(&image, &palette, pal_index);

    // Free the palette data.
    FFT_MEM_FREE(palette.data);

    return image;
}

static fft_image_desc_t image_get_desc(fft_io_entry_e entry) {
    fft_image_desc_t found = { 0 };
    for (uint32_t i = 0; i < FFT_IMAGE_DESC_COUNT; i++) {
        if (image_desc_list[i].entry == entry) {
            // FIXME: This is just a development saftey measure. Might be better
            // to find by name since there can be multiple desc per file.
            FFT_ASSERT(found.entry == 0, "Duplicate image descriptor for entry %d", entry);
            found = image_desc_list[i];
        }
    }
    FFT_ASSERT(found.entry != 0, "Image descriptor not found for entry %d", entry);
    return found;
}

// This will scale the image data from 4bpp to 32bpp by multiplying each pixel
// value by 16. This is useful for debugging since the max value of 4bpp is 15,
// and we want to scale it to 255 (15*17 = 255).
static void fft_image_scale_paletted(fft_image_t* image) {
    if (!image || !image->valid || !image->data) {
        return;
    }

    const uint32_t pixel_count = image->width * image->height;

    for (uint32_t i = 0; i < pixel_count; i++) {
        uint8_t* px = &image->data[i * 4];

        // Scale each RGB channel from [0..15] → [0..255]
        px[0] = px[0] * 17; // R
        px[1] = px[1] * 17; // G
        px[2] = px[2] * 17; // B
        px[3] = 0xFF;       // A
    }
}

static bool fft_image_write_ppm(const fft_image_t* image, const char* path) {
    if (!image || !image->valid || !image->data) {
        return false;
    }

    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }

    // Write PPM header (P6 = binary RGB)
    // Max color value is 255
    fprintf(f, "P6\n%d %d\n255\n", image->width, image->height);

    // Write RGB data (ignore alpha)
    for (uint32_t y = 0; y < image->height; y++) {
        for (uint32_t x = 0; x < image->width; x++) {
            uint8_t* px = &image->data[(y * image->width + x) * 4];
            fwrite(px, 1, 3, f); // only write R,G,B
        }
    }

    fclose(f);
    return true;
}
/*
================================================================================
Entrypoint Implementation
================================================================================
 */

void fft_init(const char* filename) {
    fft_mem_init();
    fft_io_init(filename);
}

void fft_shutdown(void) {
    fft_io_shutdown();
    fft_mem_shutdown();
}

#endif // FFT_IMPLEMENTATION
