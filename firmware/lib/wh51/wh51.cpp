#include "wh51.h"

namespace wh51 {

uint8_t crc8(const uint8_t* data, size_t len) {
  uint8_t crc = 0;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31) : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

uint8_t checksum(const uint8_t* data, size_t len) {
  uint8_t sum = 0;
  for (size_t i = 0; i < len; ++i) sum = static_cast<uint8_t>(sum + data[i]);
  return sum;
}

Status decode(const uint8_t* b, size_t len, Reading* out) {
  if (!b || len < kFrameLen) return Status::TooShort;
  if (b[0] != kFamily) return Status::BadFamily;
  if (crc8(b, 7) != b[7]) return Status::BadCrc;
  if (checksum(b, 8) != b[8]) return Status::BadChecksum;
  if (b[5] > 100) return Status::ImplausibleMoisture;

  if (out) {
    out->id = (static_cast<uint32_t>(b[1]) << 16) | (static_cast<uint32_t>(b[2]) << 8) | b[3];
    out->boost = b[4] >> 5;
    out->battery_mv = static_cast<uint16_t>((b[4] & 0x1F) * 100);
    out->moisture_pct = b[5];
  }
  return Status::Ok;
}

const char* name(Status s) {
  switch (s) {
    case Status::Ok: return "ok";
    case Status::TooShort: return "too_short";
    case Status::BadFamily: return "bad_family";
    case Status::BadCrc: return "bad_crc";
    case Status::BadChecksum: return "bad_checksum";
    case Status::ImplausibleMoisture: return "implausible_moisture";
  }
  return "unknown";
}

}  // namespace wh51
