/*
 * ESP32 Matrix Digital Rain + Real-Time Wi-Fi Sniffer HUD
 * Controlador OLED SH1106 128x64 por Hardware I2C (SDA=21, SCL=22)
 *
 * Características:
 * - Lluvia de código estilo Matrix en tiempo real.
 * - Modo Promiscuo Wi-Fi (captura paquetes en el aire).
 * - Rotación automática de canales (Hopping CH 1-13).
 * - Telemetría de paquetes por segundo (PKTS/s), total de APs y red detectada.
 */

#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <U8g2lib.h>

#define SDA_PIN 21
#define SCL_PIN 22

// Driver para pantalla OLED SH1106 128x64 por Hardware I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL_PIN, /* data=*/ SDA_PIN);

// -------------------------------------------------------------
// VARIABLES MATRIX RAIN
// -------------------------------------------------------------
#define MATRIX_COLS 16 // 128px / 8px por carácter = 16 columnas
struct MatrixColumn {
  int y;
  int speed;
  int length;
  char chars[8];
};
MatrixColumn columns[MATRIX_COLS];

// Caracteres aleatorios estilo Matrix / Hexadecimal
const char matrixGlyphs[] = "0123456789ABCDEF*$#@%+=-<>:;[]{}";

// -------------------------------------------------------------
// VARIABLES TELEMETRÍA WI-FI SNIFFER
// -------------------------------------------------------------
volatile unsigned long totalPackets = 0;
volatile unsigned long packetsLastSec = 0;
volatile unsigned long packetCounterTmp = 0;
unsigned long lastSecTimer = 0;

int currentChannel = 1;
unsigned long lastChannelHop = 0;

String lastDetectedSSID = "SCANNING...";
int lastDetectedRSSI = -90;
int totalAPsFound = 0;

// Callback de captura de paquetes en modo promiscuo
void IRAM_ATTR wifi_promiscuous_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
  packetCounterTmp++;
  totalPackets++;

  // Extraer RSSI si es paquete Beacon o de Datos
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  int rssi = pkt->rx_ctrl.rssi;
  
  if (rssi > -60) {
    lastDetectedRSSI = rssi;
  }
}

// Inicialización de la lluvia Matrix
void initMatrix() {
  for (int i = 0; i < MATRIX_COLS; i++) {
    columns[i].y = random(-15, 0);
    columns[i].speed = random(1, 4);
    columns[i].length = random(4, 8);
    for (int j = 0; j < 8; j++) {
      columns[i].chars[j] = matrixGlyphs[random(0, sizeof(matrixGlyphs) - 1)];
    }
  }
}

// Actualización de la física de la lluvia Matrix
void updateMatrix() {
  for (int i = 0; i < MATRIX_COLS; i++) {
    columns[i].y += columns[i].speed;
    
    // Cambiar caracteres aleatoriamente
    if (random(0, 4) == 1) {
      int idx = random(0, 8);
      columns[i].chars[idx] = matrixGlyphs[random(0, sizeof(matrixGlyphs) - 1)];
    }

    // Reiniciar columna cuando sale de la pantalla
    if (columns[i].y - columns[i].length * 8 > 64) {
      columns[i].y = random(-10, 0);
      columns[i].speed = random(1, 3);
      columns[i].length = random(4, 7);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializar Pantalla OLED SH1106
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);

  initMatrix();

  // Configurar Wi-Fi en Modo Promiscuo (Sniffer)
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_promiscuous_cb);
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

  // Realizar un escaneo inicial rápido de redes
  int n = WiFi.scanNetworks(true); // Escaneo asíncrono
  if (n > 0) totalAPsFound = n;

  lastSecTimer = millis();
  lastChannelHop = millis();
}

void loop() {
  unsigned long now = millis();

  // 1. Calcular paquetes por segundo (PKTS/s)
  if (now - lastSecTimer >= 1000) {
    packetsLastSec = packetCounterTmp;
    packetCounterTmp = 0;
    lastSecTimer = now;

    // Actualizar conteo de APs
    int scanRes = WiFi.scanComplete();
    if (scanRes >= 0) {
      totalAPsFound = scanRes;
      if (scanRes > 0) {
        lastDetectedSSID = WiFi.SSID(random(0, scanRes));
      }
      WiFi.scanDelete();
      WiFi.scanNetworks(true); // Reiniciar escaneo en fondo
    }
  }

  // 2. Rotación de Canales (Channel Hopping 1 a 13)
  if (now - lastChannelHop >= 350) {
    lastChannelHop = now;
    currentChannel++;
    if (currentChannel > 13) currentChannel = 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  }

  // 3. Renderizado de Gráficos en la OLED
  updateMatrix();
  u8g2.clearBuffer();

  // --- DIBUJAR LLUVIA MATRIX ---
  u8g2.setFont(u8g2_font_profont10_tr);
  for (int i = 0; i < MATRIX_COLS; i++) {
    int x = i * 8;
    for (int j = 0; j < columns[i].length; j++) {
      int y = columns[i].y - (j * 8);
      if (y >= 10 && y <= 54) {
        // Cabeza de la columna (más brillante/destacada)
        if (j == 0) {
          u8g2.drawGlyph(x, y, columns[i].chars[j]);
        } else {
          // Cola atenuada
          if ((x + y) % 2 == 0) {
            u8g2.drawGlyph(x, y, columns[i].chars[j]);
          }
        }
      }
    }
  }

  // --- OVERLAY HUD MATRIX HACKER ---
  // Banner Superior
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 128, 10);
  u8g2.setDrawColor(1);
  u8g2.drawHLine(0, 10, 128);

  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(1, 8, "MATRIX SNIFFER");
  
  String chStr = "CH:" + String(currentChannel);
  u8g2.drawStr(98, 8, chStr.c_str());

  // Alerta central de Redes/Señal
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 26, 128, 16);
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 26, 128, 16);

  String alertStr = "> " + lastDetectedSSID;
  if (alertStr.length() > 14) alertStr = alertStr.substring(0, 13) + ".";
  u8g2.drawStr(4, 37, alertStr.c_str());

  String rssiStr = String(lastDetectedRSSI) + "dB";
  u8g2.drawStr(96, 37, rssiStr.c_str());

  // Banner Inferior de Telemetría
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 54, 128, 10);
  u8g2.setDrawColor(1);
  u8g2.drawHLine(0, 53, 128);

  String pktStr = "PKT:" + String(packetsLastSec) + "/s";
  u8g2.drawStr(1, 63, pktStr.c_str());

  String apStr = "APs:" + String(totalAPsFound);
  u8g2.drawStr(85, 63, apStr.c_str());

  u8g2.sendBuffer();
  delay(20);
}
