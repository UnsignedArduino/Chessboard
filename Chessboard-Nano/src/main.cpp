#include <Arduino.h>
#include <Wire.h>

const uint8_t shiftInLoadInPin = 8;
const uint8_t shiftInClockEnablePin = 7;
const uint8_t shiftInClockPin = 6;
const uint8_t shiftInDataInPin = 5;

void setupShiftInRegister();
uint8_t readShiftInRegister();

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  setupShiftInRegister();
}

void loop() {
  uint8_t data = readShiftInRegister();
  Serial.print("Pin stats: ");
  Serial.println(data, BIN);
  delay(250);
}

void setupShiftInRegister() {
  pinMode(shiftInLoadInPin, OUTPUT);
  pinMode(shiftInClockEnablePin, OUTPUT);
  pinMode(shiftInClockPin, OUTPUT);
  pinMode(shiftInDataInPin, INPUT);

  digitalWrite(shiftInLoadInPin, HIGH);
  digitalWrite(shiftInClockEnablePin, HIGH);
  digitalWrite(shiftInClockPin, LOW);
}

uint8_t readShiftInRegister() {
  digitalWrite(shiftInLoadInPin, LOW);
  delayMicroseconds(5);
  digitalWrite(shiftInLoadInPin, HIGH);
  delayMicroseconds(5);

  digitalWrite(shiftInClockPin, HIGH);
  digitalWrite(shiftInClockEnablePin, LOW);
  uint8_t data = ~shiftIn(shiftInDataInPin, shiftInClockPin, LSBFIRST);
  digitalWrite(shiftInClockEnablePin, HIGH);

  return data;
}
