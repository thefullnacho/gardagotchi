#include "calib_log.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <sys/time.h>

namespace {
constexpr const char* kPath = "/calib.csv";
constexpr const char* kHeader = "boot,uptime_s,unix_time,moisture,soil_temp_c,lux,event";

bool fs_ok = false;
uint32_t boot_id = 0;
bool clock_set = false;  // true after a "time" command this boot
String line;
bool erase_armed = false;

void printHelp() {
  Serial.println(
      "commands:\n"
      "  status          rows logged, flash space\n"
      "  dump            print the whole log as CSV\n"
      "  time <unix>     set the clock (tools/calib_pull.py does this for you)\n"
      "  erase           delete the log (asks you to type erase twice)\n"
      "  help");
}

void status() {
  if (!fs_ok) {
    Serial.println("flash: NOT MOUNTED, logging to serial only");
    return;
  }
  File f = LittleFS.open(kPath, "r");
  size_t rows = 0;
  while (f && f.available()) {
    if (f.read() == '\n') ++rows;
  }
  if (f) f.close();
  Serial.printf("boot %lu, %u rows (incl. header), %u of %u KB used, clock %s\n",
                (unsigned long)boot_id, (unsigned)rows, (unsigned)(LittleFS.usedBytes() / 1024),
                (unsigned)(LittleFS.totalBytes() / 1024), clock_set ? "set" : "not set");
}

void dump() {
  File f = fs_ok ? LittleFS.open(kPath, "r") : File();
  if (!f) {
    Serial.println("no log file");
    return;
  }
  Serial.println("---BEGIN---");
  while (f.available()) Serial.write(f.read());
  Serial.println("---END---");
  f.close();
}

void runCommand(String cmd) {
  cmd.trim();
  if (cmd.isEmpty()) return;
  if (cmd != "erase") erase_armed = false;
  if (cmd == "help") {
    printHelp();
  } else if (cmd == "status") {
    status();
  } else if (cmd == "dump") {
    dump();
  } else if (cmd.startsWith("time ")) {
    long t = cmd.substring(5).toInt();
    if (t > 1700000000) {
      timeval tv{t, 0};
      settimeofday(&tv, nullptr);
      clock_set = true;
      Serial.println("clock set");
    } else {
      Serial.println("usage: time <unix seconds>");
    }
  } else if (cmd == "erase") {
    if (!erase_armed) {
      erase_armed = true;
      Serial.println("type erase again to really delete the log");
    } else {
      erase_armed = false;
      LittleFS.remove(kPath);
      File f = LittleFS.open(kPath, "w");
      if (f) {
        f.println(kHeader);
        f.close();
      }
      Serial.println("log erased");
    }
  } else {
    Serial.println("unknown command");
    printHelp();
  }
}
}  // namespace

namespace calib_log {

void begin() {
  // Count boots, so a power cut shows up as a new boot number in the data.
  Preferences prefs;
  prefs.begin("calib", false);
  boot_id = prefs.getUInt("boot", 0) + 1;
  prefs.putUInt("boot", boot_id);
  prefs.end();

  fs_ok = LittleFS.begin(true);  // true = format on first use
  if (fs_ok && !LittleFS.exists(kPath)) {
    File f = LittleFS.open(kPath, "w");
    if (f) {
      f.println(kHeader);
      f.close();
    }
  }
  Serial.printf("calibration logger: boot %lu, flash %s\n", (unsigned long)boot_id,
                fs_ok ? "ok" : "FAILED");
  Serial.println(kHeader);
}

void append(const sensors::Reading& r, const char* event) {
  char row[96];
  snprintf(row, sizeof(row), "%lu,%lu,%lu,%s,%s,%s,%s", (unsigned long)boot_id,
           (unsigned long)(millis() / 1000), clock_set ? (unsigned long)time(nullptr) : 0UL,
           r.soil_ok ? String(r.moisture).c_str() : "",
           r.soil_ok ? String(r.soil_temp_c, 1).c_str() : "",
           r.light_ok ? String(r.lux, 0).c_str() : "", event);
  Serial.println(row);
  if (!fs_ok) return;
  File f = LittleFS.open(kPath, "a");
  if (f) {
    f.println(row);
    f.close();  // close every time so a power cut loses at most this row
  }
}

void pollSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (!line.isEmpty()) runCommand(line);
      line = "";
    } else if (line.length() < 64) {
      line += c;
    }
  }
}

}  // namespace calib_log
