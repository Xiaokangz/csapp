/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Author: Xiaokang Zhang
 * Email: xiaokangz00@gmail.com
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l, temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
    // 32 by 32 matrix
    if (M == 32 && N == 32) {
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (k = i; k < i + 8; k++) {
                    temp0 = A[k][j + 0];
                    temp1 = A[k][j + 1];
                    temp2 = A[k][j + 2];
                    temp3 = A[k][j + 3];
                    temp4 = A[k][j + 4];
                    temp5 = A[k][j + 5];
                    temp6 = A[k][j + 6];
                    temp7 = A[k][j + 7];
                    B[j + 0][k] = temp0;
                    B[j + 1][k] = temp1;
                    B[j + 2][k] = temp2;
                    B[j + 3][k] = temp3;
                    B[j + 4][k] = temp4;
                    B[j + 5][k] = temp5;
                    B[j + 6][k] = temp6;
                    B[j + 7][k] = temp7;
                }
            }
        }
    } else if (M == 64 && N == 64) {          // 64 by 64 matrix
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (k = i; k < i + 4; k++) {
                    temp0 = A[k][j + 0];
                    temp1 = A[k][j + 1];
                    temp2 = A[k][j + 2];
                    temp3 = A[k][j + 3];
                    temp4 = A[k][j + 4];
                    temp5 = A[k][j + 5];
                    temp6 = A[k][j + 6];
                    temp7 = A[k][j + 7];
                    B[j + 0][k] = temp0;
                    B[j + 0][k + 4] = temp4;
                    B[j + 1][k] = temp1;
                    B[j + 1][k + 4] = temp5;
                    B[j + 2][k] = temp2;
                    B[j + 2][k + 4] = temp6;
                    B[j + 3][k] = temp3;
                    B[j + 3][k + 4] = temp7;
                }
                for (k = j; k < j + 4; k++) {
                    temp0 = A[i + 4][k];
                    temp1 = A[i + 5][k];
                    temp2 = A[i + 6][k];
                    temp3 = A[i + 7][k];
                    temp4 = B[k][i + 4];
                    temp5 = B[k][i + 5];
                    temp6 = B[k][i + 6];
                    temp7 = B[k][i + 7];
                    B[k][i + 4] = temp0;
                    B[k][i + 5] = temp1;
                    B[k][i + 6] = temp2;
                    B[k][i + 7] = temp3;
                    B[k + 4][i + 0] = temp4;
                    B[k + 4][i + 1] = temp5;
                    B[k + 4][i + 2] = temp6;
                    B[k + 4][i + 3] = temp7;
                }
                for (k = i + 4; k < i + 8; k++) {
                    temp4 = A[k][j + 4];
                    temp5 = A[k][j + 5];
                    temp6 = A[k][j + 6];
                    temp7 = A[k][j + 7];
                    B[j + 4][k] = temp4;
                    B[j + 5][k] = temp5;
                    B[j + 6][k] = temp6;
                    B[j + 7][k] = temp7;
                }
            }
        }
    } else {                                 // 61 by 67 matrix
        for (i = 0; i < N; i += 16) {
            for (j = 0; j < M; j += 16) {
                for (k = i; k < i + 16 && k < N; k++) {
                    for (l = j; l < j + 16 && l < M; l++) {
                        B[l][k] = A[k][l];
                    }
                }
            }
        }  
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
