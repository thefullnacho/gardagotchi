// Calibration-week logger: one CSV row per minute to serial and to flash.
#pragma once
#include "sensors.h"

namespace calib_log {
void begin();
// Append a row. event is "" for a plain sample, or e.g. "watered".
void append(const sensors::Reading& r, const char* event);
// Handle typed commands from the serial monitor (help, dump, status, time, erase).
void pollSerial();
}  // namespace calib_log
