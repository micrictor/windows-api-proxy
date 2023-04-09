# A makefile for compiling main.c using gcc

CC = gcc

all: main

main: main.c main.h loadlibrary.h thunk.h
	$(CC) -o main main.c

debug:
	$(CC) -g -o main main.c