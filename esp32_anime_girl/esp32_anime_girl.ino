/*
 * ESP32 OLED 128x64 - Anime Girl Character Animation
 * Dibuja un personaje Anime animado con expresiones, parpadeo, movimiento de cabello
 * y efectos de destellos/corazones en vivo.
 *
 * Pantalla OLED SSD1306/SH1106 I2C (SDA=21, SCL=22)
 */

#include <Wire.h>
#include <U8g2lib.h>

#define SDA_PIN 21
#define SCL_PIN 22

// Driver para pantalla OLED SH1106 128x64 por Hardware I2C (SDA=21, SCL=22)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL_PIN, /* data=*/ SDA_PIN);

// Estados de animación
int frameCount = 0;
int blinkState = 0;       // 0=abierto, 1=cerrando, 2=cerrado, 3=abriendo
int expressionMode = 0;   // 0=Normal/Feliz, 1=Guiño, 2=Hablando, 3=Sorprendida
unsigned long lastExprChange = 0;

// Partículas (Corazones y Destellos)
struct Particle {
  float x, y;
  float vy;
  int type; // 0=estrella, 1=corazon
};
#define NUM_PARTICLES 6
Particle particles[NUM_PARTICLES];

void initParticles() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x = random(5, 123);
    particles[i].y = random(40, 60);
    particles[i].vy = -0.4 - (random(0, 5) / 10.0);
    particles[i].type = random(0, 2);
  }
}

void updateParticles() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].y += particles[i].vy;
    if (particles[i].y < 2) {
      particles[i].x = random(5, 123);
      particles[i].y = random(50, 62);
    }
  }
}

void drawSparkle(int x, int y, int size) {
  u8g2.drawLine(x - size, y, x + size, y);
  u8g2.drawLine(x, y - size, x, y + size);
}

void drawHeart(int x, int y) {
  u8g2.drawPixel(x - 1, y); u8g2.drawPixel(x + 1, y);
  u8g2.drawPixel(x - 2, y - 1); u8g2.drawPixel(x, y - 1); u8g2.drawPixel(x + 2, y - 1);
  u8g2.drawPixel(x - 1, y - 2); u8g2.drawPixel(x + 1, y - 2);
}

