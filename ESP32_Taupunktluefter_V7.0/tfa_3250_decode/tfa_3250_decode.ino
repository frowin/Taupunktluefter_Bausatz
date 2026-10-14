// Offline demo — copy tfa_3250_decode.h/.cpp into other PIO projects' src/ or lib/
#include "tfa_3250_decode.h"

static const TfaHlPair kRawPairs[] = {
    {1168, 794}, {1136, 821}, {1111, 847}, {361, 381}, {352, 382},
    {585, 140},  {346, 398},  {575, 159},  {327, 403}, {570, 158},
    {568, 173},  {559, 172},  {554, 179},  {305, 444}, {530, 192},
    {297, 438},  {290, 440},  {536, 200},  {535, 197}, {285, 450},
    {280, 456},  {281, 450},  {280, 452},  {523, 210}, {520, 212},
    {275, 460},  {272, 459},  {270, 466},  {516, 213}, {270, 464},
    {513, 221},  {267, 468},  {263, 468},  {263, 466}, {511, 225},
    {263, 529},  {994, 957},  {993, 961},  {990, 972}, {985, 967},
};

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
  }

  TfaReading r;
  if (!tfa3250Decode(kRawPairs, sizeof(kRawPairs) / sizeof(kRawPairs[0]), &r) ||
      !r.valid) {
    Serial.println("null");
    return;
  }

  Serial.print("id=0x");
  if (r.id < 16) Serial.print('0');
  Serial.print(r.id, HEX);
  Serial.print(" ch=");
  Serial.print(r.channel);
  Serial.print(" temp=");
  Serial.print(r.tempC, 1);
  Serial.print("C bat=");
  Serial.print(r.batteryOk ? "ok" : "low");
  Serial.print(" repeats=");
  Serial.print(r.repeats);
  Serial.print(" raw=0x");
  Serial.println(r.raw32, HEX);
}

void loop() {}
