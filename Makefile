CXX := g++
CXX_FLAGS := -std=c++17 -lpthread -g -O3
OBJECT := build/UCI.o build/thc.o build/util.o 


NVCC = nvcc
CUDA_LINK_FLAGS =  -rdc=true -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' 
CUDA_COMPILE_FLAGS = --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3
CUDA_OBJECT = build/main_leaf.o build/leaf.o build/bridge.o build/MCTS_leaf.o 

all: makebuild src/main.cpp thc UCI src/utils.cpp
	$(CXX) -Isrc/MCTS/ -Isrc/ src/main.cpp src/utils.cpp \
	src/MCTS/MCTS_root.cpp $(OBJECT) $(CXX_FLAGS) -o build/main.out

#TODO:nvcc compile


MCTScuda:  thc UCI util leaf main_leaf bridge MCTS_leaf
	$(NVCC) ${CUDA_LINK_FLAGS} -o build/MCTScuda.out $(CUDA_OBJECT) $(OBJECT)

main_leaf: src/main_leaf.cpp src/MCTScuda/MCTS_leaf.cpp
	$(CXX) src/main_leaf.cpp $(CXX_FLAGS) -c -o build/main_leaf.o 

MCTS_leaf: src/MCTScuda/MCTS_leaf.cpp
	$(CXX) src/MCTScuda/MCTS_leaf.cpp $(CXX_FLAGS) -c -o build/MCTS_leaf.o 

leaf:
	${NVCC} ${CUDA_COMPILE_FLAGS} src/MCTScuda/leaf.cu -c -o build/leaf.o

bridge: src/MCTScuda/bridge.cpp
	$(CXX) src/MCTScuda/bridge.cpp $(CXX_FLAGS) -c -o build/bridge.o

util:
	$(CXX) src/utils.cpp $(CXX_FLAGS) -c -o build/util.o 

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
