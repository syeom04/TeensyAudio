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

float findMax(const float* data, int size);
float findMin(const float* data, int size);
void varAndSum(const float* data, int size, float& variance, float& sum);

void transformMultiply(const float* data1, const float* data2, float* result, int size);
void memMove(float* dest, const float* src, int size);
float accumulate(const float* data, int size);
void partialSum(const float* input, float* output, int size);



#endif 