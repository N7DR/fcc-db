# makefile for fcc-db

# execute "make fcc-db" to create the executable ./bin/fcc-db

CC = ccache g++
CPP = /usr/local/bin/cpp
LIB = ar
ASM = as

# The places to look for include files (in order).
INCL =  -I./include

# Additional libraries
LIBRARIES = -lpthread

LIBINCL = -L.

# Extra Defs

# utility routines
DEL = rm
COPY = cp

# name of main executable to build
PROG = all

CFLAGS = $(INCL) -c -g -O2 -Wall -std=c++2b

LINKFLAGS = $(LIBINCL)

include/fcc-db.h : include/fcc-strings.h
	touch include/fcc-db.h
	
src/fcc-db.cpp : include/fcc-db.h
	touch src/fcc-db.cpp
	
src/fcc-strings.cpp : include/fcc-strings.h
	touch src/fcc-strings.cpp
	
bin/fcc-db.o : src/fcc-db.cpp
	$(CC) $(CFLAGS) -o $@ src/fcc-db.cpp

bin/fcc-strings.o : src/fcc-strings.cpp
	$(CC) $(CFLAGS) -o $@ src/fcc-strings.cpp

bin/fcc-db : bin/fcc-db.o bin/fcc-strings.o
	mkdir -p bin
	$(CC) $(LINKFLAGS) bin/fcc-db.o bin/fcc-strings.o $(LIBRARIES) \
	-o bin/fcc-db
	
fcc-db : directories bin/fcc-db

directories: bin

bin:
	mkdir -p bin

# clean everything
clean :
	rm bin/*
	
FORCE:
