#include <stdio.h>
#include <stdlib.h>


void inputMatrix(int *matrix, int matrixSize) {
    printf("Enter %d×%d matrix elements (0–255):\n", matrixSize, matrixSize);
    int *ptr = matrix;
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++, ptr++) {
            int value;
            while (1) {
                if (scanf("%d", &value) != 1) {
                    printf("Invalid input. Enter an integer 0–255: ");
                    while (getchar() != '\n');
                    continue;
                }
                if (value >= 0 && value <= 255) {
                    *ptr = value;
                    break;
                } else {
                    printf("Out of range. Enter 0–255: ");
                }
            }
        }
    }
}


void printMatrix(int *matrix, int matrixSize) {
    int *ptr = matrix;
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++, ptr++) {
            printf("%d ", *ptr);
        }
        printf("\n");
    }
}


void transposeMatrix(int *matrix, int matrixSize) {
    for (int row = 0; row < matrixSize; row++) {
        for (int col = row + 1; col < matrixSize; col++) {
            int *cell1 = matrix + row * matrixSize + col;
            int *cell2 = matrix + col * matrixSize + row;
            int temp = *cell1;
            *cell1 = *cell2;
            *cell2 = temp;
        }
    }
}


void reverseRows(int *matrix, int matrixSize) {
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize / 2; col++) {
            int *leftCell = matrix + row * matrixSize + col;
            int *rightCell = matrix + row * matrixSize + (matrixSize - 1 - col);
            int temp = *leftCell;
            *leftCell = *rightCell;
            *rightCell = temp;
        }
    }
}


void rotateMatrix90Clockwise(int *matrix, int matrixSize) {
    transposeMatrix(matrix, matrixSize);
    reverseRows(matrix, matrixSize);
}

void smoothMatrixInPlace(int *matrix, int matrixSize) {
    int row, col, dRow, dCol;
    for (row = 0; row < matrixSize; row++) {
        for (col = 0; col < matrixSize; col++) {
            int sum = 0, count = 0;
         
            for (dRow = -1; dRow <= 1; dRow++) {
                int nRow = row + dRow;
                if (nRow < 0 || nRow >= matrixSize) continue;
                for (dCol = -1; dCol <= 1; dCol++) {
                    int nCol = col + dCol;
                    if (nCol < 0 || nCol >= matrixSize) continue;
                    int val = *(matrix + nRow * matrixSize + nCol);
                    
                    if (val > 255) val = val % 256;
                    sum += val;
                    count++;
                }
            }
            
            int *cell = matrix + row * matrixSize + col;
            *cell = *cell + (sum / count) * 256;
        }
    }

  
    for (row = 0; row < matrixSize; row++) {
        for (col = 0; col < matrixSize; col++) {
            int *cell = matrix + row * matrixSize + col;
            *cell = *cell / 256;
        }
    }
}

int main() {
    int matrixSize;

    printf("Enter matrix size (2-10): ");
    while (scanf("%d", &matrixSize) != 1 || matrixSize < 2 || matrixSize > 10) {
        printf("Invalid size. Enter an integer between 2–10: ");
        while (getchar() != '\n');
    }

    int *matrix = (int *)malloc(matrixSize * matrixSize * sizeof(int));
    if (!matrix) return 1;

    inputMatrix(matrix, matrixSize);

    printf("Original Matrix:\n");
    printMatrix(matrix, matrixSize);

    rotateMatrix90Clockwise(matrix, matrixSize);
    printf("Matrix after 90° Clockwise Rotation:\n");
    printMatrix(matrix, matrixSize);

    smoothMatrixInPlace(matrix, matrixSize);
    printf("Matrix after Applying 3×3 Smoothing Filter:\n");
    printMatrix(matrix, matrixSize);

    free(matrix);
    return 0;
}
