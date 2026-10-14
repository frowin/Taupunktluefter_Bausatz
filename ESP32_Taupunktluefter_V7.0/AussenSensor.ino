//////////////////////////////////////////////////////////////////////////////
// ESP32-TaupunktLuefter
// Aussen-Temperaturquelle
//////////////////////////////////////////////////////////////////////////////

void AussenSensor_setup()
{
  Funk433_setup();
}

const char* getOutsideSourceName()
{
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433MHZ) return "433MHz";
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA) return "433TFA";
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249) return "433TFA3249";
  return "DHT22";
}

float readOutsideTemperature()
{
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_DHT22)
  {
    return dht2.readTemperature() + Korrektur_t_2;
  }

  if (Funk433_hasValidTemperature())
  {
    return Funk433_getTemperature() + Korrektur_t_2;
  }

  return NAN;
}

bool outsideTemperatureIsValid()
{
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_DHT22)
  {
    float value = t2;
    return !isnan(value) && value >= -40 && value <= 80;
  }

  return Funk433_hasValidTemperature() && !isnan(t2) && t2 >= -40 && t2 <= 80;
}
