CC ?= gcc
CXX ?= g++
MODE ?= debug

CSTD := -std=c11
CXXSTD := -std=c++17
WARN := -Wall -Wextra

INCS := -I./include -I./lib/bigint -I./lib/rsa -I./lib/util
CPPFLAGS := $(INCS)

ifeq ($(MODE),debug)
  CFLAGS := -O0 -g $(WARN)
else ifeq ($(MODE),release)
  CFLAGS := -O2 -DNDEBUG $(WARN)
else ifeq ($(MODE),prof)
  CFLAGS := -O2 -g -fno-omit-frame-pointer $(WARN)
else
  $(error Unknown MODE=$(MODE))
endif

BUILD := build/$(MODE)

WORK_EXEC  := work1
BENCH_EXEC := bench_rsa
TEST_BITBIGINT_EXEC := $(BUILD)/test_bitbigint
TEST_TC_EXEC := $(BUILD)/test_tc_vs_legacy
TEST_DIVMOD_RANDOM_EXEC := $(BUILD)/test_divmod_random
TEST_DIVMOD_STRESS_EXEC := $(BUILD)/test_divmod_stress
TEST_SEEDS_0TO49_EXEC := $(BUILD)/test_seeds_0to49
TEST_RSA_TC_SMOKE_EXEC := $(BUILD)/test_rsa_tc_smoke

WORK_SOURCES := \
  tools/Coursework.c \
  lib/bigint/bit_LA.c \
  lib/rsa/RSA.c \
  lib/util/_file_.c \
  lib/util/_log_.c

BENCH_SOURCES := \
  tools/bench.c \
  lib/bigint/bit_LA.c \
  lib/util/_log_.c

WORK_OBJECTS  := $(patsubst %.c,$(BUILD)/%.o,$(WORK_SOURCES))
BENCH_OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(BENCH_SOURCES))

DEPS := $(WORK_OBJECTS:.o=.d) $(BENCH_OBJECTS:.o=.d)
-include $(DEPS)

.PHONY: all bench test test-rsa test-stress test_bitbigint test_tc_vs_legacy test_divmod_random test-divmod-stress test_seeds_0to49 test-rsa-tc-smoke tc_test clean prof release

all: $(WORK_EXEC) bench

bench: $(BENCH_EXEC)

test: test_bitbigint test_tc_vs_legacy test-rsa-tc-smoke

test-rsa: $(WORK_EXEC)
	bash ./test_all.sh

test-stress: test_divmod_random test_seeds_0to49

$(WORK_EXEC): $(WORK_OBJECTS)
	$(CC) $^ -o $@

$(BENCH_EXEC): CPPFLAGS += -DNO_LOG
$(BENCH_EXEC): $(BENCH_OBJECTS)
	$(CC) $^ -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CSTD) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# C++ compilation rule
$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# C++ test binary
test_bitbigint: $(TEST_BITBIGINT_EXEC)
	./$(TEST_BITBIGINT_EXEC)

$(TEST_BITBIGINT_EXEC): $(BUILD)/lib/bigint/BitBigInt.o tests/test_bitbigint.cpp
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) tests/test_bitbigint.cpp $(BUILD)/lib/bigint/BitBigInt.o -o $@

# BitBigIntTC legacy equivalence test
tc_test: test_tc_vs_legacy

test_tc_vs_legacy: $(TEST_TC_EXEC)
	cp $(TEST_TC_EXEC) test_tc_vs_legacy
	./$(TEST_TC_EXEC)

$(TEST_TC_EXEC): $(BUILD)/lib/bigint/BitBigIntTC.o tests/test_tc_vs_legacy.cpp $(BUILD)/lib/bigint/bit_LA.o $(BUILD)/lib/util/_log_.o
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) tests/test_tc_vs_legacy.cpp $(BUILD)/lib/bigint/BitBigIntTC.o $(BUILD)/lib/bigint/bit_LA.o $(BUILD)/lib/util/_log_.o -o $@

test_divmod_random: $(TEST_DIVMOD_RANDOM_EXEC)
	./$(TEST_DIVMOD_RANDOM_EXEC)

$(TEST_DIVMOD_RANDOM_EXEC): $(BUILD)/lib/bigint/BitBigIntTC.o test_divmod_random.cpp
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) test_divmod_random.cpp $(BUILD)/lib/bigint/BitBigIntTC.o -o $@

test-divmod-stress: $(TEST_DIVMOD_STRESS_EXEC)
	@if command -v timeout >/dev/null 2>&1; then \
		timeout 60s ./$(TEST_DIVMOD_STRESS_EXEC); \
	else \
		./$(TEST_DIVMOD_STRESS_EXEC); \
	fi

$(TEST_DIVMOD_STRESS_EXEC): $(BUILD)/lib/bigint/BitBigIntTC.o tests/test_divmod_stress.cpp $(BUILD)/lib/bigint/bit_LA.o $(BUILD)/lib/util/_log_.o
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) tests/test_divmod_stress.cpp $(BUILD)/lib/bigint/BitBigIntTC.o $(BUILD)/lib/bigint/bit_LA.o $(BUILD)/lib/util/_log_.o -o $@

test_seeds_0to49: $(TEST_SEEDS_0TO49_EXEC)
	./$(TEST_SEEDS_0TO49_EXEC)

$(TEST_SEEDS_0TO49_EXEC): $(BUILD)/lib/bigint/BitBigIntTC.o test_seeds_0to49.cpp
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) test_seeds_0to49.cpp $(BUILD)/lib/bigint/BitBigIntTC.o -o $@

test-rsa-tc-smoke: $(WORK_EXEC) $(TEST_RSA_TC_SMOKE_EXEC)
	./$(TEST_RSA_TC_SMOKE_EXEC)

$(TEST_RSA_TC_SMOKE_EXEC): $(BUILD)/lib/bigint/BitBigIntTC.o $(BUILD)/lib/rsa/RSA_TC.o tests/test_rsa_tc_smoke.cpp
	$(CXX) $(CXXSTD) $(CFLAGS) $(CPPFLAGS) tests/test_rsa_tc_smoke.cpp $(BUILD)/lib/bigint/BitBigIntTC.o $(BUILD)/lib/rsa/RSA_TC.o -o $@

prof:
	$(MAKE) MODE=prof bench

release:
	$(MAKE) MODE=release all

clean:
	rm -rf build
	rm -f $(WORK_EXEC) $(BENCH_EXEC) test_bitbigint test_tc_vs_legacy test_divmod_random test_divmod_stress test_seeds_0to49 test_rsa_tc_smoke
