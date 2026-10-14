//////////////////////////////////////////////////////////////////////////////
// ESP32-TaupunktLuefter
// 433 MHz Empfaenger WL-101-341
// Acurite 606TX + TFA 30.3250 (TX141Bv3) + TFA 30.3249.02 (TX141TH-Bv2)
//////////////////////////////////////////////////////////////////////////////

#include "driver/gpio.h"
#include "tfa_3250_decode/tfa_3250_decode.h"

#define RF433_PULSE_BUFFER_SIZE 4096
#define RF433_PULSE_BUFFER_MASK (RF433_PULSE_BUFFER_SIZE - 1)
#define RF433_PULSES_PER_LOOP 512
#define RF433_SHORT_MIN_US 1500
#define RF433_SHORT_MAX_US 2800
#define RF433_LONG_MIN_US 3200
#define RF433_LONG_MAX_US 5200
#define RF433_RESET_MIN_US 7000
#define RF433_PULSE_MAX_US 900
#define RF433_ISR_MIN_STORE_US 100
#define RF433_MIN_REPEATS 1
#define TFA_PAIR_BUFFER_SIZE 192
#define TFA_IDLE_GAP_US 4000

struct Rf433Pulse
{
  uint16_t duration;
  uint8_t wasHigh;
};

volatile Rf433Pulse rf433PulseBuffer[RF433_PULSE_BUFFER_SIZE];
volatile uint16_t rf433PulseHead = 0;
volatile uint16_t rf433PulseTail = 0;
volatile bool rf433PulseOverflow = false;
volatile unsigned long rf433LastChangeMicros = 0;
volatile unsigned long rf433OverflowCount = 0;
volatile unsigned long rf433IgnoredShortPulses = 0;

uint64_t rf433CurrentBits = 0;
byte rf433CurrentBitCount = 0;
uint64_t rf433LastCandidate = 0;
byte rf433RepeatCount = 0;

float rf433Temperature = NAN;
int rf433LastId = -1;
int rf433LastChannel = 0;
bool rf433BatteryLow = false;
bool rf433ButtonPressed = false;
unsigned long rf433LastReceiveMillis = 0;
unsigned long rf433PacketCount = 0;
uint32_t rf433LastRaw = 0;

float tfaTemperature = NAN;
float tfaHumidity = NAN;
int tfaLastId = -1;
int tfaLastChannel = 0;
bool tfaBatteryOk = false;
bool tfaHasHumidity = false;
unsigned long tfaLastReceiveMillis = 0;
unsigned long tfaPacketCount = 0;
uint32_t tfaLastRaw = 0;
uint8_t tfaLastRepeats = 0;

static bool tfaSourceSelected()
{
  return outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA ||
         outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249;
}

TfaHlPair tfaPairs[TFA_PAIR_BUFFER_SIZE];
uint16_t tfaPairCount = 0;
uint16_t tfaPendingH = 0;
bool tfaHaveH = false;

bool IRAM_ATTR rf433ShouldStorePulse(uint16_t duration, bool wasHigh)
{
  if (wasHigh)
  {
    return duration <= 1400;
  }

  return duration <= 1100 ||
         (duration >= RF433_SHORT_MIN_US && duration <= RF433_SHORT_MAX_US) ||
         duration >= RF433_LONG_MIN_US;
}

void IRAM_ATTR rf433ISR()
{
  unsigned long now = micros();
  unsigned long duration = now - rf433LastChangeMicros;
  rf433LastChangeMicros = now;

  int levelNow = gpio_get_level((gpio_num_t)RF433_DATA_PIN);
  bool wasHigh = (levelNow == 0);

  if (duration > 65535) duration = 65535;
  if (duration < RF433_ISR_MIN_STORE_US || !rf433ShouldStorePulse((uint16_t)duration, wasHigh))
  {
    rf433IgnoredShortPulses++;
    return;
  }

  uint16_t nextHead = (uint16_t)((rf433PulseHead + 1) & RF433_PULSE_BUFFER_MASK);
  if (nextHead != rf433PulseTail)
  {
    rf433PulseBuffer[rf433PulseHead].duration = (uint16_t)duration;
    rf433PulseBuffer[rf433PulseHead].wasHigh = wasHigh ? 1 : 0;
    rf433PulseHead = nextHead;
  }
  else
  {
    rf433PulseOverflow = true;
    rf433OverflowCount++;
  }
}

