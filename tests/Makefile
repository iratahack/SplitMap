# Makefile for SplitMap Test Suite

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS =

# Test executable
TEST_BIN = test_splitmap

# Source files
TEST_SRC = test_splitmap.c

# SplitMap executable (built from parent directory)
SPLITMAP = ../build/splitmap

.PHONY: all test clean

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SRC)
	@echo "Building test suite..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Test suite built successfully!"

# Ensure splitmap is built before running tests
$(SPLITMAP):
	@echo "Building splitmap..."
	@cd .. && $(MAKE)

test: $(TEST_BIN) $(SPLITMAP)
	@echo "Running SplitMap test suite..."
	@echo ""
	@# Copy splitmap to tests directory for easier execution
	@cp $(SPLITMAP) .
	@./$(TEST_BIN)
	@# Clean up copied splitmap
	@rm -f ./splitmap

clean:
	@echo "Cleaning test directory..."
	@rm -f $(TEST_BIN)
	@rm -f ./splitmap
	@rm -f test_*.nxm test_*.inc test_*.zx0
	@rm -f *.nxm *.inc *.zx0
	@echo "Clean complete!"

help:
	@echo "SplitMap Test Suite Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all    - Build the test suite"
	@echo "  test   - Build and run all tests"
	@echo "  clean  - Remove generated files"
	@echo "  help   - Show this help message"
