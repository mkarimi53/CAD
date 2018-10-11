# this makefile is intended for gcc on linux

CC = gcc

all: Project

clean:
	rm -f *.o Project

Project: Project.o Encoder.o Parse_bmp.o
	$(CC)  Project.o Encoder.o Parse_bmp.o -o Project -lm

Project.o : Project.c
Encoder.o : Encoder.c Coding.h
Parse_bmp.o : Parse_bmp.c
