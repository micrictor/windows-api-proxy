# A makefile for compiling main.c using gcc

CC = gcc

all: main

main: main.c allocvm.h loadlibrary.h protectvm.h thunk.h payload.h procaddr.h
	$(CC) -o main main.c

debug:
	$(CC) -g -o main main.c

control: control.c payload.h procaddr.h
	$(CC) -o control control.c