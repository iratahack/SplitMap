# SplitMap Test Plan

## Document Information

**Version:** 1.0
**Date:** 2026-02-25
**Status:** Active

## 1. Introduction

### 1.1 Purpose

This document outlines the comprehensive test plan for the SplitMap tilemap splitting and compression tool. It defines test objectives, scope, approach, and detailed test cases.

### 1.2 Scope

The test plan covers all major functionality of SplitMap:

- Command-line interface and argument parsing
- Tilemap splitting in per-level mode
- Tilemap splitting in horizontal-strip mode
- Item extraction and table generation
- ZX0 compression
- Assembly code generation
- Error handling and edge cases

### 1.3 Test Objectives

- Verify correct map splitting for various dimensions
- Validate item extraction and coordinate calculation
- Ensure compression produces valid output
- Verify assembly code generation is syntactically correct
- Test error handling for invalid inputs
- Validate memory management (no leaks)

## 2. Test Approach

### 2.1 Test Levels

1. **Unit Tests**: Test individual functions and features in isolation
2. **Integration Tests**: Test complete workflows with multiple features
3. **Regression Tests**: Ensure fixes don't break existing functionality
4. **Performance Tests**: Validate performance with large maps (future)

### 2.2 Test Environment

- **Platform**: Linux (WSL2 on Windows also supported)
- **Compiler**: GCC with C11 support
- **Build System**: Make
- **Test Framework**: Custom lightweight framework
- **Optional Tools**: Valgrind (memory analysis)

### 2.3 Test Data

Tests use programmatically generated tilemap data:

- Small maps: 32x21 (single level)
- Medium maps: 64x42 (2x2 levels)
- Large maps: 192x105 (6x5 levels, Pyramid Escape dimensions)
- Maps with items at known positions
- Maps with multiple item types

## 3. Test Cases

### 3.1 Command Line Parsing (TC-CLI)

#### TC-CLI-001: No Arguments
**Objective**: Verify usage message displayed when no arguments provided
**Input**: `./splitmap`
**Expected**: Usage message printed, exit code 0
**Priority**: High

#### TC-CLI-002: Missing Required Parameter - Map File
**Objective**: Verify error when --map not provided
**Input**: `./splitmap --map-size 64x42 --level-size 32x21`
**Expected**: Error message, non-zero exit code
**Priority**: High

#### TC-CLI-003: Missing Required Parameter - Map Size
**Objective**: Verify error when --map-size not provided
**Input**: `./splitmap --map test.nxm --level-size 32x21`
**Expected**: Error message, non-zero exit code
**Priority**: High

#### TC-CLI-004: Missing Required Parameter - Level Size
**Objective**: Verify error when --level-size not provided
**Input**: `./splitmap --map test.nxm --map-size 64x42`
**Expected**: Error message, non-zero exit code
**Priority**: High

#### TC-CLI-005: Nonexistent Input File
**Objective**: Verify error handling for missing input file
**Input**: `./splitmap --map missing.nxm --map-size 64x42 --level-size 32x21`
**Expected**: Error message, non-zero exit code
**Priority**: High

#### TC-CLI-006: Invalid Size Format
**Objective**: Verify error handling for malformed size parameters
**Input**: `./splitmap --map test.nxm --map-size 64 --level-size 32x21`
**Expected**: Error or undefined behavior (document actual behavior)
**Priority**: Medium

### 3.2 Basic Map Splitting (TC-SPLIT)

#### TC-SPLIT-001: Simple 2x2 Split
**Objective**: Verify basic map splitting functionality
**Setup**: Create 64x42 map filled with tile 80
**Input**: `--map test.nxm --map-size 64x42 --level-size 32x21`
**Expected**:
- 4 compressed level files: `*_0000.zx0`, `*_0100.zx0`, `*_0001.zx0`, `*_0101.zx0`
- Assembly include file with levelTable
- Exit code 0
**Priority**: Critical

#### TC-SPLIT-002: Single Level Map
**Objective**: Verify handling of single-level maps
**Setup**: Create 32x21 map filled with tile 80
**Input**: `--map test.nxm --map-size 32x21 --level-size 32x21`
**Expected**:
- 1 compressed level file: `*_0000.zx0`
- Assembly include file with levelTable containing 1 entry
- Exit code 0
**Priority**: High

