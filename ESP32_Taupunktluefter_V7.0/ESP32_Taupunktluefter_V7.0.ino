//////////////////////////////////////////////////////////////////////////////
// ESP32-TaupunktLüfter 
// mit dem ESP32 Wroom 32
// für Arduino-ESP32 Release v3.3.8 
//
// Ulrich Schmerold
// 06/2026
//////////////////////////////////////////////////////////////////////////////

String Software_version = "7.2.10";
#define Hostname     "TPL-V7.2.10"
#define use_OLED true

// Dieser Code benötig zwingend die folgenden Libraries:
#include <string.h>
#include <DHT.h>
#include <time.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>  //Version 3.7.10 !!!!!! Siehe: https://docs.arduino.cc/libraries/esp-async-webserver/#Releases
#include <Wire.h>
#include <GyverOLED.h>
#include <esp_task_wdt.h>
#include <StreamString.h>
#include "LittleFS.h"
#include "tfa_3250_decode/tfa_3250_decode.h"

GyverOLED<SSH1106_128x64> oled;
static AsyncWebServer server(80);

ESP32Time rtc;  
String stamp;  // Variable für den Zeitstempel.

//************************************** WIFI ***************************************************************
#define laenge_Zeichenfolge 36    //35 Zeichen und /0
bool DHCP = true;
char ssid [laenge_Zeichenfolge]; 
char password [laenge_Zeichenfolge]; 

IPAddress local_IP(192, 168, 178, 140);       // Diese IP-Adrese bekommt der Taupunkt-Lüfter
IPAddress gateway (192, 168, 178, 1);         // Gateway ins Internet
IPAddress subnet  (255, 255, 255, 0);         // Subnet-Mask
IPAddress dns1    (192, 168, 178, 1);         // eigener DNS-Server
IPAddress dns2    (  8,   8,   8, 8);         // DNS-Server von Google

//************************************* Prototyping (geordet nach Karteireiter  *******************************
//------------------------ Chronik -----------------------
void Datenstreams_leeren();
bool load_Chronik_from_LittleFS();
String load_Datensatz(String name);
bool chronic_update (String , float);
bool format_LittleFS();
bool format_LittleFS_kurz();
bool remove_LittleFS(String name) ;
bool write_LittleFS(String name, String daten);
void daten_serial_ausgeben();
bool chronik_update_all();
unsigned int LittleFS_print_space();
bool test_TimeDate(); //--------------------------------------- Datum und Zeit  speichern für chart labels
bool test_temp_in(); //--------------------------------------- Innentemperatur abspeichern 
bool test_temp_Out(); //--------------------------------------- Außentemperatur abspeichern 
bool test_air_in(); //--------------------------------------- Innen-Luftfeuchte abspeichern 
bool test_air_out(); //--------------------------------------- Außen-Luftfeuchte abspeichern 
bool test_delta_TP(); //-------------------------------------- Unterschied der Taupunkte
bool test_Taupunkt_1(); //--------------------------------------- Taupunkt 1
bool test_Taupunkt_2(); //--------------------------------------- Taupunkt 2
bool test_lz();         //------------------------------------ Lüfter_Laufzeit
bool test_lzR();         //------------------------------------ Radon Lüfter_Laufzeit
//------------------------ Debugging ---------------------
void TONE (int Frequenz, int Dauer);
void Serial_Debugging_println(String);
void Serial_Debugging_print(String);
void soft_reset(byte = 0);
void hw_wdt_disable();
void hw_wdt_enable();
String print_Reset_reason(int Fehlernummer);
void print_Core_task_is_running(String);
void delay_Timer(String Text = "" , int interval = 1000);
void list_all_LittleFS_files();
void Serial_print_Optionen();
void Watchdog_reset();
void serial_debugging_all();
void Flash_available();
void serial_WIFI_Status();
void SpeicherGroessen();
void SerialprintWifiStatus();
void Make_WIFI_Status_String();
void trim_length_errorlist();

