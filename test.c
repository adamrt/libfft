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

static int test_mem_basic_alloc_free(void) {
    // Initialize memory system
    fft_mem_init();

    // Test basic allocation
    void* ptr1 = FFT_MEM_ALLOC(100);
    TEST_ASSERT(ptr1 != NULL, "memory allocation succeeded");
    TEST_ASSERT(_fft_state.mem.allocations_current == 1, "allocation count incremented");
    TEST_ASSERT(_fft_state.mem.usage_current == 100, "usage tracking correct");
    TEST_ASSERT(_fft_state.mem.usage_peak == 100, "peak usage tracked");
    TEST_ASSERT(_fft_state.mem.usage_total == 100, "total usage tracked");

    // Test second allocation
    void* ptr2 = FFT_MEM_ALLOC(50);
    TEST_ASSERT(ptr2 != NULL, "second allocation succeeded");
    TEST_ASSERT(_fft_state.mem.allocations_current == 2, "allocation count is 2");
    TEST_ASSERT(_fft_state.mem.usage_current == 150, "current usage is 150");
    TEST_ASSERT(_fft_state.mem.usage_peak == 150, "peak usage updated to 150");
    TEST_ASSERT(_fft_state.mem.usage_total == 150, "total usage is 150");

    // Test freeing first allocation
    fft_mem_free(ptr1);
    TEST_ASSERT(_fft_state.mem.allocations_current == 1, "allocation count decremented");
    TEST_ASSERT(_fft_state.mem.usage_current == 50, "current usage decreased");
    TEST_ASSERT(_fft_state.mem.usage_peak == 150, "peak usage unchanged");
    TEST_ASSERT(_fft_state.mem.usage_total == 150, "total usage unchanged");

    // Test freeing second allocation
    fft_mem_free(ptr2);
    TEST_ASSERT(_fft_state.mem.allocations_current == 0, "all allocations freed");
    TEST_ASSERT(_fft_state.mem.usage_current == 0, "no memory in use");

    return 1;
}

static int test_mem_free_null(void) {
    fft_mem_init();

    // Test that freeing NULL doesn't crash or affect stats
    size_t initial_count = _fft_state.mem.allocations_current;
    size_t initial_usage = _fft_state.mem.usage_current;

    fft_mem_free(NULL);

    TEST_ASSERT(_fft_state.mem.allocations_current == initial_count, "NULL free doesn't change allocation count");
    TEST_ASSERT(_fft_state.mem.usage_current == initial_usage, "NULL free doesn't change usage");

    return 1;
}

static int test_mem_peak_tracking(void) {
    fft_mem_init();

    // Allocate and free to test peak tracking
    void* ptr1 = FFT_MEM_ALLOC(100);
    TEST_ASSERT(_fft_state.mem.usage_peak == 100, "peak starts at 100");

    void* ptr2 = FFT_MEM_ALLOC(200);
    TEST_ASSERT(_fft_state.mem.usage_peak == 300, "peak increases to 300");

    fft_mem_free(ptr1);
    TEST_ASSERT(_fft_state.mem.usage_peak == 300, "peak stays at 300 after free");
    TEST_ASSERT(_fft_state.mem.usage_current == 200, "current drops to 200");

    void* ptr3 = FFT_MEM_ALLOC(50);
    TEST_ASSERT(_fft_state.mem.usage_peak == 300, "peak unchanged at 300");
    TEST_ASSERT(_fft_state.mem.usage_current == 250, "current is 250");

    fft_mem_free(ptr2);
    fft_mem_free(ptr3);

    return 1;
}

static int test_mem_total_tracking(void) {
    fft_mem_init();

    // Test that total keeps accumulating
    void* ptr1 = FFT_MEM_ALLOC(100);
    TEST_ASSERT(_fft_state.mem.usage_total == 100, "total starts at 100");
    TEST_ASSERT(_fft_state.mem.allocations_total == 1, "total allocations is 1");

    void* ptr2 = FFT_MEM_ALLOC(50);
    TEST_ASSERT(_fft_state.mem.usage_total == 150, "total grows to 150");
    TEST_ASSERT(_fft_state.mem.allocations_total == 2, "total allocations is 2");

    fft_mem_free(ptr1);
    TEST_ASSERT(_fft_state.mem.usage_total == 150, "total unchanged after free");
    TEST_ASSERT(_fft_state.mem.allocations_total == 2, "total allocations unchanged");

    void* ptr3 = FFT_MEM_ALLOC(25);
    TEST_ASSERT(_fft_state.mem.usage_total == 175, "total grows to 175");
    TEST_ASSERT(_fft_state.mem.allocations_total == 3, "total allocations is 3");

    fft_mem_free(ptr2);
    fft_mem_free(ptr3);

    return 1;
}

