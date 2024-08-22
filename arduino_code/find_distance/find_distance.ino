#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <utility/imxrt_hw.h>

// Define the audio objects to use
AudioInputI2S   i2s1;
AudioFilterStateVariable    filter1;
AudioAmplifier  amp1;
AudioRecordQueue    queue1;
AudioConnection patchCord1(i2s1, 0, filter1, 0);
AudioConnection patchCord2(filter1, 2, amp1, 0);
AudioConnection patchCord3(i2s1, 0, queue1, 0);

AudioControlSGTL5000    sgtl5000_1;

const int myInput = AUDIO_INPUT_LINEIN;
// const int myInput = AUDIO_INPUT_MIC;
#define SDCARD_CS_PIN   10
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN  13

unsigned long Timer;
File frec;
bool recordingFinished = false;

void setup() {
    // Reduced memory size for mono audio recording
    AudioMemory(120);
    
    filter1.frequency(30);
    amp1.gain(8.5);
    Serial.begin(9600);

    // Now enable the audio shield
    sgtl5000_1.enable();
    sgtl5000_1.inputSelect(myInput);
    sgtl5000_1.volume(0.5);

    // setI2SFreq(48000);

    // Now initialize the SD card
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    if (!SD.begin(SDCARD_CS_PIN)) {
        while (1) {
            Serial.println("Unable to access SD card!");
            delay(500);
        }
    }
    Timer = millis();
    startRecording();
}

void loop() {
    if (!recordingFinished) {
        if ((millis() - Timer) > 10000) {
            stopRecording();
        }
        else {
            Serial.println("Recording in progress .");
            Serial.println("Recording in progress ..");
            Serial.println("Recording in progress ...");
            continueRecording();
            delay(1000);
        }
    }
}

void startRecording() {
    // SETUP
    Serial.println("Start recording...");
    if (SD.exists("RECORD.RAW")) {
        SD.remove("RECORD.RAW");
    }
    frec = SD.open("RECORD.RAW", FILE_WRITE);
    if (frec) {
        queue1.begin();
    }
}

void continueRecording() {
    if (queue1.available() >= 2) {
        byte buffer[512];

        // Read the first 256 bytes from queue1 into buffer
        memcpy(buffer, queue1.readBuffer(), 256);
        queue1.freeBuffer();

        // Read the next 256 bytes from queue1 into buffer
        memcpy(buffer+256, queue1.readBuffer(), 256);
        queue1.freeBuffer();

        elapsedMicros usec = 0;
        frec.write(buffer, 512);

        // Check the SD write time
        Serial.print("SD write, us=");
        Serial.println(usec);

        // Output the raw buffer data to the Serial Monitor
        Serial.print("Buffer Data: ");
        for (int i = 0; i < 512; i++) {
            Serial.print(buffer[i]);
            Serial.print(" ");
            if ((i + 1) % 16 == 0) {
                Serial.println();
            }
        }
    }
}

void stopRecording() {
    if (!recordingFinished) {
        Serial.println("Finished recording!");
        queue1.end();

        while (queue1.available() > 0) {
            frec.write((byte*)queue1.readBuffer(), 256);
            queue1.freeBuffer();
        }
        frec.close();
        recordingFinished = true;
    }
}

void setI2SFreq(int freq) {
    int n1 = 4;
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

