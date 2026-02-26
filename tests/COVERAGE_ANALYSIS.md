# SplitMap Command-Line Parameter Test Coverage Analysis

## All Command-Line Parameters

Based on splitmap's usage message and source code analysis:

```
./splitmap --map <filename> --map-size <WIDTHxHEIGHT> --level-size <WIDTHxHEIGHT>
  [--blank <tile ID>]
  [--tablessection <section name>]
  [--datasection <section name>]
  [--rodatasection <section name>]
  [--items-only]
  [--horizontal-strip]
  [--no-compress]
  [--output-size <WIDTHxHEIGHT>]
  [--item <name>,<ID>[,<FRAME>] [...]]
```

## Parameter Coverage Matrix

| Parameter | Type | Required | Test Coverage | Test Cases |
|-----------|------|----------|---------------|------------|
| `--map` | Value | ✅ Yes | ✅ **100%** | All 23 tests |
| `--map-size` | Value | ✅ Yes | ✅ **100%** | All 23 tests |
| `--level-size` | Value | ✅ Yes | ✅ **100%** | All 23 tests |
| `--output-size` | Value | ❌ No | ✅ **Tested** | test_output_size_parameter |
| `--blank` | Value | ❌ No | ✅ **Tested** | 7 item tests |
| `--tablessection` | Value | ❌ No | ✅ **Tested** | test_section_directives |
| `--datasection` | Value | ❌ No | ✅ **Tested** | test_section_directives |
| `--rodatasection` | Value | ❌ No | ✅ **Tested** | test_section_directives |
| `--items-only` | Flag | ❌ No | ✅ **Tested** | test_items_only_mode |
| `--horizontal-strip` | Flag | ❌ No | ✅ **Tested** | test_horizontal_strip_mode, test_horizontal_strip_items |
| `--no-compress` | Flag | ❌ No | ✅ **Tested** | test_no_compression |
| `--item` | Value+ | ❌ No | ✅ **Tested** | 7+ item tests |

### Coverage Summary: ✅ **12/12 parameters covered (100%)**

## Detailed Test Coverage

### Required Parameters

#### 1. `--map <filename>`
- **Purpose**: Specify input map file
- **Test Coverage**: ✅ All 23 tests
- **Edge Cases Tested**:
  - ✅ Missing parameter (test_missing_required_parameters)
  - ✅ Nonexistent file (test_invalid_map_file)
  - ✅ Various valid files

#### 2. `--map-size <WIDTHxHEIGHT>`
- **Purpose**: Define total map dimensions
- **Test Coverage**: ✅ All 23 tests
- **Edge Cases Tested**:
  - ✅ Missing parameter (test_missing_required_parameters)
  - ✅ Zero width (test_zero_size_parameters)
  - ✅ Zero height (test_zero_size_parameters)
  - ✅ Small maps (32x21)
  - ✅ Medium maps (64x42)
  - ✅ Large maps (192x105)
  - ⚠️ **NOT TESTED**: Invalid format (e.g., "64" without "x", "abc", negative values)

#### 3. `--level-size <WIDTHxHEIGHT>`
- **Purpose**: Define individual level dimensions
- **Test Coverage**: ✅ All 23 tests
- **Edge Cases Tested**:
  - ✅ Missing parameter (test_missing_required_parameters)
  - ✅ Same as map size (single level)
  - ✅ Evenly divisible (32x21 into 64x42)
  - ✅ Misaligned sizes (test_misaligned_level_size)
  - ⚠️ **NOT TESTED**: Larger than map size

### Optional Value Parameters

#### 4. `--output-size <WIDTHxHEIGHT>`
- **Purpose**: Define compression chunk dimensions (default: same as level-size)
- **Test Coverage**: ✅ 1 dedicated test
- **Tests**:
  - ✅ test_output_size_parameter (16x21 chunks)
- **Edge Cases Tested**:
  - ✅ Smaller than level size
  - ❌ **NOT TESTED**: Larger than level size
  - ❌ **NOT TESTED**: Equal to level size (explicit)
  - ❌ **NOT TESTED**: Invalid dimensions

#### 5. `--blank <tile ID>`
- **Purpose**: Specify blank tile for item replacement
- **Test Coverage**: ✅ 7 tests
- **Tests**:
  - ✅ test_single_item_extraction
  - ✅ test_multiple_item_types
  - ✅ test_items_with_frames
  - ✅ test_items_only_mode
  - ✅ test_item_blanking
  - ✅ test_horizontal_strip_items
  - ✅ test_complete_workflow
