// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

// převzato od sakul.cz z free verze firmware pro Stopky v5.0

#define HW_NAME "Sakul Stopky v5.0"

const uint8_t IN1 = 27; // Start
const uint8_t IN2 = 28; // Reset
const uint8_t IN3 = 29; // Levá Dráha
const uint8_t IN4 = 30; // Pravá Dráha
const uint8_t IN5 = 31; // Mezičas

#define HW_IN5

const uint8_t LEDdp = 22; // Dvojtečka na interním LED displeji
const uint8_t pinDispSerial = 1; // Tx pro interní LED displej

const uint8_t pinTONE = 12; // Reproduktor

//const uint8_t OUT1 = 13; // Výkonové výstupy
//const uint8_t OUT2 = 14;
const uint8_t OUT3 = 15;
const uint8_t OUT4 = 18;
//const uint8_t OUT5 = 19;

#define HW_OUT3
#define HW_OUT4

// TODO
// const uint8_t Napajeni_ADC = A0;  // Měření napájecího napětí
// const uint8_t Periferie_ADC = A1; // Měření napětí periferií

// float Delic_U = 56.83; // Konstanta pro přepočet měřeného napětí (1023ADC/5V).
// float NapajeniU;  // Hodnota napájecího napětí
// float PeriferieU; // Hodnota napětí periferií


inline void initPins() {
  pinMode(LEDdp, OUTPUT);
  pinMode(pinTONE, OUTPUT);

  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  pinMode(IN3, INPUT_PULLUP);
  pinMode(IN4, INPUT_PULLUP);
  pinMode(IN5, INPUT_PULLUP);

  // pinMode(OUT1, OUTPUT);  // Zatím bez funkce
  // pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  // pinMode(OUT5, OUTPUT);
}
