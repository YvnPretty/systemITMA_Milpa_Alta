/*
 * ESP32 OLED Control Web desde Celular (Web UI + Canvas + Text + Animations)
 * 
 * Hardware: ESP32 + Pantalla OLED SH1106/SSD1306 (128x64) I2C (SDA=21, SCL=22)
 * Librerías necesarias (Instalar en Arduino IDE):
 *  - U8g2 (por Oliver Kraus)
 *  - WiFi (Incluida en ESP32 Core)
 *  - WebServer (Incluida en ESP32 Core)
 *  - DNSServer (Incluida en ESP32 Core)
 *  - ESPmDNS (Incluida en ESP32 Core)
 *
 * Modo de uso:
 *  1. Sube este código a tu ESP32.
 *  2. Desde tu teléfono celular, conéctate a la red Wi-Fi: "ESP32-OLED-Control" (sin contraseña).
 *  3. Abre tu navegador e ingresa a: http://192.168.4.1  o  http://oled.local
 *  4. ¡Dibuja, escribe texto y controla las animaciones de la OLED en tiempo real!
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <U8g2lib.h>

// Definición de Pines I2C para ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Inicialización U8g2 para SH1106 128x64 I2C (Hardware I2C)
// Si usas SSD1306, puedes cambiar U8G2_SH1106 por U8G2_SSD1306_128X64_NONAME_F_HW_I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL_PIN, /* data=*/ SDA_PIN);

// Servidores
WebServer server(80);
DNSServer dnsServer;

// -------------------------------------------------------------
// ESTADO Y MODOS DEL SISTEMA
// -------------------------------------------------------------
enum DisplayMode {
  MODE_WIFI_INFO = 0,
  MODE_TEXT = 1,
  MODE_DRAW = 2,
  MODE_3D_CUBE = 3,
  MODE_ROBOT_EYE = 4,
  MODE_MATRIX = 5,
  MODE_STARFIELD = 6,
  MODE_CYBER_RING = 7
};

DisplayMode currentMode = MODE_WIFI_INFO;

// Variables Modo Texto
String textMessage = "¡Hola ESP32!";
String textStyle = "scroll"; // "big", "medium", "scroll"
int scrollPos = 128;

// Buffer para Modo Dibujo (128x64 = 1024 bytes)
uint8_t drawBuffer[1024];

// Ajustes de Pantalla
bool isDisplayInverted = false;
uint8_t displayContrast = 255;

// Variables Animación 3D Cubo
float angleX = 0, angleY = 0, angleZ = 0;
struct Point3D { float x, y, z; };
Point3D cubeNodes[8] = {
  {-15, -15, -15}, { 15, -15, -15}, { 15,  15, -15}, {-15,  15, -15},
  {-15, -15,  15}, { 15, -15,  15}, { 15,  15,  15}, {-15,  15,  15}
};
int cubeEdges[12][2] = {
  {0,1}, {1,2}, {2,3}, {3,0},
  {4,5}, {5,6}, {6,7}, {7,4},
  {0,4}, {1,5}, {2,6}, {3,7}
};

// Variables Ojo Robot
int eyeX = 64, eyeY = 32;
int targetEyeX = 64, targetEyeY = 32;
int blinkState = 0;

// Variables Starfield
#define NUM_STARS 30
struct Star { float x, y, z; };
Star stars[NUM_STARS];

// Variables Matrix Rain
#define MATRIX_COLS 16
struct MatrixCol {
  int y;
  int speed;
  int length;
  char chars[8];
};
MatrixCol matrixCols[MATRIX_COLS];
const char matrixGlyphs[] = "0123456789ABCDEF*$#@%+=-<>";