//----------------------------- Fehlerlisten ----------------
String Get_Fehlertext_from_Nummer(int Fehlernummer);
void trim_length_errorlist();
void Fehler_speichern(int Nummer) ;
void read_Fehler_from_FS();
//------------------------------ HTML -----------------------
String processor(const String& var);
int groesse_raw_Literal();
//------------------------------ OLED -----------------------
void OLED_init();
void OLED_update();
void OLED_clear();
void OLED_Zeile_loeschen(byte Zeilennummer);
void OLED_print(String Text);
void OLED_println(String Text, byte Zeile, byte Spalte);
void drawSonderzeichen(byte, bool invers = false);
void OLED_setCursor(byte Zeile , byte Spalte = 0 );
void show_Kontaktbildschirm();
void show_Wertebildschirm();
void show_Status();
void OLED_Draw_Bitmap();
void OLED_Draw_Logo();
//------------------------------ Wifi -------------------------
unsigned long get_Local_Time();
String make_time_stamp ();
void WIFI_Connect();
void WiFiEvent(WiFiEvent_t event);
void WIFI_watch_Task();
void own_WIFI_reconnect();
// ----------------------- RotaryEncoder-----------------------
void Encoder_initialisieren();
volatile int get_Encoder_pos(int min, int max, bool ueberlauf);
void reset_Encoder(int pos = 0);
void checkPosition();
void checkButton();
void timer_initialisieren();
void IRAM_ATTR onTimer();
void Timer_off();
void Timer_on();
//------------------------ Ampel -------------------------------
void setup_Ampel();
void blink_LED(byte n, byte z);
void LED_Test();
void Heardbeat();
void set_Ampel_1(byte color);
void set_Ampel_2(byte color, byte Helligkeit);
void Set_LED1();
void Set_LED2();
void Set_LED3();
// ----------------------- Optionen ---------------------------
void show_menue();
void show_SubMenue();
void  show_Eingabe_Bildschirm();
void alle_Optionen_speichern();
bool Save_one_Item(byte item);
void Optionen_laden();
void Optionen_loeschen();
void Optionen_Serial_ausgeben();
void Optionen_zu_Variablen();
byte String_to_MenuePos(String s);
bool Wartung(byte MenueItem);
void Optionen_reset();
void LittleFS_formatieren();
void Optionen_to_String();
//------------------------- WerteWandel ------------------------
bool StringVergleich(String A, String B);
String float_to_String(float f);
float String_to_float(String s);
String String_laenge_ergaenzen(String S, byte N);
int String_to_int( String S);
bool Bool(String s);
//-------------------------Taupunktberechnung ------------------
void Taupunkte_berechnen();
//-------------------------Aussensensor -------------------------
void AussenSensor_setup();
float readOutsideTemperature();
bool outsideTemperatureIsValid();
const char* getOutsideSourceName();
String getRF433StatusText();
String rf433FormatId(int id);
void Funk433_setup();
void Funk433_loop();
bool Funk433_hasValidTemperature();
float Funk433_getTemperature();
bool Funk433_hasValidHumidity();
float Funk433_getHumidity();
//-------------------------Web Optionen -------------------------
bool Optionen_web_aussensensor_speichern(String source, String rfId, String tfaChannel);
//************************************************** Prototyping Ende *********************************

//******************************** Deklaration von globalen Variabeln *********************************
bool chronik_is_busy = false;           // Zeigt an, dass gerade die Chronik-Daten geladen oder gespeichert werden
bool HTML_processor_is_working = false; // Zeigt an, dass gerade eine Webseite übertragen wird

int delay_interval_2 = 4000; // Dauer allgemeine Anzeigen
int delay_interval_3 = 1500; // Dauer allgemeine Anzeigen

#define Ionenkammer_pin 33// Anschluss an die Ionenkammer (Radon-Entlüftung)
#define debugingPin 18    // Anschluss Debugging Taster
#define RELAIPIN 26       // Anschluss des LüfterRelais
#define DHTPIN_1 15       // Datenleitung für den DHT Sensor 1 (innen)
#define DHTPIN_2 4        // Datenleitung für den DHT Sensor 2 (außen)
#define DHT_Power_Pin 23  //Stromversorgung der DHT-Sensoren
#define Tone_pin 19       // Pin für den Buzzer
#define RF433_DATA_PIN 16 // Datenleitung WL-101-341 433 MHz Empfänger

#define RELAIS_EIN HIGH
#define RELAIS_AUS LOW
bool rel = false; 
bool fehler = true;
volatile char focus;
//************************************** Sensoren **********************************************************
enum OutsideTempSource {
  OUTSIDE_TEMP_SOURCE_DHT22 = 0,
  OUTSIDE_TEMP_SOURCE_433MHZ = 1,
  OUTSIDE_TEMP_SOURCE_433_TFA = 2,
  OUTSIDE_TEMP_SOURCE_433_TFA3249 = 3
};

#define DHTTYPE_1 DHT22           // DHT 22 
#define DHTTYPE_2 DHT22           // DHT 22  
DHT dht1(DHTPIN_1, DHTTYPE_1);    //Der Innensensor wird ab jetzt mit dht1 angesprochen
DHT dht2(DHTPIN_2, DHTTYPE_2);    //Der Außensensor wird ab jetzt mit dht2 angesprochen
float t1, h1, t2, h2, Taupunkt_1, Taupunkt_2, DeltaTP;  // Variablen für die Sensor und Taupunktwerte
OutsideTempSource outsideTempSource = OUTSIDE_TEMP_SOURCE_DHT22;
int RF433_sensor_id = 0x61;
int RF433_tfa_channel = 2; // sensor dial 1 → CH2, dial 2 → CH3, dial 3 → CH4
unsigned long RF433_max_age_ms = 120000; // TFA 30.3249/30.3250 senden nur alle ~50-60s

