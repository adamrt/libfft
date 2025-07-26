
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

int main(void) {
    fft_init("../heretic/fft.bin");

    fft_span_t event_test = fft_io_open(F_EVENT__TEST_EVT);
    fft_span_t map_map000 = fft_io_open(F_MAP__MAP000_GNS);

    fft_io_close(event_test);
    fft_io_close(map_map000);

    fft_shutdown();
}
