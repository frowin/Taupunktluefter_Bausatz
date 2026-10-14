#pragma once
// TFA Dostmann 30.3250 (TX141Bv3, 32-bit) and
// TFA Dostmann 30.3249.02 (TX141TH-Bv2, 40-bit, temp+humidity) PWM decoder
// Version 0.7.1 — keep in sync with src/tfa3250.*

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint16_t h;  // HIGH mark µs
  uint16_t l;  // LOW gap µs
} TfaHlPair;

typedef struct {
  bool valid;
  uint8_t id;
  uint8_t channel;    // 1..3
  bool batteryOk;
  float tempC;
  bool hasHumidity;   // true for 40-bit TX141TH frames (TFA-30.3249)
  uint8_t humidity;   // 0..100 %RH, valid only when hasHumidity
  uint8_t repeats;    // identical frames in burst
  uint32_t raw32;
} TfaReading;

// Decode one burst of HIGH/LOW pairs (µs).
// On failure: out->valid == false (treat as null).
bool tfa3250Decode(const TfaHlPair* pairs, uint16_t count, TfaReading* out);

#ifdef __cplusplus
}
#endif