- **Edge Cases Tested**:
  - ✅ Standard value (80)
  - ❌ **NOT TESTED**: Tile ID 0
  - ❌ **NOT TESTED**: Tile ID 255
  - ❌ **NOT TESTED**: Invalid (negative, > 255)
  - ❌ **NOT TESTED**: Omitted when items present

#### 6. `--tablessection <section name>`
- **Purpose**: Specify linker section for item tables
- **Test Coverage**: ✅ 1 test
- **Tests**:
  - ✅ test_section_directives (code_user)
- **Edge Cases Tested**:
  - ✅ Custom section name
  - ❌ **NOT TESTED**: Empty string
  - ❌ **NOT TESTED**: Very long name
  - ❌ **NOT TESTED**: Special characters

#### 7. `--datasection <section name>`
- **Purpose**: Specify linker section for writable data
- **Test Coverage**: ✅ 1 test
- **Tests**:
  - ✅ test_section_directives (bss_user)
- **Edge Cases Tested**:
  - ✅ Custom section name
  - ❌ **NOT TESTED**: Empty string
  - ❌ **NOT TESTED**: Without --tablessection

#### 8. `--rodatasection <section name>`
- **Purpose**: Specify linker section for read-only data
- **Test Coverage**: ✅ 2 tests
- **Tests**:
  - ✅ test_section_directives (rodata_user)
  - ✅ test_complete_workflow
- **Edge Cases Tested**:
  - ✅ Custom section name
  - ❌ **NOT TESTED**: Multiple times
  - ❌ **NOT TESTED**: Alone without other sections

### Optional Flag Parameters

#### 9. `--items-only`
- **Purpose**: Generate only item tables, skip map compression
- **Test Coverage**: ✅ 1 dedicated test
- **Tests**:
  - ✅ test_items_only_mode
- **Edge Cases Tested**:
  - ✅ With items present
  - ❌ **NOT TESTED**: Without any --item parameters
  - ❌ **NOT TESTED**: Combined with --no-compress

#### 10. `--horizontal-strip`
- **Purpose**: Enable horizontal strip rendering mode
- **Test Coverage**: ✅ 2 tests
- **Tests**:
  - ✅ test_horizontal_strip_mode
  - ✅ test_horizontal_strip_items
- **Edge Cases Tested**:
  - ✅ Basic strip generation
  - ✅ Items with world coordinates
  - ✅ Multiple strips
  - ❌ **NOT TESTED**: Single strip (strip height = map height)
  - ❌ **NOT TESTED**: Combined with --output-size
  - ❌ **NOT TESTED**: Combined with --items-only

#### 11. `--no-compress`
- **Purpose**: Disable ZX0 compression
- **Test Coverage**: ✅ 1 dedicated test
- **Tests**:
  - ✅ test_no_compression
- **Edge Cases Tested**:
  - ✅ Basic functionality
  - ❌ **NOT TESTED**: Combined with --horizontal-strip
  - ❌ **NOT TESTED**: Combined with --items-only

#### 12. `--item <name>,<ID>[,<FRAME>]`
- **Purpose**: Define collectible item types
- **Test Coverage**: ✅ 7 tests
- **Tests**:
  - ✅ test_single_item_extraction (coin,145)
  - ✅ test_multiple_item_types (coin,145 + gem,146 + heart,147)
  - ✅ test_items_with_frames (key,150,3)
  - ✅ test_items_only_mode (coin,145)
  - ✅ test_item_blanking (coin,145)
  - ✅ test_horizontal_strip_items (coin,145)
  - ✅ test_complete_workflow (multiple items)
- **Edge Cases Tested**:
  - ✅ Single item type
  - ✅ Multiple item types
  - ✅ With frame value
  - ✅ Without frame value (defaults to 0)
  - ✅ Item distribution across levels
  - ✅ Empty levels (no items present)
  - ❌ **NOT TESTED**: Same tile ID for different items
  - ❌ **NOT TESTED**: Item with frame 255
  - ❌ **NOT TESTED**: Same item name multiple times
  - ❌ **NOT TESTED**: Very long item name
  - ❌ **NOT TESTED**: Invalid tile ID (negative, > 255)
  - ❌ **NOT TESTED**: MAX_ITEMS limit (256) - documented but not actually tested