uint8_t rf433LfsrDigest8(uint8_t const message[], unsigned bytes, uint8_t gen, uint8_t key)
{
  uint8_t sum = 0;
  for (unsigned k = 0; k < bytes; ++k)
  {
    uint8_t data = message[k];
    for (int i = 7; i >= 0; --i)
    {
      if ((data >> i) & 1) sum ^= key;
      if (key & 1) key = (key >> 1) ^ gen;
      else key = (key >> 1);
    }
  }
  return sum;
}

void rf433ResetRow()
{
  rf433CurrentBits = 0;
  rf433CurrentBitCount = 0;
}

void rf433AcceptAcurite606(uint32_t data)
{
  uint8_t b[4];
  b[0] = (data >> 24) & 0xFF;
  b[1] = (data >> 16) & 0xFF;
  b[2] = (data >> 8) & 0xFF;
  b[3] = data & 0xFF;

  if (b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 0) return;
  if (rf433LfsrDigest8(b, 3, 0x98, 0xF1) != b[3]) return;

  int sensorId = b[0];
  if (sensorId != RF433_sensor_id) return;

  if (rf433LastCandidate == data) rf433RepeatCount++;
  else
  {
    rf433LastCandidate = data;
    rf433RepeatCount = 1;
  }

  if (rf433RepeatCount < RF433_MIN_REPEATS) return;

  int16_t tempRaw = (int16_t)(((uint16_t)b[1] << 12) | ((uint16_t)b[2] << 4));
  tempRaw = tempRaw >> 4;
  float tempC = tempRaw * 0.1f;
  if (tempC < -50.0 || tempC > 80.0) return;

  rf433Temperature = tempC;
  rf433LastId = sensorId;
  rf433BatteryLow = ((b[1] & 0x80) != 0);
  rf433ButtonPressed = ((b[1] & 0x40) != 0);
  rf433LastChannel = ((b[1] & 0x30) >> 4) + 1;
  rf433LastReceiveMillis = millis();
  rf433LastRaw = data;
  rf433PacketCount++;
}

void rf433FinalizeRow()
{
  if (rf433CurrentBitCount >= 32 && rf433CurrentBitCount <= 33)
  {
    uint32_t data = (uint32_t)(rf433CurrentBits >> (rf433CurrentBitCount - 32));
    rf433AcceptAcurite606(data);
  }
  rf433ResetRow();
}

void rf433AddBit(bool bitValue)
{
  if (rf433CurrentBitCount >= 33)
  {
    rf433ResetRow();
    return;
  }

  rf433CurrentBits = (rf433CurrentBits << 1) | (bitValue ? 1ULL : 0ULL);
  rf433CurrentBitCount++;
}

void rf433ProcessDuration(uint16_t duration)
{
  if (duration >= RF433_RESET_MIN_US)
  {
    rf433FinalizeRow();
    return;
  }

  if (duration <= RF433_PULSE_MAX_US) return;

  if (duration >= RF433_SHORT_MIN_US && duration <= RF433_SHORT_MAX_US)
  {
    rf433AddBit(false);
    return;
  }

  if (duration >= RF433_LONG_MIN_US && duration <= RF433_LONG_MAX_US)
  {
    rf433AddBit(true);
    return;
  }

  rf433ResetRow();
}

void tfaResetBurst()
{
  tfaPairCount = 0;
  tfaHaveH = false;
  tfaPendingH = 0;
}

