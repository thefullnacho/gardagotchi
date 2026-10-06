// Desktop tests for the WH51 decoder:  cd firmware && pio test -e native
//
// The frames here are SYNTHETIC: build() makes a frame from the layout in wh51.h, so these
// tests prove the decoder is self-consistent and rejects damage, not that the layout
// matches a real sensor. test_real_capture is the gate for that and stays ignored until
// a packet off the spare WH51 is pasted in.
#include <unity.h>

#include <string.h>

#include "wh51.h"

namespace {

// Builds a valid 9-byte frame for the given fields.
void build(uint8_t out[wh51::kFrameLen], uint32_t id, uint8_t boost, uint8_t battery_steps,
           uint8_t moisture) {
  out[0] = wh51::kFamily;
  out[1] = static_cast<uint8_t>(id >> 16);
  out[2] = static_cast<uint8_t>(id >> 8);
  out[3] = static_cast<uint8_t>(id);
  out[4] = static_cast<uint8_t>((boost << 5) | (battery_steps & 0x1F));
  out[5] = moisture;
  out[6] = 0;
  out[7] = wh51::crc8(out, 7);
  out[8] = wh51::checksum(out, 8);
}

void test_crc8_known_values() {
  TEST_ASSERT_EQUAL_UINT8(0x00, wh51::crc8(nullptr, 0));
  const uint8_t one[] = {0x01};
  TEST_ASSERT_EQUAL_UINT8(0x31, wh51::crc8(one, 1));  // one byte shifted through the polynomial
  const uint8_t zeros[] = {0, 0, 0, 0};
  TEST_ASSERT_EQUAL_UINT8(0x00, wh51::crc8(zeros, 4));  // init 0, so zeros stay zero
}

void test_crc8_over_data_plus_its_crc_is_zero() {
  const uint8_t data[] = {0x51, 0x12, 0x34, 0x56, 0x0F, 0x2A, 0x00};
  uint8_t with_crc[8];
  memcpy(with_crc, data, 7);
  with_crc[7] = wh51::crc8(data, 7);
  TEST_ASSERT_EQUAL_UINT8(0x00, wh51::crc8(with_crc, 8));
}

void test_checksum_wraps_mod_256() {
  const uint8_t data[] = {0xFF, 0x02, 0x01};
  TEST_ASSERT_EQUAL_UINT8(0x02, wh51::checksum(data, 3));
}

void test_decodes_a_valid_frame() {
  uint8_t f[wh51::kFrameLen];
  build(f, 0xABCDEF, 3, 14, 42);
  wh51::Reading r;
  TEST_ASSERT_EQUAL(wh51::Status::Ok, wh51::decode(f, sizeof f, &r));
  TEST_ASSERT_EQUAL_UINT32(0xABCDEF, r.id);
  TEST_ASSERT_EQUAL_UINT8(42, r.moisture_pct);
  TEST_ASSERT_EQUAL_UINT16(1400, r.battery_mv);
  TEST_ASSERT_EQUAL_UINT8(3, r.boost);
}

void test_moisture_edges_zero_and_hundred() {
  uint8_t f[wh51::kFrameLen];
  wh51::Reading r;
  build(f, 1, 0, 15, 0);
  TEST_ASSERT_EQUAL(wh51::Status::Ok, wh51::decode(f, sizeof f, &r));
  TEST_ASSERT_EQUAL_UINT8(0, r.moisture_pct);
  build(f, 1, 0, 15, 100);
  TEST_ASSERT_EQUAL(wh51::Status::Ok, wh51::decode(f, sizeof f, &r));
  TEST_ASSERT_EQUAL_UINT8(100, r.moisture_pct);
}

void test_over_hundred_percent_is_refused_even_with_good_checks() {
  uint8_t f[wh51::kFrameLen];
  build(f, 1, 0, 15, 101);
  wh51::Reading r;
  r.moisture_pct = 77;
  TEST_ASSERT_EQUAL(wh51::Status::ImplausibleMoisture, wh51::decode(f, sizeof f, &r));
  TEST_ASSERT_EQUAL_UINT8(77, r.moisture_pct);  // out untouched on failure
}

void test_too_short_and_null_are_rejected() {
  uint8_t f[wh51::kFrameLen];
  build(f, 1, 0, 15, 50);
  TEST_ASSERT_EQUAL(wh51::Status::TooShort, wh51::decode(f, wh51::kFrameLen - 1, nullptr));
  TEST_ASSERT_EQUAL(wh51::Status::TooShort, wh51::decode(nullptr, 20, nullptr));
}

void test_wrong_family_is_rejected() {
  uint8_t f[wh51::kFrameLen];
  build(f, 1, 0, 15, 50);
  f[0] = 0x57;
  f[7] = wh51::crc8(f, 7);  // even with the checks repaired
  f[8] = wh51::checksum(f, 8);
  TEST_ASSERT_EQUAL(wh51::Status::BadFamily, wh51::decode(f, sizeof f, nullptr));
}

void test_every_single_bit_flip_is_caught() {
  uint8_t good[wh51::kFrameLen];
  build(good, 0x123456, 2, 12, 63);
  for (size_t byte = 0; byte < wh51::kFrameLen; ++byte) {
    for (int bit = 0; bit < 8; ++bit) {
      uint8_t f[wh51::kFrameLen];
      memcpy(f, good, sizeof f);
      f[byte] ^= static_cast<uint8_t>(1 << bit);
      wh51::Status s = wh51::decode(f, sizeof f, nullptr);
      TEST_ASSERT_NOT_EQUAL_MESSAGE(wh51::Status::Ok, s, "a flipped bit decoded as good");
    }
  }
}

void test_trailing_bytes_are_ignored() {
  uint8_t f[wh51::kFrameLen + 3];
  build(f, 0x00BEEF, 1, 10, 55);
  f[9] = 0xDE;
  f[10] = 0xAD;
  f[11] = 0xBE;
  wh51::Reading r;
  TEST_ASSERT_EQUAL(wh51::Status::Ok, wh51::decode(f, sizeof f, &r));
  TEST_ASSERT_EQUAL_UINT32(0x00BEEF, r.id);
}

void test_null_out_still_validates() {
  uint8_t f[wh51::kFrameLen];
  build(f, 1, 0, 15, 50);
  TEST_ASSERT_EQUAL(wh51::Status::Ok, wh51::decode(f, sizeof f, nullptr));
}

void test_status_names_are_distinct_and_nonempty() {
  const wh51::Status all[] = {wh51::Status::Ok,          wh51::Status::TooShort,
                              wh51::Status::BadFamily,   wh51::Status::BadCrc,
                              wh51::Status::BadChecksum, wh51::Status::ImplausibleMoisture};
  for (size_t i = 0; i < sizeof all / sizeof all[0]; ++i) {
    TEST_ASSERT_TRUE(strlen(wh51::name(all[i])) > 0);
    for (size_t j = i + 1; j < sizeof all / sizeof all[0]; ++j) {
      TEST_ASSERT_NOT_EQUAL(0, strcmp(wh51::name(all[i]), wh51::name(all[j])));
    }
  }
}

// The one test that says the layout is right. Paste 9 bytes from the radio's first
// "radio: WH51 found" line plus the moisture % the sensor's own app showed at the time.
void test_real_capture() {
  TEST_IGNORE_MESSAGE("no real packet yet: layout in wh51.h is unverified");
}

}  // namespace

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_crc8_known_values);
  RUN_TEST(test_crc8_over_data_plus_its_crc_is_zero);
  RUN_TEST(test_checksum_wraps_mod_256);
  RUN_TEST(test_decodes_a_valid_frame);
  RUN_TEST(test_moisture_edges_zero_and_hundred);
  RUN_TEST(test_over_hundred_percent_is_refused_even_with_good_checks);
  RUN_TEST(test_too_short_and_null_are_rejected);
  RUN_TEST(test_wrong_family_is_rejected);
  RUN_TEST(test_every_single_bit_flip_is_caught);
  RUN_TEST(test_trailing_bytes_are_ignored);
  RUN_TEST(test_null_out_still_validates);
  RUN_TEST(test_status_names_are_distinct_and_nonempty);
  RUN_TEST(test_real_capture);
  return UNITY_END();
}
