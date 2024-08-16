// https://github.com/PaulStoffregen/Audio/blob/master/examples/Recorder/Recorder.ino


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdRaw           playRaw1;     //xy=264,355
AudioSynthWaveformSine   sine1;          //xy=273,465
AudioSynthKarplusStrong  string1;        //xy=144,430
AudioRecordQueue         queue1;         //xy=425,489
AudioRecordQueue         queue2; 
AudioOutputI2S           i2s1;           //xy=451,351
AudioConnection          patchCord3(string1, queue1);
AudioConnection          patchCord4(sine1, queue2);
// GUItool: end automatically generated code

AudioControlSGTL5000     sgtl5000_1;     //xy=265,212

const int myInput = AUDIO_INPUT_MIC;

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11  // not actually used for teensy 4.1
#define SDCARD_SCK_PIN   13  // not actually used for teensy 4.1

// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing

unsigned long Timer;

// The file where data is recorded
File frec;

void setup() {
  // record queue uses this memory to buffer incoming audio.
  AudioMemory(120); // 60

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

   // set sine osc 
  sine1.amplitude(1.0);
  sine1.frequency(261);

 // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  Timer = millis();
  startRecording();
}


void loop() {
  if((millis()-Timer)>5000 && mode == 1) {
    stopRecording();
  }
  else {
    if (mode == 1) continueRecording();
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
  string1.noteOn(440, 1.0);
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
    for (int i=0; i<512; i+=4) {
      buffer[i] = bufferL[i];
      buffer[i+1] = bufferL[i+1];
      buffer[i+2] = bufferR[i];
      buffer[i+3] = bufferR[i+1];
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