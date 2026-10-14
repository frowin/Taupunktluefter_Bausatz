//////////////////////////////////////////////////////////////////////////////
// ESP32-TaupunktLüfter 
// mit dem ESP32 Wroom 32
// für Arduino-ESP32 Release v3.3.8 
//
// Ulrich Schmerold
// 06/2026
//////////////////////////////////////////////////////////////////////////////

#define interval_reconnect 60000000  // eine Minute
unsigned long Start_Millis = millis();
 
const char* ntpServer1 = "pool.ntp.org";                // 1. Zeitserver
const char* ntpServer2 = "time.nist.gov";               // 2. Zeitserver
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";   // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

void notFound(AsyncWebServerRequest *request) {request->send(404, "text/plain", "Not found");} 


unsigned long get_Local_Time()
{
  struct tm timeinfo;
  int n = 0;
  bool erfolg = true;

 
  configTzTime(time_zone, ntpServer1, ntpServer2);

  OLED_println("Suche Internetzeit", 6, 0);
  OLED_setCursor(6, 0);
  Serial_Debugging_println("Suche Internetzeit");

  while ( (n < 10) and (!getLocalTime(&timeinfo)))
  {   
    n++;
    OLED_print("X");
    OLED_update();
    Serial_Debugging_print("X");
    delay(500);
  }
  if (n > 9) {
    OLED_println("Keine Zeit ermittelt", 7, 0);
    Serial_Debugging_println("Keine Zeit ermittelt"); Fehler_speichern(300);
    erfolg = false;
  } else {
    rtc.setTimeStruct(timeinfo);
    make_time_stamp ();
    OLED_Zeile_loeschen(6);
    OLED_println(stamp, 6, 0);
    Serial_Debugging_println(stamp);
  }
  delay(delay_interval_3);
  if (erfolg) return rtc.getEpoch(); else return 0;
}

String make_time_stamp ()
{
 stamp = rtc.getTime("%d.%m.%y %H:%M");
 //stamp = rtc.getTime("%d.%m.%Y %H:%M:%S");
 return stamp;
}


