// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <avr/pgmspace.h>

#ifdef LCD_I2C0X20
  LiquidCrystal_I2C lcd(0x20, 4, 5, 6, 0, 1, 2, 3, 7, NEGATIVE);
#endif

#ifdef LCD_I2C0X27
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
#endif

#ifdef LCD_I2C0X3F         //En Rw Rs D4 D5 D6 D7
  LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
#endif

#ifdef LCD_16X2
  const uint8_t LCD_COLS = 16;
  const uint8_t LCD_ROWS = 2;
#endif

#ifdef LCD_20X4
  const uint8_t LCD_COLS = 20;
  const uint8_t LCD_ROWS = 4;
#endif

// NOTE:
//   jádro překresluje obrazovku z framebufferu
//   v modulu nepoužívej lcd.clear() nebo lcd.print() - nefunguje to
void lcdWrite(uint8_t col, uint8_t row, char c);
void lcdWrite(uint8_t col, uint8_t row, const char* text);
void lcdWrite(uint8_t col, uint8_t row, const String& text);
void lcdClearRow(uint8_t row, char fill = ' ');
void lcdClear(char fill = ' '); // lze smazat i jiným znakem

void lcdModuleTitle(const String& title);
void lcdStateTitle(const String& title);

// --------------

void updateLCD();

const uint8_t customChars[][8] PROGMEM = {
  {0x12, 0x09, 0x12, 0x09, 0x12, 0x09, 0x12, 0x00},
  {0x09, 0x12, 0x09, 0x12, 0x09, 0x12, 0x09, 0x00},
  {0x00, 0x00, 0x12, 0x09, 0x12, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x09, 0x12, 0x09, 0x00, 0x00, 0x00},

  {0x00, 0x00, 0x1E, 0x0F, 0x1E, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x0F, 0x1E, 0x0F, 0x00, 0x00, 0x00},

  {0x18, 0x1C, 0x1E, 0x1F, 0x1E, 0x1C, 0x18, 0x00}
};


inline void initLCD() {
  Wire.begin();
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.noCursor();
  lcd.backlight();

  uint8_t buf[8];
  for (uint8_t i = 1; i < 1+7; i++) {
    memcpy_P(buf, customChars[i-1], 8);
    lcd.createChar(i, buf);
  }
}
