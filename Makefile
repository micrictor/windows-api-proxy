# A makefile for compiling main.c using gcc

CC = g++

OPTS = -std=gnu++20

all: main

main: main.cpp main.h thunk.h invoker.h invoker_ioeo.h invoker_ioeo.cpp
	$(CC) $(OPTS) -o main main.cpp invoker_ioeo.cpp

debug:
	$(CC) $(OPTS) -g -o main main.cpp