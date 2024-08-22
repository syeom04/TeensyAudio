#include <Arduino.h>
#include <RangeFinder.h>

rangeFinder myRangeFinder;

#define BUFFER_SIZE 10
uint8_t dataBuffer[BUFFER_SIZE];

int distanceChange = 0;

void setup() {
    Serial.begin(9600);
    myRangeFinder.init();
}

void loop() {
    // Get data from the sensor and store it in buffer
    bool dataReceived = myRangeFinder.GetRecData(dataBuffer, BUFFER_SIZE);
    if (dataReceived) {
        distanceChange = myRangeFinder.GetDistanceChange(dataBuffer, BUFFER_SIZE);
        // Output the distance change
        Serial.print("Distance Change: ");
        Serial.println(distanceChange);
    }
    else {
        Serial.println("No data received.");
    }
    delay(100);
}


