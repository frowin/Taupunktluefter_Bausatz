//////////////////////////////////////////////////////////////////////////////
// ESP32-TaupunktLüfter 
// mit dem ESP32 Wroom 32
// für Arduino-ESP32 Release v3.3.8 
//
// Ulrich Schmerold
// 06/2026
//////////////////////////////////////////////////////////////////////////////
//**************************************************************************************************************
// Hinweise Sonderzeichen:ä = &aumln / ü = &uuml; / ö = &ouml /ß = &szlig / °C = &degC / Pfeil = &#10144 / Delts = &#916  **
//**************************************************************************************************************
#define Radon_Taste "<tr><td colspan=\"2\" class=\"Radon-row\" style=\"background:%Color-Radon%;\">%Radon%</td></tr>"

static const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
 <meta charset="UTF-8">
 <meta name="viewport" content="width=device-width,initial-scale=1">
 <link rel="icon" href="data:,">
 <title>TauPunktLüfter</title>
  
<script src="https://cdn.jsdelivr.net/npm/hammerjs@2.0.8/hammer.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.3"></script>
<script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom@2.0.0/dist/chartjs-plugin-zoom.min.js"></script>

<style>
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;margin:0;padding:10px;display:flex;flex-direction:column;align-items:center;margin-bottom:80px;}
    
 /* Datentabelle */
 .data-table {width:95%%;max-width:450px;margin-left:auto;margin-right: auto;border-collapse:separate;border-spacing:4px;background-color:white;border-radius:15px;overflow:hidden;box-shadow:0 8px 16px rgba(0,0,0,0.15);margin-bottom:20px;}
 .lbl-in{background:#7A4EBA;color:white;font-size:20pt;font-weight:bold;text-align:center;padding:10px;width:50%%;}
 .lbl-out{background:#41B853;color:white;font-size:20pt;font-weight:bold;text-align:center;padding:10px;width:50%%;}
 .row-header{background:#C6D9F1;color:#333;font-size:16pt;font-weight:bold;padding:8px 15px;text-align:left;}
 .val-in{background:#7A4EBA;color:white;font-size:22pt;text-align:center;padding:10px;}
 .val-out{background:#41B853;color:white;font-size:22pt;text-align:center;padding:10px;}
 .Radon-row{color:white;font-size:20pt;font-weight:bold;text-align:center;padding:15px;}
 .status-row{color:white;font-size:20pt;font-weight:bold;text-align:center;padding:15px;}
    
/* Chart Layout */
 .chart-wrapper{width:95%%;max-width:95%%; margin: 0 auto 50px;background:white;padding:15px;border:2px solid blue;border-radius:15px;box-sizing:border-box;position:relative;box-shadow: 0 4px 10px rgba(0,0,0,0.1);}
 .chart-header{display:flex;align-items:center;justify-content:center;width:100%%;margin-bottom:12px; height:45px}
 .chart-title{font-size:1.2rem;color:#0000FF;background-color:#E6F0FF;padding:6px 22px;border-radius:50px;border:2px solid #0000FF; margin: 0;}
    
/* Reset-Taste genau auf dem Rahmen */
 .zoom-btn{position:absolute;top:-18px;right:25px;padding:7px 15px; cursor:pointer;background:#007bff;border:2px solid white;border-radius:20px;color:white;font-size:1rem;font-weight:bold;z-index:10;box-shadow: 0 2px 4px rgba(0,0,0,0.2);}
 .zoom-btn:hover{background: #0056b3;}
 .chart-container{position:relative;height:350px;width:100%%;}

/* Styling für die Navigationsleisten */
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;margin:0;padding:15px;padding-bottom:100px !important;display:block !important;}
  .bottom-nav{position:fixed !important;bottom:5px !important;left:2.5%%;width:95%%;height:35px;display:flex;background-color:#2c3e50;box-shadow:0 -4px 15px rgba(0,0,0,0.5);z-index:99999;border-radius:12px;border:1px solid #ffffff;overflow:hidden;}
  .nav-item{flex:1;text-align:center;line-height:35px;color: white;text-decoration:none;font-weight:bold;font-size:11pt;border-right:1px solid #444;border-right: 1px solid #bdc3c7;}
  .nav-item:last-child{border-right:none;}
  .nav-item:active{background-color: #1a252f;}
  .header-box{margin-bottom:20px;}pre{background:rgba(255,255,255,0.4);padding:10px;border-radius:5px;white-space:pre-wrap;word-wrap:break-word;}
 </style>
</head>
<body>

<div style="text-align:center;margin-top:0px;margin-bottom:10px;font-family:Arial,sans-serif;">
 <div style="display:inline-block;background-color:#ffffff;padding:20px 40px;border-radius:10px;border-bottom:5px solid #007bff;box-shadow: 0 4px 12px rgba(0,0,0,0.1);">
  <h1 style="margin:0;color:#007bff;font-size:2.0rem;letter-spacing:2px;">   TAUPUNKTLÜFTER   </h1>
  <div style="margin-top:5px;font-size:1.1rem;color:#666;font-weight:bold;letter-spacing:1px;">
   Software-Version %Version%
</div></div></div>
 <p style="margin: 0 0 5px 0;font-weight:bold;color:#333;text-align:center;">Letztes Daten-Update: %Stamp%</p>

<table class="data-table">
 <tr><td class="lbl-in">Innen</td><td class="lbl-out">Außen</td></tr>
 <tr><td colspan="2" class="row-header">Temperatur:</td></tr>
 <tr><td class="val-in">%T1% &deg;C</td><td class="val-out">%T2% &deg;C</td></tr>
 <tr><td colspan="2" class="row-header">Luftfeuchtigkeit:</td></tr>
 <tr><td class="val-in">%H1% %%</td><td class="val-out">%H2% %%</td></tr>
 <tr><td colspan="2" class="row-header">Taupunkt:</td></tr>
 <tr><td class="val-in">%TP1% &deg;C</td><td class="val-out">%TP2% &deg;C</td></tr>
 <tr><td colspan="2" class="row-header" style="text-align:center;">&#916 TP: %DeltaTP% &deg;C</td></tr>
 <tr><td colspan="2" class="status-row" style="background:%Color-1%;">%Rel%</td></tr>
 %Radonfeld%
</table>  

<div class="chart-wrapper">
 <button class="zoom-btn" onclick="chart1.resetZoom()">Zoom Zur&uumlcksetzen</button>
 <div class="chart-header"><h2 class="chart-title">Temperaturen</h2></div>
 <div class="chart-container"><canvas id="myChart_1"></canvas></div>
</div>

<div class="chart-wrapper">
 <button class="zoom-btn" onclick="chart2.resetZoom()">Zoom Zur&uumlcksetzen</button>
 <div class="chart-header"><h2 class="chart-title">Luftfeuchtigkeit</h2></div>
 <div class="chart-container"><canvas id="myChart_2"></canvas></div>
</div>

<div class="chart-wrapper">
 <button class="zoom-btn" onclick="chart3.resetZoom()">Zoom Zur&uumlcksetzen</button>
 <div class="chart-header"><h2 class="chart-title">Taupunkte</h2></div>
 <div class="chart-container"><canvas id="myChart_3"></canvas></div>
</div>

<div class="chart-wrapper">
 <div class="chart-header"><h2 class="chart-title">Lüfterlaufzeit</h2></div>
 <div class="chart-container"><canvas id="runtimeChart"></canvas></div>
</div>

<script>
Chart.register(ChartZoom);
const meineGlobalenLabels=[%Chart_TimeDate%];
const baseOpt={responsive:true,maintainAspectRatio:false,interaction:{intersect:false,mode:'index'},plugins:{legend:{position:'bottom'},zoom:{pan:{enabled:true,mode:'xy',threshold:20},zoom:{wheel:{enabled:true},pinch:{enabled:true},mode:'xy'},limits:{x:{min:'original',max:'original',minRange:20},y:{min:-25,max:100,minRange:30}}}}};

var chart1=new Chart(document.getElementById('myChart_1'),{
 type:'line',
 data:{labels: meineGlobalenLabels,datasets:[
 {label:'In',data:[%Chart_temp_in%],borderColor:'blue',backgroundColor:'rgba(0,0,255,0.1)',pointRadius:0,borderWidth:2},
 {label:'Out',data:[%Chart_temp_out%],borderColor:'green',backgroundColor:'rgba(0,255,0,0.1)',pointRadius:0,borderWidth:2}]},
  options:{...baseOpt,plugins:{...baseOpt.plugins,zoom:{...baseOpt.plugins.zoom,limits:{...baseOpt.plugins.zoom.limits,y:{min:-25,max:50,minRange:5}}}},scales:{
  x:{type:'category',ticks:{maxTicksLimit:30}},
  y:{min:-20,max:50}}}
});

var chart2=new Chart(document.getElementById('myChart_2'),{
 type:'line',
 data:{labels:meineGlobalenLabels,datasets:[
  {label:'In %%', data:[%Chart_air_in%],borderColor:'blue',backgroundColor:'rgba(0,0,255,0.1)',pointRadius:0, borderWidth:2},
  {label:'Out %%', data:[%Chart_air_out%],borderColor:'green',backgroundColor:'rgba(0,255,0,0.1)',pointRadius:0, borderWidth:2}]},
  options:{...baseOpt,plugins:{...baseOpt.plugins,zoom:{...baseOpt.plugins.zoom,limits:{...baseOpt.plugins.zoom.limits,y:{min:0,max:100,minRange:5}}}},scales:{
  x:{ type:'category',ticks:{maxTicksLimit:30}},
  y:{ min:0,max:100}}}
});

var chart3=new Chart(document.getElementById('myChart_3'),{
 type:'line',
 data:{labels:meineGlobalenLabels,datasets:[
  {label:'TP In',data:[%Chart_tp_in%],borderColor:'blue',backgroundColor:'rgba(0,0,255,0.1)',pointRadius:0,borderWidth:2},
  {label:'TP Out',data:[%Chart_tp_out%],borderColor:'green',backgroundColor:'rgba(0,255,0,0.1)',pointRadius:0,borderWidth:2},
  {label:'\u0394TP   ',data:[%Chart_tp_delta%],borderColor:'red',backgroundColor:'rgba(255,0,0,0.1)',pointRadius:0,borderWidth:1},
  {label:'Lüfter Ein',data: new Array(meineGlobalenLabels.length).fill(Number([%Grenze_ein%])),borderColor:'rgba(255,0,0,0.5)',borderWidth:1,borderDash:[5,5],pointRadius:0,fill:false},
  {label:'Lüfter Aus',data: new Array(meineGlobalenLabels.length).fill(Number([%Grenze_aus%])),borderColor:'black',borderWidth:1,borderDash:[5,5],pointRadius:0,fill:false}]},
  options:{...baseOpt,plugins:{...baseOpt.plugins,zoom:{...baseOpt.plugins.zoom,limits:{...baseOpt.plugins.zoom.limits,y:{ min:-5,max:25, minRange:5}}}},scales:{
  x:{type:'category',ticks:{maxTicksLimit:30}},
  y:{min:-5,max:25}}}
});

const ctx=document.getElementById('runtimeChart').getContext('2d');
 var runtimeChart=new Chart(ctx,{
  type:'bar',
  data:{labels:meineGlobalenLabels,
  datasets:[{label:'Lüfterlaufzeit (Minuten)',data:[%Chart_Luefter%],backgroundColor:'rgba(54,162,235,0.7)',borderColor:'rgba(54,162,235,1)',borderWidth:1},
  {label:'Radon Laufzeit',data:[%Chart_LuefterR%],backgroundColor:'rgba(255,0,0,0.7)',borderColor:'rgba(255,0,0,1)',borderWidth:1}]},
  options:{responsive:true,maintainAspectRatio:false,scales:{
    y:{beginAtZero:true,title:{display:true,text:'Minuten'}}},
    plugins:{legend:{display:true,position:'bottom'}}}
  });

</script>

<!-- Fußzeile -->
<div style="font-family:Arial,sans-serif;width:100%%;max-width:800px;box-sizing:border-box;padding:20px;background-color:#ffffff;border-left:5px solid #007bff; border-radius:8px;box-shadow: 0 4px 12px rgba(0,0,0,0.1);line-height:1.6;color:#333;margin:20px auto;"><h3 style="margin-top:0;color:#007bff;margin-bottom:10px;">Ulrich Schmerold<br>Programmtechnik und Elektronik</h3>
 <a href="mailto:info@taupunktluefter.de" style="display:block;color:#007bff;text-decoration:none;font-size:14px;">E-Mail: info@taupunktluefter.de</a>
 <a href="http://taupunktluefter.de" target="_blank" style="display:block;color:#007bff;text-decoration:none;font-size:14px;">Webseite: www.taupunktluefter.de</a>
 <a href="https://github.com" target="_blank" style="display:block;margin-top:10px;color:#007bff;font-weight:bold;">Quellcode, Handbuch und mehr... bei Github</a>
</div> 

<!-- Navigationsbalken -->
<nav class="bottom-nav">
 <a href="/" class="nav-item">TPL</a>
 <a href="/Fehler" class="nav-item">Fehler</a>
 <a href="/Daten" class="nav-item">Daten</a>
 <a href="/Speicher" class="nav-item">Speicher</a>
 <a href="/Optionen" class="nav-item">Optionen</a>
</nav>

</body>
</html>
%Seitenende%
)rawliteral";
//--------------------------------------------------- Ende rawliteral Iindex_html[]----------------------------------------------------------

int groesse_raw_Literal()
{
 const size_t index_html_size = sizeof(index_html);
  long Daten_size = Chart_TimeDate.length() + Chart_temp_in.length()+ Chart_air_in.length() + Chart_tp_in.length() + Chart_tp_in.length() + Chart_temp_out.length() + Chart_air_out.length() + Chart_tp_out.length() + Chart_tp_delta.length();
   Serial.println("* Größe der index_HTML-Seite: " + String(index_html_size)+ "byte"); 
   Serial.println("* Größe der Datenstrams: " + String(Daten_size)+ "byte"); 
  return index_html_size;
}

//--------------------------------------------------------------------------- Daten ausgeben ----------------------------------
char Daten[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
 <meta charset="UTF-8">
   <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
 <style>
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;}
  .btn{color:white; border:none; padding:8px 8px;cursor:pointer;border-radius:4px;margin-right:5px;text-decoration:none;display:inline-block;font-weight:bold;}
  .data-btn{background-color:#f0ad4e;}   /* Gelb für Daten */
  .restart-btn {background-color:#ff4c4c;}
  .btn:hover{opacity: 0.8;}
   body{background-color:#A2C2E8;font-family:Arial,sans-serif;margin:0;padding:15px;padding-bottom:100px !important;display:block !important;}

  .bottom-nav{position:fixed !important;bottom:5px !important;left:2.5%%;width:95%%;height:35px;display:flex;background-color:#2c3e50;box-shadow:0 -4px 15px rgba(0,0,0,0.5);z-index:99999;border-radius:12px;border:1px solid #ffffff;overflow:hidden;}
  .nav-item{flex:1;text-align:center;line-height:35px;color: white;text-decoration:none;font-weight:bold;font-size:11pt;border-right:1px solid #444;border-right: 1px solid #bdc3c7;}
  .nav-item:last-child{border-right:none;}
  .nav-item:active{background-color: #1a252f;}
  .header-box{margin-bottom:20px;}pre{background:rgba(255,255,255,0.4);padding:10px;border-radius:5px;white-space:pre-wrap;word-wrap:break-word;}
 </style>
</head>
<body>
<div style="line-height: 1.1;">
  <font size="6"><b>TauPunktL&uuml;fter</b></font>
  <font size="4"><b>V%Version%</b></font><br>  
  <div style="margin-top: 8px; margin-bottom:8px;">
  <font size="5"><b>Daten</b></font>
  <p>
  Datum/Zeit: %Chart_Anzahl_Werte_Date% (%LDZ% byte)<br> %Chart_TimeDate%<br><br>
  Innentemperatur: %Chart_Anzahl_Werte_ti% (%L% byte)<br> %Chart_temp_in%<br><br>
  Au&szlig;entemperatur: %Chart_Anzahl_Werte_ta% (%L% byte)<br> %Chart_temp_out%<br><br>
  Luftfeuchte innen: %Chart_Anzahl_Werte_hi% (%L% byte)<br> %Chart_air_in%<br><br>
  Luftfeuchte au&szlig;en: %Chart_Anzahl_Werte_ha% (%L% byte)<br> %Chart_air_out%<br><br>
  Taupunkt innen: %Chart_Anzahl_Werte_tpi% (%L% byte)<br> %Chart_tp_in%<br><br>
  Taupunkt au&szlig;en: %Chart_Anzahl_Werte_tpa% (%L% byte)<br> %Chart_tp_out%<br><br>
  Delta Taupunkt: %Chart_Anzahl_Werte_tpd% (%L% byte)<br> %Chart_tp_delta%<br><br>
  L&uuml;fter Laufzeit: %Chart_Anzahl_Werte_lz% (%L% byte)<br> %Chart_Luefter%<br><br>
  Radonsignal: %Chart_Anzahl_Werte_lzR% (%L% byte)<br> %Chart_LuefterR%<br><br>
  <button class="btn restart-btn" onclick="restartESP()">ESP Neustart</button>
  <button class="btn data-btn" onclick="clearData()">Daten l&oumlschen</button> 
  </p>   
 </div>

 <!-- Navigationsleiste -->
 <nav class="bottom-nav">
  <a href="/" class="nav-item">TPL</a>
  <a href="/Fehler" class="nav-item">Fehler</a>
  <a href="/Daten" class="nav-item">Daten</a>
  <a href="/Speicher" class="nav-item">Speicher</a>
  <a href="/Optionen" class="nav-item">Optionen</a>
 </nav>

 <script>
 function clearData(){if (confirm("Möchtest du wirklich alle Daten löschen?")){fetch('/cleardata').then(response => {location.reload();}).catch(err => console.log('Fehler beim Löschen:', err));}}
 function restartESP(){fetch('/restart');document.body.innerHTML= `<div style="text-align:center;margin-top:50px;font-family:sans-serif;"><h1>ESP startet neu...</h1><p>Bitte ca. 15 Sekunden warten, die Seite lädt dann neu.</p></div>`;
    setTimeout(() => {window.location.href = "/";},15000);
} </script>
</body>
</html>
%Seitenende%
)rawliteral";

//----------------------------------------------------------------------------- Daten als Datei  ------------------------------------
char Daten_Datei[] PROGMEM = R"rawliteral(
 Datum/Zeit: Anzahl Datensätze: %Chart_Anzahl_Werte_Date% (%LDZ% byte)\n
  %Chart_TimeDate%\n\n
 Innentemperatur: Anzahl Datensätze: %Chart_Anzahl_Werte_ti% (%L% byte)\n
  %Chart_temp_in%\n\n
 Außentemperatur: Anzahl Datenätze: %Chart_Anzahl_Werte_ta% (%L% byte)\n
  %Chart_temp_out%\n\n
 Luftfeuchte innen: Anzahl Datensätze: %Chart_Anzahl_Werte_hi% (%L% byte)\n
  %Chart_air_in%\n\n
 Luftfeuchte au&szligen: Anzahl Datensätze: %Chart_Anzahl_Werte_ha% (%L% byte)\n
  %Chart_air_out%\n\n
 Taupunkt innen: Anzahl Datensätze: %Chart_Anzahl_Werte_tpi% (%L% byte)\n
  %Chart_tp_in%\n\n
 Taupunkt au&szligen: Anzahl Datensätze: %Chart_Anzahl_Werte_tpa% (%L% byte)\n
 %Chart_tp_out%\n\n
 Delta Taupunkt: Anzahl Datensätze: %Chart_Anzahl_Werte_tpd% (%L% byte)\n
  %Chart_tp_delta%\n\n
 L&uumlfter Laufzeit: Anzahl Datensätze: %Chart_Anzahl_Werte_lz% (%L% byte)\n
  %Chart_Luefter%\n\n
 Radonsignal: Anzahl Datensätze: %Chart_Anzahl_Werte_lzR% (%L% byte)\n
  %Chart_LuefterR%\n\n
%Seitenende%
)rawliteral";

//--------------------------------------------------------------------- Optionen ausgeben ----------------------------------
char Optionen[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
 <head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
 <style>
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;margin:0;padding:15px;padding-bottom:100px !important;display:block !important;}
  .bottom-nav{position:fixed !important;bottom:5px !important;left:2.5%%;width:95%%;height:35px;display:flex;background-color:#2c3e50;box-shadow:0 -4px 15px rgba(0,0,0,0.5);z-index:99999;border-radius:12px;border:1px solid #ffffff;overflow:hidden;}
  .nav-item{flex:1;text-align:center;line-height:35px;color: white;text-decoration:none;font-weight:bold;font-size:11pt;border-right:1px solid #444;border-right: 1px solid #bdc3c7;}
  .nav-item:last-child{border-right:none;}
  .nav-item:active{background-color: #1a252f;}
  .header-box{margin-bottom:20px;}pre{background:rgba(255,255,255,0.4);padding:10px;border-radius:5px;white-space:pre-wrap;word-wrap:break-word;}
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;}
  .content { text-align: left; }
  .btn {color:white; border:none; padding:10px 20px;cursor:pointer;border-radius:4px;margin-right:5px;text-decoration:none;display:inline-block;}
  .pref-btn {background-color:#ff4c4c;}   /* Rot für Preferences */
  .save-btn {background-color:#007bff;}
  .rf-card{background:#fff;border-radius:8px;padding:12px;margin:12px 0;max-width:520px;box-shadow:0 4px 12px rgba(0,0,0,0.1);}
  .rf-card label{display:block;font-weight:bold;margin-top:8px;}
  .rf-card select,.rf-card input{font-size:16px;padding:6px;width:100%%;box-sizing:border-box;margin-top:4px;}
  .rf-status{background:#eef4ff;border-left:4px solid #007bff;padding:8px;margin-top:10px;}
 </style>

</head>
<body>
<div style="line-height: 1.1;">
  <font size="6"><b>TauPunktL&uuml;fter</b></font>
  <font size="4"><b>V%Version%</b></font><br>  
  <div style="margin-top: 8px; margin-bottom:8px;">
  <font size="5"><b>Optionen</b></font>
  <table class="MsoTableGrid" border="1" cellspacing="3" cellpadding="0">
    %Optionen_String%
  </table>
  <div class="rf-card">
    <form action="/saveOutsideTempSource" method="POST">
      <font size="4"><b>Au&szlig;en-Temperaturquelle</b></font>
      <label for="source">Quelle</label>
      <select id="source" name="source" onchange="updateRfFields()">
        <option value="DHT22" %OutsideSourceDHTSelected%>DHT22</option>
        <option value="433MHz" %OutsideSource433Selected%>433 MHz Acurite 606TX</option>
        <option value="433TFA" %OutsideSourceTfaSelected%>433 MHz TFA 30.3250 (nur Temp)</option>
        <option value="433TFA3249" %OutsideSourceTfa3249Selected%>433 MHz TFA 30.3249.02 (Temp+Feuchte)</option>
      </select>
      <div id="acuriteFields">
        <label for="rfid">433 Sensor ID</label>
        <input id="rfid" name="rfid" type="text" value="%RF433SensorID%">
      </div>
      <div id="tfaFields">
        <label for="tfach">TFA Kanal (Schalter am Sensor)</label>
        <select id="tfach" name="tfach">
          <option value="2" %TfaCh2Selected%>1 &rarr; CH2</option>
          <option value="3" %TfaCh3Selected%>2 &rarr; CH3</option>
          <option value="4" %TfaCh4Selected%>3 &rarr; CH4</option>
        </select>
      </div>
      <div class="rf-status">%RF433Status%</div>
      <p><button class="btn save-btn" type="submit">Speichern</button></p>
    </form>
  </div>
 </div> 
    <button class="btn pref-btn" onclick="clearPrefs()">Alle Optionen l&oumlschen</button>
  <!-- Navigationsleiste -->
 <nav class="bottom-nav">
  <a href="/" class="nav-item">TPL</a>
  <a href="/Fehler" class="nav-item">Fehler</a>
  <a href="/Daten" class="nav-item">Daten</a>
  <a href="/Speicher" class="nav-item">Speicher</a>
  <a href="/Optionen" class="nav-item">Optionen</a>
 </nav>
<script>
  function updateRfFields(){
    var s=document.getElementById('source').value;
    document.getElementById('acuriteFields').style.display=(s==='433MHz')?'block':'none';
    document.getElementById('tfaFields').style.display=(s==='433TFA'||s==='433TFA3249')?'block':'none';
  }
  updateRfFields();
  function clearPrefs(){if(confirm("Alle Einstellungen (Preferences) wirklich l&oumlschen? Dies setzt alle Optionen auf den Auslieferungszustand zur&uumlck!")) {fetch('/clearprefs').then(response => {location.reload();});}}
</script>
</body>
</html>
%Seitenende%
)rawliteral";

//------------------------------------------------------------------------- Speicher ausgeben ----------------------------------
char Speicher[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
 <head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
 <style>
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;margin:0;padding:15px;padding-bottom:100px !important;display:block !important;}
  .bottom-nav{position:fixed !important;bottom:5px !important;left:2.5%%;width:95%%;height:35px;display:flex;background-color:#2c3e50;box-shadow:0 -4px 15px rgba(0,0,0,0.5);z-index:99999;border-radius:12px;border:1px solid #ffffff;overflow:hidden;}
  .nav-item{flex:1;text-align:center;line-height:35px;color: white;text-decoration:none;font-weight:bold;font-size:11pt;border-right:1px solid #444;border-right: 1px solid #bdc3c7;}
  .nav-item:last-child{border-right:none;}
  .nav-item:active{background-color:#1a252f;}
  .header-box{margin-bottom:20px;}pre{background:rgba(255,255,255,0.4);padding:10px;border-radius:5px;white-space:pre-wrap;word-wrap:break-word;}
  </style>
</head> 

<body>
<div style="line-height: 1.1;">
  <font size="6"><b>TauPunktL&uuml;fter</b></font>
  <font size="4"><b>V%Version%</b></font><br>  
  <div style="margin-top: 8px; margin-bottom:8px;">
  <font size="5"><b>Speicher</b></font>
  <pre>%SpeicherString%</pre>
  </div>
</div>

 <!-- Navigationsleiste -->
  <nav class="bottom-nav">
  <a href="/" class="nav-item">TPL</a>
  <a href="/Fehler" class="nav-item">Fehler</a>
  <a href="/Daten" class="nav-item">Daten</a>
  <a href="/Speicher" class="nav-item">Speicher</a>
  <a href="/Optionen" class="nav-item">Optionen</a>
 </nav>
</body>
</html>
%Seitenende%
)rawliteral";

//---------------------------------------------------------------------- Fehler ----------------------------------------------------------------------------------------------------------------------
char Fehler[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
 <head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
 <style>
  body{background-color:#A2C2E8;font-family:Arial,sans-serif;margin:0;padding:15px;padding-bottom:100px !important;display:block !important;}
  .bottom-nav{position:fixed !important;bottom:5px !important;left:2%%;width:95%%;height:35px;display:flex;background-color:#2c3e50;box-shadow:0 -4px 15px rgba(0,0,0,0.5);z-index:99999;border-radius:12px;border:1px solid #ffffff;overflow:hidden;}
  .nav-item{flex:1;text-align:center;line-height:35px;color: white;text-decoration:none;font-weight:bold;font-size:11pt;border-right:1px solid #444;border-right: 1px solid #bdc3c7;}
  .nav-item:last-child{border-right:none;}
  .nav-item:active{background-color: #1a252f;}
  .header-box{margin-bottom:10px;}pre{background:rgba(255,255,255,0.4);padding:5px;border-radius:5px;white-space:pre;overflow-x: auto; font-size:10pt;padding:0 2px;}
 </style>
</head> 
<body>

<div style="line-height: 1.1;">
  <font size="6"><b>TauPunktL&uuml;fter</b></font>
  <font size="4"><b>V%Version%</b></font><br>  
  <div style="margin-top: 8px; margin-bottom:8px;">
  <font size="5"><b>Fehlerliste</b></font>
  <button style="padding:5px 12px;background:#f0ad4e;border:none;border-radius:4px;color:white;font-weight:bold;margin-left:10px;cursor:pointer;"onclick="clearLog()">
   Log l&ouml;schen
  </button>
  </div>
</div>

 <div align="left">
  <pre id="log">%Fehler%</pre>
   --- ENDE ---
 </div>

  <!-- Navigationsleiste -->
 <nav class="bottom-nav">
  <a href="/" class="nav-item">TPL</a>
  <a href="/Fehler" class="nav-item">Fehler</a>
  <a href="/Daten" class="nav-item">Daten</a>
  <a href="/Speicher" class="nav-item">Speicher</a>
  <a href="/Optionen" class="nav-item">Optionen</a>
 </nav>

 <script>
  function clearLog(){if(confirm("Log loeschen?")){fetch('/clearlog').then(()=>location.reload());}}
  
/* Ans Ende der Liste scrollen */
  window.onload=function(){setTimeout(function(){window.scrollTo(0,document.body.scrollHeight);},500);};
 </script>
</body>
</html>
%Seitenende%
)rawliteral";


String processor(const String& var)
{ 
 static unsigned int LDZ; //Länge der Datenreihe Datum/Zeit
 static unsigned int L; //Länge einer Datenreihe
 Watchdog_reset(); 

 String f;

 //-----------------------------------------  Variablen für die Tabelle in den HTML-Code einfügen  
   if(var == "Version") return Software_version;
   if(var == "T1")      return String(t1);
   if(var == "T2")      return String(t2);
   if(var == "H1")      return String(h1);
   if(var == "H2")      return String(h2);
   if(var == "TP1")     return String(Taupunkt_1);
   if(var == "TP2")     return String(Taupunkt_2);
   if(var == "DeltaTP") return String(DeltaTP);
   if(var == "Stamp")   return make_time_stamp();
   if(var == "OutsideSourceDHTSelected") if(outsideTempSource == OUTSIDE_TEMP_SOURCE_DHT22) return "selected"; else return "";
   if(var == "OutsideSource433Selected") if(outsideTempSource == OUTSIDE_TEMP_SOURCE_433MHZ) return "selected"; else return "";
   if(var == "OutsideSourceTfaSelected") if(outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA) return "selected"; else return "";
   if(var == "OutsideSourceTfa3249Selected") if(outsideTempSource == OUTSIDE_TEMP_SOURCE_433_TFA3249) return "selected"; else return "";
   if(var == "RF433SensorID") return rf433FormatId(RF433_sensor_id);
   if(var == "TfaCh2Selected") if(RF433_tfa_channel == 2) return "selected"; else return "";
   if(var == "TfaCh3Selected") if(RF433_tfa_channel == 3) return "selected"; else return "";
   if(var == "TfaCh4Selected") if(RF433_tfa_channel == 4) return "selected"; else return "";
   if(var == "RF433Status") return getRF433StatusText();
   
   if(var == "Color-1")      if(rel == true) return "#009231"; else return "#364592";
   if(var == "Rel")          if(rel == true) return "L&uumlfter ist in Betrieb"; else return "L&uumlfter ist nicht in Betrieb";
   if(var == "Color-Radon") if(Radonsignal == true) return "red"; else return "green";
   if(var == "Radon")       if(Radonsignal == true) return "! Radonkonzentration zu hoch !"; else return "Radonkonzentration okay &#10004";
   if(var == "Radonfeld")   if(Radonfunktionalitaet == true)return Radon_Taste; else return"";
   //-------------------------------------------- Sicherstellen, dass gerade keine Routine auf die Chronik-Daten zugreift -----
   if (chronik_is_busy)
   {
    Serial_Debugging_println("chronik_is_busy");
    HTML_processor_is_working = false; 
    return " ";
   }

   //--------------------------------------------------------------- Chronik Datensätze in den HTML-Code einfügen -------------
   if (use_Charts == true) {
        if(var == "Chart_TimeDate") return Chart_TimeDate;
        if(var == "Chart_temp_in")  { f = Chart_temp_in;  f.replace(" ",""); return f; }
        if(var == "Chart_temp_out") { f = Chart_temp_out; f.replace(" ",""); return f; }   
        if(var == "Chart_air_in")   { f = Chart_air_in;   f.replace(" ",""); return f; } 
        if(var == "Chart_air_out")  { f = Chart_air_out;  f.replace(" ",""); return f; } 
        if(var == "Chart_tp_in")    { f = Chart_tp_in;    f.replace(" ",""); return f; }
        if(var == "Chart_tp_out")   { f = Chart_tp_out;   f.replace(" ",""); return f; }
        if(var == "Chart_tp_delta") { f = Chart_tp_delta; f.replace(" ",""); return f; }
        if(var == "Chart_Luefter")  { f = Chart_lz;       f.replace(" ",""); return f; }
        if(var == "Chart_LuefterR") { f = Chart_lzR;      f.replace(" ",""); return f; }


        if(var == "Chart_Anzahl_Werte_Date") {LDZ =(Chart_TimeDate.length()+1);   return String( LDZ / Chart_TimeDate_length);}
        if(var == "Chart_Anzahl_Werte_ti")   {L =  (Chart_temp_in.length()+1);    return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_ta")   {L =  (Chart_temp_out.length()+1);   return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_hi")   {L =  (Chart_air_in.length()+1);     return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_ha")   {L =  (Chart_air_out.length()+1);    return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_tpi")  {L =  (Chart_tp_in.length()+1);      return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_tpa")  {L =  (Chart_tp_out.length()+1);     return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_tpd")  {L =  (Chart_tp_delta.length()+1);   return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_lz")   {L =  (Chart_lz.length()+1);         return String( L / laenge_Datensatz);}
        if(var == "Chart_Anzahl_Werte_lzR")  {L =  (Chart_lzR.length()+1);        return String( L / laenge_Datensatz);}
        if(var == "L") return String(L);
        if(var == "LDZ") return String(LDZ);
        if(var == "Grenze_aus") return String(SCHALTmin,1) ;
        if(var == "Grenze_ein") return String((SCHALTmin + HYSTERESE),1) ;
      };
  //------------------------------------------------------------------------------------------------------------------------------
   if(var == "Optionen_String") return OptionenString;
   if(var == "SpeicherString") return ("========= Speicher  =========<br>" + SpeicherString + "<br>==========  WIFI ===========<br>" + WIFI_Status_String);
   if(var == "Fehler") return FehlerString;
 //------------------------------------------------------------------------------------------------------------------------------
   if(var == "Seitenende"){ HTML_processor_is_working = false; 
                       Serial_Debugging_println("HTML_processor ist fertig!"); 
                      TONE (800, 200);
                       return "";
                    };   
   return String();
}


//----------------------------------------------------------------------------------------------------------------------------------
