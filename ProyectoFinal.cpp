#include <iostream>
#include <chrono>
#include <fstream>
#include <cstdint>
#include <omp.h>
#include <pthread.h>
#include <cmath>
#include <iomanip>

uint32_t rows_a, cols_a, rows_b, cols_b, rows_c, cols_c;
uint32_t size_a, size_b, size_c;
double* A = nullptr, * B = nullptr, * C = nullptr;


int validateSize(int rows, int cols, char matrix) {
    FILE* file;
    errno_t err;
    double value;
    int count = 0;

    // Open the file in read mode
    if (matrix == 'A')
        err = fopen_s(&file, "matrizA.txt", "r");
    else
        err = fopen_s(&file, "matrizB.txt", "r");

    if (err != 0) {
        printf("Failed to open the file.\n");
        return 0; // Exit the program with an error
    }
    else
    {
        printf("File opened successfully.\n");
    }

    // Read double values until the end of the file
    while (fscanf_s(file, "%lf", &value) != EOF) {
        count++;
        char nextChar = fgetc(file);
    }

    // Close the file
    fclose(file);

    // Output the count of double values
    // printf("Number of lines: %d\n", count);

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
    scanf_s("%d", &rows_a);
    printf("Enter number of columns for matrix A: ");
    scanf_s("%d", &cols_a);
}

void askForInputsB() {
    printf("Enter number of rows for matrix B: ");
    scanf_s("%d", &rows_b);
    printf("Enter number of columns for matrix B: ");
    scanf_s("%d", &cols_b);
}

uint8_t validateInputs() {
    if (cols_a == rows_b) {
        printf("Valid multiplication\n");
        return 1;
    }
    else {
        printf("Multiplication not valid\n");
        return 0;
    }
}
bool readMatrixFromFile(const std::string& filename, double*& matrix, uint32_t rows, uint32_t cols) {
    std::ifstream inputFile(filename);
    if (inputFile.is_open()) {
        for (uint32_t i = 0; i < rows; i++) {
            for (uint32_t j = 0; j < cols; j++) {
                if (!(inputFile >> matrix[i * cols + j])) {
                    inputFile.close();
                    return false;  // Failed to read a value from the file
                }
            }
        }
        inputFile.close();
        return true;  // Matrix values successfully read from the file
    }
    return false;  // Failed to open the file
}

void multiplyMatrices(double* A, double* B, double* C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b)
{
    // Realiza la multiplicación de matrices y almacena el resultado en C
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            C[i * cols_b + j] = 0;
            for (uint32_t k = 0; k < cols_a; k++) {
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
            //Round Results to 10 decimals
            C[i * cols_b + j] = std::round(C[i * cols_b + j] * 1e10) / 1e10;
        }
    }
}


void multiplyMatrices_openmp(double* A, double* B, double* C, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b)
{
    omp_set_num_threads(omp_get_num_procs());
    // Realiza la multiplicación de matrices y almacena el resultado en C
    #pragma omp parallel for
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            C[i * cols_b + j] = 0;
            #pragma omp simd
            for (uint32_t k = 0; k < cols_a; k++) {
                C[i * cols_b + j] += A[i * cols_a + k] * B[k * cols_b + j];
            }
            //Round Results to 10 decimals
            C[i * cols_b + j] = std::round(C[i * cols_b + j] * 1e10) / 1e10;
        }
    }
}

bool compareMatrices(double* C, const std::string& filename, uint32_t rows, uint32_t cols) {
    std::ifstream inputFile(filename);
    if (inputFile.is_open()) {
        for (uint32_t i = 0; i < rows; i++) {
            for (uint32_t j = 0; j < cols; j++) {
                double value;
                if (!(inputFile >> value)) {
                    inputFile.close();
                    return false;  // Failed to read a value from the file
                }
                // Round the value in matrix C to 10 decimals after the point
                double roundedC = std::round(C[i * cols + j] * 1e10) / 1e10;
                // Round the value from the file to 10 decimals after the point
                double roundedValue = std::round(value * 1e10) / 1e10;
                if (roundedC != roundedValue) {
                    inputFile.close();
                    return false;  // Values don't match
                }
            }
        }
        inputFile.close();
        return true;  // Matrices are identical
    }
    return false;  // Failed to open the file
}


int main()
{
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

    if (A == nullptr || B == nullptr || C == nullptr) {
        printf("Not enough memory for the desired sizes of matrices\n");
        return 0;
    }
    else {
        printf("Memory allocated successfully\n");
    }
    // Read matrix A from file
    if (!readMatrixFromFile("matrizA.txt", A, rows_a, cols_a)) {
        printf("Failed to read matrix A from file.\n");
        free(A);
        free(B);
        free(C);
        return 0;
    }
    else {
        printf("Matrix A read from file.\n");
    }
    // Read matrix B from file
    if (!readMatrixFromFile("matrizB.txt", B, rows_b, cols_b)) {
        printf("Failed to read matrix B from file.\n");
        free(A);
        free(B);
        free(C);
        return 0;
    }
    else {
        printf("Matrix B read from file.\n");
    }

    // Realizar la multiplicación de matrices y medir el tiempo de ejecución
    auto start = std::chrono::high_resolution_clock::now();
    multiplyMatrices(A, B, C, rows_a, cols_a, cols_b);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
   
    printf("Matrix multiplication completed in %lld microseconds.\n", duration);

    // Escribir la matriz resultante en el archivo matrizC.txt
    std::ofstream outputFile("matrizC.txt");
    if (outputFile.is_open()) {
        outputFile << std::fixed;
        outputFile.precision(10);
        for (uint32_t i = 0; i < rows_c; i++) {
            for (uint32_t j = 0; j < cols_c; j++) {
                outputFile << C[i * cols_c + j] << "\n";
            }
        }
        outputFile.close();
        printf("Matrix C written to matrizC.txt.\n");
    }
    else {
        printf("Failed to open matrizC.txt for writing.\n");
    }

     // Realizar la multiplicación de matrices y medir el tiempo de ejecución con openmp
    auto start_openmp = std::chrono::high_resolution_clock::now();
    multiplyMatrices_openmp(A, B, C, rows_a, cols_a, cols_b);
    auto end_openmp = std::chrono::high_resolution_clock::now();
    auto duration_openmp = std::chrono::duration_cast<std::chrono::microseconds>(end_openmp - start_openmp).count();

    printf("Matrix openmp multiplication completed in %lld microseconds.\n", duration_openmp);
    //comparar la matrix
    if (compareMatrices(C, "matrizC.txt", rows_c, cols_c)) {
        printf("Matrices match.\n");
    } else {
        printf("Matrices do not match.\n");
    }
    // Liberar memoria
    free(A);
    free(B);
    free(C);

    return 0;
}