// -------------------------------------------------------------
// INTERFAZ WEB EMBEBIDA (HTML5 / CSS3 / JavaScript)
// -------------------------------------------------------------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>ESP32 OLED Control Panel</title>
  <style>
    :root {
      --bg: #090d16;
      --card: #121826;
      --border: #232d42;
      --primary: #00f0ff;
      --secondary: #ff007f;
      --text: #e0e6ed;
      --subtext: #8a99ad;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; touch-action: manipulation; }
    body { background: var(--bg); color: var(--text); display: flex; flex-direction: column; min-height: 100vh; padding-bottom: 20px; }
    header { background: rgba(18, 24, 38, 0.8); backdrop-filter: blur(10px); border-bottom: 1px solid var(--border); padding: 15px 20px; text-align: center; position: sticky; top: 0; z-index: 100; }
    header h1 { font-size: 1.2rem; text-transform: uppercase; letter-spacing: 2px; color: var(--primary); display: flex; align-items: center; justify-content: center; gap: 8px; }
    header h1::before { content: ''; display: inline-block; width: 10px; height: 10px; background: var(--primary); border-radius: 50%; box-shadow: 0 0 10px var(--primary); }
    
    .nav { display: flex; background: var(--card); border-bottom: 1px solid var(--border); overflow-x: auto; }
    .nav button { flex: 1; min-width: 80px; padding: 12px 10px; background: none; border: none; color: var(--subtext); font-weight: 600; font-size: 0.85rem; cursor: pointer; border-bottom: 2px solid transparent; transition: all 0.3s; white-space: nowrap; }
    .nav button.active { color: var(--primary); border-bottom-color: var(--primary); background: rgba(0, 240, 255, 0.05); }

    .container { padding: 16px; max-width: 500px; margin: 0 auto; width: 100%; }
    .tab-content { display: none; }
    .tab-content.active { display: block; animation: fadeIn 0.3s ease; }
    @keyframes fadeIn { from { opacity: 0; transform: translateY(5px); } to { opacity: 1; transform: translateY(0); } }

    .card { background: var(--card); border: 1px solid var(--border); border-radius: 14px; padding: 18px; margin-bottom: 16px; box-shadow: 0 4px 20px rgba(0,0,0,0.3); }
    .card h2 { font-size: 1rem; color: var(--primary); margin-bottom: 12px; display: flex; align-items: center; justify-content: space-between; }

    input[type="text"], select { width: 100%; padding: 12px 14px; background: #0b0f19; border: 1px solid var(--border); border-radius: 8px; color: var(--text); font-size: 1rem; margin-bottom: 12px; outline: none; }
    input[type="text"]:focus, select:focus { border-color: var(--primary); box-shadow: 0 0 8px rgba(0,240,255,0.3); }

    .btn-group { display: flex; gap: 10px; }
    .btn { flex: 1; padding: 12px 16px; border: none; border-radius: 8px; font-weight: 700; font-size: 0.9rem; cursor: pointer; transition: all 0.2s; text-align: center; text-transform: uppercase; letter-spacing: 1px; }
    .btn-primary { background: linear-gradient(135deg, var(--primary), #00a8ff); color: #000; box-shadow: 0 4px 15px rgba(0,240,255,0.3); }
    .btn-primary:active { transform: scale(0.97); }
    .btn-secondary { background: #1c2638; color: var(--text); border: 1px solid var(--border); }
    .btn-danger { background: linear-gradient(135deg, var(--secondary), #d60060); color: #fff; }

    /* Canvas Drawing Area */
    .canvas-container { display: flex; flex-direction: column; align-items: center; gap: 12px; }
    #drawCanvas { background: #000; border: 2px solid var(--primary); border-radius: 6px; box-shadow: 0 0 15px rgba(0,240,255,0.2); image-rendering: pixelated; touch-action: none; cursor: crosshair; }
    .tools-bar { display: flex; gap: 8px; width: 100%; justify-content: center; }
    .tool-btn { padding: 8px 14px; background: #1a2336; border: 1px solid var(--border); border-radius: 6px; color: var(--text); font-size: 0.8rem; font-weight: 600; cursor: pointer; }
    .tool-btn.active { background: var(--primary); color: #000; border-color: var(--primary); }

    /* Grid for Animations */
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .anim-card { background: #0d1320; border: 1px solid var(--border); border-radius: 10px; padding: 14px; text-align: center; cursor: pointer; transition: all 0.2s; }
    .anim-card:hover, .anim-card.active { border-color: var(--primary); background: rgba(0,240,255,0.08); transform: translateY(-2px); }
    .anim-card span { display: block; font-size: 1.5rem; margin-bottom: 6px; }
    .anim-card p { font-size: 0.8rem; font-weight: 600; color: var(--text); }

    /* Slider styling */
    .slider-container { margin-bottom: 14px; }
    .slider-container label { display: flex; justify-content: space-between; font-size: 0.85rem; color: var(--subtext); margin-bottom: 6px; }
    input[type="range"] { width: 100%; accent-color: var(--primary); }

    /* Toast Notification */
    #toast { position: fixed; bottom: 20px; left: 50%; transform: translateX(-50%) translateY(100px); background: rgba(0, 240, 255, 0.9); color: #000; font-weight: bold; padding: 10px 20px; border-radius: 30px; box-shadow: 0 4px 15px rgba(0,0,0,0.5); transition: transform 0.3s ease; z-index: 1000; }
    #toast.show { transform: translateX(-50%) translateY(0); }
  </style>
</head>
<body>

  <header>
    <h1>ESP32 OLED Control</h1>
  </header>

  <div class="nav">
    <button class="active" onclick="switchTab('text')">💬 Texto</button>
    <button onclick="switchTab('draw')">🎨 Dibujar</button>
    <button onclick="switchTab('anim')">🎬 Animación</button>
    <button onclick="switchTab('settings')">⚙️ Ajustes</button>
  </div>

  <div class="container">
    
    <!-- TAB TEXTO -->
    <div id="tab-text" class="tab-content active">
      <div class="card">
        <h2>Enviar Mensaje a Pantalla</h2>
        <input type="text" id="textInput" placeholder="Escribe un mensaje..." value="¡Hola desde el celular!">
        
        <label style="font-size:0.85rem; color:var(--subtext); margin-bottom:6px; display:block;">Estilo de Texto:</label>
        <select id="styleSelect">
          <option value="scroll" selected>📜 Marquesina Flotante (Scroll)</option>
          <option value="big">🔠 Texto Grande Centrado</option>
          <option value="medium">📝 Texto Mediano con Marco</option>
        </select>
        
        <button class="btn btn-primary" onclick="sendText()" style="width:100%;">Mostrar en OLED</button>
      </div>
    </div>

    <!-- TAB DIBUJAR -->
    <div id="tab-draw" class="tab-content">
      <div class="card">
        <h2>Canvas Interactivo (128x64)</h2>
        <div class="canvas-container">
          <canvas id="drawCanvas" width="128" height="64" style="width: 256px; height: 128px;"></canvas>
          <div class="tools-bar">
            <button class="tool-btn active" id="toolLencil" onclick="setTool('pencil')">✏️ Lápiz</button>
            <button class="tool-btn" id="toolEraser" onclick="setTool('eraser')">🧹 Goma</button>
            <button class="tool-btn" onclick="clearCanvas()">🗑️ Limpiar</button>
            <button class="tool-btn" onclick="invertCanvas()">🔄 Invertir</button>
          </div>
          <button class="btn btn-primary" onclick="sendDrawing()" style="width:100%; margin-top:8px;">Enviar Dibujo a OLED</button>
        </div>
      </div>
    </div>

    <!-- TAB ANIMACIONES -->
    <div id="tab-anim" class="tab-content">
      <div class="card">
        <h2>Seleccionar Animación en Vivo</h2>
        <div class="grid">
          <div class="anim-card" onclick="setMode(3)">
            <span>🧊</span>
            <p>Cubo 3D</p>
          </div>
          <div class="anim-card" onclick="setMode(4)">
            <span>👁️</span>
            <p>Ojo Cibernético</p>
          </div>
          <div class="anim-card" onclick="setMode(5)">
            <span>🟢</span>
            <p>Lluvia Matrix</p>
          </div>
          <div class="anim-card" onclick="setMode(6)">
            <span>🌌</span>
            <p>Viaje Espacial</p>
          </div>
          <div class="anim-card" onclick="setMode(7)">
            <span>⚡</span>
            <p>Cyber Ring</p>
          </div>
          <div class="anim-card" onclick="setMode(0)">
            <span>📶</span>
            <p>Info Wi-Fi</p>
          </div>
        </div>
      </div>
    </div>

    <!-- TAB AJUSTES -->
    <div id="tab-settings" class="tab-content">
      <div class="card">
        <h2>Control de Pantalla</h2>
        <div class="slider-container">
          <label><span>Contraste / Brillo:</span> <span id="contrastVal">255</span></label>
          <input type="range" id="contrastSlider" min="0" max="255" value="255" oninput="updateContrast(this.value)">
        </div>

        <div class="btn-group" style="margin-bottom:12px;">
          <button class="btn btn-secondary" onclick="toggleInvert()">🔄 Invertir Colores</button>
        </div>

        <h2>Estado del Dispositivo</h2>
        <p style="font-size:0.85rem; color:var(--subtext); line-height:1.6;" id="systemStatus">
          Cargando telemetría...
        </p>
      </div>
    </div>

  </div>

  <div id="toast">Acción enviada</div>

  <script>
    function switchTab(tabId) {
      document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
      document.querySelectorAll('.nav button').forEach(b => b.classList.remove('active'));
      document.getElementById('tab-' + tabId).classList.add('active');
      event.target.classList.add('active');
    }

    function showToast(msg) {
      const toast = document.getElementById('toast');
      toast.innerText = msg;
      toast.classList.add('show');
      setTimeout(() => toast.classList.remove('show'), 2000);
    }

    // -------------------------------------------------------------
    // CANVAS DRAWING LOGIC
    // -------------------------------------------------------------
    const canvas = document.getElementById('drawCanvas');
    const ctx = canvas.getContext('2d');
    let isDrawing = false;
    let currentTool = 'pencil';

    // Inicializar canvas en negro
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, 128, 64);
    ctx.fillStyle = '#ffffff';

    function getCanvasCoords(e) {
      const rect = canvas.getBoundingClientRect();
      const clientX = e.touches ? e.touches[0].clientX : e.clientX;
      const clientY = e.touches ? e.touches[0].clientY : e.clientY;
      const scaleX = canvas.width / rect.width;
      const scaleY = canvas.height / rect.height;
      return {
        x: Math.floor((clientX - rect.left) * scaleX),
        y: Math.floor((clientY - rect.top) * scaleY)
      };
    }

    function drawPixel(e) {
      if (!isDrawing) return;
      const { x, y } = getCanvasCoords(e);
      ctx.fillStyle = (currentTool === 'pencil') ? '#ffffff' : '#000000';
      ctx.fillRect(x, y, 2, 2);
    }

    canvas.addEventListener('mousedown', (e) => { isDrawing = true; drawPixel(e); });
    canvas.addEventListener('mousemove', drawPixel);
    window.addEventListener('mouseup', () => isDrawing = false);

    canvas.addEventListener('touchstart', (e) => { isDrawing = true; drawPixel(e); e.preventDefault(); });
    canvas.addEventListener('touchmove', (e) => { drawPixel(e); e.preventDefault(); });
    window.addEventListener('touchend', () => isDrawing = false);

    function setTool(tool) {
      currentTool = tool;
      document.getElementById('toolLencil').classList.toggle('active', tool === 'pencil');
      document.getElementById('toolEraser').classList.toggle('active', tool === 'eraser');
    }

    function clearCanvas() {
      ctx.fillStyle = '#000000';
      ctx.fillRect(0, 0, 128, 64);
      sendDrawing();
    }

    function invertCanvas() {
      const imgData = ctx.getImageData(0, 0, 128, 64);
      for (let i = 0; i < imgData.data.length; i += 4) {
        imgData.data[i] = 255 - imgData.data[i];
        imgData.data[i+1] = 255 - imgData.data[i+1];
        imgData.data[i+2] = 255 - imgData.data[i+2];
      }
      ctx.putImageData(imgData, 0, 0);
      sendDrawing();
    }

    function sendDrawing() {
      const imgData = ctx.getImageData(0, 0, 128, 64).data;
      let hexStr = '';
      
      // Convertir imagen RGBA 128x64 a mapa de bits en formato Hexadecimal (1024 bytes)
      for (let page = 0; page < 8; page++) {
        for (let x = 0; x < 128; x++) {
          let byteVal = 0;
          for (let bit = 0; bit < 8; bit++) {
            const y = page * 8 + bit;
            const idx = (y * 128 + x) * 4;
            const isWhite = imgData[idx] > 128;
            if (isWhite) byteVal |= (1 << bit);
          }
          hexStr += byteVal.toString(16).padStart(2, '0');
        }
      }

      fetch('/api/draw', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ hex: hexStr })
      }).then(() => showToast('✏️ Dibujo enviado'));
    }

    // -------------------------------------------------------------
    // API CALLS
    // -------------------------------------------------------------
    function sendText() {
      const text = document.getElementById('textInput').value;
      const style = document.getElementById('styleSelect').value;
      fetch('/api/text', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text, style })
      }).then(() => showToast('💬 Mensaje enviado'));
    }

    function setMode(mode) {
      fetch('/api/animation', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mode })
      }).then(() => showToast('🎬 Animación activada'));
    }

    function updateContrast(val) {
      document.getElementById('contrastVal').innerText = val;
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ contrast: parseInt(val) })
      });
    }

    function toggleInvert() {
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ invertToggle: true })
      }).then(() => showToast('🔄 Inversión cambiada'));
    }

    // Cargar telemetría periódicamente
    setInterval(() => {
      fetch('/api/status').then(r => r.json()).then(data => {
        document.getElementById('systemStatus').innerHTML = `
          <strong>SSID Wi-Fi:</strong> ESP32-OLED-Control<br>
          <strong>IP Dispositivo:</strong> ${data.ip}<br>
          <strong>Clientes Conectados:</strong> ${data.clients}<br>
          <strong>Memoria RAM Libre:</strong> ${Math.round(data.freeRam / 1024)} KB<br>
          <strong>Tiempo Activo:</strong> ${data.uptime}s
        `;
      }).catch(() => {});
    }, 3000);
  </script>