// ***************************   Korrekturwerte der einzelnen Sensoren*****  ********************************
float Korrektur_t_1 = 0;   // Korrekturwert Innensensor Temperatur
float Korrektur_t_2 = 0;   // Korrekturwert Außensensor Temperatur
float Korrektur_h_1 = 0;   // Korrekturwert Innensensor Luftfeuchtigkeit
float Korrektur_h_2 = 0;   // Korrekturwert Außensensor Luftfeuchtigkeit

//*********************** Minimal- / Maximalwerte vom Relais **********************************************
float SCHALTmin =  3.0;             // minnimaler Taupuntunterschied, bei dem das Relais schaltet
float HYSTERESE =  1.0;             // Abstand von Ein- und Ausschaltpunkt        ==> fest, nicht einstellbar
int   Luefter_Mindestlaufzeit = 5;  // Mindestlaufzeit vom Lüfter in Minuten
float TEMP1_min = 10.0;             // Minnimale Innentemperatur, bei der die Lüftung aktiviert wird
float TEMP1_max = 30.0;             // Maximale Innentemperatur, bei der die Lüftung aktiviert wird
float TEMP2_min =-30.0;             // Minnimale Außentemperatur, bei der die Lüftung aktiviert wird
float TEMP2_max = 30.0;             // Maximale Außentemperatur, bei der die Lüftung aktiviert wird

//********************************* "Schalter" *************************************************************
bool use_WIFI = false;
int Datenspeicher = 400; // Wie viele Datensätze sollen gespeichert werden / 0 = Datenspeicherung deaktiviert
bool use_Charts = false;
bool use_Ampel = false;
bool debugging = false;
// ***************************** Variablen für die Diagramme ***********************************************
String Chart_TimeDate = "";
String Chart_temp_in = "";
String Chart_air_in  = "";
String Chart_tp_in   = "";
String Chart_temp_out= "";
String Chart_air_out = "";
String Chart_tp_out  = "";
String Chart_tp_delta= "";
//******************************** Lüfter-Laufzeit ********************************************************
String Chart_lz = "";
unsigned long Luefter_Laufzeit = 0;
//*********************************** Debugging ***********************************************************
 esp_task_wdt_config_t config = {
    .timeout_ms = 60000,                    // 60 Sekunden Toleranz
    .idle_core_mask = (1 << 0) | (1 << 1),  // Überwacht Idle-Tasks beider Kerne
    .trigger_panic = true                   // Reset bei Fehler
  };  

String FehlerString ="";
String OptionenString = "";
String SpeicherString = "";
String WIFI_Status_String = "";
 void Fehler_speichern(int Nummer);
String Get_Fehlertext_from_Nummer(int Fehlernummer);
void read_Fehler_from_FS();
//**************************** Radon **********************************************************************
String Chart_lzR = "";
bool Radonsignal = false;
bool Radonfunktionalitaet = false;
unsigned long Luefter_Radon_Start;
unsigned long Luefter_Laufzeit_Radon = 0;
//*************************** Variablen für Zeitfunktionen  ***********************************************
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
unsigned long previousMillis = 0;
unsigned long Luefter_Start  = 0;
long last_calculation = -10000;  // alle 10 Sekunden Taupunkt berechnen
//******************************************** Ende Variabeln ***********************************************

//-----------------------------------------------------------------------------------------------------------
//----------------------------- SETUP ----- Hinweis: Diese Task läuft auf Core-1-----------------------------
//-----------------------------------------------------------------------------------------------------------

