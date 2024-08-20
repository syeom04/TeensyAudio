#include "AudioController.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    const int maxFramesPerSlice = 1024;
    const int numFreq = 16;
    const float starTfreq = 30.0;
    const float freqInterval = 2.0;

    AudioController audioController(maxFramesPerSlice, numFreq, starTfreq, freqInterval);

    // Seed the random number generator with current time
    srand(static_cast<unsigned int>(time(0)));
    // Produce random data
    float dummyData[maxFramesPerSlice];
    for (int i = 0; i < maxFramesPerSlice; i++) {
        // Produce random value between 0.0 and 1.0
        dummyData[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    // Insert data into data buffer
    audioController.GetRecDataBuffer(dummyData, maxFramesPerSlice);
    audioController.printDistanceChange();

    return 0;
}