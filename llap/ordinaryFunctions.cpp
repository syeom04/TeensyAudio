#include "ordinaryFunctions.h"
#include <cmath> // for atan2, fabs, M_PI

// Define a structure for complex numbers with the real and imag components
struct Complex {
    float real;
    float imag;
};

void calcSquareMag(Complex* input, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input[i].real * input[i].real + input[i].imag * input[i].imag;
    }
}

void calculateTotal(float* input, int size, float* result) {
    *result = 0.0;
    for (int i = 0; i < size; i++) {
        *result += input[i];
    }
}

void calculatePhases(Complex* input, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = atan2(input[i].imag, input[i].real);
    }
}

void addScalarToVector(float* input, float scalar, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input[i] + scalar;
    }
}

void divideVectorByScalar(float* input, float scalar, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input[i] / scalar;
    }
}

void multiplyVectors(float* input1, float* input2, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input1[i] * input2[i];
    }
}

void sumVectorElements(float* input, float* result, int size) {
    *result = 0.0;
    for (int i = 0; i < size; i++) {
        *result += input[i];
    }
}

void subtractVectors(float* vec1, float* vec2, float* result, int size) {
    for (int i = 0; i < size; i++) {
        result[i] = vec1[i] - vec2[i];
    }
}

void squareVector(float* input, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input[i] * input[i];
    }
}