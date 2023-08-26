#include <Arduino.h>
#include <Wire.h>

const uint8_t I2C_ADDRESS = 0x50;

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

// 90, 180, 270 degree rotation
// Comment out for no rotation
#define ROTATE_BOARD 90
// Comment out to not flip the board
#define FLIP_BOARD

uint64_t board = 0;
uint64_t lastBoard = 0;
bool boardChanged = false;
uint8_t registerAddr = 0;

// clang-format off
union packed_uint64_t {
  uint64_t number;
  uint8_t bytes[8];
} packedBoard;
// clang-format on

uint64_t scanBoard();
#ifdef ROTATE_BOARD
uint64_t rotateBoard90Degrees(uint64_t board);
#endif
void printBoard();
void setupShiftInRegister();
void setupShiftOutRegister();
uint8_t readShiftInRegister();
void writeShiftOutRegister(uint8_t data);
void onWireReceiveEvent(int count);
void onWireRequestEvent();

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing chess board");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  setupShiftInRegister();
  setupShiftOutRegister();

#ifdef ROTATE_BOARD
  Serial.print("Will rotate board by ");
  Serial.print(ROTATE_BOARD);
  Serial.println(" degrees");
#else
  Serial.print("Will rotate board by 0 degrees");
#endif
#ifdef FLIP_BOARD
  Serial.println("Will flip board");
#else
  Serial.println("Will not flip board");
#endif

  Wire.begin(I2C_ADDRESS);
  Serial.print("Joining I2C bus at address 0x");
  Serial.println(I2C_ADDRESS, HEX);
  Wire.onReceive(onWireReceiveEvent);
  Wire.onRequest(onWireRequestEvent);
}

void loop() {
  board = scanBoard();
  if (lastBoard != board) {
    lastBoard = board;
    noInterrupts();
    packedBoard.number = board;
    boardChanged = true;
    interrupts();
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
#if ROTATE_BOARD == 90
  temp = rotateBoard90Degrees(temp);
#elif ROTATE_BOARD == 180
  temp = rotateBoard90Degrees(temp);
  temp = rotateBoard90Degrees(temp);
#elif ROTATE_BOARD == 270
  temp = rotateBoard90Degrees(temp);
  temp = rotateBoard90Degrees(temp);
  temp = rotateBoard90Degrees(temp);
#endif
  return temp;
}

#ifdef ROTATE_BOARD
uint64_t rotateBoard90Degrees(uint64_t board) {
  const uint8_t n = 8;
  // https://stackoverflow.com/a/35137982/10291933
  for (int i = 0; i < n; i += 1) {
    for (int j = i + 1; j < n; j += 1) {
      // swap(data[i][j], data[j][i]);
      const uint8_t ijPos = i * 8 + j;
      const uint8_t jiPos = j * 8 + i;
      const bool ij = bitRead(board, ijPos);
      const bool ji = bitRead(board, jiPos);
      bitWrite64(board, jiPos, ij);
      bitWrite64(board, ijPos, ji);
    }
  }
  for (int i = 0; i < n; i += 1) {
    for (int j = 0; j < n / 2; j += 1) {
      // swap(data[i][j], data[i][n - 1 - j]);
      const uint8_t ijPos = i * 8 + j;
      const uint8_t in1jPos = i * 8 + (n - 1 - j);
      const bool ij = bitRead(board, ijPos);
      const bool in1j = bitRead(board, in1jPos);
      bitWrite64(board, in1jPos, ij);
      bitWrite64(board, ijPos, in1j);
    }
  }
  return board;
}
#endif

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
  uint8_t data = ~shiftIn(shiftInDataInPin, shiftInClockPin,
#ifdef FLIP_BOARD
                          LSBFIRST
#else
                          MSBFIRST
#endif
  );
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

void onWireReceiveEvent(int count) {
  registerAddr = Wire.read();
}

void onWireRequestEvent() {
  const uint8_t boardOffset = 0x90;
  if (registerAddr == boardOffset - 1) {
    Wire.write((uint8_t)boardChanged);
    boardChanged = false;
  } else if (registerAddr >= boardOffset && registerAddr < boardOffset + 8) {
    Wire.write(packedBoard.bytes[registerAddr - boardOffset]);
  } else {
    Wire.write((uint8_t)0);
  }
}
