#include <Wire.h>

#define SLAVE_ADDRESS 0x08

const int frequencies[] = {300, 400, 500, 600, 700, 800, 900, 1000};
const int numButtons = 8;
const int speakerPin = 10;

byte buttonStatus = 0;

void setup() {
  // Initialize I2C as slave
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  
  // Initialize speaker pin
  pinMode(speakerPin, OUTPUT);
}

void loop() {
  // Most work is done in the I2C receive event
  // This helps prevent blocking the communication
  delay(10);
}

void receiveEvent(int howMany) {
  if (Wire.available() >= 1) {
    // Read the button status byte
    buttonStatus = Wire.read();
    
    // Generate tone based on button status
    int sumFrequency = 0;
    for (int i = 0; i < numButtons; i++) {
      // Check if the corresponding bit is set (button is active)
      if (buttonStatus & (1 << i)) {
        sumFrequency += frequencies[i];
      }
    }

    // Output tone
    if (sumFrequency > 0) {
      tone(speakerPin, sumFrequency);
    } else {
      noTone(speakerPin);
    }
  }
}