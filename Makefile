CXX := g++
CXX_FLAGS := -std=c++17 -O3 -lpthread -mavx2


.PHONY: clean

all: src/**
	mkdir build
	$(CXX) 

thc: src/THC-chess/src/**
	$(CXX) $(CXX_FLAGS) -I 


clean:
	rm -r build/