// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

void setup();
void loop();

uint8_t switchMode = 0;
bool switchModuleNow = false;
void switchModule();

void checkResetLongPress();
void loopWhileResetHeld();

void systemEcho(const String& msg, bool lf = true);
void moduleEcho(const String& msg, bool lf = true);