#### TC-SPLIT-003: Large Map (Pyramid Escape Dimensions)
**Objective**: Verify handling of production-scale maps
**Setup**: Create 192x105 map filled with tile 80
**Input**: `--map test.nxm --map-size 192x105 --level-size 32x21`
**Expected**:
- 30 compressed level files (6x5 grid)
- Assembly include file with levelTable containing 30 entries
- Exit code 0
**Priority**: Critical

#### TC-SPLIT-004: Custom Output Size
**Objective**: Verify --output-size parameter
**Setup**: Create 64x42 map
**Input**: `--map test.nxm --map-size 64x42 --level-size 32x21 --output-size 16x21`
**Expected**:
- Files split by output-size dimensions (8 files: 4 cols x 2 rows)
- Correct levelTable entries
**Priority**: Medium

### 3.3 Item Extraction (TC-ITEM)

#### TC-ITEM-001: Single Item Type
**Objective**: Verify extraction of a single item type
**Setup**: Create 32x21 map with coins (tile 145) at positions [10, 50, 100]
**Input**: `--map test.nxm --map-size 32x21 --level-size 32x21 --blank 80 --item coin,145`
**Expected**:
- Include file contains `_coinTables` symbol
- Include file contains `level0coin` with 3 entries
- Each entry has correct X,Y coordinates (multiples of 8)
- Items replaced with blank tile (80) in output map
**Priority**: Critical

#### TC-ITEM-002: Multiple Item Types
**Objective**: Verify extraction of multiple item types
**Setup**: Create 32x21 map with coins (145), gems (146), hearts (147)
**Input**: `--map test.nxm ... --item coin,145 --item gem,146 --item heart,147`
**Expected**:
- Include file contains `_coinTables`, `_gemTables`, `_heartTables`
- Each has corresponding level data
**Priority**: High

#### TC-ITEM-003: Items with Frame Values
**Objective**: Verify custom frame values in item data
**Setup**: Create map with keys (tile 150)
**Input**: `--map test.nxm ... --item key,150,3`
**Expected**:
- Item entries include frame value 3 as 4th byte
- Format: `db $01, X, Y, $03`
**Priority**: High

#### TC-ITEM-004: Items-Only Mode
**Objective**: Verify --items-only flag
**Setup**: Create map with coins
**Input**: `--map test.nxm ... --item coin,145 --items-only`
**Expected**:
- Include file contains item tables
- NO compressed map files created
- Exit code 0
**Priority**: Medium

#### TC-ITEM-005: Item Blanking
**Objective**: Verify items are replaced with blank tiles
**Setup**: Create map with item at position 100
**Input**: `--map test.nxm ... --blank 80 --item coin,145`
**Expected**:
- Read input file after processing
- Position 100 contains tile 80 (not 145)
**Priority**: High

#### TC-ITEM-006: Empty Level (No Items)
**Objective**: Verify handling of levels with no items
**Setup**: Create map with no item tiles
**Input**: `--map test.nxm ... --item coin,145`
**Expected**:
- Include file contains `level0coin equ eot`
- No crash or error
**Priority**: Medium

#### TC-ITEM-007: Items Across Multiple Levels
**Objective**: Verify item distribution across levels
**Setup**: Create 64x42 map with items in different quadrants
**Input**: `--map test.nxm --map-size 64x42 --level-size 32x21 --item coin,145`
**Expected**:
- Separate item tables for each level (level0coin, level1coin, etc.)
- Coordinates are relative to each level (0-248 X, 0-160 Y)
**Priority**: High

### 3.4 Horizontal Strip Mode (TC-STRIP)

#### TC-STRIP-001: Basic Strip Mode
**Objective**: Verify horizontal strip generation
**Setup**: Create 96x42 map (2 strips of 21 height)
**Input**: `--map test.nxm --map-size 96x42 --level-size 96x21 --horizontal-strip`
**Expected**:
- 2 strip files: `*_strip0.zx0`, `*_strip1.zx0`
- Include file contains `_stripTable` symbol
- Include file contains `_stripWidthTiles` constant (96)
- Backward compatible `_levelTable equ _stripTable`
**Priority**: High