void tfaAcceptReading(const TfaReading &reading)
{
  if (!reading.valid) return;
  if (reading.channel != RF433_tfa_channel) return;

  // 30.3250 = temp-only 32-bit; 30.3249.02 = temp+humidity 40-bit (+CRC)
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA && reading.hasHumidity) return;
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249 && !reading.hasHumidity) return;

  tfaTemperature = reading.tempC;
  tfaHasHumidity = reading.hasHumidity;
  tfaHumidity = reading.hasHumidity ? (float)reading.humidity : NAN;
  tfaLastId = reading.id;
  tfaLastChannel = reading.channel;
  tfaBatteryOk = reading.batteryOk;
  tfaLastReceiveMillis = millis();
  tfaLastRaw = reading.raw32;
  tfaLastRepeats = reading.repeats;
  tfaPacketCount++;
}

void tfaFinalizeBurst()
{
  if (tfaPairCount >= 8)
  {
    TfaReading reading;
    if (tfa3250Decode(tfaPairs, tfaPairCount, &reading) && reading.valid)
    {
      tfaAcceptReading(reading);
    }
  }
  tfaResetBurst();
}

void tfaProcessPulse(uint16_t duration, bool wasHigh)
{
  if (wasHigh)
  {
    tfaPendingH = duration;
    tfaHaveH = true;
    return;
  }

  if (duration >= TFA_IDLE_GAP_US)
  {
    tfaFinalizeBurst();
    return;
  }

  if (!tfaHaveH) return;

  if (tfaPairCount < TFA_PAIR_BUFFER_SIZE)
  {
    tfaPairs[tfaPairCount].h = tfaPendingH;
    tfaPairs[tfaPairCount].l = duration;
    tfaPairCount++;
  }
  tfaHaveH = false;
}

void Funk433_setup()
{
  pinMode(RF433_DATA_PIN, INPUT);
  rf433PulseHead = 0;
  rf433PulseTail = 0;
  rf433PulseOverflow = false;
  rf433LastChangeMicros = micros();
  attachInterrupt(digitalPinToInterrupt(RF433_DATA_PIN), rf433ISR, CHANGE);
}

void Funk433_loop()
{
  bool localOverflow = false;
  uint16_t processed = 0;

  noInterrupts();
  if (rf433PulseOverflow)
  {
    localOverflow = true;
    rf433PulseOverflow = false;
  }
  interrupts();

  if (localOverflow)
  {
    rf433ResetRow();
    tfaHaveH = false;
  }

  while (rf433PulseTail != rf433PulseHead && processed < RF433_PULSES_PER_LOOP)
  {
    Rf433Pulse pulse;
    pulse.duration = rf433PulseBuffer[rf433PulseTail].duration;
    pulse.wasHigh = rf433PulseBuffer[rf433PulseTail].wasHigh;
    rf433PulseTail = (uint16_t)((rf433PulseTail + 1) & RF433_PULSE_BUFFER_MASK);

    rf433ProcessDuration(pulse.duration);
    tfaProcessPulse(pulse.duration, pulse.wasHigh != 0);
    processed++;
  }
}

bool Funk433_hasValidTemperature()
{
  if (tfaSourceSelected())
  {
    return !isnan(tfaTemperature) && ((millis() - tfaLastReceiveMillis) <= RF433_max_age_ms);
  }
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433MHZ)
  {
    return !isnan(rf433Temperature) && ((millis() - rf433LastReceiveMillis) <= RF433_max_age_ms);
  }
  return false;
}

float Funk433_getTemperature()
{
  if (tfaSourceSelected()) return tfaTemperature;
  return rf433Temperature;
}

bool Funk433_hasValidHumidity()
{
  return outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249 &&
         tfaHasHumidity &&
         !isnan(tfaHumidity) &&
         tfaHumidity >= 1.0f &&
         tfaHumidity <= 100.0f &&
         ((millis() - tfaLastReceiveMillis) <= RF433_max_age_ms);
}

float Funk433_getHumidity()
{
  return tfaHumidity;
}

