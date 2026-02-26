# SplitMap Testing Quick Start

Get started testing SplitMap in 5 minutes!

## Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install gcc make

# Optional for memory leak detection
sudo apt-get install valgrind
```

## Quick Start

### 1. Build and Run Tests

```bash
cd SplitMap/tests
make test
```

That's it! The Makefile will:
- Build splitmap if needed
- Compile the test suite
- Run all tests
- Display results

### 2. Expected Output

```
=================================================================
                    SplitMap Test Suite
=================================================================

Command Line Parsing Tests:
-----------------------------------------------------------------
Running test_no_arguments...
  PASS
Running test_missing_required_parameters...
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

### 3. Common Tasks

#### Run with Memory Leak Detection

```bash
./run_tests.sh --valgrind
```

#### Clean Up Generated Files

```bash
make clean
```

#### Run Individual Test (Advanced)

```bash
# Build first
make

# Copy splitmap
cp ../splitmap .

# Run test executable
./test_splitmap
```

## Interpreting Results

### ✅ All Tests Pass

```
Test Summary:
Total:  28
Passed: 28
Failed: 0
```

**Action**: None needed, everything works!

### ❌ Test Failures

```
Running test_simple_map_split...
FAIL: test_simple_map_split
  test_splitmap.c:145: Output .inc file should exist
```

**What it means**: The test expected a file that wasn't created.

**Action**:
1. Check if splitmap built correctly: `../splitmap --help`
2. Look at the failing test in `test_splitmap.c` line 145
3. Run the failing test manually to debug

### 🔍 Memory Leaks (Valgrind)

```
HEAP SUMMARY:
    definitely lost: 64 bytes in 2 blocks
```

**Action**: Review splitmap.c for missing `free()` calls

## Troubleshooting

### "splitmap: command not found"

```bash
# Build splitmap first
cd ..
make
cd tests
```

### "Permission denied"

```bash
# Make scripts executable
chmod +x run_tests.sh
chmod +x ../splitmap
```

### Tests hang or run forever

```bash
# Kill hung processes
killall splitmap test_splitmap

# Clean up
make clean
```

### "Cannot create output file"

```bash
# Check disk space
df -h .

# Check permissions
ls -la .
```

## Next Steps

- Read [README.md](README.md) for detailed documentation
- Review [TEST_PLAN.md](TEST_PLAN.md) for comprehensive test coverage
- Examine [test_splitmap.c](test_splitmap.c) to understand test implementation

## Adding Your First Test

1. Open `test_splitmap.c`

2. Add your test function:

```c
bool test_my_feature() {
    // Setup
    create_test_map("test_my.nxm", 32, 21, 80);

    // Execute
    int result = run_splitmap("--map test_my.nxm --map-size 32x21 --level-size 32x21");

    // Verify
    TEST_ASSERT(result == 0, "Should succeed");
    TEST_ASSERT(file_exists("test_my.inc"), "Output should exist");

    // Cleanup
    cleanup_test_files("test_my*");

    return true;
}
```

3. Register test in `main()`:

```c
RUN_TEST(test_my_feature);
```

4. Run tests:

```bash
make test
```

## Getting Help

- **General questions**: Read [README.md](README.md)
- **Test failures**: Check [TEST_PLAN.md](TEST_PLAN.md) for expected behavior
- **Code issues**: Review [../splitmap.c](../splitmap.c)
- **Bug reports**: Document the failure and report it

## Continuous Integration

Add to your CI pipeline:

```yaml
# .github/workflows/test.yml
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install gcc make
      - name: Run tests
        run: |
          cd SplitMap
          make
          cd tests
          make test
```

## Performance Testing

```bash
# Time the test suite
time make test

# Profile a specific test (requires gprof)
gcc -pg test_splitmap.c -o test_splitmap
./test_splitmap
gprof test_splitmap gmon.out > analysis.txt
```

## Tips

1. **Run tests often**: After every change
2. **Add tests first**: Write tests before fixing bugs (TDD)
3. **Keep tests fast**: Tests should complete in < 30 seconds
4. **Clean up**: Always cleanup test files in your test functions
5. **Use descriptive names**: `test_horizontal_strip_with_items` is better than `test5`

## Example Test Session

```bash
# Full workflow
cd SplitMap/tests

# First time setup
make

# Make a change to splitmap.c
vim ../splitmap.c

# Rebuild and test
cd .. && make && cd tests
make clean && make test

# If tests fail, debug
./run_tests.sh -v

# Check for memory leaks
./run_tests.sh -m

# Everything passes, commit
git add ../splitmap.c test_splitmap.c
git commit -m "Fix item extraction bug"
```

---

**You're ready to go!** Run `make test` and start testing SplitMap.
