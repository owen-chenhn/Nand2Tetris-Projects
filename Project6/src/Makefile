CFLAGS = -std=c++11 -Wall -O2 -lm 
CC = g++


source = $(wildcard *.cpp)
obj = $(source:.cpp=)


all: $(obj)

Assembler : Assembler.cpp
	$(CC) $(CFLAGS) Assembler.cpp -o Assembler

p2 : p2.cpp p1.cpp p1.h
	$(CC) $(CFLAGS) p2.cpp p1.cpp p1.h -o p2

clear:
	rm -f $(obj)
