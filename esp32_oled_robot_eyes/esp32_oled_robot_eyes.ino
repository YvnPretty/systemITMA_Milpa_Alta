/*
 * =================================================================================
 * PROYECTO: ESP32 OLED ROBOT ASSISTANT EYES & RGB BREATHING SYNC
 * DESCRIPCIÓN: Ojos interactivos de asistente robot con interpolación suave (easing)
 *              que expresan emociones, parpadean y cambian de color el LED RGB.
 *              Soporta comandos seriales para cambiar estados de ánimo.
 * 
 * CONFIGURACIÓN DE PINES:
 *  - OLED I2C:  SCL = GPIO 21, SDA = GPIO 22 (autodetección inteligente)
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
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2_ssd(U8G2_R0, 21, 22, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_SW_I2C  u8g2_sh(U8G2_R0,  21, 22, U8X8_PIN_NONE);

// Puntero a la pantalla activa
U8G2 *u8g2 = &u8g2_ssd;
bool useSH1106 = false; // Se puede alternar con el comando serial 'X'

// -------------------------------------------------------------
// CONTROL DE ANIMACIONES DE OJOS (EASING)
// -------------------------------------------------------------
struct Eye {
  float currentX, currentY;
  float targetX, targetY;
  float currentW, currentH;
  float targetW, targetH;
  float defaultX, defaultY;
};

Eye leftEye;
Eye rightEye;

enum Mood {
  MOOD_NORMAL,
  MOOD_HAPPY,
  MOOD_SAD,
  MOOD_ANGRY,
  MOOD_SLEEPY,
  MOOD_CONFUSED,
  MOOD_WINK
};

Mood currentMood = MOOD_NORMAL;

// Timers para comportamiento autónomo
unsigned long lastEyeMove = 0;
unsigned long eyeMoveInterval = 2000;
unsigned long lastBlink = 0;
unsigned long blinkInterval = 4000;
bool isBlinking = false;
unsigned long blinkStart = 0;

// Variables para animación de ronquido (Zzz)
struct Snore {
  float x, y;
  float size;
  float speedY;
  bool active;
};
#define MAX_SNORES 3
Snore snores[MAX_SNORES];

// -------------------------------------------------------------
// CONFIGURACIÓN E INICIALIZACIÓN
// -------------------------------------------------------------
void setRGBColor(int r, int g, int b) {
  analogWrite(PIN_RED,   constrain(r, 0, 255));
  analogWrite(PIN_GREEN, constrain(g, 0, 255));
  analogWrite(PIN_BLUE,  constrain(b, 0, 255));
}

void scanI2C() {
  Serial.println("\n🔍 Buscando pantalla OLED en bus I2C...");
  setRGBColor(0, 0, 150); // LED Azul tenue durante escaneo

  int testPairs[][2] = {
    {21, 22}, // SCL=21, SDA=22 (Por defecto ESP32)
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
        Serial.print("🟢 ¡PANTALLA DETECTADA! Dirección: 0x");
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
    Serial.println("❌ ERROR: No se encontró respuesta I2C en ningún par de pines.");
    Serial.println("Por favor, verifica las conexiones VCC, GND, SDA y SCL.");
    setRGBColor(255, 0, 0); // Rojo error constante
  } else {
    setRGBColor(0, 255, 0); // Verde éxito
    delay(400);
  }
}

float getMoodHeight(Mood m) {
  switch (m) {
    case MOOD_NORMAL:   return 32;
    case MOOD_HAPPY:    return 30;
    case MOOD_SAD:      return 26;
    case MOOD_ANGRY:    return 26;
    case MOOD_SLEEPY:   return 5;
    case MOOD_CONFUSED: return 24;
    case MOOD_WINK:     return 32;
  }
  return 32;
}

float getMoodWidth(Mood m) {
  switch (m) {
    case MOOD_NORMAL:   return 24;
    case MOOD_HAPPY:    return 24;
    case MOOD_SAD:      return 24;
    case MOOD_ANGRY:    return 24;
    case MOOD_SLEEPY:   return 24;
    case MOOD_CONFUSED: return 20;
    case MOOD_WINK:     return 24;
  }
  return 24;
}

void setMood(Mood m) {
  currentMood = m;
  
  if (m == MOOD_CONFUSED) {
    leftEye.targetW = 16;
    leftEye.targetH = 16;
    rightEye.targetW = 28;
    rightEye.targetH = 28;
  } else if (m == MOOD_WINK) {
    leftEye.targetW = 24;
    leftEye.targetH = 30;
    rightEye.targetW = 24;
    rightEye.targetH = 2; // Ojo cerrado (línea)
  } else {
    leftEye.targetW = getMoodWidth(m);
    leftEye.targetH = getMoodHeight(m);
    rightEye.targetW = getMoodWidth(m);
    rightEye.targetH = getMoodHeight(m);
  }

  // Regresar targets de posición a por defecto
  leftEye.targetX = leftEye.defaultX;
  leftEye.targetY = leftEye.defaultY;
  rightEye.targetX = rightEye.defaultX;
  rightEye.targetY = rightEye.defaultY;

  // Limpiar snores si no está dormido
  if (m != MOOD_SLEEPY) {
    for (int i = 0; i < MAX_SNORES; i++) snores[i].active = false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  setRGBColor(0, 0, 255);
  delay(300);

  scanI2C();

  // Reconfigurar los objetos con los pines dinámicos
  u8g2_ssd = U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, sclPin, sdaPin, U8X8_PIN_NONE);
  u8g2_sh  = U8G2_SH1106_128X64_NONAME_F_SW_I2C(U8G2_R0, sclPin, sdaPin, U8X8_PIN_NONE);
  
  u8g2 = &u8g2_ssd;
  u8g2->begin();
  u8g2->setContrast(255);

  // Inicializar geometría por defecto de los ojos
  leftEye.defaultX = 40;
  leftEye.defaultY = 32;
  leftEye.currentX = leftEye.defaultX;
  leftEye.currentY = leftEye.defaultY;
  leftEye.targetX = leftEye.defaultX;
  leftEye.targetY = leftEye.defaultY;

  rightEye.defaultX = 88;
  rightEye.defaultY = 32;
  rightEye.currentX = rightEye.defaultX;
  rightEye.currentY = rightEye.defaultY;
  rightEye.targetX = rightEye.defaultX;
  rightEye.targetY = rightEye.defaultY;

  setMood(MOOD_NORMAL);

  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_helvB10_tr);
  u8g2->drawStr(22, 28, "ROBOT EYES");
  u8g2->setFont(u8g2_font_6x10_tr);
  u8g2->drawStr(12, 48, "[ INICIANDO SISTEMA ]");
  u8g2->drawFrame(0, 0, 128, 64);
  u8g2->sendBuffer();
  delay(1000);

  Serial.println("\n🤖 Sistema de Ojos Robot Iniciado.");
  Serial.println("Comandos seriales para cambiar expresiones:");
  Serial.println(" 'N' -> Normal");
  Serial.println(" 'H' -> Feliz (Happy)");
  Serial.println(" 'S' -> Triste (Sad)");
  Serial.println(" 'A' -> Enojado (Angry)");
  Serial.println(" 'C' -> Confundido");
  Serial.println(" 'P' -> Durmiendo (Sleepy)");
  Serial.println(" 'W' -> Guiño (Wink)");
  Serial.println(" 'B' -> Forzar parpadeo (Blink)");
  Serial.println(" 'X' -> Cambiar driver de pantalla (SSD1306 <-> SH1106)");
}

// -------------------------------------------------------------
// CONTROL DEL LED RGB CON EFECTO RESPIRACIÓN
// -------------------------------------------------------------
void updateRGBBreathing() {
  static float pulseAngle = 0;
  pulseAngle += 0.06;
  if (pulseAngle > 6.28) pulseAngle -= 6.28;

  // Modulación de brillo sinusoidal suave (de 0.6 a 1.0)
  float pulse = 0.75 + 0.25 * sin(pulseAngle);
  
  int r = 0, g = 0, b = 0;
  switch (currentMood) {
    case MOOD_NORMAL:   r = 0;   g = 180; b = 255; break; // Cian
    case MOOD_HAPPY:    r = 0;   g = 255; b = 0;   break; // Verde
    case MOOD_SAD:      r = 0;   g = 0;   b = 255; break; // Azul
    case MOOD_ANGRY:    r = 255; g = 0;   b = 0;   break; // Rojo
    case MOOD_SLEEPY:   
      r = 40;  g = 0;   b = 80;  
      pulse = 0.35 + 0.25 * sin(pulseAngle * 0.5); // Respiración más lenta y oscura
      break;
    case MOOD_CONFUSED: r = 255; g = 0;   b = 255; break; // Magenta
    case MOOD_WINK:     r = 255; g = 80;  b = 150; break; // Rosado
  }

  setRGBColor((int)(r * pulse), (int)(g * pulse), (int)(b * pulse));
}

// -------------------------------------------------------------
// LÓGICA Y DIBUJO DE EXPRESIONES EN OLED
// -------------------------------------------------------------
void processAIBehavior() {
  unsigned long now = millis();

  // Administrador de parpadeo automático (evitar en Sleepy)
  if (now - lastBlink > blinkInterval && currentMood != MOOD_SLEEPY) {
    isBlinking = true;
    blinkStart = now;
    blinkInterval = random(3000, 6000);
    lastBlink = now;
  }

  if (isBlinking) {
    unsigned long elapsed = now - blinkStart;
    if (elapsed < 100) {
      leftEye.targetH = 2;
      rightEye.targetH = 2;
    } else if (elapsed < 200) {
      leftEye.targetH = getMoodHeight(currentMood);
      rightEye.targetH = getMoodHeight(currentMood);
    } else {
      isBlinking = false;
    }
  }

  // Movimiento ocular de look-around autónomo (no en Sleepy ni parpadeo)
  if (currentMood != MOOD_SLEEPY && !isBlinking) {
    if (now - lastEyeMove > eyeMoveInterval) {
      lastEyeMove = now;
      eyeMoveInterval = random(2000, 5000);

      int dx = random(-6, 7);
      int dy = random(-4, 5);

      if (currentMood == MOOD_CONFUSED) {
        // En confundido, los ojos miran en diferentes direcciones levemente
        leftEye.targetX = leftEye.defaultX + dx;
        leftEye.targetY = leftEye.defaultY + dy;
        rightEye.targetX = rightEye.defaultX + random(-6, 7);
        rightEye.targetY = rightEye.defaultY + random(-4, 5);
      } else {
        leftEye.targetX = leftEye.defaultX + dx;
        leftEye.targetY = leftEye.defaultY + dy;
        rightEye.targetX = rightEye.defaultX + dx;
        rightEye.targetY = rightEye.defaultY + dy;
      }
    }
  }
}

void updateSnoringZzz() {
  if (currentMood != MOOD_SLEEPY) return;

  static unsigned long lastSpawn = 0;
  if (millis() - lastSpawn > 1400) {
    lastSpawn = millis();
    for (int i = 0; i < MAX_SNORES; i++) {
      if (!snores[i].active) {
        snores[i].active = true;
        snores[i].x = 104 + random(-4, 4);
        snores[i].y = 44;
        snores[i].speedY = -0.35;
        snores[i].size = (random(0, 2) == 0) ? 6 : 8;
        break;
      }
    }
  }

  for (int i = 0; i < MAX_SNORES; i++) {
    if (snores[i].active) {
      snores[i].y += snores[i].speedY;
      snores[i].x += sin(millis() * 0.004) * 0.25; // Oscilación sinusoidal
      
      if (snores[i].y < 6) {
        snores[i].active = false;
      } else {
        if (snores[i].size > 6) {
          u8g2->setFont(u8g2_font_profont10_tr);
          u8g2->drawStr((int)snores[i].x, (int)snores[i].y, "Z");
        } else {
          u8g2->setFont(u8g2_font_5x7_tr);
          u8g2->drawStr((int)snores[i].x, (int)snores[i].y, "z");
        }
      }
    }
  }
}

void drawEyes() {
  u8g2->clearBuffer();

  // Actualizar interpolaciones físicas (easing)
  float easeRate = 0.22;
  leftEye.currentX  += (leftEye.targetX  - leftEye.currentX)  * easeRate;
  leftEye.currentY  += (leftEye.targetY  - leftEye.currentY)  * easeRate;
  leftEye.currentW  += (leftEye.targetW  - leftEye.currentW)  * easeRate;
  leftEye.currentH  += (leftEye.targetH  - leftEye.currentH)  * easeRate;

  rightEye.currentX += (rightEye.targetX - rightEye.currentX) * easeRate;
  rightEye.currentY += (rightEye.targetY - rightEye.currentY) * easeRate;
  rightEye.currentW += (rightEye.targetW - rightEye.currentW) * easeRate;
  rightEye.currentH += (rightEye.targetH - rightEye.currentH) * easeRate;

  // Dibujar Zzz flotantes si el robot duerme
  updateSnoringZzz();

  // 1. Dibujar estructura base de los ojos
  if (currentMood == MOOD_WINK && !isBlinking) {
    // Guiño: Ojo izquierdo normal/feliz, ojo derecho guiño (línea feliz)
    u8g2->drawRBox(leftEye.currentX - leftEye.currentW/2, leftEye.currentY - leftEye.currentH/2, leftEye.currentW, leftEye.currentH, 6);
    
    // Ojo derecho guiño: arco curvo horizontal
    int rx = (int)rightEye.currentX;
    int ry = (int)rightEye.currentY;
    u8g2->drawHLine(rx - 11, ry, 22);
    u8g2->drawPixel(rx - 12, ry + 1);
    u8g2->drawPixel(rx + 11, ry + 1);
    u8g2->drawPixel(rx - 13, ry + 2);
    u8g2->drawPixel(rx + 12, ry + 2);
  } else {
    // Dibujar ambos ojos normalmente con rectángulos redondeados
    u8g2->drawRBox(leftEye.currentX - leftEye.currentW/2, leftEye.currentY - leftEye.currentH/2, leftEye.currentW, leftEye.currentH, 6);
    u8g2->drawRBox(rightEye.currentX - rightEye.currentW/2, rightEye.currentY - rightEye.currentH/2, rightEye.currentW, rightEye.currentH, 6);
  }

  // 2. Modificaciones de expresión mediante recortes negros (setDrawColor(0))
  u8g2->setDrawColor(0);

  if (currentMood == MOOD_HAPPY && !isBlinking) {
    // Ojos felices kawaii: recorte curvo limpio y estilizado
    u8g2->drawBox(leftEye.currentX - leftEye.currentW/2 - 2, leftEye.currentY + 1, leftEye.currentW + 4, leftEye.currentH/2 + 4);
    u8g2->drawDisc(leftEye.currentX, leftEye.currentY + leftEye.currentH/2 + 4, leftEye.currentW/2);
    u8g2->drawBox(rightEye.currentX - rightEye.currentW/2 - 2, rightEye.currentY + 1, rightEye.currentW + 4, rightEye.currentH/2 + 4);
    u8g2->drawDisc(rightEye.currentX, rightEye.currentY + rightEye.currentH/2 + 4, rightEye.currentW/2);
  }
  else if (currentMood == MOOD_SAD && !isBlinking) {
    // Ojos tristes (recortar esquinas exteriores superiores)
    // Izquierdo: cortar arriba-izquierda
    u8g2->drawTriangle(
      leftEye.currentX - leftEye.currentW/2 - 2, leftEye.currentY - leftEye.currentH/2 - 2,
      leftEye.currentX + 2, leftEye.currentY - leftEye.currentH/2 - 2,
      leftEye.currentX - leftEye.currentW/2 - 2, leftEye.currentY + 2
    );
    // Derecho: cortar arriba-derecha
    u8g2->drawTriangle(
      rightEye.currentX + rightEye.currentW/2 + 2, rightEye.currentY - rightEye.currentH/2 - 2,
      rightEye.currentX - 2, rightEye.currentY - rightEye.currentH/2 - 2,
      rightEye.currentX + rightEye.currentW/2 + 2, rightEye.currentY + 2
    );
  }
  else if (currentMood == MOOD_ANGRY && !isBlinking) {
    // Ojos enojados (recortar esquinas interiores superiores para enojo "\ /")
    // Izquierdo: cortar arriba-derecha
    u8g2->drawTriangle(
      leftEye.currentX + leftEye.currentW/2 + 2, leftEye.currentY - leftEye.currentH/2 - 2,
      leftEye.currentX - 2, leftEye.currentY - leftEye.currentH/2 - 2,
      leftEye.currentX + leftEye.currentW/2 + 2, leftEye.currentY + 2
    );
    // Derecho: cortar arriba-izquierda
    u8g2->drawTriangle(
      rightEye.currentX - rightEye.currentW/2 - 2, rightEye.currentY - rightEye.currentH/2 - 2,
      rightEye.currentX + 2, rightEye.currentY - rightEye.currentH/2 - 2,
      rightEye.currentX - rightEye.currentW/2 - 2, rightEye.currentY + 2
    );
  }

  // 3. Dibujar brillos/pupilas de vida (círculo negro superior derecho)
  // No dibujar en pestañeos o guiños para no distorsionarlos
  if (!isBlinking && currentMood != MOOD_SLEEPY) {
    // Ojo Izquierdo brillo
    u8g2->drawDisc(leftEye.currentX + 3, leftEye.currentY - 3, 3);
    // Ojo Derecho brillo (solo si no es el guiño cerrado)
    if (currentMood != MOOD_WINK) {
      u8g2->drawDisc(rightEye.currentX + 3, rightEye.currentY - 3, 3);
    }
  }

  // Restaurar color estándar
  u8g2->setDrawColor(1);

  // 4. Dibujar detalles alrededor de los ojos para darle carácter
  if (currentMood == MOOD_ANGRY) {
    // Pequeña ceja enojada arriba de los ojos
    u8g2->drawLine(leftEye.currentX - 10, leftEye.currentY - leftEye.currentH/2 - 4, leftEye.currentX + 6, leftEye.currentY - leftEye.currentH/2 - 1);
    u8g2->drawLine(rightEye.currentX + 10, rightEye.currentY - rightEye.currentH/2 - 4, rightEye.currentX - 6, rightEye.currentY - rightEye.currentH/2 - 1);
  }

  u8g2->sendBuffer();
}

// -------------------------------------------------------------
// LECTURA DE COMANDOS EN SERIAL
// -------------------------------------------------------------
void checkSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Limpiar saltos de línea adicionales
    while(Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }
    
    switch (cmd) {
      case 'n': case 'N':
        Serial.println("🟢 Expresión: NORMAL");
        setMood(MOOD_NORMAL);
        break;
      case 'h': case 'H':
        Serial.println("🟢 Expresión: FELIZ (HAPPY)");
        setMood(MOOD_HAPPY);
        break;
      case 's': case 'S':
        Serial.println("🟢 Expresión: TRISTE (SAD)");
        setMood(MOOD_SAD);
        break;
      case 'a': case 'A':
        Serial.println("🟢 Expresión: ENOJADO (ANGRY)");
        setMood(MOOD_ANGRY);
        break;
      case 'c': case 'C':
        Serial.println("🟢 Expresión: CONFUNDIDO");
        setMood(MOOD_CONFUSED);
        break;
      case 'p': case 'P':
        Serial.println("🟢 Expresión: DURMIENDO (SLEEPY)");
        setMood(MOOD_SLEEPY);
        break;
      case 'w': case 'W':
        Serial.println("🟢 Expresión: GUIÑO (WINK)");
        setMood(MOOD_WINK);
        break;
      case 'b': case 'B':
        Serial.println("⚡ Forzando pestañeo instantáneo...");
        isBlinking = true;
        blinkStart = millis();
        break;
      case 'x': case 'X':
        useSH1106 = !useSH1106;
        if (useSH1106) {
          u8g2 = &u8g2_sh;
          Serial.println("🔄 Driver Cambiado: SH1106 (OLED 1.3 pulgadas)");
        } else {
          u8g2 = &u8g2_ssd;
          Serial.println("🔄 Driver Cambiado: SSD1306 (OLED 0.96 pulgadas)");
        }
        u8g2->begin();
        u8g2->setContrast(255);
        break;
      default:
        Serial.print("⚠️ Comando desconocido '");
        Serial.print(cmd);
        Serial.println("'. Comandos válidos: N, H, S, A, C, P, W, B, X");
        break;
    }
  }
}

// -------------------------------------------------------------
// BUCLE PRINCIPAL
// -------------------------------------------------------------
void loop() {
  checkSerialCommands();
  processAIBehavior();
  updateRGBBreathing();
  drawEyes();
  delay(25); // ~40 FPS fluidos
}
