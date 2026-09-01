/*
 * =================================================================================
 * ESP32 I2C MONITOR & DIAGNÓSTICO EN TIEMPO REAL
 * 
 * Este programa escanea continuamente los pines I2C y usa el LED RGB para avisar:
 *   - ROJO: Ningún dispositivo I2C detectado (Revisar cables / contactos)
 *   - VERDE: ¡Pantalla OLED detectada! (0x3C o 0x3D)
 *   - AZUL: Probando alternativas de pines
 * =================================================================================
 */

#include <Wire.h>
#include <U8g2lib.h>

#define PIN_RED   25
#define PIN_GREEN 26
#define PIN_BLUE  27

void setRGB(int r, int g, int b) {
  analogWrite(PIN_RED, constrain(r, 0, 255));
  analogWrite(PIN_GREEN, constrain(g, 0, 255));
  analogWrite(PIN_BLUE, constrain(b, 0, 255));
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  setRGB(0, 0, 255); // Azul al iniciar
  delay(1000);
  Serial.println("\n=======================================================");
  Serial.println("🔍 DIAGNÓSTICO I2C EN TIEMPO REAL INICIADO");
  Serial.println("=======================================================");
}

void loop() {
  bool found = false;
  int foundSDA = -1, foundSCL = -1, foundAddr = -1;

  // Probar configuraciones principales
  int pairs[][2] = {
    {21, 22}, // SDA=21, SCL=22 (Default ESP32)
    {22, 21}, // SDA=22, SCL=21 (Invertido)
    {18, 19},
    {4, 5}
  };

  for (int p = 0; p < 4; p++) {
    int sda = pairs[p][0];
    int scl = pairs[p][1];

    Wire.end();
    delay(20);
    Wire.begin(sda, scl);
    Wire.setTimeOut(50);

    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        found = true;
        foundSDA = sda;
        foundSCL = scl;
        foundAddr = addr;
        break;
      }
    }
    if (found) break;
  }

  if (found) {
    setRGB(0, 255, 0); // VERDE: ¡Detectada OLED!
    Serial.print("🟢 ¡DISPOSITIVO I2C ENCONTRADO! Dirección: 0x");
    if (foundAddr < 16) Serial.print("0");
    Serial.print(foundAddr, HEX);
    Serial.print(" | Pines: SDA=");
    Serial.print(foundSDA);
    Serial.print(", SCL=");
    Serial.println(foundSCL);

    // Intentar inicializar pantalla OLED con Software I2C para garantizar dibujo
    U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, foundSCL, foundSDA, U8X8_PIN_NONE);
    u8g2.begin();
    u8g2.setContrast(255);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB12_tr);
    u8g2.drawStr(10, 30, "OLED CONECTADA");
    u8g2.setFont(u8g2_font_6x10_tr);
    String str = "SDA:" + String(foundSDA) + " SCL:" + String(foundSCL);
    u8g2.drawStr(10, 50, str.c_str());
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.sendBuffer();

  } else {
    setRGB(255, 0, 0); // ROJO: Sin respuesta I2C
    Serial.println("❌ Sin respuesta I2C. Revisa cables en la protoboard / alimentación...");
  }

  delay(1500);
}
