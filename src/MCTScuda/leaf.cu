#include <cuda.h>
#include "leaf.h"

__global__ void leaf_simple_evaluation(char *chess_raw, float *scores, int n) {
    int id = blockIdx.x * blockDim.x + threadIdx.x;
    int result = 0;

    if (id < n){
        char *p = chess_raw + id * BOARD_LENGTH;

        for(int i = 0; i < BOARD_LENGTH; i++){
            if(p[i] == '0')
                continue;
            else if(p[i] == 'K')
                result += 200;
            else if(p[i] == 'k')
                result -= 200;
            else if(p[i] == 'Q')
                result += 9;
            else if(p[i] == 'q')
                result -= 9;
            else if(p[i] == 'R')
                result += 5;
            else if(p[i] == 'r')
                result -= 5;
            else if(p[i] == 'B')
                result += 3;
            else if(p[i] == 'b')
                result -= 3;
            else if(p[i] == 'N')
                result += 3;
            else if(p[i] == 'n')
                result -= 3;
            else if(p[i] == 'P')
                result += 1;
            else if(p[i] == 'p')
                result -= 1;
        }
        scores[id] = result / 20.0;
    }
    
}

void hostFE(char *chess_raw, float *scores, int n)
{
    float *scores_d;
    char *chess_raw_d;
    cudaMalloc((void **)&chess_raw_d, n * BOARD_LENGTH * sizeof(char));
    cudaMalloc((void **)&scores_d, n * sizeof(float));

    //move data from h to d
    cudaMemcpy(chess_raw_d, chess_raw, n * BOARD_LENGTH * sizeof(char), cudaMemcpyHostToDevice);

    //kernel function 1-D
    int blockSize = 1024;
    int gridSize = (int)ceil((float)n/blockSize);
    leaf_simple_evaluation<<<blockSize, gridSize>>>(chess_raw_d, scores_d, n);

    //move data from d to h
    cudaDeviceSynchronize();
    cudaMemcpy(scores, scores_d, n * sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(chess_raw_d);
    cudaFree(scores_d);
}
