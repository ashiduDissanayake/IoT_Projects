// 220135N - Dissanayake D.M.A.K.

const int frequencies[] = {300, 400, 500, 600, 700, 800, 900, 1000};
const int numButtons = 8;
const int speakerPin = 10;

byte buttonStatus = 0;

void setup() {
  // Initialize Serial for UART communication with Board 1
  Serial.begin(9600);
  
  // Initialize speaker pin
  pinMode(speakerPin, OUTPUT);
}

void loop() {
  // Check if data is available from Board 1
  if (Serial.available() > 0) {
    // Read the button status byte
    buttonStatus = Serial.read();
    
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