</body>
</html>
)rawliteral";

// -------------------------------------------------------------
// INICIALIZACIÓN MATRIX Y ESTRELLAS
// -------------------------------------------------------------
void initEffects() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = random(-64, 64);
    stars[i].y = random(-32, 32);
    stars[i].z = random(1, 100);
  }
  for (int i = 0; i < MATRIX_COLS; i++) {
    matrixCols[i].y = random(-15, 0);
    matrixCols[i].speed = random(1, 4);
    matrixCols[i].length = random(4, 8);
    for (int j = 0; j < 8; j++) {
      matrixCols[i].chars[j] = matrixGlyphs[random(0, sizeof(matrixGlyphs) - 1)];
    }
  }
}

// Helper para convertir caracter Hex a valor numérico
uint8_t hexNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

// -------------------------------------------------------------
// MANEJO DE RUTAS WEB (API REST)
// -------------------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleRedirect() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void handleApiText() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    int textIdx = body.indexOf("\"text\":\"");
    if (textIdx != -1) {
      int endIdx = body.indexOf("\"", textIdx + 8);
      if (endIdx != -1) {
        textMessage = body.substring(textIdx + 8, endIdx);
      }
    }
    int styleIdx = body.indexOf("\"style\":\"");
    if (styleIdx != -1) {
      int endIdx = body.indexOf("\"", styleIdx + 9);
      if (endIdx != -1) {
        textStyle = body.substring(styleIdx + 9, endIdx);
      }
    }
    currentMode = MODE_TEXT;
    scrollPos = 128;
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"error\":\"bad request\"}");
  }
}

