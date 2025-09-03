#ifndef FFT_H
#define FFT_H


/*
SPDX-License-Identifier: BSD-2-Clause
Copyright (c) 2025 Adam Patterson

================================================================================
libFFT - A Final Fantasy Tactics Library
================================================================================

For the latest version: - https://github.com/adamrt/libfft

❤️ This project is an ode to FFHacktics. This library would not be possible
without their amazing work (https://ffhacktics.com/wiki/).

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
    then in a single translation unit (C file), define `FFT_IMPLEMENTATION`
    before including the header. You can include the file without defining
    `FFT_IMPLEMENTATION` in multiple translation units to use the library.

        ```c
        #include <stdio.h>

        #define FFT_IMPLEMENTATION
        #include "fft.h"

        int main() {
            fft_init("my_fft_file.bin");
            fft_do_thing(...);
            fft_shutdown();
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
about the users intentions. For instance, we keep the original types where
possible, like using i16 for vertex position data instead of casting to the more
common f32. We use custom types for fixed-point math to inform the user of the
intended use in the game. We store that data in its original format but provide
helper functions and macros to convert to commonly desired types.

One exception where we don't stick to the original types is when there are
multiple values in a single byte. We typically split these into separate fields.
This makes accessing fields easier at the expense of a little more memory.

For example:

    Polygon Tile Locations:
    +------+------+-----------+
    | Bits | Type | Purpose   |
    |------+------+-----------+
    | 7    | uint | Z coord   |
    | 1    | N/A  | Elevation |
    | 8    | uint | X coord   |
    +------+------+-----------+

Our data structure splits the values from the first byte into two separate fields:

    ```c
    typedef struct {
        u8 x;
        u8 z;
        u8 elevation;
    } fft_tileinfo_t;
    ```

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

Fixed-point types are used in FFT for various data like vertex normals and some
colors. This library doesn't use fixed-point math internally, but provides types
and conversion functions to help users interpret the data correctly. A common
way to use this data, for instance on vertex normals, is to convert to a f32 and
divide by the fixed point's 1.0 value, which for fft_fixed16_t is 4096.0f.

================================================================================
*/

// Fixed-point types for FFT
typedef int16_t fft_fixed16_t;

// The value of 1.0 in fixed-point format.
static const float FFT_FIXED16_ONE = 4096.0f;

float fft_fixed16_to_f32(fft_fixed16_t value);

/*
================================================================================
Span
================================================================================

Span is a structure and set of functions that represents a contiguous block of
data in memory. A span is used to represent a file or part of a file in the FFT
BIN filesystem.

The related functions allow simple reading of specific datatypes. The functions
are the defacto way to read data from the FFT BIN filesystem.

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
// BIN file. They are stored in this macro so we can generate the enum and the
// lists without having to duplicate and manage all the entires in two places.
//
// NOTE: Some files are not included in this list to keep it more manageable:
//   - Non-GNS Map files: Sector and size can be determined from the GNS files.
//   - EFFECT/*: We don't use EFFECTS yet and the list is huge.
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

// This is an enum of (almost) all files in the filesystem. This is useful for
// referencing files in the filesystem and allowing indexing into file_list.
//
// typedef enum {
//     F_BATTLE_BIN,
//     F_EVENT__TEST_EVT,
//     F_MAP__MAP000_GNS,
//     ...
// } fft_io_entry_e;
//
typedef enum {
#define X(name, sector, size, path) name,
    FFT_IO_INDEX
#undef X
        F_FILE_COUNT // Automatically represents the count of files
} fft_io_entry_e;

/*
================================================================================
Map state
================================================================================

The map state represents the specific state of a map. A map typically has
multiple meshes and textures for use in different events and random battles. The
state is used to determine which resources to load based on the time of day,
weather and layout. Each resource has a specified state.

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
records (max 40 records). These records describe the state of the map. They have
a type (fft_recordtype_e), which specifies if its a texture, mesh data, etc.
They also have the weather/time/layout they are valid for. Then the location
(sector) and size in the BIN file for that data.

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

// This is used to store metadata for the record, after reading the related
// file. This is only relevant for mesh files because textures have no meta
// data. This struct is attached to the fft_mesh_t and the fft_record_t.
// It is mostly to be able to show each record in the UI with some metadata.
typedef struct {
    bool has_geometry;
    bool has_clut;
    bool has_lighting;
    bool has_terrain;

    // Geometry
    uint16_t polygon_count;
    uint16_t tex_tri_count;    // Number of textured triangles
    uint16_t tex_quad_count;   // Number of textured quads
    uint16_t untex_tri_count;  // Number of untextured triangles
    uint16_t untex_quad_count; // Number of untextured quads

    // Lighting
    uint8_t light_count;
} fft_record_meta_t;

typedef struct {
    fft_recordtype_e type;
    fft_state_t state;
    uint32_t sector;
    uint32_t length;

    // Padding or unknown fields
    uint16_t unknown_aa;
    uint16_t unknown_ee;
    uint16_t unknown_gg;
    uint16_t unknown_ii;
    uint16_t unknown_jj;

    uint8_t raw[FFT_RECORD_SIZE]; // Raw data for debugging

    fft_record_meta_t meta;
} fft_record_t;

/*
================================================================================
Colors
================================================================================

There a multiple different color types in the game data. This was done to strike
a balance between precision and memory usage.

There are these types in the game data:
- 4BPP (4-bit per pixel)
- 5551 (RGB555 + 1-bit alpha)
- FX16 (16-bit per channel fixed-point RGB)
- RGB8 (8-bit per channel RGB)

This one was added by us to have a consistent type to export to:
- RGBA8 (packed 32-bit RGBA)

================================================================================
*/

// === fft_color_4bpp_t
//
// This is a 4-bit per pixel. It is commonly used for map textures and sprite
// sheets. This allow 16 values per pixel. Then the CLUT (Color Look-Up Table)
// have 16 colors that can be indexed by the pixel values.
typedef uint8_t fft_color_4bpp_t; // two 4-bit pixels

fft_color_4bpp_t fft_color_4bpp_read(fft_span_t* span);

uint8_t fft_color_4bpp_left(fft_color_4bpp_t px);
uint8_t fft_color_4bpp_right(fft_color_4bpp_t px);

// === fft_color_5551_t
//
// This is a 16-bit BGR555 + 1-bit alpha format. They are used for CLUTs (Color
// Look-Up Table), which are palettes for map textures and sprites.
typedef uint16_t fft_color_5551_t; // ABBBBBGGGGGRRRRR

fft_color_5551_t fft_color_5551_read(fft_span_t* span);

// These all scale the 5-bit values to 8-bit values (0-31 -> 0-255).
uint8_t fft_color_5551_r8(fft_color_5551_t c);
uint8_t fft_color_5551_g8(fft_color_5551_t c);
uint8_t fft_color_5551_b8(fft_color_5551_t c);
uint8_t fft_color_5551_a8(fft_color_5551_t c);

bool fft_color_5551_is_transparent(fft_color_5551_t color);

// === fft_color_rgbfx16_t
//
// This is a 48-bit RGB fixed-point format. This color is used for
// lighting colors and possibly other places that require higher precision colors.
typedef struct {
    fft_fixed16_t r;
    fft_fixed16_t g;
    fft_fixed16_t b;
} fft_color_rgbfx16_t;

fft_color_rgbfx16_t fft_color_rgbfx16_read(fft_span_t* span);
float fft_color_rgbfx16_r8(fft_color_rgbfx16_t c);
float fft_color_rgbfx16_g8(fft_color_rgbfx16_t c);
float fft_color_rgbfx16_b8(fft_color_rgbfx16_t c);

// === fft_color_rgb8_t
//
// This is a 24-bit RGB888 format. This color is used for backgrounds, ambient
// light, and possibly other places that require standard colors.
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} fft_color_rgb8_t;

fft_color_rgb8_t fft_color_rgb8_read(fft_span_t* span);

// === fft_color_t
//
// This is a 32-bit RGBA8888 packed format. Each component is scaled to 0-255.
// This is not used in the game data, but we have it to represent colors in a
// standard way for rendering and processing.
typedef uint32_t fft_color_t; // RGBA8888 packed

fft_color_t fft_color_from_5551(fft_color_5551_t c);
fft_color_t fft_color_from_rgbfx16(fft_color_rgbfx16_t c);
fft_color_t fft_color_from_rgb8(fft_color_rgb8_t c);

#define FFT_COLOR_RGBA(r, g, b, a) \
    ((((uint32_t)(a) & 0xFF) << 24) | (((uint32_t)(b) & 0xFF) << 16) | (((uint32_t)(g) & 0xFF) << 8) | (((uint32_t)(r) & 0xFF) << 0))

/*
================================================================================
CLUT
================================================================================

A CLUT (Color Look-Up Table) is a palette of colors used for 4bpp and 8bpp
images. Map textures and sprites often use CLUTs to define the colors used in
the image. Each CLUT is a set of colors that can be indexed by the image data.

================================================================================

*/

enum {
    FFT_CLUT_ROW_WIDTH = 16, // Number of colors in a row of a CLUT
    FFT_CLUT_ROW_COUNT = 16, // Number of rows in a CLUT
};

typedef struct {
    fft_color_5551_t colors[FFT_CLUT_ROW_WIDTH];
} fft_clut_row_t;