void setup() 
{ 
  Serial.begin(115200);
  pinMode(RELAIPIN, OUTPUT);          // Relaispin als Output definieren
  digitalWrite(RELAIPIN, RELAIS_EIN); // Relais einschalten

  pinMode (debugingPin, INPUT_PULLUP);    // Taste zur Ausgabe von Debugging-Infos
  pinMode (Ionenkammer_pin,INPUT_PULLUP); //Radon Vorsorge
  Optionen_laden();
  
  if (use_WIFI == false) use_Charts = false;  //--- Charts geht nicht ohne WIFI!
  
   Serial_Debugging_println("");
  Serial_Debugging_println("----------------- Taupunktlüfter -----------------------------"); 
  Serial_Debugging_println("----------------  "  + String(Software_version) +  " ------------------------------");
  Serial_Debugging_println("");

  //-------------------------------- Watchdog -----------------------------------------------------------//
   esp_task_wdt_init(&config);               // Den Watchdog starten
   esp_task_wdt_add(NULL);                   // Den Haupt-Task (loop) registrieren
   esp_task_wdt_reconfigure(&config);
   esp_task_wdt_reset();
  Serial.print("Der task_wdt wurde auf "); Serial.print (config.timeout_ms); Serial.println(" Millisekunden gesetzt");
  //-------------------------------- Watchdog -----------------------------------------------------------//
      
  pinMode(DHT_Power_Pin, OUTPUT);    // Port, von dem die DHT-Sensoren den Strom bekommen
  digitalWrite(DHT_Power_Pin, HIGH); // Strom für DHT-Sensoren einschalten
  Encoder_initialisieren();
  reset_Encoder(0);
  
  timer_initialisieren();
  OLED_init();  // OLED initialisieren
  if (!LittleFS.begin(true)) Serial.println(F("Kritischer Fehler: LittleFS konnte nicht gemountet werden!"));  // Dateisystem überprüfen / erstellen

   setup_Ampel();
  //--------------------------------------------------- WIFI -------------------------------------------------------------------------------
  
 if (use_WIFI == true) 
 {  
  // Damit der Server bei Klein- und Großschreibung reagiert
  server.rewrite("/daten", "/Daten");
  server.rewrite("/optionen", "/Optionen");
  server.rewrite("/fehler", "/Fehler");
  server.rewrite("/speicher", "/Speicher");
  server.rewrite("/datendatei", "/Datendatei.txt");
  
  WIFI_Connect();  // WIFI-Verbindung herstellen und Server starten
 }
  print_Reset_reason(0); // Zur Fehlersuche: Warum ist der ESP gerade neu gestartet?

  //--------------------------------------------- Sensoren starten ------------------------------------------------------------------------
  dht1.begin(); 
  dht2.begin(); 
  AussenSensor_setup();
  
  digitalWrite(RELAIPIN, RELAIS_AUS); // Relais ausschalten
  Serial_Debugging_println("Relais ausgeschaltet");
 //---------------------------------------------- Chronik / Charts ----------------------------------------------------------------------------
  if (Datenspeicher > 0){ load_Chronik_from_LittleFS(); } // Alle Datensätze aus dem LittleFS-Speicher einlesen, für die Verwendung von Charts
  //-------------------------------------------- Debugging ------------------------------------------------------------------------------------
  //if (debugging) serial_debugging_all();
   focus = 0;         // Das Logo vom Taukmunktlüfter auf dem Display als Startbildschirm
   checkButton();
//------------------------------------------------------------ Ende Setup  --------------------------------------------------------------------
}

void loop() 
{    
 Watchdog_reset();
 Funk433_loop();
 if (use_WIFI == true) WIFI_watch_Task(); // Kontrolliert alle 60000 ms, ob WIFI noch vorhanden ist
 if ((last_calculation + 10000) < millis()) Taupunkte_berechnen(); // alle 10 Sekunden die Werte neu ermitteln  
 if ((digitalRead(debugingPin)) == LOW)serial_debugging_all();
 
 //------------------------------------------------- Radon funktionalität --------------------------------------------------------
 bool Radonsignal_old = Radonsignal;
 if (digitalRead(Ionenkammer_pin) == LOW){ Radonsignal = true; Radonfunktionalitaet = true;} else Radonsignal = false;
 if  (Radonsignal_old != Radonsignal) if(Radonsignal == true)
   {
      Fehler_speichern(400);
      Luefter_Radon_Start = millis();
         
   }else{
     Luefter_Laufzeit_Radon = millis() - Luefter_Radon_Start;
     Fehler_speichern(401);
    }
 //---------------------------------------------------------------------------------------------------------------------------------

 if ((int(focus)) < 4)  focus = get_Encoder_pos(0,3 ,true);  // Durchlauf durch die einzelnen Bildschirme
   switch (focus) 
 {
  case 0: OLED_Draw_Logo(); //-------------- Taupunkt Logo anzeigen
    break;    
  case 1:     //------------------------------ Temperatur- und Luftfeuchtigkeitswerte
     show_Wertebildschirm();
    break;    
  case 2:   //-------------------------------- Status- Bildschirmf
    show_Status();
    break;    
  case 3:
    show_Kontaktbildschirm(); //---------------- Kontaktdaten
    break;    
 case 'A':      //---------------------------- OptionsMenü
    show_menue();
    break;    
 case 'B':
    show_SubMenue(); //---------------------- jeweiliges Untermenü
    break;    
 case 'C':
    show_Eingabe_Bildschirm(); //------------ Eingabe der Optionswerte
    break;
  default:
     Set_LED1(); // dürfte eigentlich nie hier ankommen
    break; 
 }
}