void handleApiDraw() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    int hexIdx = body.indexOf("\"hex\":\"");
    if (hexIdx != -1) {
      int start = hexIdx + 7;
      int end = body.indexOf("\"", start);
      if (end != -1 && (end - start) == 2048) {
        // Parsear 1024 bytes hexadecimales
        for (int i = 0; i < 1024; i++) {
          char h1 = body.charAt(start + i * 2);
          char h2 = body.charAt(start + i * 2 + 1);
          drawBuffer[i] = (hexNibble(h1) << 4) | hexNibble(h2);
        }
        currentMode = MODE_DRAW;
        server.send(200, "application/json", "{\"status\":\"ok\"}");
        return;
      }
    }
  }
  server.send(400, "application/json", "{\"error\":\"invalid hex length\"}");
}

void handleApiAnimation() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    int modeIdx = body.indexOf("\"mode\":");
    if (modeIdx != -1) {
      int modeVal = body.substring(modeIdx + 7).toInt();
      currentMode = (DisplayMode)modeVal;
      server.send(200, "application/json", "{\"status\":\"ok\"}");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"bad request\"}");
}

void handleApiConfig() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    if (body.indexOf("\"invertToggle\":true") != -1) {
      isDisplayInverted = !isDisplayInverted;
    }
    int contrastIdx = body.indexOf("\"contrast\":");
    if (contrastIdx != -1) {
      displayContrast = body.substring(contrastIdx + 11).toInt();
      u8g2.setContrast(displayContrast);
    }
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"error\":\"bad request\"}");
  }
}

