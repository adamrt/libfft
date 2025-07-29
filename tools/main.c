
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

void read_gns_records(void);
void read_and_write_image(void);

int main(void) {
    fft_init("../heretic/fft.bin");
    {
        read_and_write_image();
    }
    fft_shutdown();
}

void read_gns_records(void) {
    fft_record_t records[100]; // reused buffer for reading records

    fft_io_entry_e map_ids[] = {
        F_MAP__MAP001_GNS,
        F_MAP__MAP002_GNS,
        F_MAP__MAP003_GNS,
        F_MAP__MAP004_GNS,
        F_MAP__MAP005_GNS,
        F_MAP__MAP006_GNS,
        F_MAP__MAP007_GNS,
        F_MAP__MAP008_GNS,
        F_MAP__MAP009_GNS,
        F_MAP__MAP010_GNS,
    };

    for (uint32_t i = 0; i < 10; i++) {
        fft_span_t map = fft_io_open(map_ids[i]);

        uint32_t count = fft_record_read_all(&map, records);
        printf("Map %d Records: %d\n", i, count);

        fft_io_close(map);
    }
}

void read_and_write_image(void) {
    // { .type = IMG_4BPP_PAL, .width = 256, .height = 256, .pal_offset = 32768, .pal_count = 16 },
    // fft_span_t file = fft_io_open(F_EVENT__ITEM_BIN);
    fft_span_t file = fft_io_open(F_EVENT__UNIT_BIN);

    // fft_image_t image = fft_image_read_4bpp(&file, 256, 256);
    fft_image_t image = fft_image_read_4bpp(&file, 256, 480);

    // file.offset = 32768; // Skip the palettes offset
    file.offset = 61440; // Skip the palettes offset
    fft_image_t palette = fft_image_read_16bpp(&file, 16, 1);

    fft_image_palettize(&image, &palette);

    fft_image_write_ppm(&image, "output_image.ppm");

    fft_io_close(file);
}
