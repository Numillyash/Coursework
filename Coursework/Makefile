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

CPPFLAGS ?=
LDFLAGS ?=

# Main executable sources
WORK_SOURCES = Coursework.c RSA.c bit_LA.c _file_.c _log_.c
WORK_OBJECTS = $(WORK_SOURCES:.c=.o)
WORK_EXEC = work1

# Benchmark executable
BENCH_SOURCES = bench.c bit_LA.c _log_.c
BENCH_OBJECTS = $(BENCH_SOURCES:.c=.o)
BENCH_EXEC = bench_rsa

PHONY += all work1 bench prof release clean

all: work1

work1: $(WORK_OBJECTS)
	$(CC) $(LDFLAGS) $(WORK_OBJECTS) -o $@

bench: CPPFLAGS += -DNO_LOG
bench: $(BENCH_OBJECTS)
	$(CC) $(LDFLAGS) $(BENCH_OBJECTS) -o $(BENCH_EXEC)

%.o: %.c
	$(CC) $(CSTD) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: all work1 bench prof release clean

prof:
	$(MAKE) MODE=prof bench

release:
	$(MAKE) MODE=release all

clean:
	rm -f *.o $(WORK_EXEC) $(BENCH_EXEC)