/*
 * =================================================================================
 * PROYECTO: ESP8266 DIRECT 16-PIN LCD - BLUE SCREEN DIAGNOSTIC & CONTRAST FIX
 * DESCRIPCIÓN: Solución completa para pantalla azul sin caracteres ("nada más se ve la pantalla azul").
 *
 * CAUSAS PRINCIPALES DE LA PANTALLA AZUL SIN TEXTO:
 *   1. PIN 3 (V0) SIN CONTRASTE: Para probar si la pantalla responde sin potenciómetro,
 *      conecta el PIN 3 (V0) DIRECTAMENTE A GND. Esto forzará el contraste al máximo
 *      y verás cuadritos negros (████████).
 *   2. PIN 5 (RW) FLOTANTE: El Pin 5 (RW) DEBE ir a GND obligatoriamente.
 *
 * ESQUEMA DE PINES PROBADO Y VERIFICADO (NodeMCU / WeMos D1 Mini):
 *   LCD Pin 1 (VSS)  ---> GND
 *   LCD Pin 2 (VDD)  ---> 5V (o 3.3V)
 *   LCD Pin 3 (V0)   ---> CONECTAR A GND DIRECTO (O Pata Central del Potenciómetro)
 *   LCD Pin 4 (RS)   ---> Pin D1 (GPIO 5)
 *   LCD Pin 5 (RW)   ---> GND (OBLIGATORIO)
 *   LCD Pin 6 (E)    ---> Pin D2 (GPIO 4)
 *   LCD Pin 11 (D4)  ---> Pin D3 (GPIO 0)
 *   LCD Pin 12 (D5)  ---> Pin D5 (GPIO 14)
 *   LCD Pin 13 (D6)  ---> Pin D6 (GPIO 12)
 *   LCD Pin 14 (D7)  ---> Pin D7 (GPIO 13)
 *   LCD Pin 15 (LED+)---> 5V
 *   LCD Pin 16 (LED-)---> GND
 * =================================================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LiquidCrystal.h>

// Pines para modo 4-bits: LiquidCrystal(RS, E, D4, D5, D6, D7)
// D1=5, D2=4, D3=0, D5=14, D6=12, D7=13
LiquidCrystal lcd(5, 4, 0, 14, 12, 13);

const char* AP_SSID = "HACKER-LCD-DIRECT";
const char* AP_PASS = "";
const byte DNS_PORT = 53;

DNSServer dnsServer;
ESP8266WebServer server(80);

String line1Text = "LCD DIAGNOSTIC";
String line2Text = "TEXT IS VISIBLE!";

int scrollPos1 = 0;
int scrollPos2 = 0;
unsigned long lastScrollTime = 0;

void updateLCDDisplay() {
  if (millis() - lastScrollTime < 300) return;
  lastScrollTime = millis();

  lcd.setCursor(0, 0);
  if (line1Text.length() <= 16) {
    String padded = line1Text;
    while (padded.length() < 16) padded += " ";
    lcd.print(padded.substring(0, 16));
  } else {
    scrollPos1++;
    if (scrollPos1 > (int)line1Text.length()) scrollPos1 = 0;
    String displayStr = line1Text + "   " + line1Text;
    lcd.print(displayStr.substring(scrollPos1, scrollPos1 + 16));
  }

  lcd.setCursor(0, 1);
  if (line2Text.length() <= 16) {
    String padded = line2Text;
    while (padded.length() < 16) padded += " ";
    lcd.print(padded.substring(0, 16));
  } else {
    scrollPos2++;
    if (scrollPos2 > (int)line2Text.length()) scrollPos2 = 0;
    String displayStr = line2Text + "   " + line2Text;
    lcd.print(displayStr.substring(scrollPos2, scrollPos2 + 16));
  }
}

const char HTML_INDEX[] PROGMEM = R"HTML_CONTENT(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 Blue Screen Fix</title>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@700;900&display=swap" rel="stylesheet">
  <style>
    :root {
      --neon-green: #00ff66;
      --bg: #030805;
      --panel: #07140b;
      --accent: #ff0055;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Outfit', sans-serif; }
    body { background: var(--bg); color: #fff; padding: 16px; min-height: 100vh; display: flex; justify-content: center; align-items: center; }
    .container { width: 100%; max-width: 480px; }

    .header { text-align: center; margin-bottom: 20px; }
    .header h1 { font-size: 26px; font-weight: 900; color: var(--neon-green); text-shadow: 0 0 20px var(--neon-green); }
    .header p { color: #80ffaa; font-size: 14px; font-weight: 700; margin-top: 4px; }

    .lcd-box {
      background: #002b0f; border: 4px solid var(--neon-green); border-radius: 12px; padding: 16px;
      box-shadow: 0 0 30px rgba(0,255,102,0.4); text-align: center; margin-bottom: 20px;
      font-family: monospace; color: #00ff66; font-size: 20px; font-weight: bold; letter-spacing: 2px;
    }
    .lcd-line { background: #001a0a; border: 1px solid #005522; padding: 8px; margin: 4px 0; border-radius: 6px; overflow: hidden; white-space: nowrap; }

    .card { background: var(--panel); border: 1px solid var(--neon-green); border-radius: 16px; padding: 20px; margin-bottom: 16px; }
    .input-group { display: flex; gap: 8px; margin-top: 10px; }
    .input-group input { flex: 1; padding: 12px; border-radius: 10px; border: 1px solid var(--neon-green); background: #000; color: #fff; font-size: 16px; outline: none; }
    .input-group button { padding: 12px 18px; border-radius: 10px; border: none; background: var(--neon-green); color: #000; font-weight: 900; cursor: pointer; }
  </style>
</head>
<body>

<div class="container">
  <div class="header">
    <h1>💡 BLUE SCREEN CONTRAST FIX</h1>
    <p>ESP8266 Direct 16-Pin Diagnostic</p>
  </div>

  <div class="lcd-box">
    <div class="lcd-line" id="preview-1">LCD DIAGNOSTIC</div>
    <div class="lcd-line" id="preview-2">TEXT IS VISIBLE!</div>
  </div>

  <div class="card">
    <div style="font-weight:900; color:var(--neon-green); margin-bottom:8px;">ENVIAR MENSAJE A PANTALLA</div>
    <div class="input-group">
      <input type="text" id="msg-input" placeholder="Escribe algo..." onkeypress="if(event.key==='Enter') sendText()">
      <button onclick="sendText()">ENVIAR</button>
    </div>
  </div>
</div>

<script>
  const sendText = () => {
    const val = document.getElementById('msg-input').value.trim();
    if (val.length > 0) {
      document.getElementById('preview-1').innerText = val;
      fetch('/set?text=' + encodeURIComponent(val));
      document.getElementById('msg-input').value = '';
    }
  };
</script>

</body>
</html>
)HTML_CONTENT";

void handleRoot() { server.send(200, "text/html", FPSTR(HTML_INDEX)); }

void handleSet() {
  if (server.hasArg("text")) {
    String text = server.arg("text");
    line1Text = text;
    line2Text = "LIVE STREAM...";
    scrollPos1 = 0;
    scrollPos2 = 0;
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\"}");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[ESP8266 Blue Screen Diagnostic]");

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("* LCD DIAGNOSTIC *");
  lcd.setCursor(0, 1);
  lcd.print("TEXT IS VISIBLE!");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("[WebServer] Dashboard listo en 192.168.4.1");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  updateLCDDisplay();

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("LIVE:")) {
      line1Text = input.substring(5);
      line2Text = "SERIAL STREAM";
      scrollPos1 = 0;
      scrollPos2 = 0;
    } else if (input.length() > 0) {
      line1Text = input;
      line2Text = "ESP8266 LCD";
      scrollPos1 = 0;
      scrollPos2 = 0;
    }
  }
}
