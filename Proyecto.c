#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
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

void print_table(double serial[5], double openmp[5], double cuda[5]) {
    int rows = 8;
    int columns = 4;
    char* table[8][4] = {
        {"Corrida ", "Serial", "OpenMP", "CUDA"},
        {"1", "", "", ""},
        {"2", "", "", ""},
        {"3", "", "", ""},
        {"4", "", "", ""},
        {"5", "", "", ""},
        {"Promedio", "", "", ""},
        {"% vs Serial", "", "", ""}
    };


    // Print the table header
    printf("| %-12s ", "Corrida ");
    printf("| %-12s ", "Serial");
    printf("| %-12s ", "OpenMP");
    printf("| %-12s ", "CUDA");
    printf("|\n");

    // Print the horizontal line
    for (int j = 0; j < columns; j++) {
        printf("+--------------");
    }
    printf("+\n");

    // Print the table rows
    double promedio_serial = 0;
    double promedio_openmp = 0;
    double promedio_cuda = 0;
    for(int i=0;i<5;i++){
        printf("| %-12d ", i+1);
        printf("| %-12f ", serial[i]);
        printf("| %-12f ", openmp[i]);
        printf("| %-12f ", cuda[i]);
        printf("|\n");
        promedio_serial += serial[i];
        promedio_openmp += openmp[i];
        promedio_cuda += cuda[i];
    }
    
    promedio_serial = promedio_serial/5;
    promedio_openmp = promedio_openmp/5;
    promedio_cuda = promedio_cuda/5;
    //print promedios
    printf("| %-12s ", "Promedio");
    printf("| %-12f ", promedio_serial);
    printf("| %-12f ", promedio_openmp);
    printf("| %-12f ", promedio_cuda);
    printf("|\n");
    //print porcentajes
    printf("| %-12s ", "% vs Serial");
    printf("| %-12f ", 100);
    printf("| %-12f ", promedio_openmp/promedio_serial*100);
    printf("| %-12f ", promedio_cuda/promedio_serial*100);
    printf("|\n");

    // Print the horizontal line
    for (int j = 0; j < columns; j++) {
        printf("+--------------");
    }
    printf("+\n");


}

int main() {
    clock_t start, end;
    double cpu_time_used;
    double serial[5];
    double openmp[5];
    double cuda[5];

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
    for(int i = 0; i < 5; i++){
        start = clock();
        multiplyMatrices(A, B, C, rows_a, cols_a, cols_b);
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        serial[i] = cpu_time_used;        
    }
    
    printf("Serial multiplications completed.\n");

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
    //start openmp
    for(int i = 0; i < 5; i++){
        start = clock();
        multiplyMatrices_openmp(A, B, C, rows_a, cols_a, cols_b);
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        openmp[i] = cpu_time_used;        
    }
    printf("Openmp multiplications completed.\n");

    if (compareMatrices(C, "matrizC.txt", rows_c, cols_c)) {
        printf("Matrices match.\n");
    } else {
        printf("Matrices do not match.\n");
    }
    //start cuda HERE
    for(int i = 0; i < 5; i++){
        start = clock();
        //cambiar por cuda
        multiplyMatrices_openmp(A, B, C, rows_a, cols_a, cols_b);
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        cuda[i] = cpu_time_used;        
    }
    printf("Cuda multiplications completed.\n");

    if (compareMatrices(C, "matrizC.txt", rows_c, cols_c)) {
        printf("Matrices match.\n");
    } else {
        printf("Matrices do not match.\n");
    }


    print_table(serial, openmp, cuda);
    free(A);
    free(B);
    free(C);

    return 0;
}
