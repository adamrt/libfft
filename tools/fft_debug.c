// This is mainly for debugging and testing purposes.
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

void read_geometry(void);

int main(void) {
    fft_init("../heretic/fft.bin");
    {
        read_geometry();
    }
    fft_shutdown();
}

void read_geometry(void) {

    for (uint32_t i = 0; i < FFT_MAP_DESC_LIST_COUNT; i++) {
        fft_map_desc_t desc = fft_map_list[i];
        if (desc.valid == false) {
            continue;
        }

        fft_map_data_t* data = fft_map_data_read(desc.id);

        for (uint32_t j = 0; j < data->record_count; j++) {
            fft_record_t record = data->records[j];
            if (record.type == FFT_RECORDTYPE_TEXTURE) {
                continue;
            }

            printf("------------------------\n");
            printf("MAP ID: %u\n", desc.id);
            printf("Record ID: %u\n", j);
            printf("has_geometry: %s\n", record.meta.has_geometry ? "true" : "false");
            printf("has_clut: %s\n", record.meta.has_clut ? "true" : "false");
            printf("has_lighting: %s\n", record.meta.has_lighting ? "true" : "false");
            printf("has_terrain: %s\n", record.meta.has_terrain ? "true" : "false");
            printf("polygon_count: %u\n", record.meta.polygon_count);
            printf("tex_tri_count: %u\n", record.meta.tex_tri_count);
            printf("tex_quad_count: %u\n", record.meta.tex_quad_count);
            printf("untex_tri_count: %u\n", record.meta.untex_tri_count);
            printf("untex_quad_count: %u\n", record.meta.untex_quad_count);
            printf("light_count: %u\n", record.meta.light_count);
        }

        fft_map_data_destroy(data);
    }
}
