//////////////////////////////////////////////////////////////////////////////
// ESP32-TaupunktLüfter 
// mit dem ESP32 Wroom 32
// für Arduino-ESP32 Release v3.3.8 
//
// Ulrich Schmerold
// 06/2026
//////////////////////////////////////////////////////////////////////////////

float taupunkt(float t, float r) 
{  
 float a, b;
  
  if (t >= 0) {
    a = 7.5;
    b = 237.3;
  } else if (t < 0) {
    a = 7.6;
    b = 240.7;
  }
  
  // Sättigungsdampfdruck in hPa
  float sdd = 6.1078 * pow(10, (a*t)/(b+t));
  
  // Dampfdruck in hPa
  float dd = sdd * (r/100);
  
  // v-Parameter
  float v = log10(dd/6.1078);
  
  // Taupunkttemperatur (°C)
  float tt = (b*v) / (a-v);
  return  tt ;  
}

void Taupunkte_berechnen()
   {  
   last_calculation = millis();
   h1 = dht1.readHumidity()+Korrektur_h_1;       // Innenluftfeuchtigkeit auslesen und unter „h1“ speichern
   t1 = dht1.readTemperature()+ Korrektur_t_1;   // Innentemperatur auslesen und unter „t1“ speichern
   h2 = dht2.readHumidity()+Korrektur_h_2;       // Außenluftfeuchtigkeit auslesen und unter „h2“ speichern
   t2 = readOutsideTemperature();                // Außentemperatur aus der gewählten Quelle
   if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249 && Funk433_hasValidHumidity())
   {
     h2 = Funk433_getHumidity() + Korrektur_h_2; // 30.3249.02 liefert Feuchte per 433 MHz
   }

  bool sensor1_ok = !(isnan(h1) || isnan(t1) || h1 > 100 || h1 < 1 || t1 < -40 || t1 > 80);
  bool sensor2_ok = outsideTemperatureIsValid();
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_DHT22)
  {
    sensor2_ok = sensor2_ok && !(isnan(h2) || h2 > 100 || h2 < 1);
  }
  if (outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249)
  {
    sensor2_ok = sensor2_ok && Funk433_hasValidHumidity();
  }
  bool taupunkt2_ok = sensor2_ok && !(isnan(h2) || h2 > 100 || h2 < 1);

  if (!sensor1_ok || !sensor2_ok) fehler = true;
  
  if (fehler == true)  // Prüfen, ob gültige Werte von den Sensoren kommen -----------------------------
  {   
    OLED_clear();
    OLED_println("Teste Sensoren..",0,0); 
    Serial_Debugging_println ("Teste Sensoren..");    
    fehler = false; 
    if (!sensor1_ok)   // wenn eins zutrifft, dann fehler = true
    {
      blink_LED(2,5);
      OLED_println("Fehler Sensor 1",2,0);
      Serial_Debugging_println("Fehler Sensor 1");
      Fehler_speichern(201);
      fehler = true;
     }else {
        OLED_println("Sensor 1 in Ordnung",2,0);
        Serial_Debugging_println("Sensor 1 in Ordnung");
     } 
     OLED_update();
     delay(500);  // Zeit um das oled zu lesen
  
      if (!sensor2_ok)  // DHT22 mode checks DHT22, 433 MHz mode checks the fresh RF temperature
      {
        blink_LED(3,5);
        OLED_println("Fehler Sensor 2",3,0);
        Serial_Debugging_println("Fehler Sensor 2");
        Fehler_speichern(202);
        fehler = true;
      } else {
       OLED_println("Sensor 2 in Ordnung",3,0);
        Serial_Debugging_println("Sensor 2 in Ordnung");
      }
    OLED_update();
    delay(delay_interval_3);  // Zeit um das oled zu lesen
  } 
   
 if (fehler == true) 
 {
    digitalWrite(RELAIPIN, RELAIS_AUS); // Relais ausschalten 
    digitalWrite(DHT_Power_Pin, LOW); // Strom für DHT-Sensoren ausschalten
    Serial_Debugging_println("Neustart Sensoren.....");
    OLED_println("Neustart Sensoren.....",5,0);
    for (int i=0; i < 5; i++)
    {
     OLED_println(String(5-i),6,10);
     OLED_update();
     TONE(800, 250);
     delay(250);
    }
     digitalWrite(DHT_Power_Pin, HIGH); // Strom für DHT-Sensoren einschalten
  } //----------------------------- Ende Sensorenprüfung -----------------------

    //------------------ Taupunkte errechnen ----------------------------------
 
 Taupunkt_1 = sensor1_ok ? taupunkt(t1, h1) : NAN;
 Taupunkt_2 = taupunkt2_ok ? taupunkt(t2, h2) : NAN;
 DeltaTP = (!isnan(Taupunkt_1) && !isnan(Taupunkt_2)) ? (Taupunkt_1 - Taupunkt_2) : NAN; 
   //---------------------------------------------- Chronik aktualisieren ---------------------------------------------
  if (Datenspeicher > 0) 
    if (chronik_update_all() == false) Serial_Debugging_println("Chronik Update fehlgeschlagen");
    
 //------------------------------------------------ Relais schalten ---------------------------------------------------
  if (isnan(DeltaTP)) rel = false;
  if (DeltaTP > (SCHALTmin + HYSTERESE))rel = true;
  if (DeltaTP < SCHALTmin)rel = false;
  if ((t1 < TEMP1_min) or (t1 >TEMP1_max)) rel = false;
  if ((t2 < TEMP2_min) or (t2 >TEMP2_max)) rel = false;
  
  if (Radonsignal) rel = true;

  if (rel == true)
  {  
    digitalWrite(RELAIPIN, RELAIS_EIN); // Relais einschalten
    if (Luefter_Start == 0)
    {
      Luefter_Start = millis(); 
    }    
  } else if ((millis() - Luefter_Start) > (Luefter_Mindestlaufzeit*1000*60)) // Mindestlaufzeit in Minuten
    {
     digitalWrite(RELAIPIN, RELAIS_AUS); // Relais ausschalten
     if (Luefter_Start > 0) {
      Luefter_Laufzeit = Luefter_Laufzeit + (millis() - Luefter_Start); // Lüfterlaufzeit in Millisekunden
      Luefter_Start = 0;
    }
  }
 //------------------------------------------------ Ampel schalten ----------------------------------
  if (use_Ampel == true)
  {
    if (DeltaTP > SCHALTmin ) set_Ampel_2(1,100); // grün
    if ((DeltaTP < SCHALTmin) and (DeltaTP > 0)) set_Ampel_2(2,100); // gelb
    if (DeltaTP <= 0) set_Ampel_2(3,100); // rot
    if ((t1 < TEMP1_min) or (t1 >TEMP1_max)) set_Ampel_2(3,100); // rot
    if ((t2 < TEMP2_min) or (t2 >TEMP2_max)) set_Ampel_2(3,100); // rot
  } //-----Ende Ampel  
 }
