#include <stdio.h>
#include <sys/stat.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

#define MKDIRF(mode, fmt, ...)                      \
    do {                                            \
        char _p[64];                                \
        snprintf(_p, sizeof(_p), fmt, __VA_ARGS__); \
        mkdir(_p, mode);                            \
    } while (0)

static void write_images_to_disk(fft_span_t* file, fft_image_desc_t desc);

int main(void) {
    mkdir("./images", 0777);

    fft_init("../heretic/fft.bin");
    {
        for (uint32_t i = 0; i < FFT_IMAGE_DESC_COUNT; i++) {
            fft_image_desc_t desc = image_desc_list[i];
            fft_span_t file = fft_io_open(desc.entry);
            write_images_to_disk(&file, desc);
            fft_io_close(file);
        }
    }
    fft_shutdown();
}

static void write_images_to_disk(fft_span_t* file, fft_image_desc_t desc) {
    uint32_t repeat = desc.repeat > 0 ? desc.repeat : 1;

    for (uint8_t i = 0; i < repeat; i++) {
        for (uint8_t j = 0; j < desc.pal_count; j++) {

            file->offset = desc.data_offset + (desc.repeat_offset * i);
            fft_image_t image = fft_image_read_4bpp_palettized(file, desc, j);

            // Write the image to disk
            char path[64];
            if (desc.pal_count == 1) {
                snprintf(path, sizeof(path), "./images/%s.ppm", desc.name);
            } else {
                MKDIRF(0777, "./images/%s", desc.name);
                if (desc.repeat > 1) {
                    snprintf(path, sizeof(path), "./images/%s/%d_%d.ppm", desc.name, i, j);
                } else {
                    snprintf(path, sizeof(path), "./images/%s/%d.ppm", desc.name, j);
                }
            }

            fft_image_write_ppm(&image, path);

            FFT_MEM_FREE(image.data);
        }
    }
    printf("Processed %s\n", desc.name);
}
