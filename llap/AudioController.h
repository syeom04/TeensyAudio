#ifndef AUDIO_CONTROLLER_H
#define AUDIO_CONTROLLER_H

#include "RangeFinder.h"

class AudioController {
public: 
    AudioController(int maxFramesPerSlice, int numFreq, float startFreq, float freqInterval);
    void GetRecDataBuffer(float* data, int size);
    void printDistanceChange();

private:
    RangeFinder* rangeFinder;
};

#endif
