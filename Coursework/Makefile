CC ?= gcc
MODE ?= debug
COMMON_WARN = -Wall -Wextra
CSTD = -std=c11

ifeq ($(MODE),debug)
CFLAGS = -O0 -g $(COMMON_WARN)
endif
ifeq ($(MODE),release)
CFLAGS = -O2 -DNDEBUG $(COMMON_WARN)
endif
ifeq ($(MODE),prof)
CFLAGS = -O2 -g -fno-omit-frame-pointer $(COMMON_WARN)
endif

CPPFLAGS ?= -I./include -I./lib/bigint -I./lib/rsa -I./lib/util
LDFLAGS ?=

# Library object files
LIB_BIGINT = lib/bigint/bit_LA.o
LIB_RSA = lib/rsa/RSA.o
LIB_UTIL = lib/util/_file_.o lib/util/_log_.o

# All library objects
LIB_OBJECTS = $(LIB_BIGINT) $(LIB_RSA) $(LIB_UTIL)

# Main executable sources
WORK_SOURCES = tools/Coursework.c
WORK_OBJECTS = $(WORK_SOURCES:.c=.o) $(LIB_OBJECTS)
WORK_EXEC = work1

# Benchmark executable
BENCH_SOURCES = tools/bench.c
BENCH_OBJECTS = $(BENCH_SOURCES:.c=.o) $(LIB_BIGINT) lib/util/_log_.o
BENCH_EXEC = bench_rsa

.PHONY: all work1 bench prof release clean

all: work1

work1: $(WORK_OBJECTS)
	$(CC) $(LDFLAGS) $(WORK_OBJECTS) -o $@

bench: CPPFLAGS += -DNO_LOG
bench: $(BENCH_OBJECTS)
	$(CC) $(LDFLAGS) $(BENCH_OBJECTS) -o $(BENCH_EXEC)

lib/%.o: lib/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CSTD) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

tools/%.o: tools/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CSTD) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CSTD) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: all work1 bench prof release clean

prof:
	$(MAKE) MODE=prof bench

release:
	$(MAKE) MODE=release all

clean:
	rm -f lib/**/*.o tools/*.o $(WORK_EXEC) $(BENCH_EXEC)clean:
	rm -f *.o $(WORK_EXEC) $(BENCH_EXEC)