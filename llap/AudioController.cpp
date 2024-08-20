#include "AudioController.h"
#include <iostream>

AudioController::AudioController(int maxFramesPerSlice, int numFreq, float startFreq, float freqInterval) {
    rangeFinder = new RangeFinder(maxFramesPerSlice, numFreq, startFreq, freqInterval);
}

void AudioController::GetRecDataBuffer(float* data, int size) {
    rangeFinder->processData(data, size);
}

void AudioController::printDistanceChange() {
    float distanceChange = rangeFinder->getDistanceChange();
    std::cout << "Distance Change: " << distanceChange << std::endl;
}


