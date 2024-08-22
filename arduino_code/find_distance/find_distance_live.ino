#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

// Be careful whether to use "" (locally defined in the same directory) or <> (system wide) when defining libraries
#include "libraries/RangeFinder.h"

#define MAX_FRAMES_PER_SLICE 1024
#define NUM_FREQS 8
#define START_FREQ 20.0f
#define FREQ_INTERVAL 5.0f
#define BUFFER_SIZE 10

RangeFinder myRangeFinder(MAX_FRAMES_PER_SLICE, NUM_FREQS, START_FREQ, FREQ_INTERVAL);
uint8_t dataBuffer[BUFFER_SIZE];

const int SD_CS_PIN = 10;

File audioFile;

void setup() {
    Serial.begin(9600);

    // Initialize the SD card
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Failed to check SD card!");
        while (1);
    }

    audioFile = SD.open("RECORD.RAW");
    if (!audioFile) {
        Serial.println("RECORD.RAW file cannot be found!");
        while (1);
    }
}

void loop() {
    if (audioFile.available()) {
        byte buffer[512];
        int bytesRead = audioFile.read(buffer, sizeof(buffer));
        if (bytesRead > 0) {
            float data[BUFFER_SIZE];
            for (int i = 0; i < BUFFER_SIZE; i++) {
                data[i] = (float)buffer[i];
            }
            myRangeFinder.processData(data, BUFFER_SIZE);
            int distanceChange = myRangeFinder.getDistanceChange();

            Serial.print("Distance Change: ");
            Serial.println(distanceChange);
        }
    }
    else {
        Serial.println("End of file or no more data left.");
        audioFile.close();
        while(1);
    }
    delay(100);
}
