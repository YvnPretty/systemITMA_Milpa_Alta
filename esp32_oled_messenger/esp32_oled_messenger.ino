/*
 * =================================================================================
 * PROYECTO: ESP32 OLED Messenger (v44.0 - GIANT 22PX MONSTER FONT EDITION)
 * DESCRIPCIÓN: Fuentes Ultra Gigantes de hasta 22-24 Píxeles de Alto (`u8g2_font_logisoso22_tf`).
 *              - Frases cortas (1-3 palabras): Tipografía MONSTRUO GIGANTE 22px (`logisoso22`).
 *              - Frases medianas (4-8 palabras): Tipografía SUPER BOLD 18px (`logisoso18`).
 *              - Frases largas (9+ palabras): Tipografía BOLD 14px en Carrusel Continuo.
 *              - Eliminación total de letras pequeñas o cuadritos feos.
 *              - Orbe Neural Centrado Neón en Reposo.
 *              - Pines OLED verificados: SDA=GPIO 22, SCL=GPIO 21.
 * =================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <math.h>

#define SDA_PIN 22
#define SCL_PIN 21

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock/SCL=*/ SCL_PIN, /* data/SDA=*/ SDA_PIN);

const char* AP_SSID = "VOICE-DICTATION-ESP32";
const char* AP_PASS = "";
const byte DNS_PORT = 53;

DNSServer dnsServer;
WebServer server(80);

// Estado de Animación y Dictado
String activeDictatedText = "";
bool hasActiveText = false;
unsigned long textStartTime = 0;
const unsigned long TEXT_DISPLAY_DURATION = 15000;

float marqueeX = 128.0;
float marqueeSpeed = 2.6;
float pulsePhase = 0.0;

int countWords(String str) {
  str.trim();
  if (str.length() == 0) return 0;
  int count = 1;
  for (unsigned int i = 0; i < str.length(); i++) {
    if (str.charAt(i) == ' ') count++;
  }
  return count;
}

void renderGiantMonsterScreen() {
  u8g2.clearBuffer();

  pulsePhase += 0.08;
  if (pulsePhase > 6.28318) pulsePhase -= 6.28318;

  if (hasActiveText && (millis() - textStartTime > TEXT_DISPLAY_DURATION)) {
    hasActiveText = false;
    activeDictatedText = "";
  }

  if (!hasActiveText) {
    // Modo Reposo: Orbe Neural Centrado Neón
    int centerX = 64;
    int centerY = 32;

    int coreRadius = (int)(12.0 + 4.0 * sin(pulsePhase));
    int outerRing1  = (int)(20.0 + 3.0 * sin(pulsePhase * 1.3));
    int outerRing2  = (int)(27.0 + 2.0 * cos(pulsePhase * 0.8));

    u8g2.drawCircle(centerX, centerY, outerRing2);
    u8g2.drawCircle(centerX, centerY, outerRing1);
    u8g2.drawDisc(centerX, centerY, coreRadius);

    u8g2.setDrawColor(0);
    u8g2.drawDisc(centerX, centerY, (int)(coreRadius * 0.4));
    u8g2.setDrawColor(1);
  } else {
    // -----------------------------------------------------------------------------
    // MODO ACTIVO: TIPOGRAFÍA MONSTRUO GIGANTE 22PX / 18PX EN CARRUSEL FLUIDO 60 FPS
    // -----------------------------------------------------------------------------
    int numWords = countWords(activeDictatedText);

    if (numWords <= 3) {
      // 1. FRASES CORTAS: FUENTE MONSTRUO GIGANTE 22PX (Ocupa el 40% del alto total)
      u8g2.setFont(u8g2_font_logisoso22_tf);
      int textWidth = u8g2.getUTF8Width(activeDictatedText.c_str());

      u8g2.setCursor((int)marqueeX, 44);
      u8g2.print(activeDictatedText);

      marqueeX -= marqueeSpeed;
      if (marqueeX < -textWidth) {
        marqueeX = 128.0;
      }

    } else if (numWords <= 8) {
      // 2. FRASES MEDIANAS: FUENTE SUPER BOLD 16PX (Logisoso 16)
      u8g2.setFont(u8g2_font_logisoso16_tf);
      int textWidth = u8g2.getUTF8Width(activeDictatedText.c_str());

      u8g2.setCursor((int)marqueeX, 40);
      u8g2.print(activeDictatedText);

      marqueeX -= marqueeSpeed;
      if (marqueeX < -textWidth) {
        marqueeX = 128.0;
      }

    } else {
      // 3. FRASES LARGAS: FUENTE KARAOKE BOLD 14PX (7x14B)
      u8g2.setFont(u8g2_font_7x14B_tf);
      int textWidth = u8g2.getUTF8Width(activeDictatedText.c_str());

      u8g2.setCursor((int)marqueeX, 38);
      u8g2.print(activeDictatedText);

      marqueeX -= marqueeSpeed;
      if (marqueeX < -textWidth) {
        marqueeX = 128.0;
      }
    }
  }

  u8g2.sendBuffer();
}

