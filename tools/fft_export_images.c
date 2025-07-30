#include <stdio.h>
#include <sys/stat.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

void read_gns_records(void);
void write_images_to_disk(void);

int main(void) {
    fft_init("../heretic/fft.bin");
    {
        write_images_to_disk();
    }
    fft_shutdown();
}

// Save images files into "./images/<FILE>/<PALLETE.ID>.ppm"
void write_images_to_disk(void) {
    for (uint32_t i = 0; i < FFT_IMAGE_DESC_COUNT; i++) {
        fft_image_desc_t desc = image_desc_list[i];
        fft_span_t file = fft_io_open(desc.entry);

        mkdir("./images/", 0777);

        char dir[64];
        snprintf(dir, 64, "./images/%s", desc.name);
        mkdir(dir, 0777);

        for (uint8_t j = 0; j < desc.pal_count; j++) {
            char path[64];
            file.offset = desc.data_offset;
            fft_image_t image = fft_image_read_4bpp_palettized(&file, desc, j);
            snprintf(path, 64, "./images/%s/%d.ppm", desc.name, j);
            printf("Writing image %s\n", path);
            fft_image_write_ppm(&image, path);
            FFT_MEM_FREE(image.data);
        }

        fft_io_close(file);
    }
}
