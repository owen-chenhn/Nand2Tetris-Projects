CFLAGS = -std=c++11 -Wall -O2 -lm
CC = g++


source = $(wildcard *.cpp)
obj = $(source:.cpp=.o)


all: compile.o JackCompiler.o
	$(CC) -o compile compile.o JackCompiler.o

compile.o: compile.cpp
	$(CC) -I. $(CFLAGS) -c compile.cpp

JackCompiler.o: JackCompiler.cpp
	$(CC) -I. $(CFLAGS) -c JackCompiler.cpp

clear:
	rm -f $(obj) compile
