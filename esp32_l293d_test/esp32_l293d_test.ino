/*
 * ESP32 + L293D Motor Shield - Servidor Web de Control Remoto Wi-Fi
 *
 * Mapeo de Pines:
 * D3   -> GPIO 25 (PWM Motor 2)
 * D4   -> GPIO 17 (DIR CLK)
 * D5   -> GPIO 16 (PWM Motor 4)
 * D6   -> GPIO 27 (PWM Motor 3)
 * D7   -> GPIO 14 (DIR EN)
 * D8   -> GPIO 13 (DIR SER)
 * D11  -> GPIO 23 (PWM Motor 1)
 * D12  -> GPIO 19 (DIR LATCH)
 */

#include <WiFi.h>
#include <WebServer.h>

// Definición de Pines ESP32
const int PIN_D3  = 25; // PWM M2
const int PIN_D4  = 17; // DIR CLK
const int PIN_D5  = 16; // PWM M4
const int PIN_D6  = 27; // PWM M3
const int PIN_D7  = 14; // DIR EN
const int PIN_D8  = 13; // DIR SER
const int PIN_D11 = 23; // PWM M1
const int PIN_D12 = 19; // DIR LATCH

// Pines 74HC595 para L293D Shield
const int MOTORCLK    = PIN_D4;   
const int MOTORENABLE = PIN_D7;   
const int MOTORDATA   = PIN_D8;   
const int MOTORLATCH  = PIN_D12;  

const int PWM_M1 = PIN_D11; 
const int PWM_M2 = PIN_D3;  
const int PWM_M3 = PIN_D6;  
const int PWM_M4 = PIN_D5;  

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR3_A 5
#define MOTOR3_B 7
#define MOTOR4_A 0
#define MOTOR4_B 6

static uint8_t latch_state = 0;
int currentSpeed = 220; // Velocidad por defecto (0-255)

// Credenciales Wi-Fi de casa del usuario (Red 2.4GHz detectada en el scanner)
const char* sta_ssid_primary   = "INFINITUM8788_2.4";
const char* sta_ssid_secondary = "infinitum8788_5";
const char* sta_pass           = "chkQwQ9Y9f";

// Servidor Web en puerto 80 y Red Wi-Fi AP de respaldo
WebServer server(80);
const char* ap_ssid = "Carrito_ESP32";
const char* ap_pass = "12345678";

void writeLatch() {
  digitalWrite(MOTORLATCH, LOW);
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_state);
  digitalWrite(MOTORLATCH, HIGH);
}

void setMotor(int motorNum, int speed, int dir) {
  uint8_t a = 0, b = 0;
  int pwmPin = 0;

  switch (motorNum) {
    case 1: a = MOTOR1_A; b = MOTOR1_B; pwmPin = PWM_M1; break;
    case 2: a = MOTOR2_A; b = MOTOR2_B; pwmPin = PWM_M2; break;
    case 3: a = MOTOR3_A; b = MOTOR3_B; pwmPin = PWM_M3; break;
    case 4: a = MOTOR4_A; b = MOTOR4_B; pwmPin = PWM_M4; break;
  }

  if (dir == 1) { 
    latch_state |= (1 << a); latch_state &= ~(1 << b);
  } else if (dir == 2) { 
    latch_state &= ~(1 << a); latch_state |= (1 << b);
  } else { 
    latch_state &= ~(1 << a); latch_state &= ~(1 << b);
  }

  writeLatch();
  analogWrite(pwmPin, speed);
}

void driveCar(const String& command, int spd) {
  if (command == "forward") {
    setMotor(1, spd, 1); // M1 (Enfrente) Adelante
    setMotor(4, spd, 1); // M4 (Atrás) Adelante
    setMotor(2, 0, 0);   // No usado
    setMotor(3, 0, 0);   // No usado
  } else if (command == "backward") {
    setMotor(1, spd, 2); // M1 (Enfrente) Atrás
    setMotor(4, spd, 2); // M4 (Atrás) Atrás
    setMotor(2, 0, 0);
    setMotor(3, 0, 0);
  } else if (command == "left") {
    setMotor(1, spd, 2); // M1 Giro
    setMotor(4, spd, 1); // M4 Giro
    setMotor(2, 0, 0);
    setMotor(3, 0, 0);
  } else if (command == "right") {
    setMotor(1, spd, 1); // M1 Giro
    setMotor(4, spd, 2); // M4 Giro
    setMotor(2, 0, 0);
    setMotor(3, 0, 0);
  } else { // stop
    setMotor(1, 0, 0); setMotor(2, 0, 0);
    setMotor(3, 0, 0); setMotor(4, 0, 0);
  }
}

