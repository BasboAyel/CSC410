#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>

bool isSafe(int board[], int row, int col) {
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col))
            return false;
    }
    return true;
}

void solveNQueensUtil(int board[], int col, int n, long *solutions) {
    if (col >= n) {
        #pragma omp atomic
        (*solutions)++;
        return;
    }
    for (int i = 0; i < n; i++) {
        if (isSafe(board, i, col)) {
            board[col] = i;
            solveNQueensUtil(board, col + 1, n, solutions);
            board[col] = -1;
        }
    }
}

int main() {
    int n = 15;
    long solutions = 0;
    double start = omp_get_wtime();

    #pragma omp parallel
    {
        int board[n];
        for (int i = 0; i < n; i++) board[i] = -1;

        #pragma omp for schedule(dynamic)
        for (int i = 0; i < n; i++) {
            board[0] = i;
            solveNQueensUtil(board, 1, n, &solutions);
        }
    }

    double end = omp_get_wtime();
    printf("%d-Queens total solutions: %ld\n", n, solutions);
    printf("Time: %f seconds\n", end - start);
    return 0;
}
