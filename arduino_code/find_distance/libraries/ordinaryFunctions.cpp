#include "ordinaryFunctions.h"
#include <cmath> // for atan2, fabs, M_PI
#include <algorithm>
#include <limits>
#include <cstring>

void calcSquareMag(Complex* input, float* output, int size) {
    for (int i = 0; i < size; i++) {
        output[i] = input->real[i] * input->real[i] + input->imag[i] * input->imag[i];
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
        output[i] = atan2(input->imag[i], input->real[i]);
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

float findMax(const float* data, int size) {
    float max_val = data[0];
    for (int i = 1; i < size; ++i) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    return max_val;
}

float findMin(const float* data, int size) {
    float min_val = data[0];
    for (int i = 1; i < size; ++i) {
        if (data[i] < min_val) {
            min_val = data[i];
        }
    }
    return min_val;
}

void varAndSum(const float* data, int size, float& variance, float& sum) {
    float tempdata[4096], tempdata2[4096];
    float temp_val = -data[0];

    for (int i = 0; i < size; ++i) {
        tempdata[i] = data[i] + temp_val;
    }

    float temp_sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        temp_sum += tempdata[i];
    }
    sum = fabs(temp_sum / size);

    for (int i = 0; i < size; ++i) {
        tempdata2[i] = tempdata[i] * tempdata[i];
    }

    float temp_variance = 0.0f;
    for (int i = 0; i < size; ++i) {
        temp_variance += tempdata2[i];
    }
    variance = fabs(temp_variance / size);
}

void transformMultiply(const float* data1, const float* data2, float* result, int size) {
    for (int i = 0; i < size; ++i) {
        result[i] = data1[i] * data2[i];
    }
}

void memMove(float* dest, const float* src, int size) {
    memmove(dest, src, size * sizeof(float));
}

float accumulate(const float* data, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum;
}

void partialSum(const float* input, float* output, int size, int length) {
    for (int i = 0; i < size; i++) {
        output[i] = 0;
        for (int j = 0; j < length; j++) {
            output[i] += input[i + j];
        }
    }
}