void WIFI_Connect()
{ 
  OLED_clear();
  OLED_println("WIFI-1", 0, 7);
  OLED_println("Verbinde: ",2,0);
  OLED_println(ssid, 3, 0);
  OLED_update();
  Serial_Debugging_println("Verbinde zu: " + String(ssid));

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(Hostname);
  if (DHCP == false)
  {
    Serial.println("DHCP = false"); 
    WiFi.config(local_IP, gateway, subnet, dns1, dns2 ); 
  }else Serial.println("DHCP = true");
  
  WiFi.onEvent(WiFiEvent);
  WiFi.mode (WIFI_STA);
  //WiFi.setSleep(false);
  WiFi.begin(ssid, password);
 
  OLED_setCursor(3, 0);
  //--------------------------------------------------   Verbindungsversuche ins Netz-----------------------------------------------
  byte n = 0;
  while ((WiFi.status() != WL_CONNECTED) and (n < 14))
  {    
    n++;
    OLED_setCursor(4,n-1);
    drawSonderzeichen(4,false);
    OLED_update();
    Serial_Debugging_print(".");
    delay(500);
  } Serial_Debugging_println("");

  if (WiFi.status() == WL_CONNECTED) //--------------------- WIFI wurde verbunden --------------------------------------------------
  {
    OLED_println("WIFI ist verbunden", 4, 0);
    Serial_Debugging_println("WIFI ist verbunden");

    // -------------------------------------------------  Zeit aus dem Internet holen und RTC damit einstellen----------------------
    get_Local_Time();
    // ---------------------------------------   Jetzt wird der Webserver gestartet und der request angelegt ------------------------
    Serial_Debugging_println("Server Starten");
    WiFi.onEvent(WiFiEvent);  
         //------------------------------------- HTML Taupunktlüfter Seite -------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) 
     { 
        TONE (800, 300);
        Serial.print("Received request from client with IP: ");
        Serial.println(request->client()->remoteIP());

       HTML_processor_is_working = true;
       Serial_Debugging_println("index_html wird angefordert");
       
        delay(1);  // um den Watchdog zurückzusetzen     
        request->send(200, "text/html", index_html, processor);
     });
  //---------------------------------------------- Daten ------------------------------------------------
   server.on("/Daten", HTTP_GET, [](AsyncWebServerRequest * request) 
     { 
        TONE (800, 300);
        Serial.print("Received request from client with IP: ");
        Serial.println(request->client()->remoteIP());

       HTML_processor_is_working = true;
       Serial_Debugging_println("/Daten/ werden angefordert");
       
        delay(1);  // um den Watchdog zurückzusetzen       
        request->send(200, "text/html", Daten, processor);
     });
  //---------------------------------------------- Optionen ------------------------------------------------
   server.on("/Optionen", HTTP_GET, [](AsyncWebServerRequest * request) 
     { 
        Optionen_to_String();
        TONE (800, 300);
        Serial.print("Received request from client with IP: ");
        Serial.println(request->client()->remoteIP());

       HTML_processor_is_working = true;
       Serial_Debugging_println("/Optionen/ werden angefordert");
       
        delay(1);  // um den Watchdog zurückzusetzen       
        request->send(200, "text/html", Optionen, processor);
     });    
  server.on("/saveOutsideTempSource", HTTP_POST, [](AsyncWebServerRequest * request) 
     {
       String source = "DHT22";
       String rfId = rf433FormatId(RF433_sensor_id);
       String tfaChannel = String(RF433_tfa_channel);

       if (request->hasParam("source", true)) source = request->getParam("source", true)->value();
       if (request->hasParam("rfid", true)) rfId = request->getParam("rfid", true)->value();
       if (request->hasParam("tfach", true)) tfaChannel = request->getParam("tfach", true)->value();

       if (Optionen_web_aussensensor_speichern(source, rfId, tfaChannel))
       {
         TONE (800, 200);
         request->redirect("/Optionen");
       }
       else
       {
         request->send(400, "text/plain", "Ungueltige Aussen-Sensor Optionen");
       }
     });    
 //---------------------------------------------- Speicher------------------------------------------------
   server.on("/Speicher", HTTP_GET, [](AsyncWebServerRequest * request) 
     { 
        SpeicherGroessen();
        Make_WIFI_Status_String();
        TONE (800, 300);
        Serial.print("Received request from client with IP: ");
        Serial.println(request->client()->remoteIP());

       HTML_processor_is_working = true;
       Serial_Debugging_println("/Speicher/ wird angefordert");
       
        delay(1);  // um den Watchdog zurückzusetzen       
        request->send(200, "text/html", Speicher, processor);
     });   
 
 //---------------------------------------------- Daten als Datei------------------------------------------------
   server.on("/Datendatei.txt", HTTP_GET, [](AsyncWebServerRequest * request) 
     { 
        TONE (800, 300);
        Serial.print("Received request from client with IP: ");
        Serial.println(request->client()->remoteIP());

       HTML_processor_is_working = true;
       Serial_Debugging_println("/Daten_Datei/ wird angefordert");
       
        delay(1);  // um den Watchdog zurückzusetzen       
        request->send(200, "multipart/form-data", Daten_Datei, processor);
     });
//---------------------------------------------- Fehler------------------------------------------------
   server.on("/Fehler", HTTP_GET, [](AsyncWebServerRequest * request) 
     { 
        read_Fehler_from_FS();
        TONE (800, 300);
        Serial.print("Received request from client with IP: ");
        Serial.println(request->client()->remoteIP());

       HTML_processor_is_working = true;
       Serial_Debugging_println("/Fehler/ wird angefordert");
       
        delay(1);  // um den Watchdog zurückzusetzen       
        request->send(200, "text/html", Fehler, processor);
     });   
// ------------------------------------------------------------  ESP Restart ---------------------------------
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "ESP startet neu...");  
    TONE (1000, 1000);
  DefaultHeaders::Instance().addHeader("Connection", "close");
    // Kleiner Trick: Den Restart um 2 Sekunden verzögern
  xTaskCreate([](void*){vTaskDelay(pdMS_TO_TICKS(2000)); ESP.restart();}, "restart_task", 1000, NULL, 1, NULL);
  TONE (400, 1000);
});
//--------------------------------------- Fehlerprotokoll löschen ---------------------------------------------
 server.on("/clearlog", HTTP_GET, [](AsyncWebServerRequest *request) {
  File f = LittleFS.open("/Fehler.txt", "w");
  if (f) {
    f.println("--- Protokoll gelöscht ---");
    f.close();
    request->send(200, "text/plain", "Protokoll wurde geleert.");
  } else {
    request->send(500, "text/plain", "Fehler beim Loeschen!");
  }
});
//----------------------------------------------------------------------------Daten löschen -----------------------------------
server.on("/cleardata", HTTP_GET, [](AsyncWebServerRequest *request){
 Datenstreams_leeren();
 request->send(200, "text/plain", "Löschen gestartet");
 format_LittleFS_kurz();
 if (Datenspeicher > 0){load_Chronik_from_LittleFS();}
  TONE(1000, 1000);  
});
//--------------------------------------------------------------------------Preferences löschen -------------------------------
server.on("/clearPrefs", HTTP_GET, [](AsyncWebServerRequest *request){
 Optionen_loeschen(); 
 TONE(1000, 200);
  request->send(200, "text/plain", "OK");
});
//--------------------------------------------------------------------------------------------------------------------------------

    server.onNotFound(notFound);
    server.begin();
    if (debugging) SerialprintWifiStatus();

  } else { //------------------------------------------------ WIFI ist nicht verbunden ---------------------------------------------
    OLED_println("nicht verbunden!", 4, 0);
    Serial_Debugging_println("WIFI ist nicht verbunden"); Fehler_speichern(301);
   // use_WIFI = false;  // kein erneuter Versuch, ins Netz zu kommen
  }
  OLED_update();
 
  delay(delay_interval_2);
}
//-------------------------------------------------------------------------------------------------------------------------------------

