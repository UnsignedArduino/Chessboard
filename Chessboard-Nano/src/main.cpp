#include <Arduino.h>
#include <Wire.h>

const uint8_t shiftInLoadInPin = 8;
const uint8_t shiftInClockEnablePin = 7;
const uint8_t shiftInClockPin = 6;
const uint8_t shiftInDataInPin = 5;

const uint8_t shiftOutLatchPin = 4;
const uint8_t shiftOutClockPin = 3;
const uint8_t shiftOutDataPin = 2;

void setupShiftInRegister();
void setupShiftOutRegister();

uint8_t readShiftInRegister();
void writeShiftOutRegister(uint8_t data);

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  setupShiftInRegister();
  setupShiftOutRegister();
}

void loop() {
  const uint8_t data = readShiftInRegister();
  writeShiftOutRegister(data);
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

void setupShiftOutRegister() {
  pinMode(shiftOutLatchPin, OUTPUT);
  pinMode(shiftOutClockPin, OUTPUT);
  pinMode(shiftOutDataPin, OUTPUT);

  digitalWrite(shiftOutLatchPin, HIGH);
  digitalWrite(shiftOutClockPin, LOW);
  digitalWrite(shiftOutDataPin, LOW);
}

void writeShiftOutRegister(uint8_t data) {
  digitalWrite(shiftOutLatchPin, LOW);
  shiftOut(shiftOutDataPin, shiftOutClockPin, LSBFIRST, data);
  digitalWrite(shiftOutLatchPin, HIGH);
}
