# SplitMap Test Suite

Comprehensive test suite for the SplitMap tilemap splitting and compression tool.

## Overview

This test suite validates all major functionality of SplitMap including:

- Command-line argument parsing
- Map splitting (per-level and horizontal-strip modes)
- Item extraction and table generation
- ZX0 compression
- File I/O operations
- Edge cases and error handling

## Test Categories

### 1. Command Line Parsing Tests

Tests validation of command-line arguments:

- `test_no_arguments` - Verifies usage message when no arguments provided
- `test_missing_required_parameters` - Tests error handling for missing required parameters
- `test_invalid_map_file` - Tests error handling for nonexistent input files

### 2. Basic Map Splitting Tests

Tests core map splitting functionality:

- `test_simple_map_split` - Basic 2x2 level split (64x42 map into 32x21 levels)
- `test_single_level_map` - Single level map (32x21)
- `test_large_map_split` - Large 6x5 grid (192x105 map, Pyramid Escape dimensions)

### 3. Item Extraction Tests

Tests item detection and table generation:

- `test_single_item_extraction` - Extract coins from a single level
- `test_multiple_item_types` - Extract multiple item types (coins, gems, hearts)
- `test_items_with_frames` - Test items with custom frame values
- `test_items_only_mode` - Test `--items-only` flag (no map compression)
- `test_item_blanking` - Verify items are replaced with blank tiles in the map

### 4. Horizontal Strip Mode Tests

Tests the horizontal strip rendering mode:

- `test_horizontal_strip_mode` - Basic horizontal strip generation
- `test_horizontal_strip_items` - Item extraction with world coordinates in strip mode

### 5. Compression Tests

Tests ZX0 compression functionality:

- `test_compression_enabled` - Verify default compression behavior
- `test_no_compression` - Test `--no-compress` flag

### 6. Output Size Tests

Tests custom output size parameter:

- `test_output_size_parameter` - Test `--output-size` for custom chunk dimensions

### 7. Section Directive Tests

Tests linker section directives:

- `test_section_directives` - Verify `--tablessection`, `--datasection`, `--rodatasection`

### 8. Edge Cases and Error Handling

Tests boundary conditions and error scenarios:

- `test_empty_level` - Levels with no items present
- `test_max_items_limit` - Document MAX_ITEMS limit (256)
- `test_zero_size_parameters` - Invalid zero-sized dimensions
- `test_misaligned_level_size` - Level sizes that don't evenly divide map

### 9. Integration Tests

End-to-end workflow tests:

- `test_pyramid_escape_dimensions` - Real-world Pyramid Escape game dimensions
- `test_complete_workflow` - Complete workflow with all features enabled

## Building and Running

### Prerequisites

- GCC compiler
- Make
- Built SplitMap executable (built automatically if needed)

### Build the Test Suite

```bash
cd SplitMap/tests
make
```

### Run All Tests

```bash
make test
```

This will:
1. Build the test executable
2. Ensure splitmap is built
3. Run all test cases
4. Display results summary

### Clean Up

```bash
make clean
```

Removes test executables and generated test files.

## Test Output

Tests produce output in the following format:

```
Running test_simple_map_split...
  PASS

Running test_invalid_map_file...
  PASS

...

=================================================================
Test Summary:
=================================================================
Total:  28
Passed: 28
Failed: 0
=================================================================
```

Failed tests display diagnostic information:

```
Running test_example...
FAIL: test_example
  test_splitmap.c:123: Expected value did not match (expected: 5, actual: 3)
```

## Test Data

Tests automatically create temporary test files:

- `test_*.nxm` - Input tilemap files
- `test_*.inc` - Generated assembly include files
- `test_*_XXYY.nxm.zx0` - Compressed level chunks
- `test_*_stripN.nxm.zx0` - Compressed strip files

All test files are automatically cleaned up after each test.

## Test Coverage

The test suite covers:

| Feature | Coverage |
|---------|----------|
| Command-line parsing | ✓ Full |
| Per-level mode splitting | ✓ Full |
| Horizontal strip mode | ✓ Full |
| Item extraction | ✓ Full |
| Item blanking | ✓ Full |
| ZX0 compression | ✓ Full |
| No-compression mode | ✓ Full |
| Section directives | ✓ Full |
| Output file generation | ✓ Full |
| Assembly code generation | ✓ Partial (structure only) |
| Error handling | ✓ Partial |
| Memory management | ✗ None (requires instrumentation) |

## Adding New Tests

To add a new test:

1. Write a test function following the naming convention `test_<feature_name>()`:

```c
bool test_my_new_feature() {
    // Setup
    const char *map_file = "test_mynew.nxm";
    create_test_map(map_file, 32, 21, 80);

    // Execute
    int result = run_splitmap("--map test_mynew.nxm --map-size 32x21 --level-size 32x21");

    // Verify
    TEST_ASSERT(result == 0, "Should succeed");
    TEST_ASSERT(file_exists("test_mynew.inc"), "Output should exist");

    // Cleanup
    cleanup_test_files("test_mynew*");

    return true;
}
```

2. Add the test to `main()` in the appropriate category:

```c
printf("\nMy Feature Tests:\n");
printf("-----------------------------------------------------------------\n");
RUN_TEST(test_my_new_feature);
```

3. Rebuild and run:

```bash
make clean
make test
```

## Helper Functions

The test suite provides several helper functions:

### File Operations

- `create_test_map(filename, width, height, fill_value)` - Create a uniform tilemap
- `create_test_map_with_items(filename, w, h, base, item, positions, count)` - Create a map with items
- `file_exists(filename)` - Check if file exists
- `get_file_size(filename)` - Get file size in bytes

### Execution

- `run_splitmap(args)` - Execute splitmap with arguments
- `cleanup_test_files(pattern)` - Remove test files matching pattern

### Assertions

- `TEST_ASSERT(condition, message)` - Assert condition is true
- `TEST_ASSERT_EQ(expected, actual, message)` - Assert values are equal

## Known Limitations

1. **Memory leak detection**: Tests do not currently check for memory leaks. Consider using valgrind:

   ```bash
   valgrind --leak-check=full ./test_splitmap
   ```

2. **ZX0 decompression**: Tests verify compressed files are created but don't validate decompression correctness.

3. **Assembly syntax**: Tests check for structure but don't validate full z80asm compatibility.

4. **Performance**: No performance benchmarks or stress tests included.

## Continuous Integration

To integrate with CI/CD:

```yaml
# .github/workflows/test.yml
- name: Run SplitMap Tests
  run: |
    cd SplitMap
    make
    cd tests
    make test
```

## Troubleshooting

### Tests fail to build

- Ensure GCC is installed: `gcc --version`
- Check that C11 standard is supported

### Tests fail with "splitmap not found"

- Build splitmap first: `cd .. && make`
- Check executable permissions: `chmod +x ../splitmap`

### Tests leave temporary files

- Run cleanup: `make clean`
- Check for hung processes: `ps aux | grep splitmap`

### All tests fail immediately

- Verify splitmap builds successfully: `../splitmap --help`
- Check file permissions in tests directory
- Ensure sufficient disk space for test files

## Contributing

When adding new features to SplitMap:

1. Write tests FIRST (TDD approach)
2. Ensure all existing tests pass
3. Add integration tests for complex features
4. Document any new test helpers or patterns
5. Update this README with new test categories

## License

This test suite is part of the Pyramid Escape project and follows the same license as the main codebase.
