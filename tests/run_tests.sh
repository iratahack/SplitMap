#!/bin/bash
# SplitMap Test Runner Script
#
# Provides convenient ways to run the test suite with various options

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default options
VERBOSE=0
VALGRIND=0
QUICK=0
COVERAGE=0

# Print usage
usage() {
    cat << EOF
SplitMap Test Runner

Usage: $0 [OPTIONS]

Options:
    -h, --help          Show this help message
    -v, --verbose       Enable verbose output
    -q, --quick         Run quick tests only (skip slow integration tests)
    -m, --valgrind      Run tests under valgrind for memory leak detection
    -c, --coverage      Generate code coverage report (requires gcov)
    --clean             Clean all generated files and exit

Examples:
    $0                  Run all tests
    $0 -v               Run with verbose output
    $0 -m               Run with memory leak detection
    $0 --clean          Clean generated files

EOF
    exit 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -q|--quick)
            QUICK=1
            shift
            ;;
        -m|--valgrind)
            VALGRIND=1
            shift
            ;;
        -c|--coverage)
            COVERAGE=1
            shift
            ;;
        --clean)
            echo "Cleaning test files..."
            make clean
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

# Print header
echo "======================================================================="
echo "                      SplitMap Test Suite"
echo "======================================================================="
echo ""

# Check for splitmap
if [ ! -f "../build/splitmap" ]; then
    echo -e "${YELLOW}Building splitmap...${NC}"
    cd ..
    make
    cd tests
    echo ""
fi

# Build test suite
echo -e "${YELLOW}Building test suite...${NC}"
if [ $COVERAGE -eq 1 ]; then
    make clean
    CFLAGS="-Wall -Wextra -std=c11 -g --coverage" make
else
    make
fi
echo ""

# Copy splitmap to tests directory
cp ../build/splitmap ./splitmap

# Run tests
echo -e "${YELLOW}Running tests...${NC}"
echo ""

if [ $VALGRIND -eq 1 ]; then
    # Check if valgrind is installed
    if ! command -v valgrind &> /dev/null; then
        echo -e "${RED}Error: valgrind is not installed${NC}"
        echo "Install with: sudo apt-get install valgrind"
        exit 1
    fi

    echo -e "${YELLOW}Running with valgrind (memory leak detection)...${NC}"
    echo ""
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --verbose \
             --log-file=valgrind-report.txt \
             ./test_splitmap

    echo ""
    echo -e "${YELLOW}Valgrind report saved to: valgrind-report.txt${NC}"

    # Show summary
    if grep -q "ERROR SUMMARY: 0 errors" valgrind-report.txt; then
        echo -e "${GREEN}No memory errors detected!${NC}"
    else
        echo -e "${RED}Memory errors detected! Check valgrind-report.txt${NC}"
    fi

elif [ $VERBOSE -eq 1 ]; then
    ./test_splitmap 2>&1 | tee test-output.txt

else
    ./test_splitmap
fi

TEST_RESULT=$?

# Generate coverage report if requested
if [ $COVERAGE -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}Generating coverage report...${NC}"
    gcov test_splitmap.c
    echo ""
    echo "Coverage report generated: test_splitmap.c.gcov"
fi

# Cleanup
rm -f ./splitmap

echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
else
    echo -e "${RED}Some tests failed!${NC}"
fi

echo "======================================================================="

exit $TEST_RESULT
