/*
 * =================================================================================
 * PROYECTO: ESP32 OLED HACKER CYBER-SUITE & RGB SYNC (AUTO-DETECT & SW_I2C)
 * DESCRIPCIÓN: Sistema autónomo para pantallas OLED SSD1306 / SH1106 (0.96" y 1.3")
 *              con autodetección de pines I2C y LED RGB indicador de estado.
 * 
 * CONFIGURACIÓN DE PINES:
 *  - OLED I2C:  SCL = GPIO 21, SDA = GPIO 22 (probado también en modo invertido)
 *  - LED RGB:   ROJO = GPIO 25, VERDE = GPIO 26, AZUL = GPIO 27 (GND común)
 * =================================================================================
 */

#include <Wire.h>
#include <U8g2lib.h>
#include <math.h>

#define PIN_RED   25
#define PIN_GREEN 26
#define PIN_BLUE  27

// Variables de asignación dinámica de pines I2C
int sclPin = 21;
int sdaPin = 22;
byte oledAddress = 0x3C;
bool i2cDeviceFound = false;

// Instancia U8g2 en modo Software I2C (SCL, SDA) para máxima compatibilidad
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2_ssd(U8G2_R0, /* clock/SCL=*/ 21, /* data/SDA=*/ 22, /* reset=*/ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_SW_I2C  u8g2_sh(U8G2_R0,  /* clock/SCL=*/ 21, /* data/SDA=*/ 22, /* reset=*/ U8X8_PIN_NONE);

// Puntero genérico a la pantalla activa
U8G2 *u8g2 = &u8g2_ssd;

// Control de Animaciones
unsigned long lastModeSwitch = 0;
int currentMode = 0;
const int TOTAL_MODES = 5;
const unsigned long MODE_DURATION = 7000;

void setRGBColor(int r, int g, int b) {
  analogWrite(PIN_RED, constrain(r, 0, 255));
  analogWrite(PIN_GREEN, constrain(g, 0, 255));
  analogWrite(PIN_BLUE, constrain(b, 0, 255));
}

// -------------------------------------------------------------
// ESCÁNER Y AUTODETECCIÓN I2C EN SETUP
// -------------------------------------------------------------
void scanI2C() {
  Serial.println("\n🔍 Escaneando bus I2C para pantalla OLED...");
  setRGBColor(0, 0, 255); // Azul intentando conectar

  int testPairs[][2] = {
    {21, 22}, // SCL=21, SDA=22
    {22, 21}, // SCL=22, SDA=21
    {19, 18},
    {5, 4}
  };

  for (int i = 0; i < 4; i++) {
    int scl = testPairs[i][0];
    int sda = testPairs[i][1];

    Wire.end();
    delay(20);
    Wire.begin(sda, scl);
    Wire.setTimeOut(50);

    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        sclPin = scl;
        sdaPin = sda;
        oledAddress = addr;
        i2cDeviceFound = true;
        Serial.print("🟢 ¡OLED ENCONTRADA! Dirección 0x");
        Serial.print(addr, HEX);
        Serial.print(" | SCL=");
        Serial.print(scl);
        Serial.print(", SDA=");
        Serial.println(sda);
        break;
      }
    }
    if (i2cDeviceFound) break;
  }

  if (!i2cDeviceFound) {
    Serial.println("❌ ADVERTENCIA: No se detecto respuesta I2C en ningun pin.");
    Serial.println("Revisa los cables jumpers o alimentacion VCC/GND en la protoboard.");
    setRGBColor(255, 0, 0); // Rojo de error
  } else {
    setRGBColor(0, 255, 0); // Verde de éxito
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  setRGBColor(0, 0, 255);
  delay(500);

  scanI2C();

  // Reconfigurar U8g2 con los pines detectados o por defecto
  u8g2_ssd = U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, sclPin, sdaPin, U8X8_PIN_NONE);
  u8g2_ssd.begin();
  u8g2_ssd.setContrast(255);

  u8g2 = &u8g2_ssd;

  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_helvB10_tr);
  u8g2->drawStr(12, 28, "CYBER-CORE v2.0");
  u8g2->setFont(u8g2_font_6x10_tr);
  u8g2->drawStr(18, 48, "[ SYSTEM ACTIVE ]");
  u8g2->drawFrame(0, 0, 128, 64);
  u8g2->sendBuffer();
  delay(1200);
}

