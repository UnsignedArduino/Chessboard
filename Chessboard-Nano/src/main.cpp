#include <Arduino.h>
#include <Wire.h>

// #define FAKE_BOARD

const uint8_t I2C_ADDRESS = 0x50;

const uint8_t CD74HC4067_COM_S0 = 2;
const uint8_t CD74HC4067_COM_S1 = 3;
const uint8_t CD74HC4067_COM_S2 = 4;
const uint8_t CD74HC4067_COM_S3 = 5;
const uint8_t CD74HC4067_0_EN = 6;
const uint8_t CD74HC4067_1_EN = 7;
const uint8_t CD74HC4067_2_EN = 8;
const uint8_t CD74HC4067_3_EN = 9;
const uint8_t CD74HC4067_COM_SIG = A7;

// 90, 180, 270 degree rotation
// Comment out for no rotation
// #define ROTATE_BOARD
// Comment out to not flip the board
// #define FLIP_BOARD

// #define DEBUG_LINEAR_HALLS

const uint8_t MUX_EN[] = {CD74HC4067_0_EN, CD74HC4067_1_EN, CD74HC4067_2_EN, CD74HC4067_3_EN};
const uint8_t MUX_SIG[] = {CD74HC4067_COM_S0, CD74HC4067_COM_S1, CD74HC4067_COM_S2, CD74HC4067_COM_S3};

#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))
#define bitClear64(value, bit) ((value) &= ~(1ULL << (bit)))
#define bitWrite64(value, bit, bitvalue) (bitvalue ? bitSet64(value, bit) : bitClear64(value, bit))

const uint8_t debounceTime = 50;

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

void setupLinearHalls();
void readLinearHalls(int16_t result[64]);
void printLinearHalls(int16_t halls[64]);
uint64_t scanBoard();
void printBoard(uint64_t b);
#ifdef ROTATE_BOARD
uint64_t rotateBoard90Degrees(uint64_t board);
#endif
void onWireReceiveEvent(int count);
void onWireRequestEvent();

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing chess board");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  setupLinearHalls();

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
#ifdef FAKE_BOARD
  Serial.println("Faking board enabled, send file then rank to toggle bit.");
#endif

  Wire.begin(I2C_ADDRESS);
  Serial.print("Joining I2C bus at address 0x");
  Serial.println(I2C_ADDRESS, HEX);
  Wire.onReceive(onWireReceiveEvent);
  Wire.onRequest(onWireRequestEvent);

  printBoard(board);
}

void loop() {
#ifdef FAKE_BOARD
  if (Serial.available() >= 2) {
    const uint8_t file = Serial.peek() >= 'a' && Serial.peek() <= 'h' ? Serial.read() - 'a' : Serial.read() - 'A';
    const uint8_t rank = Serial.parseInt();
    Serial.print("Toggling ");
    Serial.write(file + 'A');
    Serial.println(rank);
    const uint8_t bitPos = (8 - rank) * 8 + file;
    bitWrite64(board, bitPos, !bitRead(board, bitPos));
  }
#else
  static uint32_t lastDebounce = 0;
  static uint64_t lastBouncingBoard = 0;

  const uint64_t bouncingBoard = scanBoard();

  if (bouncingBoard != lastBouncingBoard) {
    lastDebounce = millis();
    lastBouncingBoard = bouncingBoard;
  }

  if (millis() - lastDebounce > debounceTime) {
    board = lastBouncingBoard;
  }
#endif

  if (board != lastBoard) {
    lastBoard = board;
    noInterrupts();
    packedBoard.number = board;
    boardChanged = true;
    interrupts();
    Serial.println("Board state:");
    printBoard(board);
  }
}

void setupLinearHalls() {
  Serial.println("Setting up linear hall sensors and multiplexers");
  for (const uint8_t pin : MUX_EN) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
  for (const uint8_t pin : MUX_SIG) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  pinMode(CD74HC4067_COM_SIG, INPUT);
}

void readLinearHalls(int16_t result[64]) {
  for (uint8_t mux = 0; mux < 4; mux++) {
    digitalWrite(MUX_EN[mux], LOW);
    for (uint8_t channel = 0; channel < 16; channel++) {
      for (uint8_t control = 0; control < 4; control++) {
        digitalWrite(MUX_SIG[control], bitRead(channel, control));
      }
      result[mux * 16 + channel] = analogRead(CD74HC4067_COM_SIG);
    }
    digitalWrite(MUX_EN[mux], HIGH);
  }
}

void printLinearHalls(int16_t halls[64]) {
  Serial.print("   ");
  for (uint8_t chan = 0; chan < 16; chan++) {
    char buf[6];
    snprintf(buf, 6, "%4d ", chan);
    Serial.print(buf);
  }
  Serial.println();
  for (uint8_t mux = 0; mux < 4; mux++) {
    Serial.print(mux);
    Serial.print(": ");
    for (uint8_t chan = 0; chan < 16; chan++) {
      char buf[6];
      snprintf(buf, 6, "%4d ", halls[mux * 16 + chan]);
      Serial.print(buf);
    }
    Serial.println();
  }
}

uint64_t scanBoard() {
  int16_t linearHalls[64] = {};
  readLinearHalls(linearHalls);

#ifdef DEBUG_LINEAR_HALLS
  Serial.println("Linear halls: ");
  printLinearHalls(linearHalls);
#endif

  uint64_t temp = 0;

  for (uint8_t i = 0; i < 64; i++) {
    const uint16_t deviation = abs(512 - linearHalls[i]);
    const bool piecePresent = deviation > 10;
    bitWrite64(temp,
#ifdef FLIP_BOARD
               63 - i,
#else
               i,
#endif
               piecePresent);
  }

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

#ifdef DEBUG_LINEAR_HALLS
  Serial.println("Board state: ");
  printBoard(temp);
  delay(1000);
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

void printBoard(uint64_t b) {
  for (uint8_t i = 0; i < 64; i++) {
    if (i % 8 == 0) {
      Serial.print((64 - i) / 8);
      Serial.print(' ');
    }
    Serial.print(bitRead(b, i) ? '#' : '.');
    Serial.print(' ');
    if ((i + 1) % 8 == 0) {
      Serial.println();
    }
  }
  Serial.print("  ");
  for (char file = 'A'; file <= 'H'; file++) {
    Serial.print(file);
    Serial.print(' ');
  }
  Serial.println();
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