// ---------------------------------------------------------------------------------
// DASHBOARD WEB CON TIPOGRAFÍA MONSTRUO GIGANTE (192.168.4.1)
// ---------------------------------------------------------------------------------
const char HTML_INDEX[] PROGMEM = R"HTML_CONTENT(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>GIANT 22PX TELEPROMPTER // ESP32</title>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@400;700;900&family=Space+Grotesk:wght@700&display=swap" rel="stylesheet">
  <style>
    :root {
      --neon-emerald: #00ff88;
      --neon-cyan: #00e5ff;
      --neon-pink: #ff0077;
      --bg: #020608;
      --glass-bg: rgba(6, 20, 15, 0.85);
      --glass-border: rgba(0, 255, 136, 0.35);
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Outfit', sans-serif; -webkit-tap-highlight-color: transparent; }

    html, body {
      width: 100%; min-height: 100dvh; background: radial-gradient(circle at 50% 20%, #002b1a 0%, var(--bg) 80%);
      color: #fff; display: flex; justify-content: center; align-items: center; padding: 12px;
    }

    .container {
      width: min(100%, 460px); display: flex; flex-direction: column; gap: 14px; margin: auto;
    }

    .header { text-align: center; }
    .header h1 {
      font-family: 'Space Grotesk', sans-serif; font-size: clamp(24px, 6vw, 30px); font-weight: 900;
      background: linear-gradient(135deg, var(--neon-emerald), var(--neon-cyan));
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
      text-shadow: 0 0 25px rgba(0,255,136,0.4);
    }
    .header p { color: #80ffc4; font-size: clamp(11px, 3vw, 13px); font-weight: 700; letter-spacing: 1px; margin-top: 2px; text-transform: uppercase; }

    .ai-orb-container { display: flex; justify-content: center; }
    .ai-orb {
      width: clamp(65px, 17vw, 80px); height: clamp(65px, 17vw, 80px); border-radius: 50%;
      background: radial-gradient(circle, var(--neon-emerald) 0%, rgba(0,229,255,0.4) 60%, transparent 100%);
      box-shadow: 0 0 35px var(--neon-emerald); animation: pulseOrb 2.2s infinite ease-in-out;
    }
    @keyframes pulseOrb {
      0%, 100% { transform: scale(1); box-shadow: 0 0 25px var(--neon-emerald); }
      50% { transform: scale(1.15); box-shadow: 0 0 45px var(--neon-cyan), 0 0 20px var(--neon-pink); }
    }

    .dictation-box {
      background: var(--glass-bg); backdrop-filter: blur(16px); -webkit-backdrop-filter: blur(16px);
      border: 1px solid var(--glass-border); border-radius: 20px; padding: 18px; text-align: center;
      min-height: 130px; display: flex; align-items: center; justify-content: center;
      box-shadow: 0 8px 32px rgba(0,0,0,0.6); font-size: clamp(18px, 4.8vw, 22px); font-weight: 900; color: var(--neon-emerald); line-height: 1.45;
    }

    .mic-btn {
      width: 100%; min-height: 56px; padding: 14px; border: 2px solid var(--neon-emerald); border-radius: 16px;
      background: linear-gradient(135deg, #002b17, #00150b); color: var(--neon-emerald);
      font-size: clamp(15px, 4vw, 17px); font-weight: 900; cursor: pointer; text-transform: uppercase;
      box-shadow: 0 0 30px rgba(0,255,136,0.25); transition: all 0.2s; display: flex; align-items: center; justify-content: center; gap: 8px;
    }
    .mic-btn:active { transform: scale(0.97); }
    .mic-btn.recording { background: linear-gradient(135deg, #44001c, #22000e); border-color: var(--neon-pink); color: var(--neon-pink); box-shadow: 0 0 35px var(--neon-pink); }

    .input-group { display: flex; gap: 8px; width: 100%; }
    .input-group input {
      flex: 1; min-height: 48px; padding: 12px 14px; border-radius: 14px; border: 1px solid var(--glass-border);
      background: rgba(0,0,0,0.7); color: #fff; font-size: 15px; outline: none; transition: border 0.2s;
    }
    .input-group input:focus { border-color: var(--neon-emerald); }
    .input-group button {
      min-height: 48px; padding: 0 20px; border-radius: 14px; border: none;
      background: linear-gradient(135deg, var(--neon-emerald), var(--neon-cyan)); color: #000; font-weight: 900; cursor: pointer; font-size: 15px;
    }
  </style>
</head>
<body>

<div class="container">
  <div class="header">
    <h1>🔤 TIPOGRAFÍA MONSTRUO 22PX</h1>
    <p>Letra Extra Grande • ESP32 OLED</p>
  </div>

  <div class="ai-orb-container">
    <div class="ai-orb" id="ai-orb"></div>
  </div>

  <div class="dictation-box" id="dictation-box">
    🗣️ Toca el botón para hablar desde tu celular con letra MONSTRUO GIGANTE (22px).
  </div>

  <button class="mic-btn" id="mic-btn" onclick="togglePhoneMic()">
    🎙️ USAR MICRÓFONO DEL CELULAR
  </button>

  <div class="input-group">
    <input type="text" id="user-input" placeholder="Escribe tu mensaje..." onkeypress="if(event.key==='Enter') sendText()">
    <button onclick="sendText()">ENVIAR</button>
  </div>
</div>

<script>
  let recognition = null;
  let isListening = false;

  const sendTextToESP = (text) => {
    document.getElementById('dictation-box').innerText = "🗣️ " + text;
    fetch('/dictate?text=' + encodeURIComponent(text));
  };

  const sendText = () => {
    const input = document.getElementById('user-input');
    const val = input.value.trim();
    if (val.length > 0) {
      sendTextToESP(val);
      input.value = '';
    }
  };

  const initSpeechAPI = () => {
    if ('webkitSpeechRecognition' in window || 'SpeechRecognition' in window) {
      const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
      recognition = new SpeechRecognition();
      recognition.continuous = true;
      recognition.interimResults = true;
      recognition.lang = 'es-MX';

      recognition.onstart = () => {
        isListening = true;
        document.getElementById('mic-btn').classList.add('recording');
        document.getElementById('mic-btn').innerText = '🔴 ESCUCHANDO TU VOZ...';
      };

      recognition.onresult = (event) => {
        let liveText = '';
        for (let i = event.resultIndex; i < event.results.length; i++) {
          liveText += event.results[i][0].transcript;
        }
        liveText = liveText.trim();
        if (liveText.length > 0) {
          sendTextToESP(liveText);
        }
      };

      recognition.onend = () => {
        if (isListening) {
          try { recognition.start(); } catch(err) {}
        } else {
          document.getElementById('mic-btn').classList.remove('recording');
          document.getElementById('mic-btn').innerText = '🎙️ USAR MICRÓFONO DEL CELULAR';
        }
      };
    }
  };

  const togglePhoneMic = () => {
    if (!recognition) initSpeechAPI();
    if (!recognition) return;

    if (isListening) {
      isListening = false;
      recognition.stop();
    } else {
      isListening = true;
      try { recognition.start(); } catch(e) {}
    }
  };
</script>

</body>
</html>
)HTML_CONTENT";

void handleRoot() { server.send(200, "text/html", FPSTR(HTML_INDEX)); }

void handleDictate() {
  if (server.hasArg("text")) {
    activeDictatedText = server.arg("text");
    hasActiveText = true;
    textStartTime = millis();
    marqueeX = 128.0;

    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\"}");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[ESP32 Giant 22px Monster Font v44.0]");

  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/dictate", handleDictate);
  server.on("/set", handleDictate);
  server.begin();
  Serial.println("[WebServer] Dashboard listo en 192.168.4.1");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  renderGiantMonsterScreen();

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("LIVE:")) {
      activeDictatedText = input.substring(5);
      hasActiveText = true;
      textStartTime = millis();
      marqueeX = 128.0;
    } else if (input.length() > 0) {
      activeDictatedText = input;
      hasActiveText = true;
      textStartTime = millis();
      marqueeX = 128.0;
    }
  }
}