void handleApiStatus() {
  String json = "{";
  json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"freeRam\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"mode\":" + String((int)currentMode);
  json += "}";
  server.send(200, "application/json", json);
}

// -------------------------------------------------------------
// RENDERS EN OLED SEGÚN EL MODO SELECCIONADO
// -------------------------------------------------------------
void renderWifiInfo() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 10, "== ESP32 OLED WEB ==");
  u8g2.drawHLine(0, 12, 128);
  
  u8g2.drawStr(0, 26, "WiFi: ESP32-OLED-Control");
  u8g2.drawStr(0, 38, "IP:   192.168.4.1");
  u8g2.drawStr(0, 50, "URL:  oled.local");
  u8g2.drawStr(0, 62, "Conectate con tu cel");
  u8g2.sendBuffer();
}

void renderText() {
  u8g2.clearBuffer();

  if (textStyle == "big") {
    u8g2.setFont(u8g2_font_helvB12_tr);
    int w = u8g2.getStrWidth(textMessage.c_str());
    int x = (128 - w) / 2;
    if (x < 0) x = 0;
    u8g2.drawStr(x, 38, textMessage.c_str());
  } else if (textStyle == "medium") {
    u8g2.drawFrame(2, 2, 124, 60);
    u8g2.setFont(u8g2_font_profont12_tr);
    int w = u8g2.getStrWidth(textMessage.c_str());
    int x = (128 - w) / 2;
    if (x < 4) x = 4;
    u8g2.drawStr(x, 36, textMessage.c_str());
  } else { // "scroll" marquesina
    u8g2.setFont(u8g2_font_helvB14_tr);
    u8g2.drawStr(scrollPos, 40, textMessage.c_str());
    int w = u8g2.getStrWidth(textMessage.c_str());
    scrollPos -= 2;
    if (scrollPos < -w) {
      scrollPos = 128;
    }
  }

  u8g2.sendBuffer();
}

