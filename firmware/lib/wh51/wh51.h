// Decodes one WH51 soil-moisture packet (915 MHz FSK) into a reading.
// Plain C++ (no Arduino, no radio), tested with `pio test -e native`.
//
// Input is the packet body the CC1101 hands over AFTER it matches the sync word, so
// preamble and sync are the radio's job. Written from the protocol facts, not ported
// from rtl_433 (GPL-2.0; decided 2026-10-04), so hestia can reuse this file as is.
//
// UNVERIFIED AGAINST A REAL PACKET. The layout below is what the protocol is believed to
// be. Until a capture from the spare WH51 passes `test_real_capture` (see
// test/test_wh51), treat every field offset as a hypothesis. The layout lives in
// wh51.cpp's decode() and nowhere else, so fixing it is a one-place change.
//
// Frame, 9 bytes:
//   [0] family 0x51    [1..3] sensor id, big-endian   [4] boost (top 3 bits) | battery (low 5)
//   [5] moisture %     [6] reserved / AD low bits     [7] CRC-8 of bytes 0..6
//   [8] sum of bytes 0..7, mod 256
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace wh51 {

constexpr uint8_t kFamily = 0x51;
constexpr size_t kFrameLen = 9;

struct Reading {
  uint32_t id = 0;           // 24-bit sensor id; a pot's sensor and the bed sensors differ
  uint8_t moisture_pct = 0;  // 0 to 100
  uint16_t battery_mv = 0;   // 100 mV steps
  uint8_t boost = 0;         // the sensor's 3-bit boost field, kept raw
};

enum class Status : uint8_t {
  Ok,
  TooShort,
  BadFamily,
  BadCrc,
  BadChecksum,
  ImplausibleMoisture,  // passed both checks but reads over 100%: refuse rather than guess
};

// CRC-8, polynomial 0x31, initial value 0, no reflection, over `len` bytes.
uint8_t crc8(const uint8_t* data, size_t len);

// Sum of `len` bytes, mod 256.
uint8_t checksum(const uint8_t* data, size_t len);

// Decodes `len` bytes (at least kFrameLen; trailing bytes are ignored). `out` is only
// written when the result is Status::Ok.
Status decode(const uint8_t* data, size_t len, Reading* out);

const char* name(Status s);

}  // namespace wh51