// -------------------------------------------------------------
// 1. MATRIX DIGITAL RAIN
// -------------------------------------------------------------
struct MatrixDrop {
  int x;
  float y;
  float speed;
  char charCode;
};
const int NUM_DROPS = 14;
MatrixDrop drops[NUM_DROPS];

void initMatrix() {
  for (int i = 0; i < NUM_DROPS; i++) {
    drops[i].x = i * 9 + 2;
    drops[i].y = random(-40, 0);
    drops[i].speed = random(15, 35) / 10.0;
    drops[i].charCode = random(33, 126);
  }
}

void drawHackerMatrix() {
  static bool initialized = false;
  if (!initialized) { initMatrix(); initialized = true; }

  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_5x7_tr);
  u8g2->drawStr(0, 7, "ROOT@ESP32:~# MATRIX");
  u8g2->drawHLine(0, 9, 128);

  static float pulse = 0;
  pulse += 0.1;
  int greenBrightness = (int)(160 + 95 * sin(pulse));
  setRGBColor(0, greenBrightness, 20);

  u8g2->setFont(u8g2_font_profont10_tr);
  for (int i = 0; i < NUM_DROPS; i++) {
    drops[i].y += drops[i].speed;
    if (drops[i].y > 64) {
      drops[i].y = random(-20, 0);
      drops[i].speed = random(15, 35) / 10.0;
      drops[i].charCode = random(33, 126);
    }

    int iy = (int)drops[i].y;
    if (iy >= 12 && iy <= 64) {
      char str[2] = { drops[i].charCode, '\0' };
      u8g2->drawStr(drops[i].x, iy, str);
      if (iy - 8 >= 12) u8g2->drawPixel(drops[i].x + 2, iy - 8);
    }
  }

  if ((int)(millis() / 500) % 2 == 0) {
    u8g2->setFont(u8g2_font_5x7_tr);
    u8g2->drawBox(20, 48, 88, 12);
    u8g2->setDrawColor(0);
    u8g2->drawStr(24, 57, "> ACCESS GRANTED <");
    u8g2->setDrawColor(1);
  }

  u8g2->sendBuffer();
}

// -------------------------------------------------------------
// 2. RADAR CYBERPUNK
// -------------------------------------------------------------
void drawCyberRadar() {
  u8g2->clearBuffer();
  
  static float angle = 0;
  angle += 0.08;
  if (angle > 6.28318) angle -= 6.28318;

  int cx = 36, cy = 36, r = 24;
  u8g2->drawCircle(cx, cy, r);
  u8g2->drawCircle(cx, cy, r / 2);
  u8g2->drawHLine(cx - r, cy, r * 2);
  u8g2->drawVLine(cx, cy - r, r * 2);

  int rx = cx + (int)(cos(angle) * r);
  int ry = cy + (int)(sin(angle) * r);
  u8g2->drawLine(cx, cy, rx, ry);

  if (sin(angle) > 0) u8g2->drawDisc(cx + 10, cy - 8, 2);
  if (cos(angle) < 0) u8g2->drawDisc(cx - 14, cy + 12, 2);

  u8g2->setFont(u8g2_font_5x7_tr);
  u8g2->drawStr(66, 12, "RADAR: ON");
  u8g2->drawStr(66, 24, "TARGET: 0x3C");
  u8g2->drawStr(66, 36, "SIG: -42dBm");
  
  static int packets = 100;
  packets += random(1, 5);
  String pktStr = "PKT: " + String(packets);
  u8g2->drawStr(66, 48, pktStr.c_str());

  u8g2->drawFrame(64, 2, 63, 60);

  int redVal = (int)(30 + 30 * cos(angle));
  int blueVal = (int)(200 + 55 * sin(angle));
  setRGBColor(redVal, 180, blueVal);

  u8g2->sendBuffer();
}

