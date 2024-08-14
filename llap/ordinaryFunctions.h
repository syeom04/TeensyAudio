#ifndef ORDINARYFUNCTIONS_H
#define ORDINARYFUNCTIONS_H

#include <Arduino.h>

struct Complex {
    float real;
    float imag;
};

// Function declarations
void calcSquareMag(Complex* input, float* output, int size);
void calculateTotal(float* input, int size, float* result);
void calculatePhases(Complex* input, float* output, int size);
void addScalarToVector(float* input, float scalar, float* output, int size);
void divideVectorByScalar(float* input, float scalar, float* output, int size);
void multiplyVectors(float* input1, float* input2, float* output, int size);
void sumVectorElements(float* input, float* result, int size);
void subtractVectors(float* vec1, float* vec2, float* result, int size);
void squareVector(float* input, float* output, int size);

#endif 