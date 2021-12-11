#include "leaf.h"
#include<stdio.h>


void leaf_parallel(char* chess_raw, float* scores, int n)
{
    hostFE(chess_raw, scores, n);
}
