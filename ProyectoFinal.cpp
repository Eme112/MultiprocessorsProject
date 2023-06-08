#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

uint32_t rows_a, cols_a, rows_b, cols_b, rows_c, cols_c;
uint32_t size_a, size_b, size_c;
double* A = NULL;
double* B = NULL;
double* C = NULL;

int validateSize(uint32_t rows, uint32_t cols, char matrix) {
    FILE* file;
    double value;
    int count = 0;
    char filename[10];

    if (matrix == 'A')
        strcpy(filename, "matrizA.txt");
    else
        strcpy(filename, "matrizB.txt");

    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 0;
    } else {
        printf("File opened successfully.\n");
    }

    while (fscanf(file, "%lf", &value) != EOF) {
        count++;
        char nextChar = fgetc(file);
    }

    fclose(file);

    if (count == rows * cols) {
        return count;
    } else {
        printf("Invalid size for matrix %c\n", matrix);
        return 0;
    }
}

void askForInputsA() {
    printf("Enter number of rows for matrix A: ");
    scanf("%u", &rows_a);
    printf("Enter number of columns for matrix A: ");
    scanf("%u", &cols_a);
}

void askForInputsB() {
    printf("Enter number of rows for matrix B: ");
    scanf("%u", &rows_b);
    printf("Enter number of columns for matrix B: ");
    scanf("%u", &cols_b);
}

uint8_t validateInputs() {
    if (cols_a == rows_b) {
        printf("Valid multiplication\n");
        return 1;
    } else {
        printf("Multiplication not valid\n");
        return 0;
    }
}

int readMatrixFromFile(const char* filename, double** matrix, uint32_t rows, uint32_t cols) {
    FILE* inputFile = fopen(filename, "r");
    if (inputFile != NULL) {
        *matrix = (double*)malloc(rows * cols * sizeof(double));
        for (uint32_t i = 0; i < rows; i++) {
            for (uint32_t j = 0; j < cols; j++) {
                if (fscanf(inputFile, "%lf", &(*matrix)[i * cols + j]) != 1) {
                    fclose(inputFile);
                    free(*matrix);
                    return 0;
                }
            }
        }
        fclose(inputFile);
        return 1;
    }
    return 0;
}

void multiplyMatrices(double* A, double* B, double* C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b) {
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            C[i * cols_b + j] = 0;
            for (uint32_t k = 0; k < cols_a; k++) {
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
            // Round Results to 10 decimals
            C[i * cols_b + j] = round(C[i * cols_b + j] * 1e10) / 1e10;
        }
    }
}

void multiplyMatrices_openmp(double* A, double* B, double* C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b) {
    omp_set_num_threads(omp_get_num_procs());
    #pragma omp parallel for
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            C[i * cols_b + j] = 0;
            for (uint32_t k = 0; k < cols_a; k++) {
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
            // Round Results to 10 decimals
            C[i * cols_b + j] = round(C[i * cols_b + j] * 1e10) / 1e10;
        }
    }
}

int compareMatrices(double* C, const char* filename, uint32_t rows, uint32_t cols) {
    FILE* inputFile = fopen(filename, "r");
    if (inputFile != NULL) {
        for (uint32_t i = 0; i < rows; i++) {
            for (uint32_t j = 0; j < cols; j++) {
                double value;
                if (fscanf(inputFile, "%lf", &value) != 1) {
                    fclose(inputFile);
                    return 0;
                }
                double roundedC = round(C[i * cols + j] * 1e10) / 1e10;
                double roundedValue = round(value * 1e10) / 1e10;
                if (roundedC != roundedValue) {
                    fclose(inputFile);
                    return 0;
                }
            }
        }
        fclose(inputFile);
        return 1;
    }
    return 0;
}

int main() {
    askForInputsA();
    size_a = validateSize(rows_a, cols_a, 'A');
    if (size_a == 0)
        return 0;
    askForInputsB();
    size_b = validateSize(rows_b, cols_b, 'B');
    if (size_b == 0)
        return 0;
    if (!validateInputs())
        return 0;

    rows_c = rows_a;
    cols_c = cols_b;
    size_c = rows_c * cols_c;

    A = (double*)malloc(size_a * sizeof(double));
    B = (double*)malloc(size_b * sizeof(double));
    C = (double*)malloc(size_c * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        printf("Not enough memory for the desired sizes of matrices\n");
        return 0;
    } else {
        printf("Memory allocated successfully\n");
    }

    if (!readMatrixFromFile("matrizA.txt", &A, rows_a, cols_a)) {
        printf("Failed to read matrix A from file.\n");
        free(A);
        free(B);
        free(C);
        return 0;
    } else {
        printf("Matrix A read from file.\n");
    }

    if (!readMatrixFromFile("matrizB.txt", &B, rows_b, cols_b)) {
        printf("Failed to read matrix B from file.\n");
        free(A);
        free(B);
        free(C);
        return 0;
    } else {
        printf("Matrix B read from file.\n");
    }

    multiplyMatrices(A, B, C, rows_a, cols_a, cols_b);

    printf("Matrix multiplication completed.\n");

    FILE* outputFile = fopen("matrizC.txt", "w");
    if (outputFile != NULL) {
        for (uint32_t i = 0; i < rows_c; i++) {
            for (uint32_t j = 0; j < cols_c; j++) {
                fprintf(outputFile, "%.10f\n", C[i * cols_c + j]);
            }
        }
        fclose(outputFile);
        printf("Matrix C written to matrizC.txt.\n");
    } else {
        printf("Failed to open matrizC.txt for writing.\n");
    }

    multiplyMatrices_openmp(A, B, C, rows_a, cols_a, cols_b);

    printf("Matrix openmp multiplication completed.\n");

    if (compareMatrices(C, "matrizC.txt", rows_c, cols_c)) {
        printf("Matrices match.\n");
    } else {
        printf("Matrices do not match.\n");
    }

    free(A);
    free(B);
    free(C);

    return 0;
}
