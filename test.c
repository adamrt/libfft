// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Adam Patterson

#include <stdio.h>
#include <string.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

// Test macros
#define TEST_ASSERT(condition, test_name)    \
    do {                                     \
        if (!(condition)) {                  \
            printf("FAIL: %s\n", test_name); \
            return 0;                        \
        }                                    \
    } while (0)

#define RUN_TEST(test_func)                          \
    do {                                             \
        printf("Running %s...\n", #test_func);       \
        if (!test_func()) {                          \
            printf("Test %s failed!\n", #test_func); \
            return 1;                                \
        }                                            \
    } while (0)

// Test data - raw bytes representing different types
static uint8_t test_data[] = {
    // clang-format off
    // u8 values
    0x42,                           // 66
    0xFF,                           // 255
    
    // i8 values  
    0x80,                           // -128
    0x7F,                           // 127
    
    // u16 values (little-endian)
    0x34, 0x12,                     // 0x1234 = 4660
    0xFF, 0xFF,                     // 0xFFFF = 65535
    
    // i16 values (little-endian)
    0x00, 0x80,                     // 0x8000 = -32768
    0xFF, 0x7F,                     // 0x7FFF = 32767
    
    // u32 values (little-endian)
    0x78, 0x56, 0x34, 0x12,         // 0x12345678 = 305419896
    0xFF, 0xFF, 0xFF, 0xFF,         // 0xFFFFFFFF = 4294967295
    
    // i32 values (little-endian)
    0x00, 0x00, 0x00, 0x80,         // 0x80000000 = -2147483648
    0xFF, 0xFF, 0xFF, 0x7F,         // 0x7FFFFFFF = 2147483647
    // clang-format on
};

static int test_span_read_u8(void) {
    fft_span_t span = { test_data, sizeof(test_data), 0 };

    uint8_t val1 = fft_span_read_u8(&span);
    TEST_ASSERT(val1 == 66, "u8 read first value");
    TEST_ASSERT(span.offset == 1, "u8 offset after first read");

    uint8_t val2 = fft_span_read_u8(&span);
    TEST_ASSERT(val2 == 255, "u8 read second value");
    TEST_ASSERT(span.offset == 2, "u8 offset after second read");

    return 1;
}

static int test_span_read_i8(void) {
    fft_span_t span = { test_data, sizeof(test_data), 2 }; // Start at i8 data

    int8_t val1 = fft_span_read_i8(&span);
    TEST_ASSERT(val1 == -128, "i8 read first value");
    TEST_ASSERT(span.offset == 3, "i8 offset after first read");

    int8_t val2 = fft_span_read_i8(&span);
    TEST_ASSERT(val2 == 127, "i8 read second value");
    TEST_ASSERT(span.offset == 4, "i8 offset after second read");

    return 1;
}

static int test_span_read_u16(void) {
    fft_span_t span = { test_data, sizeof(test_data), 4 }; // Start at u16 data

    uint16_t val1 = fft_span_read_u16(&span);
    TEST_ASSERT(val1 == 4660, "u16 read first value");
    TEST_ASSERT(span.offset == 6, "u16 offset after first read");

    uint16_t val2 = fft_span_read_u16(&span);
    TEST_ASSERT(val2 == 65535, "u16 read second value");
    TEST_ASSERT(span.offset == 8, "u16 offset after second read");

    return 1;
}

static int test_span_read_i16(void) {
    fft_span_t span = { test_data, sizeof(test_data), 8 }; // Start at i16 data

    int16_t val1 = fft_span_read_i16(&span);
    TEST_ASSERT(val1 == -32768, "i16 read first value");
    TEST_ASSERT(span.offset == 10, "i16 offset after first read");

    int16_t val2 = fft_span_read_i16(&span);
    TEST_ASSERT(val2 == 32767, "i16 read second value");
    TEST_ASSERT(span.offset == 12, "i16 offset after second read");

    return 1;
}

static int test_span_read_u32(void) {
    fft_span_t span = { test_data, sizeof(test_data), 12 }; // Start at u32 data

    uint32_t val1 = fft_span_read_u32(&span);
    TEST_ASSERT(val1 == 305419896, "u32 read first value");
    TEST_ASSERT(span.offset == 16, "u32 offset after first read");

    uint32_t val2 = fft_span_read_u32(&span);
    TEST_ASSERT(val2 == 4294967295, "u32 read second value");
    TEST_ASSERT(span.offset == 20, "u32 offset after second read");

    return 1;
}

static int test_span_read_i32(void) {
    fft_span_t span = { test_data, sizeof(test_data), 20 }; // Start at i32 data

    int32_t val1 = fft_span_read_i32(&span);
    TEST_ASSERT(val1 == -2147483648, "i32 read first value");
    TEST_ASSERT(span.offset == 24, "i32 offset after first read");

    int32_t val2 = fft_span_read_i32(&span);
    TEST_ASSERT(val2 == 2147483647, "i32 read second value");
    TEST_ASSERT(span.offset == 28, "i32 offset after second read");

    return 1;
}

static int test_span_read_bytes(void) {
    fft_span_t span = { test_data, sizeof(test_data), 0 };
    uint8_t buffer[4];

    fft_span_read_bytes(&span, 4, buffer);
    TEST_ASSERT(buffer[0] == 66, "span_read_bytes first byte");
    TEST_ASSERT(buffer[1] == 255, "span_read_bytes second byte");
    TEST_ASSERT(buffer[2] == 128, "span_read_bytes third byte");
    TEST_ASSERT(buffer[3] == 127, "span_read_bytes fourth byte");
    TEST_ASSERT(span.offset == 4, "span_read_bytes offset");

    return 1;
}

int main(void) {
    printf("Running libFFT tests...\n\n");

    RUN_TEST(test_span_read_u8);
    RUN_TEST(test_span_read_i8);
    RUN_TEST(test_span_read_u16);
    RUN_TEST(test_span_read_i16);
    RUN_TEST(test_span_read_u32);
    RUN_TEST(test_span_read_i32);
    RUN_TEST(test_span_read_bytes);

    printf("\nAll tests passed!\n");
    return 0;
}
