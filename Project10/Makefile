CFLAGS = -std=c++11 -Wall -O2 -lm 
CC = g++


source = $(wildcard *.cpp)
obj = $(source:.cpp=)


all: $(obj)

$(obj): %: %.cpp
	$(CC) $(CFLAGS) $< -o $@ 

clear:
	rm -f $(obj)
