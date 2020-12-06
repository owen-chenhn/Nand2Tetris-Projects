CFLAGS = -std=c++11 -Wall -O2 -lm 
CC = g++


source = $(wildcard *.cpp)
obj = $(source:.cpp=)


all: $(obj)

VMTranslate : VMTranslate.cpp
	$(CC) $(CFLAGS) VMTranslate.cpp -o VMTranslate

clear:
	rm -f $(obj)
