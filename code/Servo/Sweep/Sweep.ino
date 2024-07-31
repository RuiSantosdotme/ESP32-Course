/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/
*********/

// Install the ESP32Servo Library: https://www.arduino.cc/reference/en/libraries/esp32servo/
// Sketch > Include Library > Manage Libraries... > ESP32Servo by Kevin Harrington
#include <ESP32Servo.h>

static const int servoPin = 13;

Servo servo1;

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);
}

void loop() {
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo1.write(posDegrees);
    Serial.println(posDegrees);
    delay(20);
  }

  for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    Serial.println(posDegrees);
    delay(20);
  }
}
