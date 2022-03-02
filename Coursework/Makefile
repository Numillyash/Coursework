CC=gcc
CFLAGS=-g -Wall -O0
LDFLAGS=
SOURCES=Coursework.c bit_LA.c bit_LA.h config.h help.h
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test1

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@