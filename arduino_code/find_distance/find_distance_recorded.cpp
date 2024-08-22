// For reading wav file formats
#include <sndfile.h>
#include "RangeFinder.h"
#include "ordinaryFunctions.h"
#include <iostream>

int main() {
    // Load the audio data
    SF_INFO sfinfo;
    SNDFILE *sndfile = sf_open("RECORD.wav", SFM_READ, &sfinfo);






}