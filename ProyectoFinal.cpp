// ProyectoFinal.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Question: What happens if the size of the file is bigger than the input --> it has to be the same size
// (e.g. declaring a 3x3 matrix when the file contains 15 elements).
// 
// TODO: Review maximum size of memmory allignment.
// TODO: Calculate result using sequential algorithm.

#include <iostream>
#include <stdio.h>

// Secuential global variables
uint32_t rows_a, cols_a, rows_b, cols_b, rows_c, cols_c;
uint32_t size_a, size_b, size_c;
double* A = NULL, * B = NULL, * C = NULL;

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
    if (cols_a == rows_b) {                     // Validate multiplication
        printf("Valid multiplication\n");
        return 1;
    }
    else {
        printf("Multiplication not valid\n");
        return 0;
    }
}

int main()
{
    // Ask for inputs andn validate sizes
    askForInputsA();
    size_a = validateSize(rows_a, cols_a, 'A');
    if (size_a == 0)        // If dimensions for matrix A are invalid
        return 0;
    askForInputsB();
    size_b = validateSize(rows_b, cols_b, 'B');
    if (size_b == 0)        // If dimensions for matrix B are invalid
        return 0;
    if (!validateInputs())  // If multiplication is not valid
        return 0;

    // Validate memmory given and store values of matrix A and B
    A = (double*)malloc(size_a);
    B = (double*)malloc(size_b);
    if (A == NULL || B == NULL) {
        printf("Not enough memmory for the desired sizes of matrices\n");
        return 0;
    }


    return 0;
}