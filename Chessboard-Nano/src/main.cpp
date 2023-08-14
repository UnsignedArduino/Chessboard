#include <Arduino.h>
#include <Wire.h>

// Shift in register
const uint8_t shiftInLoadInPin = 8;      // SH/~LD (1)
const uint8_t shiftInClockEnablePin = 7; // CLK INH (15)
const uint8_t shiftInClockPin = 6;       // CLK (2)
const uint8_t shiftInDataInPin = 5;      // ~Q_H (8)
// Q_H (9) and SER (10) are unconnected

// Shift out register
const uint8_t shiftOutLatchPin = 4; // RCLK (12)
const uint8_t shiftOutClockPin = 3; // SRCLK (11)
const uint8_t shiftOutDataPin = 2;  // SER (14)
// ~SRCLR (10) to 5v
// ~OE (13) to GND
// Q_H' is unconnected

#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))
#define bitClear64(value, bit) ((value) &= ~(1ULL << (bit)))
#define bitWrite64(value, bit, bitvalue) (bitvalue ? bitSet64(value, bit) : bitClear64(value, bit))

uint64_t board = 0;
uint64_t lastBoard = 0;

uint64_t scanBoard();
void printBoard();
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
  board = scanBoard();
  if (lastBoard != board) {
    lastBoard = board;
    Serial.println("Board state:");
    printBoard();
  }
}

uint64_t scanBoard() {
  uint64_t temp = 0;
  for (uint8_t row = 0; row < 8; row++) {
    writeShiftOutRegister(1 << row);
    uint64_t colsDown = readShiftInRegister();
    colsDown <<= row * 8;
    temp |= colsDown;
  }
  writeShiftOutRegister(0);
  return temp;
}

void printBoard() {
  for (uint8_t i = 0; i < 64; i++) {
    Serial.print(bitRead(board, i) ? 1 : 0);
    if ((i + 1) % 8 == 0) {
      Serial.println();
    }
  }
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
  uint8_t data = ~shiftIn(shiftInDataInPin, shiftInClockPin, MSBFIRST);
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
  shiftOut(shiftOutDataPin, shiftOutClockPin, MSBFIRST, data);
  digitalWrite(shiftOutLatchPin, HIGH);
}
