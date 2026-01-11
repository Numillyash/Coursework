CC ?= gcc
CXX ?= g++
MODE ?= debug

CSTD := -std=c11
CXXSTD := -std=c++17
WARN := -Wall -Wextra

INCS := -I./include -I./lib/bigint -I./lib/rsa -I./lib/util
CPPFLAGS_COMMON := $(INCS)

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

.PHONY: all work1 bench test clean prof release

all: work1 bench

work1: $(WORK_EXEC)

bench: $(BENCH_EXEC)

test: test_bitbigint
	./test_bitbigint

$(WORK_EXEC): $(WORK_OBJECTS)
	$(CC) $^ -o $@

$(BENCH_EXEC): CPPFLAGS := $(CPPFLAGS_COMMON) -DNO_LOG
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
test_bitbigint: $(BUILD)/lib/bigint/BitBigInt.o tests/test_bitbigint.cpp
	$(CXX) $(CXXSTD) $(CFLAGS) -I./lib/bigint -I./include tests/test_bitbigint.cpp $(BUILD)/lib/bigint/BitBigInt.o -o test_bitbigint

prof:
	$(MAKE) MODE=prof bench

release:
	$(MAKE) MODE=release all

clean:
	rm -rf build
	rm -f $(WORK_EXEC) $(BENCH_EXEC) test_bitbigint
