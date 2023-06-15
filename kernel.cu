#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

uint32_t rows_a, cols_a, rows_b, cols_b, rows_c, cols_c;
uint32_t size_a, size_b, size_c;
double* A = NULL;
double* B = NULL;
double* C = NULL;

int validateSize(uint32_t rows, uint32_t cols, char matrix) {
    FILE* file;
    double value;
    int count = 0;

    if (matrix == 'A')
        file = fopen("matrizA.txt", "r");
    else
        file = fopen("matrizB.txt", "r");

    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 0;
    }

    while (fscanf(file, "%lf", &value) != EOF) {
        count++;
        char nextChar = fgetc(file);
    }

    fclose(file);

    if (count == rows * cols) {
        return count;
    }
    else {
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
        // printf("Valid multiplication\n");
        return 1;
    }
    else {
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

void resetMatrix(double* C, uint32_t rows, uint32_t cols) {
    for (uint32_t i = 0; i < rows; i++) {
        for (uint32_t j = 0; j < cols; j++) {
            C[i * cols + j] = 0;
        }
    }
}

void multiplyMatrices(double* A, double* B, double* C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b) {
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            C[i * cols_b + j] = 0;
            for (uint32_t k = 0; k < cols_a; k++) {
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
        }
    }
}

void multiplyMatrices_openmp(double* A, double* B, double* C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b) {
    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for collapse(2)
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            C[i * cols_b + j] = 0;
            for (uint32_t k = 0; k < cols_a; k++) {
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
        }
    }
}

__global__ void multiplyMatrices_cuda_kernel(double* d_A, double* d_B, double* d_C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b)
{
    // Multiply matrices A and B and store the result in C
    uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
    uint32_t j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i < rows_a && j < cols_b) {
        d_C[i * cols_b + j] = 0;
        for (uint32_t k = 0; k < cols_a; k++) {
            d_C[i * cols_b + j] += d_A[i * cols_a + k] * d_B[k * cols_b + j];
        }
    }
}

