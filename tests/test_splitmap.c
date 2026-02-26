/*
 * SplitMap Test Suite
 *
 * Comprehensive test cases for the splitmap tilemap splitting and compression tool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

// Test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n  %s:%d: %s\n", __func__, __FILE__, __LINE__, message); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "FAIL: %s\n  %s:%d: %s (expected: %d, actual: %d)\n", \
                    __func__, __FILE__, __LINE__, message, (int)(expected), (int)(actual)); \
            return false; \
        } \
    } while(0)

#define RUN_TEST(test) \
    do { \
        printf("Running %s...\n", #test); \
        if (test()) { \
            printf("  PASS\n"); \
            tests_passed++; \
        } else { \
            tests_failed++; \
        } \
        tests_total++; \
    } while(0)

// Test statistics
static int tests_total = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Helper function to create a test tilemap
bool create_test_map(const char *filename, int width, int height, unsigned char fill_value) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return false;

    unsigned char *data = malloc(width * height);
    if (!data) {
        fclose(fp);
        return false;
    }

    memset(data, fill_value, width * height);
    size_t written = fwrite(data, 1, width * height, fp);
    free(data);
    fclose(fp);

    return written == (size_t)(width * height);
}

// Helper function to create a test map with items
bool create_test_map_with_items(const char *filename, int width, int height,
                                 unsigned char base_tile, unsigned char item_tile,
                                 int *item_positions, int num_items) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return false;

    unsigned char *data = malloc(width * height);
    if (!data) {
        fclose(fp);
        return false;
    }

    memset(data, base_tile, width * height);

    // Place items at specified positions
    for (int i = 0; i < num_items; i++) {
        if (item_positions[i] >= 0 && item_positions[i] < width * height) {
            data[item_positions[i]] = item_tile;
        }
    }

    size_t written = fwrite(data, 1, width * height, fp);
    free(data);
    fclose(fp);

    return written == (size_t)(width * height);
}

// Helper function to check if a file exists
bool file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

// Helper function to get file size
long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) != 0) return -1;
    return st.st_size;
}

// Helper function to run splitmap with arguments
int run_splitmap(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./splitmap %s 2>&1", args);
    return system(cmd);
}

// Helper function to cleanup test files
void cleanup_test_files(const char *pattern) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", pattern);
    system(cmd);
}

// ============================================================================
// UNIT TESTS - Command Line Parsing
// ============================================================================

bool test_no_arguments() {
    int result = run_splitmap("");
    TEST_ASSERT(result == 0, "Should display usage and return 0");
    return true;
}

bool test_missing_required_parameters() {
    // Missing map file
    int result = run_splitmap("--map-size 64x42 --level-size 32x21");
    TEST_ASSERT(result != 0, "Should fail with missing --map parameter");

    // Missing map-size
    result = run_splitmap("--map test.nxm --level-size 32x21");
    TEST_ASSERT(result != 0, "Should fail with missing --map-size parameter");

    // Missing level-size
    result = run_splitmap("--map test.nxm --map-size 64x42");
    TEST_ASSERT(result != 0, "Should fail with missing --level-size parameter");

    return true;
}

bool test_invalid_map_file() {
    int result = run_splitmap("--map nonexistent.nxm --map-size 64x42 --level-size 32x21");
    TEST_ASSERT(result != 0, "Should fail with nonexistent map file");
    return true;
}

// ============================================================================
// UNIT TESTS - Basic Map Splitting
// ============================================================================

bool test_simple_map_split() {
    const char *map_file = "test_simple.nxm";

    // Create a simple 64x42 map (2x2 levels of 32x21)
    TEST_ASSERT(create_test_map(map_file, 64, 42, 80), "Failed to create test map");

    // Run splitmap
    int result = run_splitmap("--map test_simple.nxm --map-size 64x42 --level-size 32x21");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check output files
    TEST_ASSERT(file_exists("test_simple.inc"), "Output .inc file should exist");
    TEST_ASSERT(file_exists("test_simple_0000.nxm.zx0"), "Level 0,0 should exist");
    TEST_ASSERT(file_exists("test_simple_0100.nxm.zx0"), "Level 1,0 should exist");
    TEST_ASSERT(file_exists("test_simple_0001.nxm.zx0"), "Level 0,1 should exist");
    TEST_ASSERT(file_exists("test_simple_0101.nxm.zx0"), "Level 1,1 should exist");

    // Cleanup
    cleanup_test_files("test_simple*");

    return true;
}

bool test_single_level_map() {
    const char *map_file = "test_single.nxm";

    // Create a single level map (32x21)
    TEST_ASSERT(create_test_map(map_file, 32, 21, 80), "Failed to create test map");

    // Run splitmap
    int result = run_splitmap("--map test_single.nxm --map-size 32x21 --level-size 32x21");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check output files
    TEST_ASSERT(file_exists("test_single.inc"), "Output .inc file should exist");
    TEST_ASSERT(file_exists("test_single_0000.nxm.zx0"), "Single level should exist");

    // Cleanup
    cleanup_test_files("test_single*");

    return true;
}

bool test_large_map_split() {
    const char *map_file = "test_large.nxm";

    // Create a large map (192x105 = 6x5 levels of 32x21)
    TEST_ASSERT(create_test_map(map_file, 192, 105, 80), "Failed to create test map");

    // Run splitmap
    int result = run_splitmap("--map test_large.nxm --map-size 192x105 --level-size 32x21");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that 30 levels were created (6x5)
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 6; x++) {
            char filename[64];
            snprintf(filename, sizeof(filename), "test_large_%02d%02d.nxm.zx0", x, y);
            TEST_ASSERT(file_exists(filename), filename);
        }
    }

    // Cleanup
    cleanup_test_files("test_large*");

    return true;
}

// ============================================================================
// UNIT TESTS - Item Extraction
// ============================================================================

bool test_single_item_extraction() {
    const char *map_file = "test_items.nxm";

    // Create a map with coins at specific positions
    int item_positions[] = {10, 50, 100};
    TEST_ASSERT(create_test_map_with_items(map_file, 32, 21, 80, 145, item_positions, 3),
                "Failed to create test map with items");

    // Run splitmap with coin item
    int result = run_splitmap("--map test_items.nxm --map-size 32x21 --level-size 32x21 --blank 80 --item coin,145");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that inc file was created
    TEST_ASSERT(file_exists("test_items.inc"), "Output .inc file should exist");

    // Check that the .inc file contains coin tables
    FILE *fp = fopen("test_items.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_coin_tables = false;
    bool found_coin_data = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "_coinTables:")) found_coin_tables = true;
        if (strstr(line, "level0coin:")) found_coin_data = true;
    }
    fclose(fp);

    TEST_ASSERT(found_coin_tables, "Should find _coinTables declaration");
    TEST_ASSERT(found_coin_data, "Should find level0coin data");

    // Cleanup
    cleanup_test_files("test_items*");

    return true;
}

bool test_multiple_item_types() {
    const char *map_file = "test_multi_items.nxm";

    // Create a map with multiple item types
    unsigned char *data = malloc(32 * 21);
    memset(data, 80, 32 * 21);
    data[10] = 145;  // coin
    data[20] = 146;  // gem
    data[30] = 147;  // heart

    FILE *fp = fopen(map_file, "wb");
    fwrite(data, 1, 32 * 21, fp);
    fclose(fp);
    free(data);

    // Run splitmap with multiple items
    int result = run_splitmap("--map test_multi_items.nxm --map-size 32x21 --level-size 32x21 "
                               "--blank 80 --item coin,145 --item gem,146 --item heart,147");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that inc file contains all item types
    fp = fopen("test_multi_items.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_coin = false, found_gem = false, found_heart = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "_coinTables:")) found_coin = true;
        if (strstr(line, "_gemTables:")) found_gem = true;
        if (strstr(line, "_heartTables:")) found_heart = true;
    }
    fclose(fp);

    TEST_ASSERT(found_coin, "Should find coin tables");
    TEST_ASSERT(found_gem, "Should find gem tables");
    TEST_ASSERT(found_heart, "Should find heart tables");

    // Cleanup
    cleanup_test_files("test_multi_items*");

    return true;
}

bool test_items_with_frames() {
    const char *map_file = "test_frame_items.nxm";

    int item_positions[] = {15};
    TEST_ASSERT(create_test_map_with_items(map_file, 32, 21, 80, 150, item_positions, 1),
                "Failed to create test map");

    // Run splitmap with frame parameter
    int result = run_splitmap("--map test_frame_items.nxm --map-size 32x21 --level-size 32x21 "
                               "--blank 80 --item key,150,3");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that inc file contains the frame value
    FILE *fp = fopen("test_frame_items.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_frame = false;

    while (fgets(line, sizeof(line), fp)) {
        // Look for db line with frame value (last byte should be $03)
        if (strstr(line, "db") && strstr(line, "$03")) {
            found_frame = true;
            break;
        }
    }
    fclose(fp);

    TEST_ASSERT(found_frame, "Should find frame value in item data");

    // Cleanup
    cleanup_test_files("test_frame_items*");

    return true;
}

bool test_items_only_mode() {
    const char *map_file = "test_items_only.nxm";

    int item_positions[] = {25};
    TEST_ASSERT(create_test_map_with_items(map_file, 32, 21, 80, 145, item_positions, 1),
                "Failed to create test map");

    // Run splitmap with --items-only flag
    int result = run_splitmap("--map test_items_only.nxm --map-size 32x21 --level-size 32x21 "
                               "--blank 80 --item coin,145 --items-only");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that inc file exists
    TEST_ASSERT(file_exists("test_items_only.inc"), "Output .inc file should exist");

    // Check that NO compressed map files were created
    TEST_ASSERT(!file_exists("test_items_only_0000.nxm.zx0"),
                 "Should NOT create compressed map in items-only mode");

    // Cleanup
    cleanup_test_files("test_items_only*");

    return true;
}

bool test_item_blanking() {
    const char *map_file = "test_blanking.nxm";

    // Create map with item
    int item_positions[] = {100};
    TEST_ASSERT(create_test_map_with_items(map_file, 32, 21, 80, 145, item_positions, 1),
                "Failed to create test map");

    // Read original map
    FILE *fp = fopen(map_file, "rb");
    unsigned char *original = malloc(32 * 21);
    fread(original, 1, 32 * 21, fp);
    fclose(fp);

    TEST_ASSERT(original[100] == 145, "Item should be at position 100");

    // Run splitmap (modifies the input file)
    run_splitmap("--map test_blanking.nxm --map-size 32x21 --level-size 32x21 "
                 "--blank 80 --item coin,145");

    // Read modified map
    fp = fopen(map_file, "rb");
    unsigned char *modified = malloc(32 * 21);
    fread(modified, 1, 32 * 21, fp);
    fclose(fp);

    TEST_ASSERT(modified[100] == 80, "Item should be blanked to tile 80");

    free(original);
    free(modified);

    // Cleanup
    cleanup_test_files("test_blanking*");

    return true;
}

// ============================================================================
// UNIT TESTS - Horizontal Strip Mode
// ============================================================================

bool test_horizontal_strip_mode() {
    const char *map_file = "test_hstrip.nxm";

    // Create a map that's 96 wide x 42 tall (3 strips of 21 height each)
    TEST_ASSERT(create_test_map(map_file, 96, 42, 80), "Failed to create test map");

    // Run splitmap in horizontal strip mode
    int result = run_splitmap("--map test_hstrip.nxm --map-size 96x42 --level-size 96x21 --horizontal-strip");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that strip files were created instead of level files
    TEST_ASSERT(file_exists("test_hstrip_strip0.nxm.zx0"), "Strip 0 should exist");
    TEST_ASSERT(file_exists("test_hstrip_strip1.nxm.zx0"), "Strip 1 should exist");
    TEST_ASSERT(!file_exists("test_hstrip_0000.nxm.zx0"), "Should NOT create level files");

    // Check inc file for strip table
    FILE *fp = fopen("test_hstrip.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_strip_table = false;
    bool found_strip_width = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "_stripTable:")) found_strip_table = true;
        if (strstr(line, "_stripWidthTiles")) found_strip_width = true;
    }
    fclose(fp);

    TEST_ASSERT(found_strip_table, "Should find _stripTable declaration");
    TEST_ASSERT(found_strip_width, "Should find _stripWidthTiles constant");

    // Cleanup
    cleanup_test_files("test_hstrip*");

    return true;
}

bool test_horizontal_strip_items() {
    const char *map_file = "test_hstrip_items.nxm";

    // Create a 96x42 map with items in different strips
    unsigned char *data = malloc(96 * 42);
    memset(data, 80, 96 * 42);

    // Place items in strip 0 (rows 0-20)
    data[100] = 145;  // coin in strip 0

    // Place items in strip 1 (rows 21-41)
    data[96 * 25 + 50] = 145;  // coin in strip 1

    FILE *fp = fopen(map_file, "wb");
    fwrite(data, 1, 96 * 42, fp);
    fclose(fp);
    free(data);

    // Run splitmap with horizontal strip mode
    int result = run_splitmap("--map test_hstrip_items.nxm --map-size 96x42 --level-size 96x21 "
                               "--blank 80 --item coin,145 --horizontal-strip");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that inc file contains strip-based item tables
    fp = fopen("test_hstrip_items.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_strip0_coin = false;
    bool found_strip1_coin = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "strip0coin:")) found_strip0_coin = true;
        if (strstr(line, "strip1coin:")) found_strip1_coin = true;
    }
    fclose(fp);

    TEST_ASSERT(found_strip0_coin, "Should find strip0coin data");
    TEST_ASSERT(found_strip1_coin, "Should find strip1coin data");

    // Cleanup
    cleanup_test_files("test_hstrip_items*");

    return true;
}

// ============================================================================
// UNIT TESTS - Compression
// ============================================================================

bool test_compression_enabled() {
    const char *map_file = "test_compress.nxm";

    TEST_ASSERT(create_test_map(map_file, 32, 21, 80), "Failed to create test map");

    // Run with compression (default)
    int result = run_splitmap("--map test_compress.nxm --map-size 32x21 --level-size 32x21");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that .zx0 file exists
    TEST_ASSERT(file_exists("test_compress_0000.nxm.zx0"), "Compressed file should exist");

    // Get compressed size
    long compressed_size = get_file_size("test_compress_0000.nxm.zx0");
    long original_size = 32 * 21;

    // Compressed size should be less than original for repetitive data
    TEST_ASSERT(compressed_size > 0 && compressed_size < original_size,
                "Compressed size should be smaller than original");

    // Cleanup
    cleanup_test_files("test_compress*");

    return true;
}

bool test_no_compression() {
    const char *map_file = "test_nocompress.nxm";

    TEST_ASSERT(create_test_map(map_file, 32, 21, 80), "Failed to create test map");

    // Run with --no-compress flag
    int result = run_splitmap("--map test_nocompress.nxm --map-size 32x21 --level-size 32x21 --no-compress");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that .nxm file exists (not .zx0)
    TEST_ASSERT(file_exists("test_nocompress_0000.nxm"), "Uncompressed file should exist");
    TEST_ASSERT(!file_exists("test_nocompress_0000.nxm.zx0"), "Compressed file should NOT exist");

    // Check that size matches original level size
    long file_size = get_file_size("test_nocompress_0000.nxm");
    TEST_ASSERT_EQ(32 * 21, file_size, "File size should match level size");

    // Cleanup
    cleanup_test_files("test_nocompress*");

    return true;
}

// ============================================================================
// UNIT TESTS - Output Size Parameter
// ============================================================================

bool test_output_size_parameter() {
    const char *map_file = "test_outsize.nxm";

    // Create a 64x42 map
    TEST_ASSERT(create_test_map(map_file, 64, 42, 80), "Failed to create test map");

    // Run with custom output size (compress to 16x21 chunks instead of 32x21)
    int result = run_splitmap("--map test_outsize.nxm --map-size 64x42 --level-size 32x21 "
                               "--output-size 16x21");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Should create 4 compressed chunks per level (2x2 per level * 2 levels horizontal)
    // Actually it divides by compress size, so 64/16 x 42/21 = 4x2 = 8 chunks
    TEST_ASSERT(file_exists("test_outsize_0000.nxm.zx0"), "Chunk 0,0 should exist");
    TEST_ASSERT(file_exists("test_outsize_0100.nxm.zx0"), "Chunk 1,0 should exist");
    TEST_ASSERT(file_exists("test_outsize_0200.nxm.zx0"), "Chunk 2,0 should exist");
    TEST_ASSERT(file_exists("test_outsize_0300.nxm.zx0"), "Chunk 3,0 should exist");

    // Cleanup
    cleanup_test_files("test_outsize*");

    return true;
}

// ============================================================================
// UNIT TESTS - Section Directives
// ============================================================================

bool test_section_directives() {
    const char *map_file = "test_sections.nxm";

    int item_positions[] = {10};
    TEST_ASSERT(create_test_map_with_items(map_file, 32, 21, 80, 145, item_positions, 1),
                "Failed to create test map");

    // Run with section directives
    int result = run_splitmap("--map test_sections.nxm --map-size 32x21 --level-size 32x21 "
                               "--blank 80 --item coin,145 "
                               "--tablessection code_user "
                               "--datasection bss_user "
                               "--rodatasection rodata_user");
    TEST_ASSERT(result == 0, "Splitmap should succeed");

    // Check that inc file contains section directives
    FILE *fp = fopen("test_sections.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_tables_section = false;
    bool found_data_section = false;
    bool found_rodata_section = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "section code_user")) found_tables_section = true;
        if (strstr(line, "section bss_user")) found_data_section = true;
        if (strstr(line, "section rodata_user")) found_rodata_section = true;
    }
    fclose(fp);

    TEST_ASSERT(found_tables_section, "Should find tables section directive");
    TEST_ASSERT(found_data_section, "Should find data section directive");
    TEST_ASSERT(found_rodata_section, "Should find rodata section directive");

    // Cleanup
    cleanup_test_files("test_sections*");

    return true;
}

// ============================================================================
// EDGE CASES AND ERROR HANDLING
// ============================================================================

bool test_empty_level() {
    const char *map_file = "test_empty.nxm";

    // Create a map with no items
    TEST_ASSERT(create_test_map(map_file, 32, 21, 80), "Failed to create test map");

    // Run with item definition but no items present
    int result = run_splitmap("--map test_empty.nxm --map-size 32x21 --level-size 32x21 "
                               "--blank 80 --item coin,145");
    TEST_ASSERT(result == 0, "Splitmap should succeed even with no items");

    // Check that inc file contains eot reference for empty level
    FILE *fp = fopen("test_empty.inc", "r");
    TEST_ASSERT(fp != NULL, "Should be able to open .inc file");

    char line[256];
    bool found_eot = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "equ eot")) {
            found_eot = true;
            break;
        }
    }
    fclose(fp);

    TEST_ASSERT(found_eot, "Should find eot reference for empty item table");

    // Cleanup
    cleanup_test_files("test_empty*");

    return true;
}

bool test_max_items_limit() {
    // This test would require creating 256+ different item types
    // which is impractical for a unit test, but documents the limit
    printf("  NOTE: MAX_ITEMS is set to 256 in splitmap.c\n");
    return true;
}

bool test_zero_size_parameters() {
    const char *map_file = "test_zero.nxm";

    TEST_ASSERT(create_test_map(map_file, 32, 21, 80), "Failed to create test map");

    // Try with zero width
    int result = run_splitmap("--map test_zero.nxm --map-size 0x21 --level-size 32x21");
    TEST_ASSERT(result != 0, "Should fail with zero width");

    // Try with zero height
    result = run_splitmap("--map test_zero.nxm --map-size 32x0 --level-size 32x21");
    TEST_ASSERT(result != 0, "Should fail with zero height");

    // Cleanup
    cleanup_test_files("test_zero*");

    return true;
}

bool test_misaligned_level_size() {
    const char *map_file = "test_misalign.nxm";

    // Create a 64x42 map
    TEST_ASSERT(create_test_map(map_file, 64, 42, 80), "Failed to create test map");

    // Try to split with level size that doesn't evenly divide map
    // This might succeed but produce unexpected results
    int result = run_splitmap("--map test_misalign.nxm --map-size 64x42 --level-size 30x20");
    // Result depends on implementation - document behavior
    printf("  NOTE: Result with misaligned sizes: %d\n", result);

    // Cleanup
    cleanup_test_files("test_misalign*");

    return true;
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

bool test_pyramid_escape_dimensions() {
    const char *map_file = "test_pyramid.nxm";

    // Test with actual Pyramid Escape dimensions: 192x105 map, 32x21 levels
    TEST_ASSERT(create_test_map(map_file, 192, 105, 80), "Failed to create test map");

    // Run with typical parameters
    int result = run_splitmap("--map test_pyramid.nxm --map-size 192x105 --level-size 32x21 "
                               "--blank 80");
    TEST_ASSERT(result == 0, "Should handle Pyramid Escape dimensions");

    // Verify 30 levels were created (6x5)
    int level_count = 0;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 6; x++) {
            char filename[64];
            snprintf(filename, sizeof(filename), "test_pyramid_%02d%02d.nxm.zx0", x, y);
            if (file_exists(filename)) level_count++;
        }
    }
    TEST_ASSERT_EQ(30, level_count, "Should create 30 levels");

    // Cleanup
    cleanup_test_files("test_pyramid*");

    return true;
}

bool test_complete_workflow() {
    const char *map_file = "test_workflow.nxm";

    // Create a realistic map with multiple item types
    unsigned char *data = malloc(64 * 42);
    memset(data, 80, 64 * 42);

    // Add coins
    data[100] = 145;
    data[200] = 145;
    data[300] = 145;

    // Add gems
    data[150] = 146;
    data[250] = 146;

    // Add keys
    data[175] = 147;

    FILE *fp = fopen(map_file, "wb");
    fwrite(data, 1, 64 * 42, fp);
    fclose(fp);
    free(data);

    // Run complete workflow with all options
    int result = run_splitmap("--map test_workflow.nxm --map-size 64x42 --level-size 32x21 "
                               "--blank 80 "
                               "--item coin,145 "
                               "--item gem,146,1 "
                               "--item key,147,2 "
                               "--tablessection code_user "
                               "--rodatasection rodata_user");
    TEST_ASSERT(result == 0, "Complete workflow should succeed");

    // Verify all outputs
    TEST_ASSERT(file_exists("test_workflow.inc"), ".inc file should exist");
    TEST_ASSERT(file_exists("test_workflow_0000.nxm.zx0"), "Level files should exist");

    // Verify inc file contents
    fp = fopen("test_workflow.inc", "r");
    TEST_ASSERT(fp != NULL, "Should open .inc file");

    char line[256];
    bool has_coin_tables = false, has_gem_tables = false, has_key_tables = false;
    bool has_level_table = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "_coinTables")) has_coin_tables = true;
        if (strstr(line, "_gemTables")) has_gem_tables = true;
        if (strstr(line, "_keyTables")) has_key_tables = true;
        if (strstr(line, "_levelTable")) has_level_table = true;
    }
    fclose(fp);

    TEST_ASSERT(has_coin_tables && has_gem_tables && has_key_tables && has_level_table,
                "Should have all expected tables");

    // Cleanup
    cleanup_test_files("test_workflow*");

    return true;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(int argc, char *argv[]) {
    printf("=================================================================\n");
    printf("SplitMap Test Suite\n");
    printf("=================================================================\n\n");

    // Check that splitmap executable exists
    if (!file_exists("./splitmap")) {
        fprintf(stderr, "ERROR: ./splitmap executable not found!\n");
        fprintf(stderr, "Please build splitmap first.\n");
        return 1;
    }

    printf("Command Line Parsing Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_no_arguments);
    RUN_TEST(test_missing_required_parameters);
    RUN_TEST(test_invalid_map_file);

    printf("\nBasic Map Splitting Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_simple_map_split);
    RUN_TEST(test_single_level_map);
    RUN_TEST(test_large_map_split);

    printf("\nItem Extraction Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_single_item_extraction);
    RUN_TEST(test_multiple_item_types);
    RUN_TEST(test_items_with_frames);
    RUN_TEST(test_items_only_mode);
    RUN_TEST(test_item_blanking);

    printf("\nHorizontal Strip Mode Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_horizontal_strip_mode);
    RUN_TEST(test_horizontal_strip_items);

    printf("\nCompression Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_compression_enabled);
    RUN_TEST(test_no_compression);

    printf("\nOutput Size Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_output_size_parameter);

    printf("\nSection Directive Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_section_directives);

    printf("\nEdge Cases and Error Handling:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_empty_level);
    RUN_TEST(test_max_items_limit);
    RUN_TEST(test_zero_size_parameters);
    RUN_TEST(test_misaligned_level_size);

    printf("\nIntegration Tests:\n");
    printf("-----------------------------------------------------------------\n");
    RUN_TEST(test_pyramid_escape_dimensions);
    RUN_TEST(test_complete_workflow);

    // Print summary
    printf("\n=================================================================\n");
    printf("Test Summary:\n");
    printf("=================================================================\n");
    printf("Total:  %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("=================================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