void drawAnimeGirl() {
  u8g2.clearBuffer();

  // Actualizar física de cabello (oscilación por viento)
  float hairShift = sin(frameCount * 0.1) * 2.0;

  // -----------------------------------------------------------
  // 1. DIBUJAR OREJAS DE GATO / MOÑO (Cat Ears Accessories)
  // -----------------------------------------------------------
  // Oreja Izquierda
  u8g2.drawTriangle(36, 18, 28, 4, 48, 14);
  u8g2.drawTriangle(38, 16, 32, 8, 45, 14); // interior
  // Oreja Derecha
  u8g2.drawTriangle(92, 18, 100, 4, 80, 14);
  u8g2.drawTriangle(90, 16, 96, 8, 83, 14); // interior

  // -----------------------------------------------------------
  // 2. CABELLO TRASERO Y CONTORNO DE CABEZA
  // -----------------------------------------------------------
  // Mechones traseros largos
  u8g2.drawBox(22 + (int)hairShift, 25, 12, 35);
  u8g2.drawBox(94 + (int)hairShift, 25, 12, 35);

  // Cabeza (Rostro)
  u8g2.setDrawColor(1);
  u8g2.drawDisc(64, 30, 22, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
  // Barbilla suave anime
  u8g2.drawTriangle(42, 30, 86, 30, 64, 52);
  u8g2.drawBox(43, 28, 42, 12);

  // Clear espacio del rostro para ojos y boca
  u8g2.setDrawColor(0);
  u8g2.drawBox(46, 22, 36, 26);
  u8g2.setDrawColor(1);

  // -----------------------------------------------------------
  // 3. MECHONES DE CABELLO FRONTAL (Bangs)
  // -----------------------------------------------------------
  int hS = (int)hairShift;
  // Flequillo central
  u8g2.drawTriangle(54 + hS, 12, 64 + hS, 26, 74 + hS, 12);
  // Mechón izquierdo
  u8g2.drawTriangle(40 + hS, 14, 52 + hS, 28, 58 + hS, 14);
  // Mechón derecho
  u8g2.drawTriangle(70 + hS, 14, 76 + hS, 28, 88 + hS, 14);

  // -----------------------------------------------------------
  // 4. CEJAS
  // -----------------------------------------------------------
  u8g2.drawLine(48, 21, 56, 20);
  u8g2.drawLine(72, 20, 80, 21);

  // -----------------------------------------------------------
  // 5. OJOS ANIMADOS (Grandes estilo Anime)
  // -----------------------------------------------------------
  // Manejar parpadeo automático
  if (random(0, 40) == 1 && blinkState == 0) blinkState = 1;
  if (blinkState == 1) { blinkState = 2; }
  else if (blinkState == 2) { blinkState = 3; }
  else if (blinkState == 3) { blinkState = 0; }

  // Ojo Izquierdo (X: 47..57, Y: 24..35)
  if (blinkState > 0 || expressionMode == 1) { // Guiño / Cerrado
    u8g2.drawLine(47, 30, 53, 30);
    u8g2.drawLine(49, 29, 51, 29);
  } else {
    // Contorno ojo izquierdo
    u8g2.drawRFrame(47, 24, 10, 13, 3);
    u8g2.drawBox(49, 26, 6, 9); // Pupila
    u8g2.setDrawColor(0);
    u8g2.drawPixel(50, 27); // Brillo superior
    u8g2.drawPixel(53, 31); // Brillo inferior
    u8g2.setDrawColor(1);
    u8g2.drawLine(46, 23, 58, 23); // Pestaña superior
  }

  // Ojo Derecho (X: 71..81, Y: 24..35)
  if (blinkState > 0) { // Cerrado por parpadeo
    u8g2.drawLine(71, 30, 77, 30);
    u8g2.drawLine(73, 29, 75, 29);
  } else {
    // Contorno ojo derecho
    u8g2.drawRFrame(71, 24, 10, 13, 3);
    u8g2.drawBox(73, 26, 6, 9); // Pupila
    u8g2.setDrawColor(0);
    u8g2.drawPixel(74, 27); // Brillo superior
    u8g2.drawPixel(77, 31); // Brillo inferior
    u8g2.setDrawColor(1);
    u8g2.drawLine(70, 23, 82, 23); // Pestaña superior
  }

  // -----------------------------------------------------------
  // 6. NARIZ Y SONROJO (Blush)
  // -----------------------------------------------------------
  u8g2.drawPixel(64, 35); // Naríz suave
  // Rubor en mejillas
  u8g2.drawLine(44, 33, 47, 33);
  u8g2.drawLine(81, 33, 84, 33);

  // -----------------------------------------------------------
  // 7. BOCA ANIMADA SEGÚN EXPRESIÓN
  // -----------------------------------------------------------
  if (expressionMode == 2) { // Hablando (boca abierta oscilante)
    int mouthOpen = (frameCount % 4 > 1) ? 4 : 1;
    u8g2.drawRBox(61, 40, 6, mouthOpen + 1, 1);
  } else if (expressionMode == 3) { // Sorprendida 'o'
    u8g2.drawCircle(64, 41, 3, U8G2_DRAW_ALL);
  } else { // Smile :3 / ^_^
    u8g2.drawLine(61, 40, 64, 42);
    u8g2.drawLine(64, 42, 67, 40);
  }

  // -----------------------------------------------------------
  // 8. PARTÍCULAS AMBIENTALES (Corazones y Destellos)
  // -----------------------------------------------------------
  updateParticles();
  for (int i = 0; i < NUM_PARTICLES; i++) {
    if (particles[i].type == 0) {
      drawSparkle((int)particles[i].x, (int)particles[i].y, 2);
    } else {
      drawHeart((int)particles[i].x, (int)particles[i].y);
    }
  }

  // -----------------------------------------------------------
  // 9. GLOBOS DE TEXTO / SUBTÍTULOS ANIMADOS
  // -----------------------------------------------------------
  u8g2.setFont(u8g2_font_profont10_tr);
  
  if (expressionMode == 0) {
    u8g2.drawStr(2, 62, "<3 ANIME OLED ESP32 <3");
  } else if (expressionMode == 1) {
    u8g2.drawStr(12, 62, "WINK! ;-) READY!");
  } else if (expressionMode == 2) {
    u8g2.drawStr(8, 62, "KONNICHIWA USER! :D");
  } else {
    u8g2.drawStr(15, 62, "SUGOI! ESP32 AI!");
  }

  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);

  initParticles();
  lastExprChange = millis();
}

void loop() {
  frameCount++;

  // Cambiar expresión cada 3.5 segundos
  if (millis() - lastExprChange > 3500) {
    lastExprChange = millis();
    expressionMode = (expressionMode + 1) % 4;
  }

  drawAnimeGirl();

  delay(30); // ~33 FPS fluidos
}
