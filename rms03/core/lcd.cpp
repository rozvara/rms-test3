// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

static char lcdFb[LCD_ROWS][LCD_COLS];

void lcdWrite(uint8_t col, uint8_t row, char ch) {
  if (row < LCD_ROWS && col < LCD_COLS) {
    lcdFb[row][col] = ch;
  }
}


void lcdWrite(uint8_t col, uint8_t row, const char* text) {
  if (!text || row >= LCD_ROWS || col >= LCD_COLS) return;
  for (uint8_t i = 0; text[i] != '\0' && (col + i) < LCD_COLS; i++) {
    lcdFb[row][col + i] = text[i];
  }
}


void lcdWrite(uint8_t col, uint8_t row, const String& text) {
  if (row >= LCD_ROWS || col >= LCD_COLS) return;
  uint8_t len = min((uint8_t)text.length(), (uint8_t)(LCD_COLS - col));
  for (uint8_t i = 0; i < len; i++) {
    lcdFb[row][col + i] = text[i];
  }
}


void lcdClearRow(uint8_t row, char fill = ' ') {
  for (uint8_t col = 0; col < LCD_COLS; col++) {
    lcdFb[row][col] = fill;
  }
}


void lcdClear(char fill = ' ') {
  for (uint8_t row = 0; row < LCD_ROWS; row++) {
    for (uint8_t col = 0; col < LCD_COLS; col++) {
      lcdFb[row][col] = fill;
    }
  }
}


void titleRowToFb(const String& title, char left, char right) {
  for (uint8_t col = 0; col < LCD_COLS/2 ; col++) {
    lcdFb[0][col] = left;
  }
  for (uint8_t col = LCD_COLS/2; col < LCD_COLS ; col++) {
    lcdFb[0][col] = right;
  }
  uint8_t c = (LCD_COLS - title.length()) / 2;
  lcdWrite(c, 0, title);
  lcdFb[0][c-1] = ' ';
  lcdFb[0][c+title.length()] = ' ';
}


void lcdModuleTitle(const String& title) { // výchozí (IDLE) LCD stránka modulu
  titleRowToFb(title, (char)1, (char)2);
}

void lcdStateTitle(const String& title) { // stav modulu
  titleRowToFb(title, (char)3, (char)4);
}

void lcdBusyTitle(const String& title) { // modul "měří"
  titleRowToFb(title, (char)5, (char)6);
}


void updateLCD() {
  static char lcdLast[LCD_ROWS][LCD_COLS];
  char newChar;
  uint8_t row, col;
  for (row = 0; row < LCD_ROWS; row++) {
    for (col = 0; col < LCD_COLS; col++) {
      newChar = lcdFb[row][col];
      if (lcdLast[row][col] != newChar) {
        lcd.setCursor(col, row);
        lcd.write(newChar);
        lcdLast[row][col] = newChar;
      }
    }
  }
}
