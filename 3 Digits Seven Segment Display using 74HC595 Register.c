// ---------- CONFIG ----------
const bool COMMON_ANODE = true; // keep this as the same working setting

// Digit control pins (reversed wiring fix)
const int DIGIT3 = 8;   // Rightmost (ones)
const int DIGIT2 = 9;   // Middle (tens)
const int DIGIT1 = 12;  // Leftmost (hundreds)

// Shift register pins (74HC595)
const int DATA_PIN  = 2;
const int LATCH_PIN = 3;
const int CLOCK_PIN = 4;

// Segment patterns for common anode (active LOW)
const byte NUMBERS_CA[10] = {
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000010, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10010000  // 9
};

// Blank patterns from stackoverflow
const byte BLANK_CA = 0xFF;
const byte BLANK_CC = 0x00;

const byte BLANK = (COMMON_ANODE ? BLANK_CA : BLANK_CC);
const int DIGIT_ON_LEVEL  = (COMMON_ANODE ? HIGH : LOW);
const int DIGIT_OFF_LEVEL = (COMMON_ANODE ? LOW  : HIGH);

// ---------- SETUP ----------
void setup() {
  pinMode(DIGIT1, OUTPUT);
  pinMode(DIGIT2, OUTPUT);
  pinMode(DIGIT3, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  allDigitsOff();
  sendToShiftRegister(BLANK);
}

// ---------- MAIN LOOP ----------
void loop() {
  for (int num = 1; num <= 100; num++) {
    showNumber(num);
  }
}

// ---------- FUNCTIONS ----------
void showNumber(int num) {
  int hundreds = num / 100;
  int tens     = (num / 10) % 10;
  int ones     = num % 10;

  unsigned long start = millis();
  while (millis() - start < 1000) { // refresh display for 1s total

    // ones (rightmost)
    showDigit(DIGIT3, getPattern(ones));

    // tens (middle)
    if (hundreds > 0 || tens > 0)
      showDigit(DIGIT2, getPattern(tens));
    else
      blankDigit(DIGIT2);

    // hundreds (leftmost)
    if (hundreds > 0)
      showDigit(DIGIT1, getPattern(hundreds));
    else
      blankDigit(DIGIT1);
  }
}

byte getPattern(int n) {
  byte p = NUMBERS_CA[n];
  if (!COMMON_ANODE) p = ~p;
  return p;
}

void showDigit(int digitPin, byte pattern) {
  allDigitsOff();
  sendToShiftRegister(pattern);
  digitalWrite(digitPin, DIGIT_ON_LEVEL);
  delay(5);
  digitalWrite(digitPin, DIGIT_OFF_LEVEL);
}

void blankDigit(int digitPin) {
  allDigitsOff();
  sendToShiftRegister(BLANK);
  delay(5);
}

void allDigitsOff() {
  digitalWrite(DIGIT1, DIGIT_OFF_LEVEL);
  digitalWrite(DIGIT2, DIGIT_OFF_LEVEL);
  digitalWrite(DIGIT3, DIGIT_OFF_LEVEL);
}

void sendToShiftRegister(byte pattern) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, pattern);
  digitalWrite(LATCH_PIN, HIGH);
}