typedef struct {
    fft_clut_row_t rows[FFT_CLUT_ROW_COUNT];
} fft_clut_t;

fft_clut_row_t fft_clut_row_read(fft_span_t* span);
fft_clut_t fft_clut_read(fft_span_t* span);

/*
================================================================================
Images
================================================================================

Images can represent textures, sprites, or other graphical assets in the game.

They are RGBA8.

================================================================================
*/

enum {
    FFT_IMAGE_DESC_COUNT = 12, // Number of image descriptors
};

typedef enum {
    FFT_IMAGETYPE_4BPP,
    FFT_IMAGETYPE_5551,
    FFT_IMAGETYPE_RGB8,
    FFT_IMAGETYPE_FX16,
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

extern const fft_image_desc_t image_desc_list[FFT_IMAGE_DESC_COUNT];

static fft_image_desc_t image_get_desc(fft_io_entry_e entry);

/*
================================================================================
Mesh Header
================================================================================

There is a header at the beginning of each mesh file that is 196 bytes long. It
contains intra-file u32 pointers to various chunks of data in the mesh file. If
the value is 0, it means that the chunk is not present in the file.

The gaps in the 196 bytes are filled with 0x00 for every mesh file.

Reference: https://ffhacktics.com/wiki/Maps/Mesh#Header

================================================================================
*/

enum {
    FFT_MESH_HEADER_SIZE = 196, // Total size of mesh header in bytes
};

// These constants document the binary file offsets for each field in the mesh
// header. They are kept for reference but are not used by the reading code,
// which reads the header sequentially as a complete 196-byte structure.
enum {
    FFT_MESH_PTR_GEOMETRY = 0x40,
    FFT_MESH_PTR_CLUT_COLOR = 0x44,
    FFT_MESH_PTR_LIGHT_AND_BACKGROUND = 0x64,
    FFT_MESH_PTR_TERRAIN = 0x68,
    FFT_MESH_PTR_TEXTURE_ANIM_INST = 0x6c,
    FFT_MESH_PTR_PALETTE_ANIM_INST = 0x70,
    FFT_MESH_PTR_CLUT_GRAY = 0x7c,
    FFT_MESH_PTR_MESH_ANIM_INST = 0x8c,
    FFT_MESH_PTR_ANIM_MESH_1 = 0x90,
    FFT_MESH_PTR_ANIM_MESH_2 = 0x94,
    FFT_MESH_PTR_ANIM_MESH_3 = 0x98,
    FFT_MESH_PTR_ANIM_MESH_4 = 0x9c,
    FFT_MESH_PTR_ANIM_MESH_5 = 0xa0,
    FFT_MESH_PTR_ANIM_MESH_6 = 0xa4,
    FFT_MESH_PTR_ANIM_MESH_7 = 0xa8,
    FFT_MESH_PTR_ANIM_MESH_8 = 0xac,
    FFT_MESH_PTR_POLY_RENDER_PROPS = 0xb0,
};

typedef struct {
    uint32_t geometry;              // 0x40: Primary Meshes
    uint32_t clut_color;            // 0x44: Color Texture Palettes
    uint32_t lights_and_background; // 0x64: Light Colors/Positions
    uint32_t terrain;               // 0x68: Terrain Data
    uint32_t texture_anim_inst;     // 0x6C: Texture Animation Instructions
    uint32_t palette_anim_inst;     // 0x70: Palette Animation Instructions
    uint32_t clut_gray;             // 0x7C: Grayscale Texture Palettes
    uint32_t mesh_anim_inst;        // 0x8C: Meshes Animation Instructions
    uint32_t anim_mesh_1;           // 0x90: Animated Mesh 1
    uint32_t anim_mesh_2;           // 0x94: Animated Mesh 2
    uint32_t anim_mesh_3;           // 0x98: Animated Mesh 3
    uint32_t anim_mesh_4;           // 0x9C: Animated Mesh 4
    uint32_t anim_mesh_5;           // 0xA0: Animated Mesh 5
    uint32_t anim_mesh_6;           // 0xA4: Animated Mesh 6
    uint32_t anim_mesh_7;           // 0xA8: Animated Mesh 7
    uint32_t anim_mesh_8;           // 0xAC: Animated Mesh 8
    uint32_t poly_render_props;     // 0xB0: Polygon Render Properties

    uint32_t unknown_48;           // 0x48: unknown pointer
    uint32_t unknown_4c;           // 0x4C: Unknown pointer (only non-zero in MAP000.5)
    uint32_t unknown_00_to_40[16]; // 0x00-0x3F: unknown pointers
    uint32_t unknown_50_to_64[5];  // 0x50-0x63: unknown pointers
    uint32_t unknown_74_to_7c[2];  // 0x74-0x7B: unknown pointers
    uint32_t unknown_80_to_8c[3];  // 0x80-0x8B: unknown pointers
    uint32_t unknown_b4_to_c4[4];  // 0xB4-0xC3: unknown pointers
} fft_mesh_header_t;

fft_mesh_header_t fft_mesh_header_read(fft_span_t* span);

/*
================================================================================
Geometry
================================================================================

Geometry is the map's polygons, vertices, tiles and uv/clut data. It is the
first chuck of a mesh file after the header.

This is required for the default state, but is optional for all other states.

The polygons are stored on-disk in the following order:
  - Textured triangles
  - Textured quadrilaterals
  - Untextured triangles
  - Untextured quadrilaterals

We parse them into a single array of polygons. We differentiate between them
with fft_polytype_e (FFT_POLYTYPE_TRIANGLE and FFT_POLYTYPE_QUAD).

The untextured polygons should be shown as black and are often the map sides.

The Geometry section of the mesh has 6 parts

=== Header

The header contains 4x u16 with the number of each type of polyon.

+------------------+-----+
| Type             | MAX |
+------------------+-----+
| Textured tris    | 512 |
| Textured quads   | 768 |
| Untextured tris  | 64  |
| Untextured quads | 256 |
+------------------+-----+

=== Position

  The position data is stored as int16_t for X, Y and Z. The coordinates system
  is like so:

    - X+ = right
    - Y+ = down // This is non-standard for modern 3D engines, but is how FFT works.
    - Z+ = forward

=== Normals

  The normal data is stored as fft_fixed16_t. The float version of this can be
  derived by casting and dividing it by 4096.0f. The original data is in a
  1.3.12 fixed format where 4096 == 1.0.The normals are used for lighting
  calculations.

=== Textured Info

  This information contains the texture coordinates, clut, page, and some
  unknown fields. The page is a relic of the PS1 hardware. Texture sizes were
  limited to 256x256 pixels, so textures were split into pages of 256x256
  pixels. So a texture is 256x1024. This is basically 4 pages sitting on top of
  each other. To calculate the actual coordinates you can use the formula:

    - texcoord.u = (texcoord.u + (256 * page));

=== UnTextured Info

  This information is for untextured polygons. It contains 4 bytes per polygon.
  We don't know the purpose of this data.

=== Tile Info

  The tile info contains x/z coordinate indexes into the terrain map. The
  elevation is either 0 or 1, depending on when there is two levels of terrain
  (think of bridges that you can walk over and under).

Reference: https://ffhacktics.com/wiki/Maps/Mesh#Primary_mesh

================================================================================
*/

enum {
    FFT_MESH_MAX_TEX_TRIS = 512,
    FFT_MESH_MAX_TEX_QUADS = 768,
    FFT_MESH_MAX_UNTEX_TRIS = 64,
    FFT_MESH_MAX_UNTEX_QUADS = 256,
    FFT_MESH_MAX_POLYGONS = FFT_MESH_MAX_TEX_TRIS + FFT_MESH_MAX_TEX_QUADS + FFT_MESH_MAX_UNTEX_TRIS + FFT_MESH_MAX_UNTEX_QUADS,
};

typedef struct {
    int16_t x, y, z;
} fft_position_t;

// Normals are stored in fixed-point format with 16 bits for each component. If
// you want to use f32 normals, you can convert them with a helper function.
typedef struct {
    fft_fixed16_t x, y, z;
} fft_normal_t;

typedef struct {
    uint8_t u, v;
} fft_texcoord_t;

typedef struct {
    fft_position_t position;
    fft_normal_t normal;
    fft_texcoord_t texcoord;
} fft_vertex_t;

typedef enum {
    FFT_POLYTYPE_TRIANGLE,
    FFT_POLYTYPE_QUAD,
} fft_polytype_e;

typedef struct {
    fft_texcoord_t texcoords[4]; // Up to 4 texcoords for quads
    uint8_t clut;
    uint8_t page;
    // image_to_use: 3 == texture map, other values are for UI, fonts, icons
    uint8_t image_to_use;

    uint8_t unknown_a; // maps almost always have this set to 0x78 / 120dec
    uint8_t unknown_b; // 4 high bits from image_to_use
    uint8_t unknown_c;
    bool is_textured; // true if the polygon is textured, false if it is untextured
} fft_texinfo_t;

// The purpose of this data is unknown, but we know there are 4 bytes for each
// untextured polygon.
typedef struct {
    uint8_t unknown_a;
    uint8_t unknown_b;
    uint8_t unknown_c;
    uint8_t unknown_d;
} fft_untexinfo_t;

typedef struct {
    uint8_t x;
    uint8_t z;
    uint8_t elevation;
} fft_tileinfo_t;

typedef struct {
    fft_polytype_e type;
    fft_vertex_t vertices[4];

    fft_texinfo_t tex;
    fft_untexinfo_t untex;
    fft_tileinfo_t tiles;
} polygon_t;

typedef struct {
    polygon_t polygons[FFT_MESH_MAX_POLYGONS];
} fft_geometry_t;

static fft_geometry_t fft_geometry_read(fft_span_t* span);

/*
================================================================================
Lights and Background
================================================================================

Each map can have up to 3 directional lights, an ambient color and two
background colors. The background colors (top and bottom) and they are linearly
interpolated between the two colors.

The directional light section always contains the space for 3 lights. But if the
color is completely black (0, 0, 0), then the light is considered to be disabled
by the engine.

  - fft_light_t is a directional light.
  - fft_lighting_t is the full lighting state for a map.

Reference: https://ffhacktics.com/wiki/Maps/Mesh#Light_colors_and_positions.2C_background_gradient_colors

================================================================================
*/

enum {
    FFT_LIGHTING_MAX_LIGHTS = 3,
};

typedef struct {
    fft_color_rgbfx16_t color;
    fft_position_t position;
} fft_light_t;

typedef struct {
    fft_light_t lights[FFT_LIGHTING_MAX_LIGHTS];
    fft_color_rgb8_t ambient_color;
    fft_color_rgb8_t background_top;
    fft_color_rgb8_t background_bottom;

    // Extra 3 bytes that we don't know the purpose of.
    uint8_t unknown_a;
    uint8_t unknown_b;
    uint8_t unknown_c;
} fft_lighting_t;

fft_lighting_t fft_lighting_read(fft_span_t* span);

/*
================================================================================
Terrain
================================================================================

Terrain data defines the walkable areas, surface types, slopes, and movement
properties for each tile in the map.

Reference: https://ffhacktics.com/wiki/Maps/Mesh#Terrain

================================================================================
*/

enum {
    FFT_TERRAIN_MAX_X = 17,
    FFT_TERRAIN_MAX_Z = 18,
    FFT_TERRAIN_MAX_Y = 2, // Elevation

    FFT_TERRAIN_MAX_TILES = 256,
    FFT_TERRAIN_TILE_WIDTH = 28,
    FFT_TERRAIN_TILE_DEPTH = 28,
    FFT_TERRAIN_TILE_HEIGHT = 12,

    FFT_TERRAIN_STR_SIZE = 128
};

#define FFT_TERRAIN_SURFACE_INDEX                       \
    X(SURFACE_NATURAL_SURFACE, 0x00, "Natural Surface") \
    X(SURFACE_SAND, 0x01, "Sand")                       \
    X(SURFACE_STALACTITE, 0x02, "Stalactite")           \
    X(SURFACE_GRASSLAND, 0x03, "Grassland")             \
    X(SURFACE_THICKET, 0x04, "Thicket")                 \
    X(SURFACE_SNOW, 0x05, "Snow")                       \
    X(SURFACE_ROCKY_CLIFF, 0x06, "Rocky Cliff")         \
    X(SURFACE_GRAVEL, 0x07, "Gravel")                   \
    X(SURFACE_WASTELAND, 0x08, "Wasteland")             \
    X(SURFACE_SWAMP, 0x09, "Swamp")                     \
    X(SURFACE_MARSH, 0x0A, "Marsh")                     \
    X(SURFACE_POISONED_MARSH, 0x0B, "Poisoned Marsh")   \
    X(SURFACE_LAVA_ROCKS, 0x0C, "Lava Rocks")           \
    X(SURFACE_ICE, 0x0D, "Ice")                         \
    X(SURFACE_WATERWAY, 0x0E, "Waterway")               \
    X(SURFACE_RIVER, 0x0F, "River")                     \
    X(SURFACE_LAKE, 0x10, "Lake")                       \
    X(SURFACE_SEA, 0x11, "Sea")                         \
    X(SURFACE_LAVA, 0x12, "Lava")                       \
    X(SURFACE_ROAD, 0x13, "Road")                       \
    X(SURFACE_WOODEN_FLOOR, 0x14, "Wooden Floor")       \
    X(SURFACE_STONE_FLOOR, 0x15, "Stone Floor")         \
    X(SURFACE_ROOF, 0x16, "Roof")                       \
    X(SURFACE_STONEWALL, 0x17, "Stonewall")             \
    X(SURFACE_SKY, 0x18, "Sky")                         \
    X(SURFACE_DARKNESS, 0x19, "Darkness")               \
    X(SURFACE_SALT, 0x1A, "Salt")                       \
    X(SURFACE_BOOK, 0x1B, "Book")                       \
    X(SURFACE_OBSTACLE, 0x1C, "Obstacle")               \
    X(SURFACE_RUG, 0x1D, "Rug")                         \
    X(SURFACE_TREE, 0x1E, "Tree")                       \
    X(SURFACE_BOX, 0x1F, "Box")                         \
    X(SURFACE_BRICK, 0x20, "Brick")                     \
    X(SURFACE_CHIMNEY, 0x21, "Chimney")                 \
    X(SURFACE_MUD_WALL, 0x22, "Mud Wall")               \
    X(SURFACE_BRIDGE, 0x23, "Bridge")                   \
    X(SURFACE_WATER_PLANT, 0x24, "Water Plant")         \
    X(SURFACE_STAIRS, 0x25, "Stairs")                   \
    X(SURFACE_FURNITURE, 0x26, "Furniture")             \
    X(SURFACE_IVY, 0x27, "Ivy")                         \
    X(SURFACE_DECK, 0x28, "Deck")                       \
    X(SURFACE_MACHINE, 0x29, "Machine")                 \
    X(SURFACE_IRON_PLATE, 0x2A, "Iron Plate")           \
    X(SURFACE_MOSS, 0x2B, "Moss")                       \
    X(SURFACE_TOMBSTONE, 0x2C, "Tombstone")             \
    X(SURFACE_WATERFALL, 0x2D, "Waterfall")             \
    X(SURFACE_COFFIN, 0x2E, "Coffin")                   \
    X(SURFACE_CROSS_SECTION, 0x3F, "Cross Section")

#define FFT_TERRAIN_SLOPE_INDEX             \
    X(SLOPE_FLAT, 0x00, "Flat")             \
    X(SLOPE_INCLINE_N, 0x85, "Incline N")   \
    X(SLOPE_INCLINE_E, 0x52, "Incline E")   \
    X(SLOPE_INCLINE_S, 0x25, "Incline S")   \
    X(SLOPE_INCLINE_W, 0x58, "Incline W")   \
    X(SLOPE_CONVEX_NE, 0x41, "Convex NE")   \
    X(SLOPE_CONVEX_SE, 0x11, "Convex SE")   \
    X(SLOPE_CONVEX_SW, 0x14, "Convex SW")   \
    X(SLOPE_CONVEX_NW, 0x44, "Convex NW")   \
    X(SLOPE_CONCAVE_NE, 0x96, "Concave NE") \
    X(SLOPE_CONCAVE_SE, 0x66, "Concave SE") \
    X(SLOPE_CONCAVE_SW, 0x69, "Concave SW") \
    X(SLOPE_CONCAVE_NW, 0x99, "Concave NW")

typedef enum {
#define X(oname, ovalue, ostring) oname = ovalue,
    FFT_TERRAIN_SURFACE_INDEX
#undef X
} fft_terrain_surface_e;

typedef enum {
#define X(oname, ovalue, ostring) oname = ovalue,
    FFT_TERRAIN_SLOPE_INDEX
#undef X
} fft_terrain_slope_e;

typedef struct {
    fft_terrain_surface_e surface;
    fft_terrain_slope_e slope;
    uint8_t sloped_height_bottom;
    uint8_t sloped_height_top; // difference between bottom and top
    uint8_t depth;
    uint8_t shading;
    uint8_t auto_cam_dir;   // auto rotate camera if unit enters this tile
    bool pass_through_only; // Can walk/cursor but cannot stop on it
    bool cant_walk;
    bool cant_select;
} fft_terrain_tile_t;

typedef struct {
    fft_terrain_tile_t tiles[FFT_TERRAIN_MAX_Y][FFT_TERRAIN_MAX_TILES];
    uint8_t x_count;
    uint8_t z_count;
    bool valid;
} fft_terrain_t;

static fft_terrain_t fft_terrain_read(fft_span_t* span);
static const char* fft_terrain_surface_str(fft_terrain_surface_e value);
static const char* fft_terrain_slope_str(fft_terrain_slope_e value);
static const char* fft_terrain_shading_str(uint8_t value);
static void fft_terrain_camdir_str(uint8_t cam_dir, char out_str[static FFT_TERRAIN_STR_SIZE]);

/*
================================================================================
Mesh
================================================================================

Mesh is the main data structure for a map. It contains all the geometry,
cluts, lighting, terrain, animations, etc.

"Primary Mesh" below refers to recordtype FFT_RECORDTYPE_MESH_PRIMARY.

+-----------------------+----------------------------------------------+
| Field                 | Primary Mesh | Other Meshes | Notes          |
+-----------------------+----------------------------------------------+
| geometry              | Mandatory    | Optional     | MAP052 missing |
| clut_color            | Mandatory    | Optional     |                |
| lights_and_background | Mandatory    | Optional     | Confirm        |
| terrain               | Mandatory    | Optional     |                |
| texture_anim_inst     | Optional     | Optional     |                |
| palette_anim_inst     | Optional     | Optional     |                |
| clut_gray             | Mandatory    | Optional     |                |
| mesh_anim_inst        | Optional     | Optional     |                |
| anim_mesh_1           | Optional     | Optional     |                |
| anim_mesh_2           | Optional     | Optional     |                |
| anim_mesh_3           | Optional     | Optional     |                |
| anim_mesh_4           | Optional     | Optional     |                |
| anim_mesh_5           | Optional     | Optional     |                |
| anim_mesh_6           | Optional     | Optional     |                |
| anim_mesh_7           | Optional     | Optional     |                |
| anim_mesh_8           | Optional     | Optional     |                |
| poly_render_props     | Optional     | Optional     |                |
+-----------------------+----------------------------------------------+

================================================================================
*/

typedef struct {
    fft_state_t state;

    fft_mesh_header_t header;
    fft_geometry_t geometry;
    fft_clut_t clut;
    fft_lighting_t lighting;
    fft_terrain_t terrain;

    fft_record_meta_t meta;

} fft_mesh_t;

fft_mesh_t fft_mesh_read(fft_span_t* span);

/*
================================================================================
Texture
================================================================================

A texture is just an image_t with a map state. This allows us to track the state
without having to store it in the image itself.

================================================================================
*/

enum {
    FFT_TEXTURE_WIDTH = 256,
    FFT_TEXTURE_HEIGHT = 1024,
};

typedef struct {
    fft_state_t state;
    fft_image_t image;
} fft_texture_t;

static fft_texture_t fft_texture_read(fft_span_t* span, fft_state_t state);
static void fft_texture_destroy(fft_texture_t texture);

/*
================================================================================
Map Data
================================================================================

Map data is the main structure that contains all the information about a map.
This contains all the records, meshes, images, cluts, etc for all map states.

================================================================================
*/

// map_desc_t is a struct that contains information about a map.
// This lets us know if we can use the map and where on the disk it is.
typedef struct {
    uint8_t id;
    fft_io_entry_e entry;
    bool valid;
    const char* name;
} fft_map_desc_t;

typedef struct {
    fft_record_t records[FFT_RECORD_MAX];

    fft_mesh_t primary_mesh;
    fft_mesh_t override_mesh;
    fft_mesh_t alt_meshes[20];
    fft_texture_t textures[20];

    uint8_t record_count;
    uint8_t texture_count;
    uint8_t alt_mesh_count;
} fft_map_data_t;

enum {
    FFT_MAP_DESC_LIST_COUNT = 128,
};

void fft_map_data_destroy(fft_map_data_t* map);
fft_map_data_t* fft_map_data_read(int map_id);

extern const fft_map_desc_t fft_map_list[FFT_MAP_DESC_LIST_COUNT];

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
Fixed-Point Implementation
================================================================================
*/

float fft_fixed16_to_f32(fft_fixed16_t value) {
    return (float)value / FFT_FIXED16_ONE;
}

/*
================================================================================
Span Implementation
================================================================================
*/

enum {
    // This is the size of a map texture, which is the largest file size we read.
    FFT_SPAN_MAX_BYTES = 131072,
};

static void fft_span_read_bytes(fft_span_t* f, size_t size, uint8_t* out_bytes) {
    FFT_ASSERT(size <= FFT_SPAN_MAX_BYTES, "Too many bytes requested.");
    memcpy(out_bytes, &f->data[f->offset], size);
    f->offset += size;
    return;
}

static void fft_span_set_offset(fft_span_t* f, size_t offset) {
    FFT_ASSERT(offset <= f->size, "Seek out of bounds.");
    f->offset = offset;
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
    uint16_t unknown_aa = fft_span_read_u16(span);     // 0
    uint8_t layout = fft_span_read_u8(span);           // 1-2
    uint8_t time_and_weather = fft_span_read_u8(span); // 3
    fft_recordtype_e type = fft_span_read_u16(span);   // 4-5
    uint16_t unknown_ee = fft_span_read_u16(span);     // 6-7
    uint32_t sector = fft_span_read_u16(span);         // 8-9
    uint16_t unknown_gg = fft_span_read_u16(span);     // 10-11
    uint32_t length = fft_span_read_u32(span);         // 12-15
    uint16_t unknown_ii = fft_span_read_u16(span);     // 16-17
    uint16_t unknown_jj = fft_span_read_u16(span);     // 18-19

    // Split time and weather from single byte.
    // - time    = 0b10000000
    // - weather = 0b01110000
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
        .unknown_aa = unknown_aa,
        .unknown_ee = unknown_ee,
        .unknown_gg = unknown_gg,
        .unknown_ii = unknown_ii,
        .unknown_jj = unknown_jj,
    };

    // Rollback to the start of the record for raw data.
    span->offset -= FFT_RECORD_SIZE;
    fft_span_read_bytes(span, FFT_RECORD_SIZE, record.raw);

    return record;
}