void renderDraw() {
  u8g2.clearBuffer();
  // Dibujar mapa de bits del buffer recibido desde la web
  u8g2.drawBitmap(0, 0, 16, 64, drawBuffer);
  u8g2.sendBuffer();
}

void render3DCube() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "Cubo 3D Giratorio");

  angleX += 0.04;
  angleY += 0.05;
  angleZ += 0.02;

  Point3D projected[8];
  for (int i = 0; i < 8; i++) {
    float y1 = cubeNodes[i].y * cos(angleX) - cubeNodes[i].z * sin(angleX);
    float z1 = cubeNodes[i].y * sin(angleX) + cubeNodes[i].z * cos(angleX);
    float x1 = cubeNodes[i].x;

    float x2 = x1 * cos(angleY) + z1 * sin(angleY);
    float z2 = -x1 * sin(angleY) + z1 * cos(angleY);
    float y2 = y1;

    float x3 = x2 * cos(angleZ) - y2 * sin(angleZ);
    float y3 = x2 * sin(angleZ) + y2 * cos(angleZ);

    float distance = 60;
    float fov = distance / (distance + z2 + 20);
    projected[i].x = 64 + x3 * fov * 1.5;
    projected[i].y = 36 + y3 * fov * 1.5;
  }

  for (int i = 0; i < 12; i++) {
    u8g2.drawLine(
      (int)projected[cubeEdges[i][0]].x, (int)projected[cubeEdges[i][0]].y,
      (int)projected[cubeEdges[i][1]].x, (int)projected[cubeEdges[i][1]].y
    );
  }
  u8g2.sendBuffer();
}

