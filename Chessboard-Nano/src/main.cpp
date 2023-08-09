#include <Arduino.h>
#include <Wire.h>

const uint8_t I2C_ADDRESS = 0x55;

const uint8_t rowPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};         // outputs
const uint8_t colPins[8] = {10, 11, 12, A0, A1, A2, A3, A6}; // inputs

#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))
#define bitClear64(value, bit) ((value) &= ~(1ULL << (bit)))
#define bitWrite64(value, bit, bitvalue) (bitvalue ? bitSet64(value, bit) : bitClear64(value, bit))

uint64_t lastBoard = 0;
uint64_t board = 0;

uint8_t registerAddr = 0;

uint64_t scanBoard();
void printBoard();
void onWireReceiveEvent(int count);
void onWireRequestEvent();

// clang-format off
union packed_uint64_t {
  uint64_t number;
  uint8_t bytes[8];
} packedBoard;
// clang-format on

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

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

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(onWireReceiveEvent);
  Wire.onRequest(onWireRequestEvent);
}

void loop() {
  board = scanBoard();
  if (lastBoard != board) {
    lastBoard = board;
    noInterrupts();
    packedBoard.number = board;
    interrupts();
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Board state: ");
    printBoard();
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

void onWireReceiveEvent(int count) {
  registerAddr = Wire.read();
}

void onWireRequestEvent() {
  const uint8_t boardOffset = 0x90;
  if (registerAddr >= boardOffset && registerAddr < boardOffset + 8) {
    digitalWrite(LED_BUILTIN, LOW);
    Wire.write(packedBoard.bytes[registerAddr - boardOffset]);
  } else {
    Wire.write((uint8_t)0);
  }
  registerAddr++;
}