static uint8_t fft_record_read_all(fft_span_t* span, fft_record_t* out_records) {
    uint8_t count = 0;
    while (span->offset + FFT_RECORD_SIZE < span->size) {
        fft_record_t record = fft_record_read(span);
        if (record.type == FFT_RECORDTYPE_END) {
            break;
        }
        if (count >= FFT_RECORD_MAX) {
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
Colors Implementation
================================================================================
*/

uint8_t fft_color_4bpp_left(fft_color_4bpp_t raw) { return (raw & 0xF0) >> 4; }
uint8_t fft_color_4bpp_right(fft_color_4bpp_t raw) { return raw & 0x0F; }

uint8_t fft_color_5551_r8(fft_color_5551_t c) { return (uint8_t)(((c & 0x1F) << 3) | ((c & 0x1F) >> 2)); }
uint8_t fft_color_5551_g8(fft_color_5551_t c) { return (uint8_t)((((c >> 5) & 0x1F) << 3) | (((c >> 5) & 0x1F) >> 2)); }
uint8_t fft_color_5551_b8(fft_color_5551_t c) { return (uint8_t)((((c >> 10) & 0x1F) << 3) | (((c >> 10) & 0x1F) >> 2)); }
uint8_t fft_color_5551_a8(fft_color_5551_t c) { return (uint8_t)(((c >> 15) & 0x01) ? 255 : 0); }

fft_color_4bpp_t fft_color_4bpp_read(fft_span_t* span) {
    uint8_t value = fft_span_read_u8(span);
    return (fft_color_4bpp_t)value;
}

fft_color_5551_t fft_color_5551_read(fft_span_t* span) {
    fft_color_5551_t color = 0;
    uint16_t value = fft_span_read_u16(span);
    color |= (value & 0x1F);               // Red
    color |= ((value >> 5) & 0x1F) << 5;   // Green
    color |= ((value >> 10) & 0x1F) << 10; // Blue
    color |= ((value >> 15) & 0x01) << 15; // Alpha
    return color;
}

fft_color_rgbfx16_t fft_color_rgbfx16_read(fft_span_t* span) {
    fft_color_rgbfx16_t color = { 0 };
    color.r = fft_span_read_i16(span); // 16 bits for red
    color.g = fft_span_read_i16(span); // 16 bits for green
    color.b = fft_span_read_i16(span); // 16 bits for blue
    return color;
}

fft_color_rgb8_t fft_color_rgb8_read(fft_span_t* span) {
    fft_color_rgb8_t color = { 0 };
    color.r = fft_span_read_u8(span); // 8 bits for red
    color.g = fft_span_read_u8(span); // 8 bits for green
    color.b = fft_span_read_u8(span); // 8 bits for blue
    return color;
}

bool fft_color_5551_is_transparent(fft_color_5551_t color) {
    uint8_t r = fft_color_5551_r8(color);
    uint8_t g = fft_color_5551_g8(color);
    uint8_t b = fft_color_5551_b8(color);
    uint8_t a = fft_color_5551_a8(color);
    return (r + g + b + a) == 0;
}

fft_color_t fft_color_from_5551(fft_color_5551_t c) {
    uint8_t r = (uint8_t)(((c) & 0x1F) << 3) | (((c) & 0x1F) >> 2);
    uint8_t g = (uint8_t)(((c >> 5) & 0x1F) << 3) | (((c >> 5) & 0x1F) >> 2);
    uint8_t b = (uint8_t)(((c >> 10) & 0x1F) << 3) | (((c >> 10) & 0x1F) >> 2);
    uint8_t a = (c >> 15) ? 255 : 0;
    return FFT_COLOR_RGBA(r, g, b, a);
}

// Convert RGB16 fixed-point to RGBA8888 (full 255 range)
fft_color_t fft_color_from_rgbfx16(fft_color_rgbfx16_t c) {
    float r32 = fft_fixed16_to_f32(c.r);
    float g32 = fft_fixed16_to_f32(c.g);
    float b32 = fft_fixed16_to_f32(c.b);

    uint8_t r = (uint8_t)(r32 * 255.0f + 0.5f);
    uint8_t g = (uint8_t)(g32 * 255.0f + 0.5f);
    uint8_t b = (uint8_t)(b32 * 255.0f + 0.5f);
    return FFT_COLOR_RGBA(r, g, b, 255);
}

// Convert RGB8 to RGBA8888
fft_color_t fft_color_from_rgb8(fft_color_rgb8_t c) {
    return FFT_COLOR_RGBA(c.r, c.g, c.b, 255);
}

/*
================================================================================
CLUT Implementation
================================================================================
*/

fft_clut_row_t fft_clut_row_read(fft_span_t* span) {
    fft_clut_row_t row = { 0 };
    for (uint32_t i = 0; i < FFT_CLUT_ROW_WIDTH; i++) {
        row.colors[i] = fft_color_5551_read(span);
    }

    return row;
}

fft_clut_t fft_clut_read(fft_span_t* span) {
    fft_clut_t clut = { 0 };
    for (uint32_t i = 0; i < FFT_CLUT_ROW_COUNT; i++) {
        clut.rows[i] = fft_clut_row_read(span);
    }
    return clut;
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
        fft_color_4bpp_t raw_pixel = fft_color_4bpp_read(span);
        uint8_t right = fft_color_4bpp_right(raw_pixel);
        uint8_t left = fft_color_4bpp_left(raw_pixel);

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
static void fft_image_palettize(fft_image_t* image, const fft_image_t* clut, uint8_t pal_index) {
    FFT_ASSERT(image && image->data && image->valid, "Invalid image parameter");
    FFT_ASSERT(clut && clut->data && clut->valid, "Invalid clut parameter");

    const uint32_t pixel_count = image->width * image->height;
    const uint32_t pal_offset = (FFT_IMAGE_PAL_ROW_SIZE * pal_index);

    // Ensure palette index and offset are valid
    FFT_ASSERT(pal_offset + (FFT_IMAGE_PAL_COL_COUNT * 4) <= clut->size, "Palette index out of bounds");

    for (uint32_t i = 0; i < pixel_count * 4; i = i + 4) {
        uint8_t pixel = image->data[i];

        // Ensure pixel value is within palette range
        FFT_ASSERT(pixel < FFT_IMAGE_PAL_COL_COUNT, "Pixel value %d exceeds palette size", pixel);

        memcpy(&image->data[i], &clut->data[pal_offset + (pixel * 4)], 4);
    }
}

static fft_image_t fft_image_read_4bpp_palettized(fft_span_t* span, fft_image_desc_t desc, uint8_t pal_index) {
    // Read the 4bpp image data.
    fft_image_t image = fft_image_read_4bpp(span, desc.width, desc.height);

    // Read the clut data.
    fft_span_set_offset(span, desc.pal_offset);
    fft_image_t clut = fft_image_read_16bpp(span, FFT_IMAGE_PAL_COL_COUNT, desc.pal_count);

    fft_image_palettize(&image, &clut, pal_index);

    FFT_MEM_FREE(clut.data);

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
    FFT_ASSERT(image && image->valid && image->data, "Invalid image parameter");

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

static void fft_image_destroy(fft_image_t* image) {
    if (image && image->data) {
        FFT_MEM_FREE(image->data);
        image->data = NULL;
        image->valid = false;
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
Mesh Header Implementation
================================================================================
*/
fft_mesh_header_t fft_mesh_header_read(fft_span_t* span) {
    fft_mesh_header_t header = { 0 };

    // 0x00-0x3F: unknown pointers (16 × 4 bytes)
    fft_span_read_bytes(span, 64, (uint8_t*)header.unknown_00_to_40);

    header.geometry = fft_span_read_u32(span);   // 0x40
    header.clut_color = fft_span_read_u32(span); // 0x44
    header.unknown_48 = fft_span_read_u32(span); // 0x48
    header.unknown_4c = fft_span_read_u32(span); // 0x4C

    // 0x50-0x63: unknown pointers (5 × 4 bytes)
    fft_span_read_bytes(span, 20, (uint8_t*)header.unknown_50_to_64);

    header.lights_and_background = fft_span_read_u32(span); // 0x64
    header.terrain = fft_span_read_u32(span);               // 0x68
    header.texture_anim_inst = fft_span_read_u32(span);     // 0x6C
    header.palette_anim_inst = fft_span_read_u32(span);     // 0x70

    // 0x74-0x7B: unknown pointers (2 × 4 bytes)
    fft_span_read_bytes(span, 8, (uint8_t*)header.unknown_74_to_7c);

    header.clut_gray = fft_span_read_u32(span); // 0x7C

    // 0x80-0x8B: unknown pointers (3 × 4 bytes)
    fft_span_read_bytes(span, 12, (uint8_t*)header.unknown_80_to_8c);

    header.mesh_anim_inst = fft_span_read_u32(span);    // 0x8C
    header.anim_mesh_1 = fft_span_read_u32(span);       // 0x90
    header.anim_mesh_2 = fft_span_read_u32(span);       // 0x94
    header.anim_mesh_3 = fft_span_read_u32(span);       // 0x98
    header.anim_mesh_4 = fft_span_read_u32(span);       // 0x9C
    header.anim_mesh_5 = fft_span_read_u32(span);       // 0xA0
    header.anim_mesh_6 = fft_span_read_u32(span);       // 0xA4
    header.anim_mesh_7 = fft_span_read_u32(span);       // 0xA8
    header.anim_mesh_8 = fft_span_read_u32(span);       // 0xAC
    header.poly_render_props = fft_span_read_u32(span); // 0xB0

    // 0xB4-0xC3: unknown pointers (4 × 4 bytes)
    fft_span_read_bytes(span, 16, (uint8_t*)header.unknown_b4_to_c4);

    return header;
}

/*
================================================================================
Geometry Implementation
================================================================================
*/

static fft_position_t fft_geometry_read_position(fft_span_t* span) {
    int16_t x = fft_span_read_i16(span);
    int16_t y = fft_span_read_i16(span);
    int16_t z = fft_span_read_i16(span);
    return (fft_position_t) { x, y, z };
}

static fft_normal_t fft_geometry_read_normal(fft_span_t* span) {
    fft_fixed16_t x = fft_span_read_i16(span);
    fft_fixed16_t y = fft_span_read_i16(span);
    fft_fixed16_t z = fft_span_read_i16(span);
    return (fft_normal_t) { x, y, z };
}

static uint32_t read_polygons(fft_span_t* span, fft_geometry_t* g, fft_polytype_e type, bool is_textured, uint32_t poly_offset, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        polygon_t* poly = &g->polygons[poly_offset + i];
        poly->tex.is_textured = is_textured;
        poly->type = type;

        // Read vertices
        for (uint32_t j = 0; j < (type == FFT_POLYTYPE_TRIANGLE ? 3 : 4); j++) {
            poly->vertices[j].position = fft_geometry_read_position(span);
        }
    }

    return poly_offset + count;
}

static uint32_t read_normals(fft_span_t* span, fft_geometry_t* g, fft_polytype_e type, uint32_t poly_offset, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        polygon_t* poly = &g->polygons[poly_offset + i];

        // Read normals
        for (uint32_t j = 0; j < (type == FFT_POLYTYPE_TRIANGLE ? 3 : 4); j++) {
            poly->vertices[j].normal = fft_geometry_read_normal(span);
        }
    }

    return poly_offset + count;
}

static uint32_t read_texinfo(fft_span_t* span, fft_geometry_t* g, fft_polytype_e type, uint32_t poly_offset, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        // Read all data
        uint8_t au = fft_span_read_u8(span);                           // 0
        uint8_t av = fft_span_read_u8(span);                           // 1
        uint8_t clut = fft_span_read_u8(span);                         // 2
        uint8_t unknown_a = fft_span_read_u8(span);                    // 3
        uint8_t bu = fft_span_read_u8(span);                           // 4
        uint8_t bv = fft_span_read_u8(span);                           // 5
        uint8_t page_and_image_and_unknown_b = fft_span_read_u8(span); // 6
        uint8_t unknown_c = fft_span_read_u8(span);                    // 7
        uint8_t cu = fft_span_read_u8(span);                           // 8
        uint8_t cv = fft_span_read_u8(span);                           // 9

        // Split single byte values
        uint8_t page = (page_and_image_and_unknown_b >> 0) & 0x03;      // bits 0–1  (0b00000011)
        uint8_t image = (page_and_image_and_unknown_b >> 2) & 0x03;     // bits 2–3  (0b00001100)
        uint8_t unknown_b = (page_and_image_and_unknown_b >> 4) & 0x0F; // bits 4–7  (0b11110000)

        // Populate the polygon's texture info.
        polygon_t* poly = &g->polygons[poly_offset + i];
        poly->tex.texcoords[0] = (fft_texcoord_t) { .u = au, .v = av };
        poly->tex.texcoords[1] = (fft_texcoord_t) { .u = bu, .v = bv };
        poly->tex.texcoords[2] = (fft_texcoord_t) { .u = cu, .v = cv };
        poly->tex.clut = clut;
        poly->tex.page = page;
        poly->tex.image_to_use = image;
        poly->tex.unknown_a = unknown_a;
        poly->tex.unknown_b = unknown_b;
        poly->tex.unknown_c = unknown_c;

        if (type == FFT_POLYTYPE_QUAD) {
            uint8_t du = fft_span_read_u8(span);
            uint8_t dv = fft_span_read_u8(span);
            fft_texcoord_t d = { .u = du, .v = dv };
            poly->tex.texcoords[3] = d;
        }
    }

    return poly_offset + count;
}

static uint32_t read_untexinfo(fft_span_t* span, fft_geometry_t* g, uint32_t poly_offset, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        // Read all data
        uint8_t unknown_a = fft_span_read_u8(span); // 0
        uint8_t unknown_b = fft_span_read_u8(span); // 1
        uint8_t unknown_c = fft_span_read_u8(span); // 2
        uint8_t unknown_d = fft_span_read_u8(span); // 3

        // Populate the polygon's untextured info.
        polygon_t* poly = &g->polygons[poly_offset + i];
        poly->untex.unknown_a = unknown_a;
        poly->untex.unknown_b = unknown_b;
        poly->untex.unknown_c = unknown_c;
        poly->untex.unknown_d = unknown_d;
    }

    return poly_offset + count;
}

static uint32_t read_tile_locations(fft_span_t* span, fft_geometry_t* g, uint32_t poly_offset, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        // Read all data
        uint8_t z_and_y = fft_span_read_u8(span); // y is elevation 0 or 1
        uint8_t x = fft_span_read_u8(span);

        // Split single byte values
        uint8_t y = (z_and_y >> 0) & 0x01; // 0b00000001
        uint8_t z = (z_and_y >> 1) & 0xFE; // 0b11111110

        // Populate the polygon's tile info.
        polygon_t* poly = &g->polygons[poly_offset + i];
        poly->tiles.x = x;
        poly->tiles.z = z;
        poly->tiles.elevation = y;
    }

    return poly_offset + count;
}

static fft_geometry_t fft_geometry_read(fft_span_t* span) {
    fft_geometry_t geometry = { 0 };

    // The number of each type of polygon.
    uint16_t N = fft_span_read_u16(span); // Textured triangles
    uint16_t P = fft_span_read_u16(span); // Textured quads
    uint16_t Q = fft_span_read_u16(span); // Untextured triangles
    uint16_t R = fft_span_read_u16(span); // Untextured quads

    // Validate maximum values
    FFT_ASSERT(N < FFT_MESH_MAX_TEX_TRIS, "Mesh textured triangle count exceeded");
    FFT_ASSERT(P < FFT_MESH_MAX_TEX_QUADS, "Mesh textured quad count exceeded");
    FFT_ASSERT(Q < FFT_MESH_MAX_UNTEX_TRIS, "Mesh untextured triangle count exceeded");
    FFT_ASSERT(R < FFT_MESH_MAX_UNTEX_QUADS, "Mesh untextured quad count exceeded");

    // index is used to track the current position in the polygons array. It
    // gets reset to 0 most of the time because most of this data is for the
    // textured polygons, which come first.
    uint32_t index = 0;

    // Polygons
    index = read_polygons(span, &geometry, FFT_POLYTYPE_TRIANGLE, false, index, N);
    index = read_polygons(span, &geometry, FFT_POLYTYPE_QUAD, false, index, P);
    index = read_polygons(span, &geometry, FFT_POLYTYPE_TRIANGLE, false, index, Q);
    index = read_polygons(span, &geometry, FFT_POLYTYPE_QUAD, false, index, R);

    // Normals
    index = 0; // Reset for textured polygons
    index = read_normals(span, &geometry, FFT_POLYTYPE_TRIANGLE, index, N);
    index = read_normals(span, &geometry, FFT_POLYTYPE_QUAD, index, P);

    // Texture Coordinates
    index = 0; // Reset for textured polygons
    index = read_texinfo(span, &geometry, FFT_POLYTYPE_TRIANGLE, index, N);
    index = read_texinfo(span, &geometry, FFT_POLYTYPE_QUAD, index, P);

    // Unknown Untextured Polygon Data
    index = N + P; // Reset for untextured polygons
    index = read_untexinfo(span, &geometry, index, Q);
    index = read_untexinfo(span, &geometry, index, R);

    // Tile Locations
    index = 0; // Reset for textured polygons
    index = read_tile_locations(span, &geometry, index, N);
    index = read_tile_locations(span, &geometry, index, P);

    return geometry;
}

/*
================================================================================
Lights and Background Implementation
================================================================================
*/

static bool fft_light_is_valid(fft_light_t light) {
    return light.color.r + light.color.g + light.color.b > 0.0f;
}

fft_lighting_t fft_lighting_read(fft_span_t* span) {
    fft_lighting_t l = { 0 };

    // Light colors are oddly stored all 3xR then 3xG then 3xB.
    l.lights[0].color.r = fft_span_read_i16(span);
    l.lights[1].color.r = fft_span_read_i16(span);
    l.lights[2].color.r = fft_span_read_i16(span);
    l.lights[0].color.g = fft_span_read_i16(span);
    l.lights[1].color.g = fft_span_read_i16(span);
    l.lights[2].color.g = fft_span_read_i16(span);
    l.lights[0].color.b = fft_span_read_i16(span);
    l.lights[1].color.b = fft_span_read_i16(span);
    l.lights[2].color.b = fft_span_read_i16(span);

    // Positions are stored as expected.
    l.lights[0].position = fft_geometry_read_position(span);
    l.lights[1].position = fft_geometry_read_position(span);
    l.lights[2].position = fft_geometry_read_position(span);

    l.ambient_color = fft_color_rgb8_read(span);

    l.background_top = fft_color_rgb8_read(span);
    l.background_bottom = fft_color_rgb8_read(span);

    l.unknown_a = fft_span_read_u8(span);
    l.unknown_b = fft_span_read_u8(span);
    l.unknown_c = fft_span_read_u8(span);

    return l;
}

/*
================================================================================
Terrain Implementation
================================================================================
*/

static fft_terrain_t fft_terrain_read(fft_span_t* span) {
    fft_terrain_t terrain = { 0 };

    uint8_t x_count = fft_span_read_u8(span);
    uint8_t z_count = fft_span_read_u8(span);
    FFT_ASSERT(x_count <= FFT_TERRAIN_MAX_X, "Terrain X count exceeded");
    FFT_ASSERT(z_count <= FFT_TERRAIN_MAX_Z, "Terrain Z count exceeded");

    for (uint8_t level = 0; level < 2; level++) {
        for (uint8_t z = 0; z < z_count; z++) {
            for (uint8_t x = 0; x < x_count; x++) {
                // FIXME: This code needs to be validated that it is reading everything correctly.
                fft_terrain_tile_t tile = { 0 };

                uint8_t raw_surface = fft_span_read_u8(span);
                tile.surface = (fft_terrain_surface_e)(raw_surface & 0x3F); // 0b00111111
                fft_span_read_u8(span);
                tile.sloped_height_bottom = fft_span_read_u8(span);
                uint8_t slope_top_and_depth = fft_span_read_u8(span);
                tile.depth = (slope_top_and_depth >> 5) & 0x07;      // 0b11100000 -> 0b00000111
                tile.sloped_height_top = slope_top_and_depth & 0x1F; // 0b00011111
                tile.slope = (fft_terrain_slope_e)fft_span_read_u8(span);
                fft_span_read_u8(span); // Padding

                if (tile.slope == SLOPE_FLAT) {
                    // Sloped height top should be 0 for flat tiles but some
                    // maps tiles set to 1. This should be researched further.
                    FFT_ASSERT(tile.sloped_height_top == 0 || tile.sloped_height_top == 1, "Flat tile has > 1 sloped height top");
                }

                // bits 3, 4, 5, are unused
                uint8_t misc = fft_span_read_u8(span);
                tile.pass_through_only = misc & (1 << 0); // bit 0
                tile.shading = (misc >> 2) & 0x3;         // bit 1 & 2
                tile.cant_walk = misc & (1 << 6);         // bit 6
                tile.cant_select = misc & (1 << 7);       // bit 7
                tile.auto_cam_dir = fft_span_read_u8(span);

                terrain.tiles[level][z * x_count + x] = tile;
            }
        }
    }

    terrain.x_count = x_count;
    terrain.z_count = z_count;
    terrain.valid = true;
    return terrain;
}

static const char* fft_terrain_surface_str(fft_terrain_surface_e value) {
    switch (value) {
#define X(oname, ovalue, ostring) \
case oname:                       \
    return ostring;
        FFT_TERRAIN_SURFACE_INDEX
#undef X
    default:;
        static char buf[32];
        snprintf(buf, sizeof(buf), "Unknown 0x%02X", value);
        return buf;
    }
}

static const char* fft_terrain_slope_str(fft_terrain_slope_e value) {
    switch (value) {
#define X(oname, ovalue, ostring) \
case oname:                       \
    return ostring;
        FFT_TERRAIN_SLOPE_INDEX
#undef X
    default:;
        static char buf[32];
        snprintf(buf, sizeof(buf), "Unknown 0x%02X", value);
        return buf;
    }
}

static const char* fft_terrain_shading_str(uint8_t value) {
    switch (value) {
    case 0:
        return "Normal";
    case 1:
        return "Dark";
    case 2:
        return "Darker";
    case 3:
        return "Darkest";
    default:
        return "Unknown";
    }
}

static void fft_terrain_camdir_str(uint8_t cam_dir, char out_str[static FFT_TERRAIN_STR_SIZE]) {
    out_str[0] = '\0';

    static const char* labels[8] = {
        "NWT", "SWT",
        "SET", "NET",
        "NWB", "SWB",
        "SEB", "NEB"
    };

    uint32_t offset = 0;
    uint32_t written = 0;
    bool first = true;

    for (uint32_t i = 0; i < 8; i++) {
        if (cam_dir & (1u << i)) {
            if (!first) {
                written = (uint32_t)snprintf(out_str + offset, FFT_TERRAIN_STR_SIZE - offset, ", ");
                offset += (written > 0) ? written : 0;
            }

            written = (uint32_t)snprintf(out_str + offset, FFT_TERRAIN_STR_SIZE - offset, "%s", labels[i]);
            offset += (written > 0) ? written : 0;

            first = false;
        }
    }

    if (offset == 0) {
        snprintf(out_str, FFT_TERRAIN_STR_SIZE, "(None)");
    }
}

/*
================================================================================
Mesh Implementation
================================================================================
*/

fft_mesh_t fft_mesh_read(fft_span_t* span) {
    FFT_ASSERT(span->data != NULL && span->offset == 0, "Invalid span for mesh read");

    fft_mesh_t mesh = { 0 };

    mesh.header = fft_mesh_header_read(span);

    if (mesh.header.geometry != 0) {
        fft_span_set_offset(span, mesh.header.geometry);
        mesh.geometry = fft_geometry_read(span);

        // Jump back so we can read the polygon counts.
        fft_span_set_offset(span, mesh.header.geometry);
        mesh.meta.tex_tri_count = fft_span_read_u16(span);
        mesh.meta.tex_quad_count = fft_span_read_u16(span);
        mesh.meta.untex_tri_count = fft_span_read_u16(span);
        mesh.meta.untex_quad_count = fft_span_read_u16(span);
        mesh.meta.polygon_count = mesh.meta.tex_tri_count + mesh.meta.tex_quad_count + mesh.meta.untex_tri_count + mesh.meta.untex_quad_count;
        mesh.meta.has_geometry = true;
    }

    if (mesh.header.clut_color != 0) {
        fft_span_set_offset(span, mesh.header.clut_color);
        mesh.clut = fft_clut_read(span);
        mesh.meta.has_clut = true;
    }

    if (mesh.header.lights_and_background != 0) {
        fft_span_set_offset(span, mesh.header.lights_and_background);
        mesh.lighting = fft_lighting_read(span);
        mesh.meta.has_lighting = true;

        for (uint32_t i = 0; i < FFT_LIGHTING_MAX_LIGHTS; i++) {
            if (fft_light_is_valid(mesh.lighting.lights[i])) {
                mesh.meta.light_count++;
            }
        }
    }

    if (mesh.header.terrain != 0) {
        fft_span_set_offset(span, mesh.header.terrain);
        mesh.terrain = fft_terrain_read(span);
        mesh.meta.has_terrain = true;
    }

    return mesh;
}

/*
================================================================================
Texture Implementation
================================================================================
*/

static void fft_texture_destroy(fft_texture_t texture) {
    fft_image_destroy(&texture.image);
}

static fft_texture_t fft_texture_read(fft_span_t* span, fft_state_t state) {
    fft_image_t image = fft_image_read_4bpp(span, FFT_TEXTURE_WIDTH, FFT_TEXTURE_HEIGHT);
    return (fft_texture_t) {
        .state = state,
        .image = image,
    };
}

/*
================================================================================
Map Data Implementation
================================================================================
*/

fft_map_data_t* fft_map_data_read(int map_id) {
    fft_map_data_t* map_data = FFT_MEM_ALLOC(sizeof(fft_map_data_t));

    const fft_io_entry_e map_file = fft_map_list[map_id].entry;

    fft_span_t gns = fft_io_open(map_file);
    {
        map_data->record_count = fft_record_read_all(&gns, map_data->records);
    }
    fft_io_close(gns);

    for (uint32_t i = 0; i < map_data->record_count; i++) {
        fft_record_t* record = &map_data->records[i];

        // Fetch the resource file

        switch (record->type) {
        case FFT_RECORDTYPE_TEXTURE: {
            fft_span_t file = fft_io_read(record->sector, record->length);
            fft_texture_t texture = fft_texture_read(&file, record->state);
            fft_io_close(file);

            map_data->textures[map_data->texture_count++] = texture;
            break;
        }
        case FFT_RECORDTYPE_MESH_PRIMARY: {
            fft_span_t file = fft_io_read(record->sector, record->length);
            // There always only one primary mesh file and it uses default state.
            FFT_ASSERT(fft_state_is_default(record->state), "Primary mesh file has non-default state");
            map_data->primary_mesh = fft_mesh_read(&file);
            fft_io_close(file);

            record->meta = map_data->primary_mesh.meta;
            break;
        }
        case FFT_RECORDTYPE_MESH_ALT: {
            fft_span_t file = fft_io_read(record->sector, record->length);
            fft_mesh_t alt_mesh = fft_mesh_read(&file);
            fft_io_close(file);

            alt_mesh.state = record->state;
            map_data->alt_meshes[map_data->alt_mesh_count++] = alt_mesh;
            record->meta = map_data->primary_mesh.meta;
            break;
        }
        case FFT_RECORDTYPE_MESH_OVERRIDE: {
            fft_span_t file = fft_io_read(record->sector, record->length);
            // If there is an override file, there is only one and it uses default state.
            FFT_ASSERT(fft_state_is_default(record->state), "Override must be default map state");
            map_data->override_mesh = fft_mesh_read(&file);
            fft_io_close(file);

            record->meta = map_data->primary_mesh.meta;
            break;
        }

        default:
            continue;
        }
    }

    return map_data;
}

void fft_map_data_destroy(fft_map_data_t* map) {
    if (map == NULL) {
        return;
    }

    // Textures
    for (uint32_t i = 0; i < map->texture_count; i++) {
        fft_texture_destroy(map->textures[i]);
    }

    FFT_MEM_FREE(map);
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

// clang-format off
const fft_image_desc_t image_desc_list[FFT_IMAGE_DESC_COUNT] = {
    { .name = "BONUS.BIN",    .entry = F_EVENT__BONUS_BIN,    .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 200, .pal_offset = 25600, .pal_count = 6,  .repeat = 36,  .repeat_offset = 26624 },
    { .name = "CHAPTER1.BIN", .entry = F_EVENT__CHAPTER1_BIN, .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "CHAPTER2.BIN", .entry = F_EVENT__CHAPTER2_BIN, .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "CHAPTER3.BIN", .entry = F_EVENT__CHAPTER3_BIN, .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "CHAPTER4.BIN", .entry = F_EVENT__CHAPTER4_BIN, .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 62,  .pal_offset = 8160,  .pal_count = 1 },
    { .name = "EVTCHR.BIN",   .entry = F_EVENT__EVTCHR_BIN,   .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 200, .pal_offset = 1920,  .pal_count = 7,  .repeat = 137, .repeat_offset = 30720, .data_offset = 2560 },
    { .name = "FRAME.BIN",    .entry = F_EVENT__FRAME_BIN,    .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 288, .pal_offset = 36864, .pal_count = 22, .pal_default = 5 },
    { .name = "ITEM.BIN",     .entry = F_EVENT__ITEM_BIN,     .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 256, .pal_offset = 32768, .pal_count = 16 },
    { .name = "UNIT.BIN",     .entry = F_EVENT__UNIT_BIN,     .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 480, .pal_offset = 61440, .pal_count = 128 },
    { .name = "WLDFACE.BIN",  .entry = F_EVENT__WLDFACE_BIN,  .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 240, .pal_offset = 30720, .pal_count = 64, .repeat = 4,   .repeat_offset = 32768 },
    { .name = "WLDFACE4.BIN", .entry = F_EVENT__WLDFACE4_BIN, .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 240, .pal_offset = 30720, .pal_count = 64 },

    { .name = "OTHER.SPR",    .entry = F_BATTLE__OTHER_SPR,   .type = FFT_IMAGETYPE_4BPP, .width = 256, .height = 256, .pal_offset = 0,     .pal_count = 32, .data_offset = 1024 },
};
// clang-format on

const fft_map_desc_t fft_map_list[FFT_MAP_DESC_LIST_COUNT] = {
    { 0, F_MAP__MAP000_GNS, false, "Unknown" }, // No texture
    { 1, F_MAP__MAP001_GNS, true, "At Main Gate of Igros Castle" },
    { 2, F_MAP__MAP002_GNS, true, "Back Gate of Lesalia Castle" },
    { 3, F_MAP__MAP003_GNS, true, "Hall of St. Murond Temple" },
    { 4, F_MAP__MAP004_GNS, true, "Office of Lesalia Castle" },
    { 5, F_MAP__MAP005_GNS, true, "Roof of Riovanes Castle" },
    { 6, F_MAP__MAP006_GNS, true, "At the Gate of Riovanes Castle" },
    { 7, F_MAP__MAP007_GNS, true, "Inside of Riovanes Castle" },
    { 8, F_MAP__MAP008_GNS, true, "Riovanes Castle" },
    { 9, F_MAP__MAP009_GNS, true, "Citadel of Igros Castle" },
    { 10, F_MAP__MAP010_GNS, true, "Inside of Igros Castle" },
    { 11, F_MAP__MAP011_GNS, true, "Office of Igros Castle" },
    { 12, F_MAP__MAP012_GNS, true, "At the Gate of Lionel Castle" },
    { 13, F_MAP__MAP013_GNS, true, "Inside of Lionel Castle" },
    { 14, F_MAP__MAP014_GNS, true, "Office of Lionel Castle" },
    { 15, F_MAP__MAP015_GNS, true, "At the Gate of Limberry Castle (1)" },
    { 16, F_MAP__MAP016_GNS, true, "Inside of Limberry Castle" },
    { 17, F_MAP__MAP017_GNS, true, "Underground Cemetery of Limberry Castle" },
    { 18, F_MAP__MAP018_GNS, true, "Office of Limberry Castle" },
    { 19, F_MAP__MAP019_GNS, true, "At the Gate of Limberry Castle (2)" },
    { 20, F_MAP__MAP020_GNS, true, "Inside of Zeltennia Castle" },
    { 21, F_MAP__MAP021_GNS, true, "Zeltennia Castle" },
    { 22, F_MAP__MAP022_GNS, true, "Magic City Gariland" },
    { 23, F_MAP__MAP023_GNS, true, "Belouve Residence" },
    { 24, F_MAP__MAP024_GNS, true, "Military Academy's Auditorium" },
    { 25, F_MAP__MAP025_GNS, true, "Yardow Fort City" },
    { 26, F_MAP__MAP026_GNS, true, "Weapon Storage of Yardow" },
    { 27, F_MAP__MAP027_GNS, true, "Goland Coal City" },
    { 28, F_MAP__MAP028_GNS, true, "Colliery Underground First Floor" },
    { 29, F_MAP__MAP029_GNS, true, "Colliery Underground Second Floor" },
    { 30, F_MAP__MAP030_GNS, true, "Colliery Underground Third Floor" },
    { 31, F_MAP__MAP031_GNS, true, "Dorter Trade City" },
    { 32, F_MAP__MAP032_GNS, true, "Slums in Dorter" },
    { 33, F_MAP__MAP033_GNS, true, "Hospital in Slums" },
    { 34, F_MAP__MAP034_GNS, true, "Cellar of Sand Mouse" },
    { 35, F_MAP__MAP035_GNS, true, "Zaland Fort City" },
    { 36, F_MAP__MAP036_GNS, true, "Church Outside of Town" },
    { 37, F_MAP__MAP037_GNS, true, "Ruins Outside Zaland" },
    { 38, F_MAP__MAP038_GNS, true, "Goug Machine City" },
    { 39, F_MAP__MAP039_GNS, true, "Underground Passage in Goland" },
    { 40, F_MAP__MAP040_GNS, true, "Slums in Goug" },
    { 41, F_MAP__MAP041_GNS, true, "Besrodio's House" },
    { 42, F_MAP__MAP042_GNS, true, "Warjilis Trade City" },
    { 43, F_MAP__MAP043_GNS, true, "Port of Warjilis" },
    { 44, F_MAP__MAP044_GNS, true, "Bervenia Free City" },
    { 45, F_MAP__MAP045_GNS, true, "Ruins of Zeltennia Castle's Church" },
    { 46, F_MAP__MAP046_GNS, true, "Cemetery of Heavenly Knight, Balbanes" },
    { 47, F_MAP__MAP047_GNS, true, "Zarghidas Trade City" },
    { 48, F_MAP__MAP048_GNS, true, "Slums of Zarghidas" },
    { 49, F_MAP__MAP049_GNS, true, "Fort Zeakden" },
    { 50, F_MAP__MAP050_GNS, true, "St. Murond Temple" },
    { 51, F_MAP__MAP051_GNS, true, "St. Murond Temple" },
    { 52, F_MAP__MAP052_GNS, true, "Chapel of St. Murond Temple" },
    { 53, F_MAP__MAP053_GNS, true, "Entrance to Death City" },
    { 54, F_MAP__MAP054_GNS, true, "Lost Sacred Precincts" },
    { 55, F_MAP__MAP055_GNS, true, "Graveyard of Airships" },
    { 56, F_MAP__MAP056_GNS, true, "Orbonne Monastery" },
    { 57, F_MAP__MAP057_GNS, true, "Underground Book Storage First Floor" },
    { 58, F_MAP__MAP058_GNS, true, "Underground Book Storage Second Floor" },
    { 59, F_MAP__MAP059_GNS, true, "Underground Book Storage Third Floor" },
    { 60, F_MAP__MAP060_GNS, true, "Underground Book Storage Fourth Floor" },
    { 61, F_MAP__MAP061_GNS, true, "Underground Book Storage Fifth Floor" },
    { 62, F_MAP__MAP062_GNS, true, "Chapel of Orbonne Monastery" },
    { 63, F_MAP__MAP063_GNS, true, "Golgorand Execution Site" },
    { 64, F_MAP__MAP064_GNS, true, "In Front of Bethla Garrison's Sluice" },
    { 65, F_MAP__MAP065_GNS, true, "Granary of Bethla Garrison" },
    { 66, F_MAP__MAP066_GNS, true, "South Wall of Bethla Garrison" },
    { 67, F_MAP__MAP067_GNS, true, "North Wall of Bethla Garrison" },
    { 68, F_MAP__MAP068_GNS, true, "Bethla Garrison" },
    { 69, F_MAP__MAP069_GNS, true, "Murond Death City" },
    { 70, F_MAP__MAP070_GNS, true, "Nelveska Temple" },
    { 71, F_MAP__MAP071_GNS, true, "Dolbodar Swamp" },
    { 72, F_MAP__MAP072_GNS, true, "Fovoham Plains" },
    { 73, F_MAP__MAP073_GNS, true, "Inside of Windmill Shed" },
    { 74, F_MAP__MAP074_GNS, true, "Sweegy Woods" },
    { 75, F_MAP__MAP075_GNS, true, "Bervenia Volcano" },
    { 76, F_MAP__MAP076_GNS, true, "Zeklaus Desert" },
    { 77, F_MAP__MAP077_GNS, true, "Lenalia Plateau" },
    { 78, F_MAP__MAP078_GNS, true, "Zigolis Swamp" },
    { 79, F_MAP__MAP079_GNS, true, "Yuguo Woods" },
    { 80, F_MAP__MAP080_GNS, true, "Araguay Woods" },
    { 81, F_MAP__MAP081_GNS, true, "Grog Hill" },
    { 82, F_MAP__MAP082_GNS, true, "Bed Desert" },
    { 83, F_MAP__MAP083_GNS, true, "Zirekile Falls" },
    { 84, F_MAP__MAP084_GNS, true, "Bariaus Hill" },
    { 85, F_MAP__MAP085_GNS, true, "Mandalia Plains" },
    { 86, F_MAP__MAP086_GNS, true, "Doguola Pass" },
    { 87, F_MAP__MAP087_GNS, true, "Bariaus Valley" },
    { 88, F_MAP__MAP088_GNS, true, "Finath River" },
    { 89, F_MAP__MAP089_GNS, true, "Poeskas Lake" },
    { 90, F_MAP__MAP090_GNS, true, "Germinas Peak" },
    { 91, F_MAP__MAP091_GNS, true, "Thieves Fort" },
    { 92, F_MAP__MAP092_GNS, true, "Igros-Belouve Residence" },
    { 93, F_MAP__MAP093_GNS, true, "Broke Down Shed-Wooden Building" },
    { 94, F_MAP__MAP094_GNS, true, "Broke Down Shed-Stone Building" },
    { 95, F_MAP__MAP095_GNS, true, "Church" },
    { 96, F_MAP__MAP096_GNS, true, "Pub" },
    { 97, F_MAP__MAP097_GNS, true, "Inside Castle Gate in Lesalia" },
    { 98, F_MAP__MAP098_GNS, true, "Outside Castle Gate in Lesalia" },
    { 99, F_MAP__MAP099_GNS, true, "Main Street of Lesalia" },
    { 100, F_MAP__MAP100_GNS, true, "Public Cemetery" },
    { 101, F_MAP__MAP101_GNS, true, "Tutorial (1)" },
    { 102, F_MAP__MAP102_GNS, true, "Tutorial (2)" },
    { 103, F_MAP__MAP103_GNS, true, "Windmill Shed" },
    { 104, F_MAP__MAP104_GNS, true, "Belouve Residence" },
    { 105, F_MAP__MAP105_GNS, true, "TERMINATE" },
    { 106, F_MAP__MAP106_GNS, true, "DELTA" },
    { 107, F_MAP__MAP107_GNS, true, "NOGIAS" },
    { 108, F_MAP__MAP108_GNS, true, "VOYAGE" },
    { 109, F_MAP__MAP109_GNS, true, "BRIDGE" },
    { 110, F_MAP__MAP110_GNS, true, "VALKYRIES" },
    { 111, F_MAP__MAP111_GNS, true, "MLAPAN" },
    { 112, F_MAP__MAP112_GNS, true, "TIGER" },
    { 113, F_MAP__MAP113_GNS, true, "HORROR" },
    { 114, F_MAP__MAP114_GNS, true, "END" },
    { 115, F_MAP__MAP115_GNS, true, "Banished Fort" },
    { 116, F_MAP__MAP116_GNS, true, "Arena" },
    { 117, F_MAP__MAP117_GNS, true, "Unknown" },
    { 118, F_MAP__MAP118_GNS, true, "Unknown" },
    { 119, F_MAP__MAP119_GNS, true, "Unknown" },
    { 120, 0, false, "???" },
    { 121, 0, false, "???" },
    { 122, 0, false, "???" },
    { 123, 0, false, "???" },
    { 124, 0, false, "???" },
    { 125, F_MAP__MAP125_GNS, true, "Unknown" },
    { 126, 0, false, "???" },
    { 127, 0, false, "???" },
};

#endif // FFT_IMPLEMENTATION
