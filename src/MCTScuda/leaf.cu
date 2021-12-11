#include <cuda.h>
#include <stdio.h>
#define BOARD_LENGTH 64

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
   if (code != cudaSuccess)
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__global__ void leaf_simple_evaluation(char *chess_raw, float *scores, int n) {
    int id = threadIdx.x;
    float result = 0.0f;
    if (id < n){
        int lower = id * BOARD_LENGTH;

        for(int i = 0; i < BOARD_LENGTH; i++){
            char temp = chess_raw[lower + i];
            if(temp == ' ')
                continue;
            else if(temp == 'K')
                result += 200;
            else if(temp == 'k')
                result -= 200;
            else if(temp == 'Q')
                result += 9;
            else if(temp == 'q')
                result -= 9;
            else if(temp == 'R')
                result += 5;
            else if(temp == 'r')
                result -= 5;
            else if(temp == 'B')
                result += 3;
            else if(temp == 'b')
                result -= 3;
            else if(temp == 'N')
                result += 3;
            else if(temp == 'n')
                result -= 3;
            else if(temp == 'P')
                result += 1;
            else if(temp == 'p')
                result -= 1;
        }
        scores[id] = result;
    }
}
//BUG: in leaf_simple_evaluation nothing happened, scores is always 0.

void hostFE(char* chess_raw, float* scores, int n)
{
    float *scores_d;
    char *chess_raw_d;
    cudaMalloc((void **)&chess_raw_d, n * BOARD_LENGTH * sizeof(char));
    cudaMalloc((void **)&scores_d, n * sizeof(float));

    cudaMemcpy(chess_raw_d, chess_raw, n * BOARD_LENGTH * sizeof(char), cudaMemcpyHostToDevice);

    int thread_num = 128;
    int block_num = (n + thread_num - 1) / thread_num;
    leaf_simple_evaluation<<<block_num, thread_num>>>(chess_raw_d, scores_d, n);
    cudaDeviceSynchronize();

    cudaMemcpy(scores, scores_d, n * sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(chess_raw_d);
    cudaFree(scores_d);
}
