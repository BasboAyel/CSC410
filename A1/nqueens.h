#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// Function to check if placing a queen is safe
bool isSafe(int board[], int row, int col, int n) {
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col)) {
            return false;
        }
    }
    return true;
}

// Recursive utility to solve N-Queens
void solveNQueensUtil(int board[], int col, int n) {
    // Base case: all queens placed
    if (col >= n) {
        for (int i = 0; i < n; i++) {
            printf("%d ", board[i]);
        }
        printf("\n");
        return;
    }

    // Try placing a queen in each row
    for (int i = 0; i < n; i++) {
        if (isSafe(board, i, col, n)) {   // <-- must be isSafe, not issafe
            board[col] = i;
            solveNQueensUtil(board, col + 1, n);
            board[col] = -1;  // backtrack
        }
    }
}

int main() {
    int n = 4;  // Change this for different board sizes
    int board[n];
    for (int i = 0; i < n; i++) board[i] = -1;

    printf("Solutions for %d-Queens:\n", n);
    solveNQueensUtil(board, 0, n);

    return 0;
}
