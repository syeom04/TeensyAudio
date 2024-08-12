/*
Use freq bins in FFT to provide divisions of the different freq in
the wave spectrum
Using around 25 freq bins, each representing the amplitude at that region
of freq

Created by Seungyun YEOM
WIT LAB @ KAIST

Adapted from https://forum.pjrc.com/index.php?threads/teensyduino-1-52-beta-3.60599/page-3#post-238070

For changing the sampling rate: https://forum.pjrc.com/index.php?threads/change-sample-rate-for-teensy-4-vs-teensy-3.57283/
*/

#include <Audio.h>
#include <utility/imxrt_hw.h>

// Function to set I2S Frequency
void setI2SFreq(int freq) {
  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  int n1 = 4; // SAI prescaler 4 => (n1*n2) = multiple of 4
  int n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  double C = ((double)freq * 256 * n1 * n2) / 24000000;
  int c0 = C;
  int c2 = 10000;
  int c1 = C * c2 - (c0 * c2);
  set_audioClock(c0, c1, c2, true);
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
       | CCM_CS1CDR_SAI1_CLK_PRED(n1 - 1) // &0x07
       | CCM_CS1CDR_SAI1_CLK_PODF(n2 - 1); // &0x3f 
  // Serial.printf("SetI2SFreq(%d)\n", freq);
}

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
    // Set I2S frequency to 192000 Hz
    setI2SFreq(192000);
    // Initialize Serial for debugging
    Serial.begin(9600);
}

void loop() {
    // Check for availability of new FFT data
    if (fft1024_1.available()) {
        Serial.print("FFT signal now: ");
        // Print higher frequency bins (12000 Hz and above)
        int start_bin = 64; // Corresponding to approximately 12000 Hz
        int end_bin = 128;  // You can adjust this value to print more bins if needed
        for (int i = start_bin; i < end_bin; i++) {
            float n = fft1024_1.read(i);
            if (n >= 0.001) {
                Serial.print(n, 4);
                Serial.print(" ");
            } else {
                Serial.print("-");
            }
        }
        Serial.println();
    }
}