void renderRobotEye() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "Ojo Cibernetico AI");

  if (random(0, 20) == 1) {
    targetEyeX = 64 + random(-25, 25);
    targetEyeY = 36 + random(-10, 10);
  }
  eyeX += (targetEyeX - eyeX) * 0.2;
  eyeY += (targetEyeY - eyeY) * 0.2;

  if (random(0, 30) == 1 && blinkState == 0) blinkState = 1;

  if (blinkState > 0) {
    int h = 18 - (blinkState * 6);
    if (h < 2) h = 2;
    u8g2.drawBox(eyeX - 30, eyeY - (h/2), 60, h);
    blinkState++;
    if (blinkState > 3) blinkState = 0;
  } else {
    u8g2.drawRFrame(24, 16, 80, 42, 8);
    u8g2.drawDisc(eyeX, eyeY, 10, U8G2_DRAW_ALL);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(eyeX - 3, eyeY - 3, 3, U8G2_DRAW_ALL);
    u8g2.setDrawColor(1);
    u8g2.drawHLine(10, 36, 10);
    u8g2.drawHLine(108, 36, 10);
  }
  u8g2.sendBuffer();
}

void renderMatrix() {
  u8g2.clearBuffer();
  for (int i = 0; i < MATRIX_COLS; i++) {
    matrixCols[i].y += matrixCols[i].speed;
    if (random(0, 4) == 1) {
      matrixCols[i].chars[random(0, 8)] = matrixGlyphs[random(0, sizeof(matrixGlyphs) - 1)];
    }
    if (matrixCols[i].y - matrixCols[i].length * 8 > 64) {
      matrixCols[i].y = random(-10, 0);
      matrixCols[i].speed = random(1, 3);
    }
  }

  u8g2.setFont(u8g2_font_profont10_tr);
  for (int i = 0; i < MATRIX_COLS; i++) {
    int x = i * 8;
    for (int j = 0; j < matrixCols[i].length; j++) {
      int y = matrixCols[i].y - (j * 8);
      if (y >= 0 && y <= 64) {
        u8g2.drawGlyph(x, y, matrixCols[i].chars[j]);
      }
    }
  }
  u8g2.sendBuffer();
}

