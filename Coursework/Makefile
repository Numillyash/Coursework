CC=gcc
CFLAGS=-g -Wall -Wextra -O0
LDFLAGS=
SOURCES=Coursework.c bit_LA.c bit_LA.h config.h help.h _file_.c _file_.h _log_.c _log_.h RSA.c RSA.h _autorization_.c _autorization_.h
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=work1.exe

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@