String rf433FormatId(int id)
{
  String s = String(id, HEX);
  s.toUpperCase();
  return "0x" + s;
}

String rf433FormatRaw(uint32_t raw)
{
  String s = String(raw, HEX);
  s.toUpperCase();
  while (s.length() < 8) s = "0" + s;
  return "0x" + s;
}

String getAcuriteStatusText()
{
  if (rf433LastId < 0) return "Acurite-606TX " + rf433FormatId(RF433_sensor_id) + ": noch kein Paket";

  String status = String((millis() - rf433LastReceiveMillis) / 1000) + "s ago<br>";
  status += "Acurite-606TX " + rf433FormatId(rf433LastId) + "<br>";
  status += String(rf433Temperature, 1) + "&deg;C &middot; ";
  status += rf433BatteryLow ? "bat low" : "bat OK";
  status += " &middot; ch " + String(rf433LastChannel) + "<br>";
  status += "Acurite-606TX &middot; sn " + rf433FormatId(rf433LastId);
  status += " &middot; raw " + rf433FormatRaw(rf433LastRaw);
  status += " &middot; 32 bit &middot; proto 0 &middot; mark 500&micro;s";
  status += " &middot; #" + String(rf433PacketCount);
  status += "<br>overflow " + String(rf433OverflowCount);
  status += " &middot; ignored noise " + String(rf433IgnoredShortPulses);
  if (rf433ButtonPressed) status += "<br>TX button";
  return status;
}

String getTfaStatusText()
{
  int dial = RF433_tfa_channel - 1;
  if (dial < 1) dial = 1;
  const bool want3249 = (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249);
  const char* model = want3249 ? "TFA 30.3249.02" : "TFA 30.3250";
  const char* modelTag = want3249 ? "TFA-30.3249" : "TFA-30.3250";

  if (tfaLastId < 0)
  {
    return String(model) + " Schalter " + String(dial) + " (CH" + String(RF433_tfa_channel) + "): noch kein Paket";
  }

  String status = String((millis() - tfaLastReceiveMillis) / 1000) + "s ago<br>";
  status += String(model) + " " + rf433FormatId(tfaLastId) + "<br>";
  status += String(tfaTemperature, 1) + "&deg;C";
  if (tfaHasHumidity && !isnan(tfaHumidity)) status += " &middot; " + String(tfaHumidity, 0) + "% RH";
  status += " &middot; ";
  status += tfaBatteryOk ? "bat OK" : "bat low";
  status += " &middot; Schalter " + String(tfaLastChannel - 1);
  status += " (CH" + String(tfaLastChannel) + ")<br>";
  status += String(modelTag) + " &middot; sn " + rf433FormatId(tfaLastId);
  status += " &middot; raw " + rf433FormatRaw(tfaLastRaw);
  status += " &middot; repeats " + String(tfaLastRepeats);
  status += " &middot; #" + String(tfaPacketCount);
  status += "<br>overflow " + String(rf433OverflowCount);
  status += " &middot; ignored noise " + String(rf433IgnoredShortPulses);
  return status;
}

String getRF433StatusText()
{
  if (tfaSourceSelected()) return getTfaStatusText();
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433MHZ) return getAcuriteStatusText();

  String status = "DHT22 aktiv (433 MHz Standby)<br>";
  if (rf433LastId >= 0)
  {
    status += "Acurite: " + String(rf433Temperature, 1) + "&deg;C / " + rf433FormatId(rf433LastId);
    status += " / " + String((millis() - rf433LastReceiveMillis) / 1000) + "s<br>";
  }
  if (tfaLastId >= 0)
  {
    status += "TFA: " + String(tfaTemperature, 1) + "&deg;C / CH" + String(tfaLastChannel);
    status += " / " + String((millis() - tfaLastReceiveMillis) / 1000) + "s";
  }
  if (rf433LastId < 0 && tfaLastId < 0) status += "noch kein 433 MHz Paket";
  return status;
}