// -------------------------------------------------------------
// 3. CUBO 3D MATRIX
// -------------------------------------------------------------
float cubeAngleX = 0, cubeAngleY = 0, cubeAngleZ = 0;
struct Point3D { float x, y, z; };
Point3D cubeNodes[8] = {
  {-14, -14, -14}, { 14, -14, -14}, { 14,  14, -14}, {-14,  14, -14},
  {-14, -14,  14}, { 14, -14,  14}, { 14,  14,  14}, {-14,  14,  14}
};
int cubeEdges[12][2] = {
  {0,1}, {1,2}, {2,3}, {3,0},
  {4,5}, {5,6}, {6,7}, {7,4},
  {0,4}, {1,5}, {2,6}, {3,7}
};

void draw3DHackerCube() {
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_5x7_tr);
  u8g2->drawStr(0, 7, "3D NEURAL VECTOR RENDER");
  u8g2->drawHLine(0, 9, 128);

  cubeAngleX += 0.05;
  cubeAngleY += 0.06;
  cubeAngleZ += 0.03;

  Point3D projected[8];
  for (int i = 0; i < 8; i++) {
    float y1 = cubeNodes[i].y * cos(cubeAngleX) - cubeNodes[i].z * sin(cubeAngleX);
    float z1 = cubeNodes[i].y * sin(cubeAngleX) + cubeNodes[i].z * cos(cubeAngleX);
    float x1 = cubeNodes[i].x;

    float x2 = x1 * cos(cubeAngleY) + z1 * sin(cubeAngleY);
    float z2 = -x1 * sin(cubeAngleY) + z1 * cos(cubeAngleY);
    float y2 = y1;

    float x3 = x2 * cos(cubeAngleZ) - y2 * sin(cubeAngleZ);
    float y3 = x2 * sin(cubeAngleZ) + y2 * cos(cubeAngleZ);

    float fov = 60.0 / (60.0 + z2 + 20.0);
    projected[i].x = 64 + x3 * fov * 1.6;
    projected[i].y = 38 + y3 * fov * 1.6;
  }

  for (int i = 0; i < 12; i++) {
    u8g2->drawLine(
      (int)projected[cubeEdges[i][0]].x, (int)projected[cubeEdges[i][0]].y,
      (int)projected[cubeEdges[i][1]].x, (int)projected[cubeEdges[i][1]].y
    );
  }

  u8g2->drawStr(0, 62, "X:");
  u8g2->drawStr(10, 62, String((int)(cubeAngleX * 10)).c_str());
  u8g2->drawStr(45, 62, "Y:");
  u8g2->drawStr(55, 62, String((int)(cubeAngleY * 10)).c_str());
  u8g2->drawStr(90, 62, "FPS: 60");

  static float hue = 0;
  hue += 0.05;
  if (hue > 6.28) hue -= 6.28;
  int r = (int)(127 + 127 * sin(hue));
  int g = (int)(127 + 127 * sin(hue + 2.094));
  int b = (int)(127 + 127 * sin(hue + 4.188));
  setRGBColor(r, g, b);

  u8g2->sendBuffer();
}

// -------------------------------------------------------------
// 4. AI EYE OVERSEER
// -------------------------------------------------------------
int eyeX = 64, eyeY = 36;
int targetEyeX = 64, targetEyeY = 36;
int blinkState = 0;