void WIFI_watch_Task() {

  if(millis() - Start_Millis > interval_reconnect)
  {
     Start_Millis = millis(); // Zähler zurücksetzen
     own_WIFI_reconnect();
   }
} 
//-------------------------------------------------------------------------------------------------------------------------------------

void own_WIFI_reconnect()
{
     if (WiFi.status() != WL_CONNECTED)
      {
          WiFi.disconnect();
          Serial.println("WIFI ist unterbrochen! Versuche neu zu verbinden");
          OLED_clear();
          OLED_println("WIFI ist unterbrochen", 0, 0);
          OLED_println("Versuche zu verbinden", 1, 0);
          Serial_Debugging_print("Verbindungsversuch: ");
          byte n=0;
          while ((WiFi.status() != WL_CONNECTED) and (n < 10))
          {    
            n++;
            Serial_Debugging_print( String(n) + "-");
            OLED_println("Versuch " + String(n), 3, 0);
            OLED_update();
            if (DHCP == false) WiFi.config(local_IP, gateway, subnet, dns1, dns2 ) ;
            //WiFi.config(local_IP, gateway, subnet, dns1, dns2 ) ;
            WiFi.begin(ssid, password);
            delay(1000);
          } 
          Serial_Debugging_println("");
          if (n > 9) {
            OLED_println("Leider kein Erfolg!", 4, 0);
            OLED_println("ESP32 Neustart.....",6,0); Fehler_speichern(302);
            
            OLED_update();
            soft_reset(3);
          }
      }
}
//-------------------------------------------------------------------------------------------------------------------------------------