## Untested Parameter Combinations

These parameter combinations are valid but not explicitly tested:

1. ❌ `--horizontal-strip` + `--output-size`
2. ❌ `--horizontal-strip` + `--no-compress`
3. ❌ `--items-only` + `--no-compress`
4. ❌ All section directives together (tablessection + datasection + rodatasection)
   - **Actually tested in test_section_directives** ✅
5. ❌ Multiple `--item` parameters (10+ item types)
6. ❌ `--horizontal-strip` + multiple item types + section directives

## Untested Edge Cases

### Input Validation

| Scenario | Tested | Priority |
|----------|--------|----------|
| Invalid size format ("64" instead of "64x42") | ❌ | High |
| Negative dimensions | ❌ | Medium |
| Extremely large maps (>1MB) | ❌ | Low |
| Level size > map size | ❌ | Medium |
| Non-numeric tile IDs | ❌ | Medium |
| Empty section name | ❌ | Low |
| Item tile ID = blank tile ID | ❌ | Medium |
| Duplicate item names | ❌ | Medium |
| Item name with special chars | ❌ | Low |

### Error Conditions

| Scenario | Tested | Priority |
|----------|--------|----------|
| Disk full (cannot write output) | ❌ | Low |
| Read-only output directory | ❌ | Low |
| Map file corrupted/truncated | ❌ | Medium |
| Out of memory during compression | ❌ | Low |
| Too many item types (>256) | ⚠️ Documented | Low |

### Functional Edge Cases

| Scenario | Tested | Priority |
|----------|--------|----------|
| Item at position 0 | ❌ | Medium |
| Item at last position | ❌ | Medium |
| All tiles are items | ❌ | Low |
| No items in entire map | ✅ | - |
| Blank tile not in map | ❌ | Low |
| Items in strip mode at boundaries | ❌ | Medium |

## Recommendations

### High Priority Additions

1. **Invalid format tests**:
   ```c
   test_invalid_size_format() // "64" instead of "64x42"
   test_negative_dimensions() // "--map-size -32x21"
   test_level_larger_than_map() // level 64x42, map 32x21
   ```

2. **Parameter combination tests**:
   ```c
   test_horizontal_strip_with_no_compress()
   test_items_only_with_no_compress()
   test_horizontal_strip_with_output_size()
   ```

3. **Item edge cases**:
   ```c
   test_item_at_boundaries() // positions 0, last
   test_blank_equals_item_id() // conflict
   test_many_item_types() // 50+ types
   ```

### Medium Priority Additions

4. **Tile ID validation**:
   ```c
   test_blank_tile_extremes() // 0, 255
   test_invalid_tile_ids() // negative, >255
   ```

5. **Section directive edge cases**:
   ```c
   test_empty_section_name()
   test_section_directives_individually()
   ```

### Low Priority Additions

6. **Stress tests**:
   ```c
   test_very_large_map() // 512x512
   test_maximum_items() // approach MAX_ITEMS limit
   ```

7. **Error recovery**:
   ```c
   test_disk_full_scenario()
   test_corrupted_input_file()
   ```

## Coverage Statistics

### Overall Coverage: ✅ **100% of documented parameters**

- **Required parameters**: 3/3 tested (100%)
- **Optional value parameters**: 5/5 tested (100%)
- **Optional flag parameters**: 3/3 tested (100%)
- **Repeatable parameters**: 1/1 tested (100%)

### Edge Case Coverage: ⚠️ **~60% estimated**

- **Basic functionality**: 100%
- **Input validation**: ~40%
- **Parameter combinations**: ~30%
- **Error conditions**: ~20%
- **Boundary conditions**: ~50%

## Conclusion

✅ **All 12 command-line parameters are covered by at least one test.**

However, there are opportunities to improve edge case and combination testing:

- 10+ untested parameter combinations
- 15+ untested edge cases
- Limited input validation testing
- No stress testing

The current test suite provides excellent **functional coverage** (100%) but could be enhanced with additional **validation coverage** and **combination testing**.

### Suggested Next Steps

1. Add 5-10 high-priority tests for invalid inputs
2. Add 3-5 parameter combination tests
3. Add boundary condition tests for items
4. Consider adding stress tests for CI/CD performance monitoring

---

**Last Updated**: 2026-02-25
**Test Suite Version**: 1.0
**Parameter Coverage**: 12/12 (100%)
**Edge Case Coverage**: ~60% (estimated)
