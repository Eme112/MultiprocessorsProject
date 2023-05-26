// ProyectoFinal.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Question: What happens if the size of the file is bigger than the input
// (e.g. declaring a 3x3 matrix when the file contains 15 elements).
// 
// TODO: Review maximum size of memmory allignment.
// TODO: Calculate result using sequential algorithm.

#include <iostream>
#include <stdio.h>

// Matrix global var
uint32_t rows_a, cols_a, rows_b, cols_b, rows_c, cols_c;
double *A = NULL, *B= NULL, *C = NULL;

uint8_t validateSize(int rows, int cols, char matrix) {
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
    } else {
        printf("Invalid size for matrix %c\n", matrix);
        return 0;
    }
}

void validateInputs() {
    // Ask for inputs
    printf("Enter number of rows for matrix A: ");
    scanf_s("%d", &rows_a);
    printf("Enter number of columns for matrix A: ");
    scanf_s("%d", &cols_a);
    printf("Enter number of rows for matrix B: ");
    scanf_s("%d", &rows_b);
    printf("Enter number of columns for matrix B: ");
    scanf_s("%d", &cols_b);
    // Validate inputs
    if (validateSize(rows_a, cols_a, 'A') && validateSize(rows_b, cols_b, 'B')
        && cols_a == rows_b)
        printf("Valid multiplication\n");
    else
        printf("Multiplication not valid\n");
}

int main()
{
    validateInputs();

    return 0;
}