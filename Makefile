CXX := g++
CXX_FLAGS := -std=c++17 -lpthread -g -O3
OBJECT := build/UCI.o build/thc.o

NVCC = nvcc
CUDA_LINK_FLAGS =  -rdc=true -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' 
CUDA_COMPILE_FLAGS = --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3

all: makebuild src/main.cpp thc UCI src/utils.cpp
	$(CXX) -Isrc/MCTS/ -Isrc/ src/main.cpp src/utils.cpp \
	src/MCTS/MCTS_root.cpp $(OBJECT) $(CXX_FLAGS) -o build/main.out

#TODO:nvcc compile

makebuild: 
	mkdir -p build

thc: makebuild src/THC-chess/src/**
	$(CXX) $(CXX_FLAGS) -Isrc/THC-chess/src -c -o build/thc.o src/THC-chess/src/thc.cpp

UCI: makebuild src/UCI/**
	$(CXX) $(CXX_FLAGS) -c -o build/UCI.o src/UCI/UCI.cpp

benchmark: src/benchmark.cpp thc
	$(CXX) $(CXX_FLAGS) -o build/benchmark.out src/benchmark.cpp build/thc.o
	build/benchmark.out

play:
	python3 src/arbiter.py

clean:
	rm -r build/

.PHONY: 
	clean 
