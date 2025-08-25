// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial LEDDisplay(pinDispSerial);

struct SerialDisplay {
  Print* target;
  uint8_t addr;
  uint8_t offset;
  uint8_t len;
  uint8_t colonPin; // když >0 dvojtečka jde i na digitalWrite
  char fb[16];
  char last[16];
  char layout[16]; // drží aktuálně používaný, ne výchozí

  void write(const char* charbuf) {
    memcpy(fb, charbuf, len);
  }

  void write(const String& text) {   // NOTE: String jde do fb pozpátku
    uint8_t strLen = text.length();  // "5678" -> D0:8, D1:7, D2:6, D3:5
    for (uint8_t i = 0; i<len; i++) {
      fb[i] = text[strLen-1-i];
    }
  }

  void fill(char ch) {
    memset(fb, ch, len);
  }

  // src je formát produkovaný tpToChars(); šablona se v layout
  void applyLayout(const char* src, char colonOverride = 'o') {
    for (uint8_t i=0; i<len; i++) {
      if (layout[len-1-i] == ':') {
        fb[i] = src[((uint8_t)colonOverride   & 0b11111)-1];
      } else {
        fb[i] = src[((uint8_t)layout[len-1-i] & 0b11111)-1];
      }
    }
  }

  void updateDisplay(bool forceAll = false) {
    for (uint8_t i = 0; i < len; i++) {
      if (last[i] != fb[i] || forceAll) {
        sendCharToTarget(fb[i], i);
        last[i] = fb[i];
      }
    }
  }

  private:
    // TODO: zjistit, zda je potřeba zástupný znak (např '#' pro L nebo P)
    void sendCharToTarget(char ch, uint8_t pos) {
      uint8_t sf;
      if (ch >= '0' && ch <= '9') { sf = ch-'0'+1; }
      else if (ch == ':' || ch == '.') { sf = 0xF; } // dvojtečka svítí, resp '-'
      else if (ch == ' ') { sf =  0; }               //         nesvítí, resp ' '
      else if (ch =='\0') { sf =  0; }
      else if (ch == 'L') { sf = 11; }
      else if (ch == 'P') { sf = 12; }
      else if (ch == 'U') { sf = 13; }
      else if (ch == 'E') { sf = 14; }
      else                { sf = 15; } // '-'
    	target->write((sf << 4) + addr + offset + pos);
      if (pos == 0 && colonPin > 0) { digitalWrite(colonPin, (ch == ' ') ? LOW : HIGH ); }
    }
};
SerialDisplay intDisp;
SerialDisplay extDis1;
SerialDisplay extDis2;


char extDis1Layout[16] = {'f','g','i','j','l','m',':'}; // defaultní layout pro ext displeje je "MMSSss:"
char extDis2Layout[16] = {'f','g','i','j','l','m',':'}; // délka 7 je default v initSerialDisplays()

// nastavení pro interní displej je vždy "MMSSss:"
// až si někdo vyndá interní displej ven a udělá ho větší, stačí změnit délku (v init) a layout
const char intDispLayout[7] = {'f','g','i','j','l','m',':'};

// NOTE: ':' je zástupný znak (může být kdekoliv)
//       layouty pro dvojtečky jsou u stavů dráhy, jinými slovy stav dráhy ovlivňuje chování :


void reverse(const char* src, char* dst, uint8_t len);

void initSerialDisplays();
void updateSerialDisplays();
