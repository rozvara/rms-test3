// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

enum LogEventType : uint8_t {
    LOG_NONE = 0,
    LOG_START,
    LOG_SPLIT, // mezičas
    LOG_FINISH_LEFT,
    LOG_FINISH_RIGHT,
    LOG_FINISH,
    LOG_RESET
};

struct LogEntry {
    uint32_t timestamp_ms;
    LogEventType type;
};

bool logEvent(LogEventType type, uint32_t now_ms);
void logDump(Stream& out);
void logClear();