void renderStarfield() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "Starfield Warp");

  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].z -= 2.5;
    if (stars[i].z <= 0) {
      stars[i].x = random(-64, 64);
      stars[i].y = random(-32, 32);
      stars[i].z = 100;
    }
    float k = 64.0 / stars[i].z;
    int px = (int)(stars[i].x * k + 64);
    int py = (int)(stars[i].y * k + 36);

    if (px >= 0 && px < 128 && py >= 12 && py < 64) {
      int size = (stars[i].z < 30) ? 2 : 1;
      u8g2.drawBox(px, py, size, size);
    }
  }
  u8g2.sendBuffer();
}

void renderCyberRing() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "Cyber Ring Telemetry");

  static int pct = 0;
  pct = (pct + 2) % 101;

  u8g2.drawCircle(64, 38, 20, U8G2_DRAW_ALL);
  u8g2.drawCircle(64, 38, 22, U8G2_DRAW_ALL);

  u8g2.setFont(u8g2_font_helvB10_tr);
  String pStr = String(pct) + "%";
  int w = u8g2.getStrWidth(pStr.c_str());
  u8g2.drawStr(64 - (w / 2), 43, pStr.c_str());

  static float rAngle = 0;
  rAngle += 0.1;
  int lx1 = 64 + cos(rAngle) * 26;
  int ly1 = 38 + sin(rAngle) * 26;
  int lx2 = 64 + cos(rAngle + 3.14) * 26;
  int ly2 = 38 + sin(rAngle + 3.14) * 26;

  u8g2.drawLine(lx1, ly1, lx1 + cos(rAngle)*4, ly1 + sin(rAngle)*4);
  u8g2.drawLine(lx2, ly2, lx2 - cos(rAngle)*4, ly2 - sin(rAngle)*4);

  u8g2.sendBuffer();
}

// -------------------------------------------------------------
// SETUP
// -------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Inicializar Pantalla OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);

  initEffects();

  // Configurar Wi-Fi SoftAP (Punto de Acceso)
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-OLED-Control"); // Sin contraseña para fácil conexión desde celulares

  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // Servidor DNS Captive Portal (redirecciona cualquier dominio a 192.168.4.1)
  dnsServer.start(53, "*", apIP);

  // Servidor mDNS (oled.local)
  if (MDNS.begin("oled")) {
    Serial.println("mDNS iniciado: http://oled.local");
  }

  // Configurar Rutas Web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/generate_204", HTTP_GET, handleRedirect); // Android Captive Portal
  server.on("/redirect", HTTP_GET, handleRedirect);
  server.on("/hotspot-detect.html", HTTP_GET, handleRedirect); // Apple Captive Portal
  server.on("/api/text", HTTP_POST, handleApiText);
  server.on("/api/draw", HTTP_POST, handleApiDraw);
  server.on("/api/animation", HTTP_POST, handleApiAnimation);
  server.on("/api/config", HTTP_POST, handleApiConfig);
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.onNotFound(handleRedirect);

  server.begin();
  Serial.println("Servidor Web iniciado en IP: 192.168.4.1");
}

// -------------------------------------------------------------
// LOOP PRINCIPAL
// -------------------------------------------------------------
void loop() {
  // Procesar peticiones DNS y HTTP
  dnsServer.processNextRequest();
  server.handleClient();

  // Aplicar modo de inversión si cambió
  u8g2.setDrawColor(isDisplayInverted ? 0 : 1);

  // Renderizar pantalla OLED según el modo actual
  switch (currentMode) {
    case MODE_WIFI_INFO: renderWifiInfo(); break;
    case MODE_TEXT:      renderText(); break;
    case MODE_DRAW:      renderDraw(); break;
    case MODE_3D_CUBE:   render3DCube(); break;
    case MODE_ROBOT_EYE: renderRobotEye(); break;
    case MODE_MATRIX:    renderMatrix(); break;
    case MODE_STARFIELD: renderStarfield(); break;
    case MODE_CYBER_RING:renderCyberRing(); break;
  }

  delay(20); // ~50 FPS
}
