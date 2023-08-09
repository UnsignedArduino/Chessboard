#include <Arduino.h>
// #include <Wire.h>

// const uint8_t I2C_ADDRESS = 0x88;

const uint8_t rowPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};         // outputs
const uint8_t colPins[8] = {10, 11, 12, A0, A1, A2, A3, A6}; // inputs

#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))
#define bitClear64(value, bit) ((value) &= ~(1ULL << (bit)))
#define bitWrite64(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

uint64_t lastBoard = 0;
uint64_t board = 0;

uint64_t scanBoard();
void printBoard();

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  for (uint8_t row : rowPins) {
    pinMode(row, OUTPUT);
    digitalWrite(row, HIGH);
  }
  for (uint8_t col : colPins) {
    if (col == A6) {
      pinMode(col, INPUT); // pullup resistor in circuit
    } else {
      pinMode(col, INPUT_PULLUP);
    }
  }

  // Wire.begin(I2C_ADDRESS);
  // Wire.onRequest(onWireRequestEvent);
}

void loop() {
  noInterrupts();
  board = scanBoard();
  interrupts();
  if (lastBoard != board) {
    lastBoard = board;
    Serial.println("Board state: ");
    printBoard();
    Serial.println();
  }
}

uint64_t scanBoard() {
  uint64_t tempBoard = 0;
  for (uint8_t rowIndex = 0; rowIndex < 8; rowIndex++) {
    digitalWrite(rowPins[rowIndex], LOW);
    for (uint8_t colIndex = 0; colIndex < 8; colIndex++) {
      if (colPins[colIndex] == A6 ? analogRead(A6) < 512 : (digitalRead(colPins[colIndex]) == LOW)) {
        bitSet64(tempBoard, rowIndex * 8 + colIndex);
      }
    }
    digitalWrite(rowPins[rowIndex], HIGH);
  }
  return tempBoard;
}

void printBoard() {
  for (uint8_t i = 0; i < 64; i++) {
    Serial.print(bitRead(board, i) == true ? 1 : 0);
    if ((i + 1) % 8 == 0) {
      Serial.println();
    }
  }
}

// void onWireRequestEvent() {
//   uint8_t boardBytes[8];
//   uint64ToBytes(boardBytes, board);
//   Wire.write(boardBytes, 8);
// }

// void uint64ToBytes(uint8_t* bytes, uint64_t& x) {
//   x = (uint64_t(bytes[0]) << 8 * 0) | (uint64_t(bytes[1]) << 8 * 1) | (uint64_t(bytes[2]) << 8 * 2) |
//       (uint64_t(bytes[3]) << 8 * 3) | (uint64_t(bytes[4]) << 8 * 4) | (uint64_t(bytes[5]) << 8 * 5) |
//       (uint64_t(bytes[6]) << 8 * 6) | (uint64_t(bytes[7]) << 8 * 7);
// }
