/*
 * =================================================================================
 * PROYECTO: ESP8266 OLED Messenger (v3.1 - Tipografía Adaptativa y Diseño Premium)
 * DESCRIPCIÓN: Receptor de mensajes con tipografía inteligente para OLED 128x64.
 *              - Mensajes cortos: Texto Gigante (Size 2) para máxima visibilidad.
 *              - Mensajes largos: Formato multilínea (Size 1) con ajuste limpio.
 *              - Barra de encabezado invertida y banner de remitente estilo retro.
 * =================================================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* AP_SSID = "ESP8266-Messenger";
const char* AP_PASS = "";
const byte DNS_PORT = 53;

DNSServer dnsServer;
ESP8266WebServer server(80);

String currentSender = "SISTEMA";
String currentMessage = "¡Pantalla Lista! Conectate a WiFi ESP8266-Messenger";
int msgCount = 1;
bool oledReady = false;

// ---------------------------------------------------------------------------------
// RENDERIZADO CON TIPOGRAFÍA ADAPTATIVA Y DISEÑO PREMIUM
// ---------------------------------------------------------------------------------
void renderOLED() {
  if (!oledReady) return;

  display.clearDisplay();

  // 1. BARRA SUPERIOR DE ENCABEZADO (Fondo Invertido)
  display.fillRect(0, 0, 128, 13, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(3, 3);
  display.print("MESSENGER");

  // Conteo de mensajes alineado a la derecha
  String countStr = "#" + String(msgCount);
  display.setCursor(125 - (countStr.length() * 6), 3);
  display.print(countStr);

  // 2. SECCIÓN DEL REMITENTE (De: Nombre)
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 16);
  display.print("De: ");
  String sender = currentSender;
  if (sender.length() > 16) sender = sender.substring(0, 14) + "..";
  display.print(sender);

  // Línea divisora elegante
  display.drawFastHLine(0, 26, 128, SSD1306_WHITE);

  // 3. SECCIÓN DE MENSAJE ADAPTATIVA (Texto Grande vs Multilínea)
  if (currentMessage.length() <= 18) {
    // --- MODO TEXTO GIGANTE (Size 2) para mensajes cortos ---
    display.setTextSize(2);
    int textY = (currentMessage.length() <= 9) ? 38 : 31;
    display.setCursor(4, textY);
    display.println(currentMessage);
  } else {
    // --- MODO MULTILÍNEA LIMPIO (Size 1) para mensajes largos ---
    display.setTextSize(1);
    int y = 30;
    int charsPerLine = 20;
    String text = currentMessage;

    while (text.length() > 0 && y <= 54) {
      display.setCursor(4, y);
      String line = "";
      if (text.length() <= charsPerLine) {
        line = text;
        text = "";
      } else {
        int spaceIdx = text.lastIndexOf(' ', charsPerLine);
        if (spaceIdx > 0) {
          line = text.substring(0, spaceIdx);
          text = text.substring(spaceIdx + 1);
        } else {
          line = text.substring(0, charsPerLine);
          text = text.substring(charsPerLine);
        }
      }
      line.trim();
      display.println(line);
      y += 10; // Espaciado vertical entre líneas
    }
  }

  // 4. MARCO DECORATIVO CON ESQUINAS REDONDEADAS
  display.drawRoundRect(0, 0, 128, 64, 3, SSD1306_WHITE);

  display.display();
}

void notifyNewMessageAnimation() {
  if (!oledReady) return;
  for (int i = 0; i < 2; i++) {
    display.dim(true);
    delay(70);
    display.dim(false);
    delay(70);
  }
}

// ---------------------------------------------------------------------------------
// DASHBOARD WEB HTML/CSS/JS (EMBEDDED IN PROGMEM)
// ---------------------------------------------------------------------------------
const char HTML_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 OLED Messenger</title>
  <style>
    :root {
      --bg: #0b0f19;
      --panel: #131b2e;
      --border: #1e2942;
      --cyan: #00f3ff;
      --green: #10b981;
      --text: #f1f5f9;
      --muted: #64748b;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', system-ui, sans-serif; }
    body { background: var(--bg); color: var(--text); padding: 16px; min-height: 100vh; display: flex; justify-content: center; align-items: center; }
    .container { width: 100%; max-width: 480px; }
    
    .header { text-align: center; margin-bottom: 20px; }
    .header h1 { font-size: 22px; color: var(--cyan); letter-spacing: 1px; }
    .header p { color: var(--muted); font-size: 13px; margin-top: 4px; }
    
    .oled-box {
      background: #000;
      border: 3px solid #1e293b;
      border-radius: 12px;
      padding: 14px;
      margin-bottom: 20px;
      box-shadow: 0 0 20px rgba(0, 243, 255, 0.15);
      font-family: 'Courier New', monospace;
      color: #00f3ff;
    }
    .oled-status { display: flex; justify-content: space-between; font-size: 11px; border-bottom: 1px dashed #00f3ff55; padding-bottom: 6px; margin-bottom: 8px; }
    .oled-sender { font-size: 12px; font-weight: bold; color: #fff; margin-bottom: 6px; }
    .oled-body { font-size: 13px; line-height: 1.3; min-height: 52px; word-break: break-word; color: #7dd3fc; }
    
    .card { background: var(--panel); border: 1px solid var(--border); border-radius: 14px; padding: 20px; box-shadow: 0 10px 25px rgba(0,0,0,0.4); }
    .form-group { margin-bottom: 16px; }
    label { display: block; font-size: 12px; font-weight: 600; text-transform: uppercase; color: var(--muted); margin-bottom: 6px; }
    input, textarea {
      width: 100%; background: #080d1a; border: 1px solid var(--border); color: #fff;
      padding: 12px; border-radius: 8px; font-size: 14px; outline: none; transition: border 0.2s;
    }
    input:focus, textarea:focus { border-color: var(--cyan); }
    textarea { resize: vertical; min-height: 80px; }
    
    .presets { display: flex; gap: 6px; flex-wrap: wrap; margin-bottom: 16px; }
    .chip { background: #1e293b; color: #cbd5e1; border: 1px solid var(--border); padding: 6px 12px; border-radius: 20px; font-size: 12px; cursor: pointer; }
    .chip:hover { background: var(--cyan); color: #000; font-weight: bold; }
    
    .btn-group { display: flex; gap: 10px; }
    button {
      flex: 1; padding: 14px; border: none; border-radius: 10px; font-size: 14px; font-weight: bold;
      cursor: pointer; text-transform: uppercase;
    }
    .btn-send { background: linear-gradient(135deg, var(--cyan), #00a8ff); color: #000; }
    .btn-clear { background: #1e293b; color: #f87171; border: 1px solid #334155; }
    
    .footer-stats { display: flex; justify-content: space-between; margin-top: 16px; font-size: 11px; color: var(--muted); border-top: 1px solid var(--border); padding-top: 12px; }
    .alert { padding: 10px; border-radius: 8px; font-size: 12px; margin-bottom: 12px; display: none; text-align: center; font-weight: bold; }
    .alert-success { background: #064e3b; color: #34d399; }
    .alert-error { background: #7f1d1d; color: #f87171; }
  </style>
</head>
<body>

<div class="container">
  <div class="header">
    <h1>📟 OLED Messenger</h1>
    <p>Envío instantáneo de mensajes a pantalla ESP8266</p>
  </div>

  <div class="oled-box">
    <div class="oled-status">
      <span>✉️ MSG <b id="pv-count">#1</b></span>
      <span>📶 ESP8266 OLED</span>
    </div>
    <div class="oled-sender" id="pv-sender">De: SISTEMA</div>
    <div class="oled-body" id="pv-body">¡Pantalla Lista! Conectate a WiFi ESP8266-Messenger</div>
  </div>

  <div class="card">
    <div id="alert-box" class="alert"></div>

    <form id="msgForm">
      <div class="form-group">
        <label for="sender">Remitente / Nombre</label>
        <input type="text" id="sender" placeholder="Ej: Alex" value="Usuario" maxlength="20" required>
      </div>

      <div class="form-group">
        <label for="message">Mensaje para Pantalla OLED</label>
        <textarea id="message" placeholder="Escribe tu mensaje aquí..." maxlength="120" required></textarea>
      </div>

      <label>Mensajes Rápidos</label>
      <div class="presets">
        <span class="chip" onclick="setPreset('Hola! 👋')">👋 Hola!</span>
        <span class="chip" onclick="setPreset('Llego en 5 min 🚗')">🚗 Llego pronto</span>
        <span class="chip" onclick="setPreset('Llámame por favor 📞')">📞 Llámame</span>
        <span class="chip" onclick="setPreset('Entendido / OK 👍')">👍 OK</span>
        <span class="chip" onclick="setPreset('Urgente! Revisa chat 🚨')">🚨 Urgente</span>
      </div>

      <div class="btn-group">
        <button type="submit" class="btn-send">🚀 Enviar a OLED</button>
        <button type="button" class="btn-clear" onclick="clearOLED()">🧹 Limpiar</button>
      </div>
    </form>

    <div class="footer-stats">
      <span>Estado: <span style="color:#10b981;font-weight:bold">● En Línea (192.168.4.1)</span></span>
      <span id="msg-counter">Mensajes recibidos: 1</span>
    </div>
  </div>
</div>

<script>
  function setPreset(txt) { document.getElementById('message').value = txt; }
  function showAlert(msg, isSuccess = true) {
    const box = document.getElementById('alert-box');
    box.className = 'alert ' + (isSuccess ? 'alert-success' : 'alert-error');
    box.innerText = msg;
    box.style.display = 'block';
    setTimeout(() => { box.style.display = 'none'; }, 3000);
  }

  document.getElementById('msgForm').addEventListener('submit', function(e) {
    e.preventDefault();
    const sender = document.getElementById('sender').value.trim() || 'Anónimo';
    const message = document.getElementById('message').value.trim();

    if (!message) return;

    fetch('/send', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: 'sender=' + encodeURIComponent(sender) + '&msg=' + encodeURIComponent(message)
    })
    .then(res => res.json())
    .then(data => {
      if (data.status === 'ok') {
        showAlert('✅ ¡Mensaje enviado a la pantalla OLED!');
        document.getElementById('pv-sender').innerText = 'De: ' + data.sender;
        document.getElementById('pv-body').innerText = data.message;
        document.getElementById('pv-count').innerText = '#' + data.count;
        document.getElementById('msg-counter').innerText = 'Mensajes recibidos: ' + data.count;
        document.getElementById('message').value = '';
      } else { showAlert('❌ Error al enviar mensaje', false); }
    })
    .catch(err => { showAlert('❌ Error de conexión con ESP8266', false); });
  });

  function clearOLED() {
    fetch('/api/clear', { method: 'POST' })
    .then(res => res.json())
    .then(data => {
      showAlert('🧹 Pantalla OLED limpiada');
      document.getElementById('pv-sender').innerText = 'De: SISTEMA';
      document.getElementById('pv-body').innerText = '[Pantalla limpia]';
    });
  }

  function loadStatus() {
    fetch('/api/status').then(r => r.json()).then(data => {
      document.getElementById('pv-sender').innerText = 'De: ' + data.sender;
      document.getElementById('pv-body').innerText = data.message;
      document.getElementById('pv-count').innerText = '#' + data.count;
      document.getElementById('msg-counter').innerText = 'Mensajes recibidos: ' + data.count;
    }).catch(e => {});
  }
  window.onload = loadStatus;
</script>

</body>
</html>
)rawliteral";

void handleRoot() { server.send(200, "text/html", FPSTR(HTML_INDEX)); }

void handleSend() {
  String sender = "Anónimo";
  String msg = "";

  if (server.hasArg("sender") && server.arg("sender").length() > 0) sender = server.arg("sender");
  if (server.hasArg("msg") && server.arg("msg").length() > 0) msg = server.arg("msg");
  else if (server.hasArg("message") && server.arg("message").length() > 0) msg = server.arg("message");

  if (msg.length() > 0) {
    currentSender = sender;
    currentMessage = msg;
    msgCount++;
    renderOLED();
    notifyNewMessageAnimation();

    String json = "{\"status\":\"ok\",\"count\":" + String(msgCount) + 
                  ",\"sender\":\"" + currentSender + 
                  "\",\"message\":\"" + currentMessage + "\"}";
    server.send(200, "application/json", json);
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta parametro msg\"}");
  }
}

void handleStatusAPI() {
  String json = "{";
  json += "\"status\":\"online\",";
  json += "\"count\":" + String(msgCount) + ",";
  json += "\"sender\":\"" + currentSender + "\",";
  json += "\"message\":\"" + currentMessage + "\",";
  json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"free_heap\":" + String(ESP.getFreeHeap());
  json += "}";
  server.send(200, "application/json", json);
}

void handleClearAPI() {
  currentSender = "SISTEMA";
  currentMessage = "[Pantalla Limpia]";
  renderOLED();
  server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"cleared\"}");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[ESP8266 OLED Messenger v3.1] Tipografia Adaptativa Premium...");

  Wire.begin(4, 5);
  Wire.setClock(400000);
  delay(100);

  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledReady = true;
  } else if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    oledReady = true;
  }

  if (oledReady) {
    renderOLED();
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.setOutputPower(20.5);

  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.on("/api/send", handleSend);
  server.on("/api/status", handleStatusAPI);
  server.on("/api/clear", HTTP_POST, handleClearAPI);
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("[WebServer] Servidor HTTP listo en puerto 80");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("MSG:")) {
      currentSender = "SERIAL";
      currentMessage = input.substring(4);
      msgCount++;
      renderOLED();
      notifyNewMessageAnimation();
      Serial.println("OK: Mensaje recibido por Serie");
    } else if (input.indexOf("SENDER:") >= 0 && input.indexOf("|MSG:") > 0) {
      int p1 = input.indexOf("SENDER:") + 7;
      int p2 = input.indexOf("|MSG:");
      currentSender = input.substring(p1, p2);
      currentMessage = input.substring(p2 + 5);
      msgCount++;
      renderOLED();
      notifyNewMessageAnimation();
      Serial.println("OK: Mensaje con remitente recibido por Serie");
    } else if (input.equalsIgnoreCase("CLEAR")) {
      currentSender = "SISTEMA";
      currentMessage = "[Pantalla Limpia]";
      renderOLED();
      Serial.println("OK: Pantalla Limpia");
    }
  }
}
