// 220135N - Dissanayake D.M.A.K.

#include <Wire.h>

#define SLAVE_ADDRESS 0x08
#define DIAGNOSTIC_LED_PIN 11      // Diagnostic LED pin 
#define CONNECTION_TIMEOUT 2000    // Consider connection lost if no message for 2 seconds

const int frequencies[] = {300, 400, 500, 600, 700, 800, 900, 1000};
const int numButtons = 8;
const int speakerPin = 10;

byte buttonStatus = 0;

// Diagnostic variables
unsigned long lastMessageReceived = 0;
bool connectionHealthy = false;

void setup() {
  // Initialize I2C as slave
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  // Initialize speaker pin
  pinMode(speakerPin, OUTPUT);
  
  // Initialize diagnostic LED
  pinMode(DIAGNOSTIC_LED_PIN, OUTPUT);
  digitalWrite(DIAGNOSTIC_LED_PIN, LOW); // Start with LED off
}

void loop() {
  // Monitor connection health
  updateConnectionStatus();
  
  // Other non-blocking tasks could go here
  delay(10);
}

void receiveEvent(int howMany) {
  if (Wire.available() >= 1) {
    // Record time of message receipt
    lastMessageReceived = millis();
    
    // Read the received byte
    byte receivedByte = Wire.read();
    
    // Check if it's a heartbeat or button status
    if (receivedByte == 0xFF) {
      // It's a heartbeat, no need to process buttons
      return;
    } else {
      // It's button status, update and process
      buttonStatus = receivedByte;
      
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
}

void requestEvent() {
  // Send heartbeat response
  Wire.write(0xAA); // Heartbeat acknowledgment code
}

void updateConnectionStatus() {
  unsigned long currentMillis = millis();
  
  // If we haven't received a message in a while, connection is lost
  if (currentMillis - lastMessageReceived > CONNECTION_TIMEOUT) {
    if (connectionHealthy) {
      connectionHealthy = false;
      digitalWrite(DIAGNOSTIC_LED_PIN, LOW); // Turn off LED
    }
  } else {
    if (!connectionHealthy) {
      connectionHealthy = true;
      digitalWrite(DIAGNOSTIC_LED_PIN, HIGH); // Turn on LED
    }
  }
}