#### TC-STRIP-002: Strip Mode with Items
**Objective**: Verify item extraction in strip mode uses world coordinates
**Setup**: Create 96x42 map with items in both strips
**Input**: `--map test.nxm ... --horizontal-strip --item coin,145`
**Expected**:
- Include file contains `strip0coin`, `strip1coin`
- Item coordinates are 16-bit world X coordinates (0-768 pixels)
- Format: `db $01, X_low, X_high, Y, frame`
**Priority**: Critical

#### TC-STRIP-003: Multiple Strips
**Objective**: Verify handling of multiple strips
**Setup**: Create 192x105 map (5 strips)
**Input**: `--map test.nxm --map-size 192x105 --level-size 192x21 --horizontal-strip`
**Expected**:
- 5 strip files
- stripTable with 5 entries
**Priority**: Medium

### 3.5 Compression (TC-COMP)

#### TC-COMP-001: Default Compression Enabled
**Objective**: Verify ZX0 compression is applied by default
**Setup**: Create 32x21 map with repetitive data
**Input**: `--map test.nxm --map-size 32x21 --level-size 32x21`
**Expected**:
- Output file ends with `.zx0` extension
- File size < original size (32*21 = 672 bytes)
**Priority**: High

#### TC-COMP-002: No Compression Mode
**Objective**: Verify --no-compress flag
**Setup**: Create 32x21 map
**Input**: `--map test.nxm ... --no-compress`
**Expected**:
- Output file: `*_0000.nxm` (not .zx0)
- File size = 672 bytes exactly
**Priority**: Medium

#### TC-COMP-003: Compression Ratio
**Objective**: Verify reasonable compression for repetitive data
**Setup**: Create 32x21 map filled with single tile
**Input**: Default compression
**Expected**:
- Compression ratio > 50% (compressed < 336 bytes)
**Priority**: Low

### 3.6 Assembly Code Generation (TC-ASM)

#### TC-ASM-001: Level Table Structure
**Objective**: Verify levelTable assembly syntax
**Setup**: Create simple map
**Input**: Standard parameters
**Expected**:
- `public _levelTable` directive
- `_levelTable:` label
- `dw levelname_XXYY` entries
- `levelname_XXYY:` labels
- `binary "filename"` directives
**Priority**: High

#### TC-ASM-002: Item Table Structure
**Objective**: Verify item table assembly syntax
**Setup**: Create map with items
**Input**: With --item parameter
**Expected**:
- `public _itemTables` directive
- `_itemTables:` label pointing to level tables
- `levelNitem:` labels with data
- `db $01, X, Y, frame` entries
- `db $ff` terminator
**Priority**: High

#### TC-ASM-003: Section Directives
**Objective**: Verify linker section directives
**Setup**: Create map with items
**Input**: `--tablessection code_user --datasection bss_user --rodatasection rodata_user`
**Expected**:
- `section code_user` before item tables
- `section bss_user` before data
- `section rodata_user` before level data
**Priority**: Medium

#### TC-ASM-004: EOT Marker
**Objective**: Verify end-of-table marker generation
**Setup**: Create map
**Input**: With --rodatasection
**Expected**:
- Include file contains `eot: db $ff` definition
- Used for empty item tables
**Priority**: Low

### 3.7 Edge Cases (TC-EDGE)

#### TC-EDGE-001: Zero Width Map
**Objective**: Verify error handling for invalid dimensions
**Input**: `--map-size 0x21`
**Expected**: Error or graceful failure (document behavior)
**Priority**: Medium

#### TC-EDGE-002: Zero Height Map
**Objective**: Verify error handling for invalid dimensions
**Input**: `--map-size 32x0`
**Expected**: Error or graceful failure (document behavior)
**Priority**: Medium

#### TC-EDGE-003: Misaligned Level Size
**Objective**: Document behavior when level size doesn't divide map evenly
**Setup**: Create 64x42 map
**Input**: `--level-size 30x20`
**Expected**: Document whether it errors or produces partial levels
**Priority**: Low

#### TC-EDGE-004: MAX_ITEMS Limit
**Objective**: Verify behavior at item type limit
**Setup**: Would require 256+ --item parameters (impractical)
**Expected**: Document limit exists (256 item types max)
**Priority**: Low