void multiplyMatrices_cuda(double* h_C, double* d_A, double* d_B, double* d_C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b)
{
    // Launch kernel to multiply A and B
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((rows_a + threadsPerBlock.x - 1) / threadsPerBlock.x, (cols_b + threadsPerBlock.y - 1) / threadsPerBlock.y);
    multiplyMatrices_cuda_kernel << <numBlocks, threadsPerBlock >> > (d_A, d_B, d_C, rows_a, cols_a, cols_b);
    // Sync
	cudaDeviceSynchronize();
    cudaMemcpy(h_C, d_C, size_c * sizeof(double), cudaMemcpyDeviceToHost);
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
                if (abs((C[i * cols + j] - value)) > 1e-10) {
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
    for (int i = 0;i < 5;i++) {
        printf("| %-12d ", i + 1);
        printf("| %-12f ", serial[i]);
        printf("| %-12f ", openmp[i]);
        printf("| %-12f ", cuda[i]);
        printf("|\n");
        promedio_serial += serial[i];
        promedio_openmp += openmp[i];
        promedio_cuda += cuda[i];
    }

    promedio_serial = promedio_serial / 5;
    promedio_openmp = promedio_openmp / 5;
    promedio_cuda = promedio_cuda / 5;
    //print promedios
    printf("| %-12s ", "Promedio");
    printf("| %-12f ", promedio_serial);
    printf("| %-12f ", promedio_openmp);
    printf("| %-12f ", promedio_cuda);
    printf("|\n");
    //print porcentajes
    printf("| %-12s ", "% vs Serial");
    printf("| %-12d ", 100);
    printf("| %-12f ", promedio_openmp / promedio_serial * 100);
    printf("| %-12f ", promedio_cuda / promedio_serial * 100);
    printf("|\n");

    // Print the horizontal line
    for (int j = 0; j < columns; j++) {
        printf("+--------------");
    }
    printf("+\n");

    // Check for fastest method
    if(promedio_serial < promedio_openmp && promedio_serial < promedio_cuda) {
        printf("El método más rápido para realizar la multiplicación de matrices es: SERIAL\n");
    } else if(promedio_openmp < promedio_serial && promedio_openmp < promedio_cuda) {
        printf("El método más rápido para realizar la multiplicación de matrices es: OPENMP\n");
    } else {
        printf("El método más rápido para realizar la multiplicación de matrices es: CUDA\n");
    }
    
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

    // Validate there is enough resources for CUDA with the desired size of matrices
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    if( (rows_a > prop.maxThreadsDim[0]) || (cols_b > prop.maxThreadsDim[1])) {
        printf("Error: The size of the matrices is too big for CUDA using the current configuration\n");
        return 0;
    }

    rows_c = rows_a;
    cols_c = cols_b;
    size_c = rows_c * cols_c;

    A = (double*)malloc(size_a * sizeof(double));
    B = (double*)malloc(size_b * sizeof(double));
    C = (double*)malloc(size_c * sizeof(double));

    // cudaMallocManaged(&A, size_a * sizeof(double));
    // cudaMallocManaged(&B, size_b * sizeof(double));
    // cudaMallocManaged(&C, size_c * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        printf("Not enough memory for the desired sizes of matrices\n");
        return 0;
    }

    if (!readMatrixFromFile("matrizA.txt", &A, rows_a, cols_a)) {
        printf("Failed to read matrix A from file.\n");
        free(A);
        free(B);
        free(C);
        return 0;
    }

    if (!readMatrixFromFile("matrizB.txt", &B, rows_b, cols_b)) {
        printf("Failed to read matrix B from file.\n");
        free(A);
        free(B);
        free(C);
        return 0;
    }

    // Start serial multiplication
    for (int i = 0; i < 5; i++) {
        start = clock();
        multiplyMatrices(A, B, C, rows_a, cols_a, cols_b);
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        serial[i] = cpu_time_used;
    }

    // printf("Serial multiplications completed.\n");

    // Write matrix C to file
    FILE* outputFile = fopen("matrizC.txt", "w");
    if (outputFile != NULL) {
        for (uint32_t i = 0; i < rows_c; i++) {
            for (uint32_t j = 0; j < cols_c; j++) {
                fprintf(outputFile, "%.10f\n", C[i * cols_c + j]);
            }
        }
        fclose(outputFile);
        // printf("Matrix C written to matrizC.txt.\n");
    }
    else {
        printf("Failed to open matrizC.txt for writing.\n");
    }

    // Reset matrix C before executing openmp
    resetMatrix(C, rows_c, cols_c);

    // Start openmp
    for (int i = 0; i < 5; i++) {
        start = clock();
        multiplyMatrices_openmp(A, B, C, rows_a, cols_a, cols_b);
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        openmp[i] = cpu_time_used;
    }
    // printf("Openmp multiplications completed.\n");

    if (compareMatrices(C, "matrizC.txt", rows_c, cols_c)) {
        printf("OpenMP matches with serial method.\n");
    }
    else {
        printf("OpenMP does not match with serial method.\n");
    }

    // Reset matrix C before executing cuda
    resetMatrix(C, rows_c, cols_c);

    // Start cuda
    double* d_A = NULL;
    double* d_B = NULL;
    double* d_C = NULL;

    // Memory allocation for device copies of A, B, C
    cudaMalloc((void**)&d_A, size_a * sizeof(double));
    cudaMalloc((void**)&d_B, size_b * sizeof(double));
    cudaMalloc((void**)&d_C, size_c * sizeof(double));

    // Copy inputs to device
    cudaMemcpy(d_A, A, size_a * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B, size_b * sizeof(double), cudaMemcpyHostToDevice);

    for (int i = 0; i < 5; i++) {
        start = clock();
        multiplyMatrices_cuda(C, d_A, d_B, d_C, rows_a, cols_a, cols_b);
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        cuda[i] = cpu_time_used;
    }
    // printf("Cuda multiplications completed.\n");

    if (compareMatrices(C, "matrizC.txt", rows_c, cols_c)) {
        printf("CUDA matches with serial method.\n");
    }
    else {
        printf("CUDA does not match with serial method.\n");
    }


    print_table(serial, openmp, cuda);
    // Free memory
    free(A);
    free(B);
    free(C);

    // Free device memory
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}