// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

// převzato od sakul.cz z free verze firmware pro Stopky v4.1

#define HW_NAME "Sakul Stopky v4.1"

const uint8_t IN1 =  8; // Start
const uint8_t IN2 =  9; // Reset
const uint8_t IN3 = 10; // Levá Dráha
const uint8_t IN4 = 11; // Pravá Dráha

const uint8_t LEDdp = A0;  // Dvojtečka na interním LED displeji
const uint8_t pinDispSerial = 6; // Tx pro interní LED displej

const uint8_t pinTONE = 12; // Reproduktor


inline void initPins() {
  pinMode(LEDdp, OUTPUT);
  pinMode(pinTONE, OUTPUT);

  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  pinMode(IN3, INPUT_PULLUP);
  pinMode(IN4, INPUT_PULLUP);
}
