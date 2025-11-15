#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

bool isSafe(int board[], int row, int col, int n) 
{
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col)) {
            return false;
        }
    }
    return true;
}

void solveNQueensUtil(int board[], int col, int n) 
{
    // Base case
    if (col >= n) {
        // Print the solution
        for (int i = 0; i < n; i++) {
            printf("%d ", board[i]);
            }
            printf("\n");
            return;
        }
       


    // Recursive case
    for (int i = 0; i < n; i++) {
        if (isSafe(board, i, col, n)) {
            board[col] = i; // Place queen
            solveNQueensUtil(board, col + 1, n); // Recur
            board[col] = -1; // Backtrack
        }
    }

}