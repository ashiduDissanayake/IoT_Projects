#include <Wire.h>

#define SLAVE_ADDRESS 0x08

const int buttonPins[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int numButtons = 8;

// 7-segment display configuration
const int segmentPins[] = {11, 12, 13, A0, A1, A2, A3}; // a, b, c, d, e, f, g
const byte segmentCodes[] = {
  0x06, // 1 (b, c)
  0x5B, // 2 (a, b, g, e, d)
  0x4F, // 3 (a, b, g, c, d)
  0x66, // 4 (f, g, b, c)
  0x6D, // 5 (a, f, g, c, d)
  0x7D, // 6 (a, f, g, c, d, e)
  0x07, // 7 (a, b, c)
  0x7F  // 8 (all segments)
};

struct ButtonState {
  bool lastState;
  bool active;
  unsigned long releaseTime;
};

ButtonState buttons[numButtons];
int activeKeys[8];
int activeKeyCount = 0;
unsigned long lastDisplayChange = 0;
int currentDisplayIndex = 0;
byte buttonStatus = 0; // For sending button states to Board 2

void setup() {
  // Initialize I2C as master
  Wire.begin();
  
  // Initialize buttons
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttons[i].lastState = digitalRead(buttonPins[i]);
    buttons[i].active = false;
    buttons[i].releaseTime = 0;
  }

  // Initialize 7-segment display pins
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  buttonStatus = 0; // Reset button status byte

  // Read button states
  for (int i = 0; i < numButtons; i++) {
    int currentState = digitalRead(buttonPins[i]);

    if (currentState == LOW && buttons[i].lastState == HIGH) {
      buttons[i].active = true;
      buttons[i].releaseTime = 0;
    } else if (currentState == HIGH && buttons[i].lastState == LOW) {
      buttons[i].releaseTime = currentMillis;
    }

    if (currentState == LOW) {
      buttons[i].active = true;
    } else {
      if (buttons[i].releaseTime != 0 && (currentMillis - buttons[i].releaseTime) <= 20) {
        buttons[i].active = true;
      } else {
        buttons[i].active = false;
      }
    }

    buttons[i].lastState = currentState;
    
    // Set corresponding bit in buttonStatus if button is active
    if (buttons[i].active) {
      buttonStatus |= (1 << i);
    }
  }

  // Send button status to Board 2 via I2C
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(buttonStatus);
  Wire.endTransmission();
  
  // Collect active keys for display
  activeKeyCount = 0;
  for (int i = 0; i < numButtons; i++) {
    if (buttons[i].active) {
      activeKeys[activeKeyCount++] = i + 1; // Store key number (1-8)
    }
  }

  // Update 7-segment display
  if (activeKeyCount == 0) {
    clearDisplay();
  } else if (activeKeyCount == 1) {
    displayDigit(activeKeys[0]);
  } else {
    if (currentMillis - lastDisplayChange >= 500) {
      lastDisplayChange = currentMillis;
      currentDisplayIndex = (currentDisplayIndex + 1) % activeKeyCount;
    }
    displayDigit(activeKeys[currentDisplayIndex]);
  }
  
  // Small delay to avoid flooding the I2C bus
  delay(10);
}

void displayDigit(int number) {
  if (number < 1 || number > 8) {
    clearDisplay();
    return;
  }
  byte code = segmentCodes[number - 1];
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], bitRead(code, i));
  }
}

void clearDisplay() {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], LOW);
  }
}