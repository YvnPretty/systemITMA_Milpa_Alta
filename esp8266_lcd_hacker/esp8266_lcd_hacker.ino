/*
 * =================================================================================
 * PROYECTO: ESP8266 HACKER LCD DUAL-ADDRESS HIGH VISIBILITY (v3.0)
 * DESCRIPCIÓN: Inicializa simultáneamente las direcciones I2C 0x27 y 0x3F, hace
 *              parpadear la retroiluminación y muestra texto de alto contraste.
 *              - Conexiones I2C: SDA = Pin D2 (GPIO 4), SCL = Pin D1 (GPIO 5).
 * =================================================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SDA_PIN 4
#define SCL_PIN 5

// Inicializamos ambas direcciones comunes (0x27 y 0x3F)
LiquidCrystal_I2C lcd27(0x27, 16, 2);
LiquidCrystal_I2C lcd3F(0x3F, 16, 2);
LiquidCrystal_I2C lcd3C(0x3C, 16, 2);

const char* AP_SSID = "HACKER-LCD-ESP8266";
const char* AP_PASS = "";
const byte DNS_PORT = 53;

DNSServer dnsServer;
ESP8266WebServer server(80);

String line1Text = "HACKER ESP8266";
String line2Text = "IP: 192.168.4.1";

int scrollPos1 = 0;
int scrollPos2 = 0;
unsigned long lastScrollTime = 0;

void printToAllLCDs(const char* l1, const char* l2) {
  // Enviar a 0x27
  lcd27.clear();
  lcd27.setCursor(0, 0);
  lcd27.print(l1);
  lcd27.setCursor(0, 1);
  lcd27.print(l2);

  // Enviar a 0x3F
  lcd3F.clear();
  lcd3F.setCursor(0, 0);
  lcd3F.print(l1);
  lcd3F.setCursor(0, 1);
  lcd3F.print(l2);

  // Enviar a 0x3C
  lcd3C.clear();
  lcd3C.setCursor(0, 0);
  lcd3C.print(l1);
  lcd3C.setCursor(0, 1);
  lcd3C.print(l2);
}

void initAllLCDs() {
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(200);

  // Inicializar 0x27
  lcd27.init();
  lcd27.backlight();

  // Inicializar 0x3F
  lcd3F.init();
  lcd3F.backlight();

  // Inicializar 0x3C
  lcd3C.init();
  lcd3C.backlight();

  // Parpadeo de confirmación visual (3 veces)
  for (int i = 0; i < 3; i++) {
    lcd27.noBacklight(); lcd3F.noBacklight(); lcd3C.noBacklight();
    delay(150);
    lcd27.backlight(); lcd3F.backlight(); lcd3C.backlight();
    delay(150);
  }

  printToAllLCDs("* HACKER SYSTEM *", "IP: 192.168.4.1");
}

void updateLCDDisplay() {
  if (millis() - lastScrollTime < 300) return;
  lastScrollTime = millis();

  // Formatear línea 1 a 16 caracteres
  String s1 = line1Text;
  if (s1.length() < 16) {
    while (s1.length() < 16) s1 += " ";
  } else if (s1.length() > 16) {
    scrollPos1++;
    if (scrollPos1 > (int)s1.length()) scrollPos1 = 0;
    String displayStr = s1 + "   " + s1;
    s1 = displayStr.substring(scrollPos1, scrollPos1 + 16);
  }

  // Formatear línea 2 a 16 caracteres
  String s2 = line2Text;
  if (s2.length() < 16) {
    while (s2.length() < 16) s2 += " ";
  } else if (s2.length() > 16) {
    scrollPos2++;
    if (scrollPos2 > (int)s2.length()) scrollPos2 = 0;
    String displayStr = s2 + "   " + s2;
    s2 = displayStr.substring(scrollPos2, scrollPos2 + 16);
  }

  printToAllLCDs(s1.c_str(), s2.c_str());
}

// ---------------------------------------------------------------------------------
// DASHBOARD WEB CON FORZADO VISUAL HIGH CONTRAST
// ---------------------------------------------------------------------------------
const char HTML_INDEX[] PROGMEM = R"HTML_CONTENT(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 LCD Teleprompter & Visual Fix</title>
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

    .btn-row { display: flex; gap: 10px; margin-top: 10px; }
    button.btn {
      flex: 1; padding: 14px; border: none; border-radius: 12px; font-size: 15px; font-weight: 900;
      cursor: pointer; background: var(--neon-green); color: #000; text-transform: uppercase;
      box-shadow: 0 0 15px rgba(0,255,102,0.3);
    }
    button.btn-accent { background: var(--accent); color: #fff; box-shadow: 0 0 15px rgba(255,0,85,0.4); }

    .input-group { display: flex; gap: 8px; margin-top: 10px; }
    .input-group input { flex: 1; padding: 12px; border-radius: 10px; border: 1px solid var(--neon-green); background: #000; color: #fff; font-size: 16px; outline: none; }
    .input-group button { padding: 12px 18px; border-radius: 10px; border: none; background: var(--neon-green); color: #000; font-weight: 900; cursor: pointer; }
  </style>
</head>
<body>

<div class="container">
  <div class="header">
    <h1>💀 ESP8266 LCD VISUAL FIX</h1>
    <p>Controlador Dual 0x27 / 0x3F para Pantalla LCD</p>
  </div>

  <div class="lcd-box">
    <div class="lcd-line" id="preview-1">HACKER ESP8266</div>
    <div class="lcd-line" id="preview-2">IP: 192.168.4.1</div>
  </div>

  <div class="card">
    <div style="font-weight:900; color:var(--neon-green); margin-bottom:8px;">💡 FORZAR PARPADEO DE LUZ</div>
    <div class="btn-row">
      <button class="btn" onclick="fetch('/blink')">⚡ PROBAR PARPADEO</button>
    </div>
  </div>

  <div class="card">
    <div style="font-weight:900; color:var(--neon-green); margin-bottom:8px;">ENVIAR MENSAJE DIRECTO</div>
    <div class="input-group">
      <input type="text" id="msg-input" placeholder="Escribe un texto..." onkeypress="if(event.key==='Enter') sendText()">
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

void handleBlink() {
  for (int i = 0; i < 5; i++) {
    lcd27.noBacklight(); lcd3F.noBacklight(); lcd3C.noBacklight();
    delay(100);
    lcd27.backlight(); lcd3F.backlight(); lcd3C.backlight();
    delay(100);
  }
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

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
  Serial.println("\n[ESP8266 Dual Address LCD v3.0]");

  initAllLCDs();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/blink", handleBlink);
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
