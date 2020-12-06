CFLAGS = -std=c++11 -Wall -O2 -lm 
CC = g++


source = $(wildcard *.cpp)
obj = $(source:.cpp=)


all: $(obj)

text_strip : text_strip.cpp
	$(CC) $(CFLAGS) text_strip.cpp -o text_strip

p2 : p2.cpp p1.cpp p1.h
	$(CC) $(CFLAGS) p2.cpp p1.cpp p1.h -o p2

clear:
	rm -f $(obj)