void drawCyberEyeCore() {
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_5x7_tr);
  u8g2->drawStr(0, 7, "AI EYE OVERSEER v4");
  u8g2->drawHLine(0, 9, 128);

  if (random(0, 15) == 1) {
    targetEyeX = 64 + random(-25, 25);
    targetEyeY = 36 + random(-10, 10);
  }
  eyeX += (targetEyeX - eyeX) * 0.25;
  eyeY += (targetEyeY - eyeY) * 0.25;

  if (random(0, 25) == 1 && blinkState == 0) blinkState = 1;

  u8g2->drawFrame(16, 14, 96, 44);
  u8g2->drawBox(14, 12, 6, 6);
  u8g2->drawBox(108, 12, 6, 6);
  u8g2->drawBox(14, 52, 6, 6);
  u8g2->drawBox(108, 52, 6, 6);

  if (blinkState > 0) {
    int h = 20 - (blinkState * 7);
    if (h < 2) h = 2;
    u8g2->drawBox(eyeX - 25, eyeY - (h / 2), 50, h);
    blinkState++;
    if (blinkState > 3) blinkState = 0;
    setRGBColor(255, 0, 0);
  } else {
    u8g2->drawDisc(eyeX, eyeY, 11, U8G2_DRAW_ALL);
    u8g2->setDrawColor(0);
    u8g2->drawDisc(eyeX, eyeY, 5, U8G2_DRAW_ALL);
    u8g2->drawDisc(eyeX - 3, eyeY - 3, 2, U8G2_DRAW_ALL);
    u8g2->setDrawColor(1);

    static float pulse = 0;
    pulse += 0.12;
    int magentaVal = (int)(150 + 105 * sin(pulse));
    setRGBColor(magentaVal, 0, 255);
  }

  u8g2->sendBuffer();
}

// -------------------------------------------------------------
// 5. OVERCLOCK TELEMETRY
// -------------------------------------------------------------
void drawOverclockGauge() {
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_5x7_tr);
  u8g2->drawStr(0, 7, "CPU OVERCLOCK: 240MHz");
  u8g2->drawHLine(0, 9, 128);

  static int gaugeVal = 0;
  gaugeVal = (gaugeVal + 3) % 101;

  u8g2->drawCircle(64, 38, 22, U8G2_DRAW_ALL);
  u8g2->drawCircle(64, 38, 20, U8G2_DRAW_ALL);

  u8g2->setFont(u8g2_font_helvB10_tr);
  String gStr = String(gaugeVal) + "%";
  int strW = u8g2->getStrWidth(gStr.c_str());
  u8g2->drawStr(64 - (strW / 2), 43, gStr.c_str());

  u8g2->setFont(u8g2_font_4x6_tr);
  u8g2->drawStr(4, 22, "TEMP");
  u8g2->drawFrame(4, 25, 30, 6);
  u8g2->drawBox(4, 25, (gaugeVal * 30) / 100, 6);

  u8g2->drawStr(94, 22, "RAM");
  u8g2->drawFrame(94, 25, 30, 6);
  u8g2->drawBox(94, 25, ((100 - gaugeVal) * 30) / 100, 6);

  int redVal = map(gaugeVal, 0, 100, 0, 255);
  int greenVal = map(gaugeVal, 0, 100, 255, 0);
  setRGBColor(redVal, greenVal, 0);

  u8g2->sendBuffer();
}

// -------------------------------------------------------------
// BUCLE PRINCIPAL
// -------------------------------------------------------------
void loop() {
  if (millis() - lastModeSwitch > MODE_DURATION) {
    lastModeSwitch = millis();
    currentMode = (currentMode + 1) % TOTAL_MODES;
  }

  switch (currentMode) {
    case 0: drawHackerMatrix(); break;
    case 1: drawCyberRadar(); break;
    case 2: draw3DHackerCube(); break;
    case 3: drawCyberEyeCore(); break;
    case 4: drawOverclockGauge(); break;
  }

  delay(20);
}
