// This is mainly for debugging and testing purposes.
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

void read_map_data(void);
void read_scenarios(void);

int main(void) {
    fft_init("../heretic/fft.bin");
    {
        read_map_data();
        read_map_data();
    }
    fft_shutdown();
}

void read_map_data(void) {
    for (uint32_t i = 0; i < FFT_MAP_DESC_LIST_COUNT; i++) {
        fft_map_desc_t desc = fft_map_list[i];
        if (desc.valid == false) {
            continue;
        }

        fft_map_data_t* data = fft_map_data_read(desc.id);
        fft_map_data_destroy(data);
    }
}

void read_scenarios(void) {
    for (uint32_t i = 0; i < FFT_EVENT_COUNT; i++) {
        fft_event_desc_t desc = fft_event_desc_list[i];
        if (desc.usable == false) {
            continue;
        }
        fft_scenario_t scenario = fft_scenario_get_scenario(desc.scenario_id);
        (void)scenario;
    }
}
