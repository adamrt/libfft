// This is mainly for debugging and testing purposes.
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

void read_gns_records(void);
void write_images_to_disk(void);

int main(void) {
    fft_init("../heretic/fft.bin");
    {
        read_gns_records();
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
