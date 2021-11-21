CXX := g++
CXX_FLAGS := -std=c++17 -O3 -lpthread -mavx2
OBJECT := build/UCI.o build/thc.o


all: makebuild src/main.cpp thc UCI
	$(CXX) $(CXX_FLAGS) src/main.cpp $(OBJECT) -o build/main.out

makebuild: 
	mkdir -p build

thc: makebuild src/THC-chess/src/**
	$(CXX) $(CXX_FLAGS) -Isrc/THC-chess/src -c -o build/thc.o src/THC-chess/src/thc.cpp

UCI: makebuild src/UCI/**
	$(CXX) $(CXX_FLAGS) -c -o build/UCI.o src/UCI/UCI.cpp

benchmark: src/benchmark.cpp thc
	$(CXX) $(CXX_FLAGS) -o build/benchmark.out src/benchmark.cpp build/thc.o
	build/benchmark.out

clean:
	rm -r build/

.PHONY: 
	clean 
