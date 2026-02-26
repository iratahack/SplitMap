# SplitMap Test Suite - Documentation Index

Welcome to the SplitMap test suite! This index helps you navigate the test documentation.

## 📁 Directory Structure

```
SplitMap/tests/
├── test_splitmap.c     # Main test suite (807 lines, 28 tests)
├── Makefile            # Build system
├── run_tests.sh        # Advanced test runner (executable)
├── .gitignore          # Git ignore patterns
├── INDEX.md            # ← You are here
├── QUICKSTART.md       # 5-minute quick start guide
├── README.md           # Complete test documentation
├── TEST_PLAN.md        # Formal test plan & specifications
└── SUMMARY.md          # Test suite overview & statistics
```

## 🚀 Quick Navigation

### I want to...

#### → **Run the tests NOW**
→ [**QUICKSTART.md**](QUICKSTART.md) - Get running in 5 minutes

#### → **Understand what tests exist**
→ [**SUMMARY.md**](SUMMARY.md) - Overview and statistics

#### → **Learn how to use the tests**
→ [**README.md**](README.md) - Complete usage guide

#### → **See detailed test specifications**
→ [**TEST_PLAN.md**](TEST_PLAN.md) - Formal test plan

#### → **Add new tests**
→ [**README.md#adding-new-tests**](README.md#adding-new-tests)

#### → **Troubleshoot failures**
→ [**README.md#troubleshooting**](README.md#troubleshooting)

## 📚 Document Purposes

### [QUICKSTART.md](QUICKSTART.md) - Start Here! ⭐
**Best for:** New users, quick testing
**Length:** ~5 minutes
**Contains:**
- Minimal prerequisites
- Build and run commands
- Troubleshooting basics
- Your first test example

### [README.md](README.md) - Complete Guide
**Best for:** Regular users, developers
**Length:** ~15 minutes
**Contains:**
- Test category descriptions
- All helper functions
- Usage patterns
- Contributing guidelines
- Detailed troubleshooting

### [TEST_PLAN.md](TEST_PLAN.md) - Formal Specification
**Best for:** QA engineers, formal review
**Length:** ~30 minutes
**Contains:**
- 28 detailed test case specifications (TC-*)
- Coverage goals and metrics
- Risk analysis
- Test execution schedule
- Acceptance criteria

### [SUMMARY.md](SUMMARY.md) - Overview
**Best for:** Project managers, overview
**Length:** ~10 minutes
**Contains:**
- What was created and why
- Test coverage statistics
- Success criteria
- Known limitations
- Future enhancements

## 🎯 Common Tasks

### First Time Setup
```bash
cd SplitMap/tests
make test
```
See: [QUICKSTART.md](QUICKSTART.md)

### Run Tests
```bash
make test                    # Standard run
./run_tests.sh -v           # Verbose output
./run_tests.sh --valgrind   # Memory leak detection
```
See: [README.md#usage](README.md)

### Add a New Test
1. Edit `test_splitmap.c`
2. Add test function
3. Register with `RUN_TEST()`
4. Run `make test`

See: [README.md#adding-new-tests](README.md#adding-new-tests)

### Check Test Coverage
```bash
./run_tests.sh --coverage
cat test_splitmap.c.gcov
```
See: [README.md#test-coverage](README.md)

## 📊 Test Suite Statistics

```
Files:          8 (+ this index)
Documentation:  5 markdown files
Test Cases:     28 automated tests
Code Lines:     ~2,500 lines total
Test Code:      807 lines (test_splitmap.c)
Build Time:     ~2 seconds
Run Time:       ~5-10 seconds
```

## ✅ Test Categories (28 tests)

| Category | Tests | Priority |
|----------|-------|----------|
| Command Line Parsing | 3 | High |
| Basic Map Splitting | 3 | Critical |
| Item Extraction | 7 | Critical |
| Horizontal Strip Mode | 2 | High |
| Compression | 2 | High |
| Output Size | 1 | Medium |
| Section Directives | 1 | Medium |
| Edge Cases | 4 | Medium |
| Integration | 5 | Critical |

See: [SUMMARY.md#test-categories-breakdown](SUMMARY.md)

## 🔧 Tools & Requirements

### Required
- GCC with C11 support
- Make
- SplitMap (built automatically)

### Optional
- Valgrind (memory leak detection)
- gcov/lcov (code coverage)

## 📖 Reading Order

### For Developers Adding Tests
1. [QUICKSTART.md](QUICKSTART.md) - Get environment working
2. [README.md](README.md) - Learn helper functions
3. Look at existing tests in `test_splitmap.c`
4. Add your test
5. Update [TEST_PLAN.md](TEST_PLAN.md)

### For QA/Testing Review
1. [SUMMARY.md](SUMMARY.md) - Understand scope
2. [TEST_PLAN.md](TEST_PLAN.md) - Review test cases
3. Run tests: `make test`
4. Review failures in [README.md#troubleshooting](README.md)

### For Project Management
1. [SUMMARY.md](SUMMARY.md) - Quick overview
2. Check "Success Criteria" section
3. Review statistics and coverage

### For Bug Fixing
1. [QUICKSTART.md](QUICKSTART.md) - Run tests
2. Identify failing test
3. Look at test code in `test_splitmap.c`
4. Fix bug in `../splitmap.c`
5. Verify with `make test`

## 🐛 Quick Troubleshooting

| Problem | Quick Fix | Documentation |
|---------|-----------|---------------|
| Tests won't build | `cd .. && make` | [QUICKSTART.md](QUICKSTART.md) |
| All tests fail | Check `../splitmap --help` | [README.md](README.md) |
| One test fails | Read test code | `test_splitmap.c` |
| Memory leaks | Run with `--valgrind` | [README.md](README.md) |
| Permission denied | `chmod +x run_tests.sh` | [QUICKSTART.md](QUICKSTART.md) |

## 🎓 Learning Path

### Beginner
1. Read [QUICKSTART.md](QUICKSTART.md)
2. Run `make test`
3. Look at one simple test in `test_splitmap.c`

### Intermediate
1. Read [README.md](README.md)
2. Understand helper functions
3. Try modifying a test
4. Add a simple new test

### Advanced
1. Read [TEST_PLAN.md](TEST_PLAN.md)
2. Understand coverage goals
3. Add complex integration tests
4. Set up CI/CD integration

## 📞 Support

- **Test failures**: See [README.md#troubleshooting](README.md)
- **Understanding tests**: See [TEST_PLAN.md](TEST_PLAN.md)
- **Quick questions**: See [QUICKSTART.md](QUICKSTART.md)

## 🔄 Continuous Integration

Example GitHub Actions:
```yaml
- name: Run SplitMap Tests
  run: |
    cd SplitMap
    make
    cd tests
    make test
```

See: [SUMMARY.md#ci-cd-integration](SUMMARY.md)

## 📈 Test Maturity

```
✅ Test Framework:       Complete
✅ Unit Tests:           Complete (28 tests)
✅ Integration Tests:    Complete (5 tests)
✅ Documentation:        Complete (5 docs)
✅ Build Integration:    Complete
⚠️ Performance Tests:    Not implemented
⚠️ Fuzz Testing:         Not implemented
⚠️ Assembly Validation:  Partial (structure only)
```

## 🎯 Getting Started (TL;DR)

```bash
# One command to rule them all
cd SplitMap/tests && make test
```

If that works (28/28 passed), you're done! 🎉

If not, see [QUICKSTART.md](QUICKSTART.md).

---

**Last Updated:** 2026-02-25
**Version:** 1.0
**Status:** ✅ Complete and Ready