// Interfaz Web HTML/CSS/JS Servida desde el ESP32
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Control Remoto ESP32 Car</title>
  <style>
    * { box-sizing: border-box; touch-action: manipulation; user-select: none; }
    body {
      margin: 0; padding: 0;
      background: #0f172a; color: #f8fafc;
      font-family: system-ui, -apple-system, sans-serif;
      display: flex; flex-direction: column; align-items: center; justify-content: center;
      min-height: 100vh; text-align: center;
    }
    h1 { margin: 10px 0 5px; font-size: 1.6rem; color: #38bdf8; text-shadow: 0 0 10px rgba(56,189,248,0.5); }
    .status { margin-bottom: 20px; font-size: 0.95rem; color: #94a3b8; }
    .status span { color: #4ade80; font-weight: bold; }
    
    .dpad {
      display: grid;
      grid-template-columns: repeat(3, 90px);
      grid-template-rows: repeat(3, 90px);
      gap: 12px;
      margin-bottom: 25px;
    }
    .btn {
      background: rgba(30, 41, 59, 0.8);
      border: 2px solid #3b82f6;
      border-radius: 18px;
      color: #fff;
      font-size: 2.2rem;
      display: flex; align-items: center; justify-content: center;
      cursor: pointer;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
      transition: all 0.1s ease;
    }
    .btn:active, .btn.active {
      background: #3b82f6;
      transform: scale(0.92);
      box-shadow: 0 0 20px #3b82f6;
    }
    .btn-stop {
      border-color: #ef4444; color: #ef4444; background: rgba(239,68,68,0.1);
    }
    .btn-stop:active, .btn-stop.active {
      background: #ef4444; color: #fff; box-shadow: 0 0 20px #ef4444;
    }

    .slider-container { width: 280px; margin-top: 10px; }
    .slider-container label { font-size: 0.9rem; color: #cbd5e1; display: block; margin-bottom: 8px; }
    input[type=range] {
      width: 100%; height: 10px; border-radius: 5px;
      background: #334155; outline: none; accent-color: #38bdf8;
    }
  </style>
</head>
<body>
  <h1>🚗 Control Remoto ESP32</h1>
  <div class="status">Estado: <span id="state">Detenido</span></div>

  <div class="dpad">
    <div></div>
    <div class="btn" id="btn-up">▲</div>
    <div></div>
    <div class="btn" id="btn-left">◀</div>
    <div class="btn btn-stop" id="btn-stop">■</div>
    <div class="btn" id="btn-right">▶</div>
    <div></div>
    <div class="btn" id="btn-down">▼</div>
    <div></div>
  </div>

  <div class="slider-container">
    <label>Velocidad: <span id="spd-val">220</span></label>
    <input type="range" id="speed" min="100" max="255" value="220" oninput="document.getElementById('spd-val').innerText = this.value">
  </div>

  <script>
    let activeCmd = "stop";

    function sendCmd(cmd) {
      if (activeCmd === cmd) return;
      activeCmd = cmd;
      let spd = document.getElementById('speed').value;
      document.getElementById('state').innerText = cmd.toUpperCase();
      fetch(`/cmd?dir=${cmd}&speed=${spd}`).catch(()=>{});
    }

    function setupBtn(id, cmd) {
      let el = document.getElementById(id);
      const start = (e) => { e.preventDefault(); el.classList.add('active'); sendCmd(cmd); };
      const end = (e) => { e.preventDefault(); el.classList.remove('active'); sendCmd('stop'); };

      el.addEventListener('mousedown', start);
      el.addEventListener('mouseup', end);
      el.addEventListener('mouseleave', end);
      el.addEventListener('touchstart', start);
      el.addEventListener('touchend', end);
    }

    setupBtn('btn-up', 'forward');
    setupBtn('btn-down', 'backward');
    setupBtn('btn-left', 'left');
    setupBtn('btn-right', 'right');
    setupBtn('btn-stop', 'stop');
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleCmd() {
  String dir = server.hasArg("dir") ? server.arg("dir") : "stop";
  int spd = server.hasArg("speed") ? server.arg("speed").toInt() : 220;
  driveCar(dir, spd);
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_D3, OUTPUT); pinMode(PIN_D4, OUTPUT);
  pinMode(PIN_D5, OUTPUT); pinMode(PIN_D6, OUTPUT);
  pinMode(PIN_D7, OUTPUT); pinMode(PIN_D8, OUTPUT);
  pinMode(PIN_D11, OUTPUT); pinMode(PIN_D12, OUTPUT);

  digitalWrite(MOTORENABLE, LOW);
  latch_state = 0;
  writeLatch();

  // Modo Dual: Router Casa (STA) + AP Respaldo
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  
  // Iniciar Red de Respaldo AP
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid, ap_pass, 1, 0, 4);

  // Intentar conectar a la red Wi-Fi de Casa (Probar infinitum8788_5 e infinitum8788)
  Serial.print("Conectando a Wi-Fi de casa: "); Serial.println(sta_ssid_primary);
  WiFi.begin(sta_ssid_primary, sta_pass);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(400); Serial.print("."); attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("\nProbando red 2.4GHz: "); Serial.println(sta_ssid_secondary);
    WiFi.begin(sta_ssid_secondary, sta_pass);
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
      delay(400); Serial.print("."); attempts++;
    }
  }

  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  Serial.println("\n--- SERVIDOR WEB DEL CARRITO CONFIGURADO ---");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("¡CONECTADO A TU WI-FI DE CASA! IP: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Modo AP Respaldo Activo. IP: http://192.168.4.1");
  }

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();
}

void loop() {
  server.handleClient();
}