#### TC-EDGE-005: Very Large Map
**Objective**: Verify handling of large maps
**Setup**: Create 512x512 map (would produce 256 levels at 32x32 each)
**Expected**: Should complete without crash (may be slow)
**Priority**: Low

#### TC-EDGE-006: Item at Map Boundary
**Objective**: Verify items at edge coordinates
**Setup**: Place items at positions 0, 31, 20*32-1
**Input**: Standard parameters
**Expected**: Correct coordinates calculated (0, 248, 160)
**Priority**: Medium

### 3.8 Integration Tests (TC-INT)

#### TC-INT-001: Complete Workflow
**Objective**: Test full pipeline with all features
**Setup**: Create 64x42 map with multiple item types
**Input**: All parameters enabled (sections, items, compression)
**Expected**:
- All output files created
- Include file contains all expected symbols
- Assembly syntax valid
- Items blanked from map
**Priority**: Critical

#### TC-INT-002: Pyramid Escape Production Build
**Objective**: Simulate real production build
**Setup**: Create 192x105 map with coins, gems, keys, hearts
**Input**: Pyramid Escape parameters
**Expected**:
- 30 level files
- Multiple item tables
- Correct assembly structure
- Exit code 0
**Priority**: Critical

## 4. Test Execution

### 4.1 Test Execution Schedule

1. **Pre-commit**: Run quick smoke tests
2. **Daily**: Run full test suite
3. **Pre-release**: Run full suite + valgrind + manual tests

### 4.2 Test Execution Commands

```bash
# Run all tests
make test

# Run with valgrind
./run_tests.sh -m

# Run with coverage
./run_tests.sh -c

# Quick tests only
./run_tests.sh -q
```

### 4.3 Pass/Fail Criteria

- **Pass**: Test returns true, no assertions fail
- **Fail**: Test returns false or assertion fails
- **Blocked**: Cannot run due to missing dependency
- **Skip**: Not applicable to current platform

## 5. Test Metrics

### 5.1 Coverage Goals

- **Line Coverage**: > 85%
- **Branch Coverage**: > 75%
- **Function Coverage**: 100%

### 5.2 Defect Tracking

| Severity | Definition | Response Time |
|----------|------------|---------------|
| Critical | Crash, data loss, incorrect output | Immediate |
| High | Wrong behavior, wrong data | 1 day |
| Medium | Minor incorrect behavior | 1 week |
| Low | Cosmetic, documentation | Best effort |

## 6. Test Deliverables

- ✓ Test suite source code ([test_splitmap.c](test_splitmap.c))
- ✓ Test build system ([Makefile](Makefile))
- ✓ Test runner script ([run_tests.sh](run_tests.sh))
- ✓ Test documentation ([README.md](README.md))
- ✓ Test plan (this document)
- ⧗ Valgrind report (generated on demand)
- ⧗ Coverage report (generated on demand)

## 7. Risks and Mitigations

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Memory leaks | Medium | Low | Run valgrind regularly |
| Platform differences | Medium | Medium | Test on Linux and WSL |
| ZX0 algorithm changes | High | Low | Pin ZX0 version, document interface |
| Large map performance | Low | Low | Add performance tests if needed |

## 8. Appendices

### Appendix A: Test Environment Setup

```bash
# Install dependencies
sudo apt-get install gcc make valgrind

# Build splitmap
cd SplitMap
make

# Build and run tests
cd tests
make test
```

### Appendix B: Manual Test Cases

Some scenarios require manual verification:

1. **Assembly Compatibility**: Assemble generated .inc files with z80asm
2. **Decompression**: Decompress .zx0 files and verify against original
3. **Game Integration**: Test with actual Pyramid Escape build

### Appendix C: Test Data Specifications

Standard test maps:

- **Small**: 32x21 tiles (1 level)
- **Medium**: 64x42 tiles (2x2 levels)
- **Large**: 192x105 tiles (6x5 levels)
- **Tiles**: 80 (blank), 145 (coin), 146 (gem), 147 (heart), 150 (key)

### Appendix D: Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-25 | Claude Code | Initial test plan |
