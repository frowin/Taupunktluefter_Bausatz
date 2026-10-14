// TFA Dostmann 30.3250 (TX141Bv3, 32-bit) and
// TFA Dostmann 30.3249.02 (TX141TH-Bv2, 40-bit, temp+humidity) PWM decoder
// Version 0.7.1 — keep in sync with src/tfa3250.*

#include "tfa_3250_decode.h"

static bool inRange(uint16_t v, uint16_t lo, uint16_t hi) {
  return v >= lo && v <= hi;
}

static bool isSync(uint16_t h, uint16_t l) {
  return (inRange(h, 850, 1400) && inRange(l, 600, 1100)) ||
         (inRange(h, 800, 1200) && inRange(l, 800, 1200));
}

static int8_t bitFromPwm(uint16_t h, uint16_t l) {
  if (inRange(h, 200, 390) && inRange(l, 350, 650)) return 0;
  if (inRange(h, 430, 650) && inRange(l, 100, 300)) return 1;
  return -1;
}

static uint8_t lfsrDigest8Reflect(const uint8_t* message, int bytes, uint8_t gen, uint8_t key) {
  uint8_t sum = 0;
  for (int k = bytes - 1; k >= 0; --k) {
    uint8_t data = message[k];
    for (int i = 0; i < 8; ++i) {
      if ((data >> i) & 1) sum ^= key;
      if (key & 0x80) key = (uint8_t)((key << 1) ^ gen);
      else key = (uint8_t)(key << 1);
    }
  }
  return sum;
}

static void clearReading(TfaReading* out) {
  out->valid = false;
  out->id = 0;
  out->channel = 0;
  out->batteryOk = false;
  out->tempC = 0;
  out->hasHumidity = false;
  out->humidity = 0;
  out->repeats = 0;
  out->raw32 = 0;
}

static bool decodeWord(uint64_t w, uint8_t bits, TfaReading* out) {
  const uint8_t b0 = (uint8_t)(w >> (bits - 8));
  const uint8_t b1 = (uint8_t)(w >> (bits - 16));
  const uint8_t b2 = (uint8_t)(w >> (bits - 24));
  if (b0 == 0) return false;

  const int tempRaw = ((b1 & 0x0F) << 8) | b2;
  const float tempC = (tempRaw - 500) * 0.1f;
  if (tempC < -40.0f || tempC > 60.0f) return false;

  const bool hasHum = (bits >= 40);
  const uint8_t b3 = hasHum ? (uint8_t)(w >> (bits - 32)) : 0;
  const uint8_t b4 = hasHum ? (uint8_t)(w >> (bits - 40)) : 0;
  if (hasHum) {
    if (b3 == 0 || b3 > 100) return false;
    const uint8_t msg[4] = {b0, b1, b2, b3};
    if (lfsrDigest8Reflect(msg, 4, 0x31, 0xf4) != b4) return false;
  }

  out->valid = true;
  out->id = b0;
  out->channel = (uint8_t)(((b1 >> 4) & 0x03) + 1);
  out->batteryOk = hasHum ? ((b1 & 0x80) == 0) : ((b1 & 0x80) != 0);
  out->tempC = tempC;
  out->hasHumidity = hasHum;
  out->humidity = hasHum ? b3 : 0;
  out->raw32 = (uint32_t)(w >> (bits > 32 ? bits - 32 : 0));
  return true;
}

bool tfa3250Decode(const TfaHlPair* pairs, uint16_t count, TfaReading* out) {
  if (!out) return false;
  clearReading(out);
  if (!pairs || count < 8) return false;

  uint64_t cands[8];
  uint8_t  candBits[8];
  uint8_t  candN = 0;
  uint64_t acc = 0;
  uint8_t  bits = 0;
  bool     inFrame = false;

  for (uint16_t i = 0; i <= count; i++) {
    bool boundary = (i == count);
    int8_t b = -1;
    if (!boundary) {
      const uint16_t h = pairs[i].h;
      const uint16_t l = pairs[i].l;
      if (isSync(h, l)) {
        boundary = true;
      } else {
        b = bitFromPwm(h, l);
        if (b < 0) boundary = true;
      }
    }

    if (boundary) {
      if (inFrame && bits >= 32 && candN < 8) {
        const uint8_t capBits = (bits >= 40) ? 40 : 32;
        cands[candN]    = acc >> (bits - capBits);
        candBits[candN] = capBits;
        candN++;
      }
      acc = 0;
      bits = 0;
      inFrame = false;
      continue;
    }

    inFrame = true;
    if (bits < 48) {
      acc = (acc << 1) | (uint64_t)b;
      bits++;
    }
  }
  if (candN == 0) return false;

  uint64_t best = 0;
  uint8_t  bestBits = 0;
  uint8_t  bestCount = 0;
  bool     found40 = false;

  for (uint8_t i = 0; i < candN; i++) {
    if (candBits[i] < 40) continue;
    TfaReading trial;
    clearReading(&trial);
    if (!decodeWord(cands[i], 40, &trial)) continue;
    uint8_t c = 0;
    for (uint8_t j = 0; j < candN; j++) {
      if (candBits[j] == 40 && cands[j] == cands[i]) c++;
    }
    if (!found40 || c > bestCount) {
      found40 = true;
      bestCount = c;
      best = cands[i];
      bestBits = 40;
    }
  }

  if (!found40) {
    for (uint8_t i = 0; i < candN; i++) {
      uint8_t c = 0;
      for (uint8_t j = 0; j < candN; j++) {
        if (cands[j] == cands[i] && candBits[j] == candBits[i]) c++;
      }
      if (c > bestCount) {
        bestCount = c;
        best = cands[i];
        bestBits = candBits[i];
      }
    }
  }

  if (!decodeWord(best, bestBits, out)) {
    clearReading(out);
    return false;
  }
  out->repeats = bestCount;
  return true;
}
