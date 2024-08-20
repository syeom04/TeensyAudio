#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <utility/imxrt_hw.h>

// Audio Objects
AudioInputI2S            i2s1;           // I2S input for audio
AudioFilterStateVariable filter1;        // Audio filter
AudioAmplifier           amp1;           // Audio amplifier
AudioAnalyzeFFT1024      fft1024_1;      // FFT analysis
AudioRecordQueue         queue1;         // Audio recording queue
AudioRecordQueue         queue2;         // Audio recording queue
AudioOutputI2S           i2s_output;     // Audio output
AudioConnection          patchCord1(i2s1, 0, filter1, 0);
AudioConnection          patchCord2(filter1, 2, amp1, 0);
AudioConnection          patchCord3(amp1, fft1024_1);
AudioConnection          patchCord4(i2s1, 0, queue1, 0);
AudioConnection          patchCord5(i2s1, 1, queue2, 0);

AudioControlSGTL5000     sgtl5000_1;     // Audio shield control

const int myInput = AUDIO_INPUT_MIC;
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11  // not actually used for Teensy 4.1
#define SDCARD_SCK_PIN   13  // not actually used for Teensy 4.1

// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing

unsigned long Timer;
File frec;

void setup() {
  AudioMemory(120); // Allocate memory for audio processing

  filter1.frequency(30);
  amp1.gain(8.5);
  Serial.begin(9600);

  // Enable the audio shield and configure the input/output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

  // Set I2S frequency
  setI2SFreq(48000);
  
  // Initialize SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!SD.begin(SDCARD_CS_PIN)) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  Timer = millis();
  startRecording();
}

void loop() {
  if ((millis() - Timer) > 10000 && mode == 1) {
    stopRecording();
  } else {
    if (mode == 1) continueRecording();
  }

  // Check for availability of new FFT data
  if (fft1024_1.available()) {
    Serial.print("FFT signal now: ");
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

void startRecording() {
  Serial.println("StartRecording");
  if (SD.exists("RECORD.RAW")) {
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if (frec) {
    Serial.println("File Open");
    queue1.begin();
    queue2.begin();
    mode = 1;
  }
}

void continueRecording() {
  if (queue1.available() >= 2 && queue2.available() >= 2) {
    byte buffer[512];
    byte bufferL[256];
    byte bufferR[256];
    memcpy(bufferL, queue1.readBuffer(), 256);
    memcpy(bufferR, queue2.readBuffer(), 256);
    queue1.freeBuffer();
    queue2.freeBuffer();
    for (int i = 0; i < 512; i += 4) {
      buffer[i] = bufferL[i];
      buffer[i + 1] = bufferL[i + 1];
      buffer[i + 2] = bufferR[i];
      buffer[i + 3] = bufferR[i + 1];
    }
    frec.write(buffer, 256);
  }
}

void stopRecording() {
  Serial.println("StopRecording");
  queue1.end();
  queue2.end();
  // flush buffer
  while (queue1.available() > 0 && queue2.available() > 0) {
    queue1.readBuffer();
    queue1.freeBuffer();
    queue2.readBuffer();
    queue2.freeBuffer();
  }
  frec.close(); // close file
  mode = 0;
}

void setI2SFreq(int freq) {
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
}
