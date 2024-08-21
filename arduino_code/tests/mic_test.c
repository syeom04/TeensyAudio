/*
Use freq bins in FFT to provide divisions of the different freq in
the wave spectrum
Using around 25 freq bins, each representing the amplitude at that region
of freq

Created by Seungyun YEOM
WIT LAB @ KAIST

Adapted from https://forum.pjrc.com/index.php?threads/teensyduino-1-52-beta-3.60599/page-3#post-238070
*/

#include <Audio.h>


// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=180,111
AudioFilterStateVariable filter1;        //xy=325,101
AudioAmplifier           amp1;           //xy=470,93
AudioAnalyzeFFT1024      fft1024_1;      //xy=616,102
AudioConnection          patchCord1(i2s1, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 2, amp1, 0);
AudioConnection          patchCord3(amp1, fft1024_1);
// GUItool: end automatically generated code

void setup() {
    // Allocate some memory
    AudioMemory(50);
    // Filter out the DC and very low freq
    filter1.frequency(30);
    // Amplify signal appropriately to useful range
    amp1.gain(8.5);
}

void loop() {
    // Check for availability of new FFT data
    if (fft1024_1.available()) {
        Serial.print("FFT signal now: ");
        for (int i = 0; i < 40; i++) {
            float n = fft1024_1.read(i);
            if (n >= 0.001) {
                Serial.print(n, 4);
                Serial.print(" ");
            }
            else {
                Serial.print("-");
            }
        }
    Serial.println();
    }
}