static int test_time_str(void) {
    const char* day_str = fft_time_str(FFT_TIME_DAY);
    TEST_ASSERT(strcmp(day_str, "Day") == 0, "FFT_TIME_DAY string");

    const char* night_str = fft_time_str(FFT_TIME_NIGHT);
    TEST_ASSERT(strcmp(night_str, "Night") == 0, "FFT_TIME_NIGHT string");

    const char* unknown_str = fft_time_str((fft_time_e)99);
    TEST_ASSERT(strcmp(unknown_str, "Unknown") == 0, "invalid time enum string");

    return 1;
}

static int test_weather_str(void) {
    const char* none_str = fft_weather_str(FFT_WEATHER_NONE);
    TEST_ASSERT(strcmp(none_str, "None") == 0, "FFT_WEATHER_NONE string");

    const char* none_alt_str = fft_weather_str(FFT_WEATHER_NONE_ALT);
    TEST_ASSERT(strcmp(none_alt_str, "NoneAlt") == 0, "FFT_WEATHER_NONE_ALT string");

    const char* normal_str = fft_weather_str(FFT_WEATHER_NORMAL);
    TEST_ASSERT(strcmp(normal_str, "Normal") == 0, "FFT_WEATHER_NORMAL string");

    const char* strong_str = fft_weather_str(FFT_WEATHER_STRONG);
    TEST_ASSERT(strcmp(strong_str, "Strong") == 0, "FFT_WEATHER_STRONG string");

    const char* very_strong_str = fft_weather_str(FFT_WEATHER_VERY_STRONG);
    TEST_ASSERT(strcmp(very_strong_str, "VeryStrong") == 0, "FFT_WEATHER_VERY_STRONG string");

    const char* unknown_str = fft_weather_str((fft_weather_e)99);
    TEST_ASSERT(strcmp(unknown_str, "Unknown") == 0, "invalid weather enum string");

    return 1;
}

static int test_recordtype_str(void) {
    const char* none_str = fft_recordtype_str(FFT_RECORDTYPE_NONE);
    TEST_ASSERT(strcmp(none_str, "Unknown") == 0, "FFT_RECORDTYPE_NONE string");

    const char* texture_str = fft_recordtype_str(FFT_RECORDTYPE_TEXTURE);
    TEST_ASSERT(strcmp(texture_str, "Texture") == 0, "FFT_RECORDTYPE_TEXTURE string");

    const char* primary_str = fft_recordtype_str(FFT_RECORDTYPE_MESH_PRIMARY);
    TEST_ASSERT(strcmp(primary_str, "Primary") == 0, "FFT_RECORDTYPE_MESH_PRIMARY string");

    const char* override_str = fft_recordtype_str(FFT_RECORDTYPE_MESH_OVERRIDE);
    TEST_ASSERT(strcmp(override_str, "Override") == 0, "FFT_RECORDTYPE_MESH_OVERRIDE string");

    const char* alt_str = fft_recordtype_str(FFT_RECORDTYPE_MESH_ALT);
    TEST_ASSERT(strcmp(alt_str, "Alt") == 0, "FFT_RECORDTYPE_MESH_ALT string");

    const char* end_str = fft_recordtype_str(FFT_RECORDTYPE_END);
    TEST_ASSERT(strcmp(end_str, "End") == 0, "FFT_RECORDTYPE_END string");

    const char* unknown_str = fft_recordtype_str((fft_recordtype_e)0x9999);
    TEST_ASSERT(strcmp(unknown_str, "Unknown") == 0, "invalid recordtype enum string");

    return 1;
}

static int test_state_functions(void) {
    // Test default state
    fft_state_t default_state = { FFT_TIME_DAY, FFT_WEATHER_NONE, 0 };
    TEST_ASSERT(fft_state_is_default(default_state), "default state is recognized");

    // Test non-default states
    fft_state_t night_state = { FFT_TIME_NIGHT, FFT_WEATHER_NONE, 0 };
    TEST_ASSERT(!fft_state_is_default(night_state), "night state is not default");

    fft_state_t weather_state = { FFT_TIME_DAY, FFT_WEATHER_NORMAL, 0 };
    TEST_ASSERT(!fft_state_is_default(weather_state), "weather state is not default");

    fft_state_t layout_state = { FFT_TIME_DAY, FFT_WEATHER_NONE, 1 };
    TEST_ASSERT(!fft_state_is_default(layout_state), "layout state is not default");

    // Test state equality
    fft_state_t state1 = { FFT_TIME_NIGHT, FFT_WEATHER_STRONG, 2 };
    fft_state_t state2 = { FFT_TIME_NIGHT, FFT_WEATHER_STRONG, 2 };
    fft_state_t state3 = { FFT_TIME_DAY, FFT_WEATHER_STRONG, 2 };

    TEST_ASSERT(fft_state_is_equal(state1, state2), "identical states are equal");
    TEST_ASSERT(!fft_state_is_equal(state1, state3), "different states are not equal");

    return 1;
}

