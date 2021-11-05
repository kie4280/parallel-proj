CXX := g++
CXX_FLAGS := -std=c++17 -O3 -lpthread -mavx2


all: makebuild src/** thc
	$(CXX) $(CXX_FLAGS)

makebuild: 
	mkdir -p build


thc: makebuild src/THC-chess/src/**
	$(CXX) $(CXX_FLAGS) -Isrc/THC-chess/src -c -o build/thc.o src/THC-chess/src/thc.cpp


clean:
	rm -r build/



.PHONY: clean 