static int test_record_parsing(void) {
    // Create simple test record data - test basic bit manipulation
    uint8_t test_byte = 0x90; // 1001 0000

    // Test time extraction (high bit)
    fft_time_e time = (fft_time_e)((test_byte >> 7) & 0x1);
    TEST_ASSERT(time == FFT_TIME_NIGHT, "time bit extraction");

    // Test weather extraction (bits 4-6)
    fft_weather_e weather = (fft_weather_e)((test_byte >> 4) & 0x7);
    TEST_ASSERT(weather == FFT_WEATHER_NONE_ALT, "weather bit extraction");

    // Test basic constants
    TEST_ASSERT(FFT_RECORD_SIZE == 20, "record size constant");
    TEST_ASSERT(FFT_RECORDTYPE_TEXTURE == 0x1701, "texture record type constant");

    return 1;
}

static int test_mem_alloc_with_tag(void) {
    fft_mem_init();

    // Test allocation with tag
    void* ptr1 = FFT_MEM_ALLOC_TAG(100, "test_buffer");
    TEST_ASSERT(ptr1 != NULL, "tagged allocation succeeded");
    TEST_ASSERT(_fft_state.mem.allocations_current == 1, "allocation count correct");
    TEST_ASSERT(_fft_state.mem.usage_current == 100, "usage tracking correct");

    // Test allocation without tag (should still work)
    void* ptr2 = FFT_MEM_ALLOC(50);
    TEST_ASSERT(ptr2 != NULL, "untagged allocation succeeded");
    TEST_ASSERT(_fft_state.mem.allocations_current == 2, "allocation count is 2");

    FFT_MEM_FREE(ptr1);
    FFT_MEM_FREE(ptr2);
    TEST_ASSERT(_fft_state.mem.allocations_current == 0, "all allocations freed");

    return 1;
}

static int test_io_file_desc_lookup(void) {
    // Test finding a file that exists
    fft_io_desc_t desc = fft_io_get_file_desc(1000); // F_BATTLE_BIN sector
    TEST_ASSERT(desc.sector == 1000, "found correct sector");
    TEST_ASSERT(desc.size == 1397096, "found correct size");
    TEST_ASSERT(strcmp(desc.name, "BATTLE.BIN") == 0, "found correct name");

    // Test finding a file that doesn't exist
    fft_io_desc_t invalid_desc = fft_io_get_file_desc(99999);
    TEST_ASSERT(invalid_desc.sector == 0, "invalid file returns zero sector");
    TEST_ASSERT(invalid_desc.size == 0, "invalid file returns zero size");
    TEST_ASSERT(invalid_desc.name == NULL, "invalid file returns null name");

    return 1;
}

int main(void) {
    printf("Running libFFT tests...\n\n");

    // Span tests
    RUN_TEST(test_span_read_u8);
    RUN_TEST(test_span_read_i8);
    RUN_TEST(test_span_read_u16);
    RUN_TEST(test_span_read_i16);
    RUN_TEST(test_span_read_u32);
    RUN_TEST(test_span_read_i32);
    RUN_TEST(test_span_read_bytes);

    // Memory tests
    RUN_TEST(test_mem_basic_alloc_free);
    RUN_TEST(test_mem_free_null);
    RUN_TEST(test_mem_peak_tracking);
    RUN_TEST(test_mem_total_tracking);
    RUN_TEST(test_mem_alloc_with_tag);

    // String function tests
    RUN_TEST(test_time_str);
    RUN_TEST(test_weather_str);
    RUN_TEST(test_recordtype_str);

    // State function tests
    RUN_TEST(test_state_functions);

    // Record parsing tests
    RUN_TEST(test_record_parsing);

    // IO function tests
    RUN_TEST(test_io_file_desc_lookup);

    printf("\nAll tests passed!\n");
    return 0;
}
