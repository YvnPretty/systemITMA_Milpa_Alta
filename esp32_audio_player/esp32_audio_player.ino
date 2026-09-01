/*
 * =====================================================================
 * ESP32 ULTIMATE RETRO AUDIO PLAYER + CYBERPUNK OLED ANIMATION SUITE
 * =====================================================================
 * Audio: Megalovania, Super Mario Bros, Star Wars, Tetris & Sound Effects
 * Visuals: Cubo 3D, Ojo Robot Cibernético, Viaje Espacial Starfield 3D,
 *          Ecualizador de Onda Senoidal, Personaje Anime Neko Girl y HUD Cyber.
 *
 * Conexiones:
 * - Speaker/Buzzer -> GPIO 25 (y GND)
 * - OLED I2C 128x64 -> SDA: GPIO 21, SCL: GPIO 22
 * =====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#define SPEAKER_PIN 25
#define SDA_PIN 21
#define SCL_PIN 22
#define LEDC_CHANNEL 0

// Inicialización OLED SH1106 / SSD1306 (I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

// =====================================================================
// DEFINICIÓN DE NOTAS MUSICALES
// =====================================================================
#define REST      0
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951

// =====================================================================
// VARIABLES DE ANIMACIÓN Y ESTADOS
// =====================================================================
int currentAnimMode = 0;
const int TOTAL_ANIM_MODES = 5;

// 1. Cubo 3D
float angleX = 0, angleY = 0, angleZ = 0;
struct Point3D { float x; float y; float z; };
Point3D cubeNodes[8] = {
  {-14, -14, -14}, { 14, -14, -14}, { 14,  14, -14}, {-14,  14, -14},
  {-14, -14,  14}, { 14, -14,  14}, { 14,  14,  14}, {-14,  14,  14}
};
int cubeEdges[12][2] = {
  {0,1}, {1,2}, {2,3}, {3,0},
  {4,5}, {5,6}, {6,7}, {7,4},
  {0,4}, {1,5}, {2,6}, {3,7}
};

// 2. Starfield 3D
#define NUM_STARS 35
struct Star { float x, y, z; };
Star stars[NUM_STARS];

// 3. Ojo Robot
int eyeX = 64, eyeY = 36;
int targetEyeX = 64, targetEyeY = 36;
int blinkState = 0;

// 4. Waveform phase
float phaseWave = 0;

// =====================================================================
// MOTOR AUDIO (COMPATIBLE CON CUALQUIER CORE ESP32)
// =====================================================================
void playTone(int pin, int frequency, int durationMs) {
  if (frequency <= 0) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    noTone(pin);
#else
    ledcWriteTone(LEDC_CHANNEL, 0);
#endif
    delay(durationMs);
    return;
  }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  tone(pin, frequency, durationMs);
  delay(durationMs);
  noTone(pin);
#else
  static bool setupDone = false;
  if (!setupDone) {
    ledcSetup(LEDC_CHANNEL, 2000, 8);
    ledcAttachPin(pin, LEDC_CHANNEL);
    setupDone = true;
  }
  ledcWriteTone(LEDC_CHANNEL, frequency);
  delay(durationMs);
  ledcWriteTone(LEDC_CHANNEL, 0);
#endif
}

// =====================================================================
// FUNCIONES DE DIBUJO DE ANIMACIONES CHINGONAS
// =====================================================================

// --- ANIMACIÓN 0: CUBO 3D GIRATORIO CON ESPECTRO DE AUDIO ---
void render3DCube(const char* title, int noteFreq) {
  u8g2.clearBuffer();

  // Título e indicador
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "MODE: 3D WIREFRAME CUBE");
  u8g2.drawStr(0, 63, title);

  angleX += 0.08; angleY += 0.09; angleZ += 0.04;
  Point3D proj[8];

  for (int i = 0; i < 8; i++) {
    float y1 = cubeNodes[i].y * cos(angleX) - cubeNodes[i].z * sin(angleX);
    float z1 = cubeNodes[i].y * sin(angleX) + cubeNodes[i].z * cos(angleX);
    float x1 = cubeNodes[i].x;

    float x2 = x1 * cos(angleY) + z1 * sin(angleY);
    float z2 = -x1 * sin(angleY) + z1 * cos(angleY);
    float y2 = y1;

    float x3 = x2 * cos(angleZ) - y2 * sin(angleZ);
    float y3 = x2 * sin(angleZ) + y2 * cos(angleZ);

    float fov = 60.0 / (60.0 + z2 + 20);
    proj[i].x = 64 + x3 * fov * 1.5;
    proj[i].y = 34 + y3 * fov * 1.5;
  }

  for (int i = 0; i < 12; i++) {
    u8g2.drawLine((int)proj[cubeEdges[i][0]].x, (int)proj[cubeEdges[i][0]].y,
                  (int)proj[cubeEdges[i][1]].x, (int)proj[cubeEdges[i][1]].y);
  }

  // Barras de ecualizador laterales
  for (int i = 0; i < 6; i++) {
    int h = (noteFreq > 0) ? ((noteFreq * (i + 1)) % 25 + 4) : 2;
    u8g2.drawBox(2 + i * 4, 52 - h, 3, h);
    u8g2.drawBox(102 + i * 4, 52 - h, 3, h);
  }

  u8g2.sendBuffer();
}

// --- ANIMACIÓN 1: OJO CYBERPUNK CON MIRADA DINÁMICA ---
void renderRobotEye(const char* title, int noteFreq) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "MODE: CYBERPUNK AI EYE");
  u8g2.drawStr(0, 63, title);

  if (random(0, 8) == 1) {
    targetEyeX = 64 + random(-22, 22);
    targetEyeY = 34 + random(-8, 8);
  }
  eyeX += (targetEyeX - eyeX) * 0.3;
  eyeY += (targetEyeY - eyeY) * 0.3;

  if (random(0, 15) == 1 && blinkState == 0) blinkState = 1;

  if (blinkState > 0) {
    int h = 18 - (blinkState * 6);
    if (h < 2) h = 2;
    u8g2.drawBox(eyeX - 30, eyeY - (h/2), 60, h);
    blinkState++;
    if (blinkState > 3) blinkState = 0;
  } else {
    u8g2.drawRFrame(24, 16, 80, 38, 8);
    u8g2.drawDisc(eyeX, eyeY, 9, U8G2_DRAW_ALL);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(eyeX - 3, eyeY - 3, 3, U8G2_DRAW_ALL);
    u8g2.setDrawColor(1);
    u8g2.drawHLine(10, 35, 10);
    u8g2.drawHLine(108, 35, 10);
  }

  u8g2.sendBuffer();
}

// --- ANIMACIÓN 2: WARP SPEED STARFIELD 3D ---
void renderStarfield(const char* title, int noteFreq) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "MODE: WARP SPEED 3D");
  u8g2.drawStr(0, 63, title);

  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].z -= 4.0;
    if (stars[i].z <= 0) {
      stars[i].x = random(-64, 64);
      stars[i].y = random(-32, 32);
      stars[i].z = 100;
    }

    float k = 64.0 / stars[i].z;
    int px = (int)(stars[i].x * k + 64);
    int py = (int)(stars[i].y * k + 34);

    if (px >= 0 && px < 128 && py >= 12 && py < 54) {
      int sz = (stars[i].z < 30) ? 2 : 1;
      u8g2.drawBox(px, py, sz, sz);
    }
  }

  u8g2.sendBuffer();
}

// --- ANIMACIÓN 3: SINE WAVE AUDIO SPECTRUM ---
void renderSineSpectrum(const char* title, int noteFreq) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "MODE: WAVEFORM SPECTRUM");
  u8g2.drawStr(0, 63, title);

  phaseWave += 0.25;

  // Barras de ecualizador de fondo
  for (int i = 0; i < 16; i++) {
    int barHeight = (int)(sin(phaseWave + i * 0.4) * 16 + 18);
    int x = i * 8 + 2;
    u8g2.drawBox(x, 54 - barHeight, 6, barHeight);
  }

  // Onda senoidal flotante
  for (int x = 0; x < 128; x++) {
    int y = 24 + sin((x * 0.1) + phaseWave) * 8;
    u8g2.drawPixel(x, y);
  }

  u8g2.sendBuffer();
}

// --- ANIMACIÓN 4: PERSONAJE ANIME NEKO GIRL ---
void renderAnimeGirl(const char* title, int noteFreq) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont10_tr);
  u8g2.drawStr(0, 8, "MODE: ANIME NEKO GIRL");

  static float shift = 0;
  shift += 0.15;
  float hairMove = sin(shift) * 2.0;

  // Orejas de gato
  u8g2.drawTriangle(36, 20, 28, 6, 48, 16);
  u8g2.drawTriangle(92, 20, 100, 6, 80, 16);

  // Cabello trasero
  u8g2.drawBox(22 + (int)hairMove, 25, 10, 30);
  u8g2.drawBox(96 + (int)hairMove, 25, 10, 30);

  // Rostro
  u8g2.drawDisc(64, 30, 20, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
  u8g2.drawTriangle(44, 30, 84, 30, 64, 50);

  // Limpiar zona de ojos/boca
  u8g2.setDrawColor(0);
  u8g2.drawBox(48, 22, 32, 24);
  u8g2.setDrawColor(1);

  // Ojos Anime Grandes
  u8g2.drawRFrame(50, 24, 10, 14, 3);
  u8g2.drawRFrame(68, 24, 10, 14, 3);
  u8g2.drawBox(53, 27, 4, 6);
  u8g2.drawBox(71, 27, 4, 6);

  // Boca sonriente / cantando
  if (noteFreq > 0) {
    u8g2.drawDisc(64, 43, 3, U8G2_DRAW_ALL); // boca abierta cantando
  } else {
    u8g2.drawPixel(62, 42); u8g2.drawPixel(63, 43); u8g2.drawPixel(64, 43); u8g2.drawPixel(65, 43); u8g2.drawPixel(66, 42);
  }

  // Corazones / Notas flotantes
  if (noteFreq > 0) {
    int fx = 15 + (int)(sin(shift * 2) * 8);
    int fy = 40 - (int)((millis() / 50) % 25);
    u8g2.drawStr(fx, fy, "<3");
    u8g2.drawStr(100 - fx, fy, "♪");
  }

  u8g2.drawStr(0, 63, title);
  u8g2.sendBuffer();
}

// Selector central de renderizado
void renderFrame(const char* title, int noteFreq) {
  switch (currentAnimMode) {
    case 0: render3DCube(title, noteFreq); break;
    case 1: renderRobotEye(title, noteFreq); break;
    case 2: renderStarfield(title, noteFreq); break;
    case 3: renderSineSpectrum(title, noteFreq); break;
    case 4: renderAnimeGirl(title, noteFreq); break;
  }
}

// =====================================================================
// REPRODUCCIÓN DE CANCIONES
// =====================================================================

// 1. MEGALOVANIA (UNDERTALE)
const int megalovaniaMelody[] = {
  NOTE_D4, 8, NOTE_D4, 8, NOTE_D5, 4, NOTE_A4, 4, REST, 8, NOTE_GS4, 8, REST, 8, NOTE_G4, 8, REST, 8, NOTE_F4, 4, NOTE_D4, 8, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_C4, 8, NOTE_C4, 8, NOTE_D5, 4, NOTE_A4, 4, REST, 8, NOTE_GS4, 8, REST, 8, NOTE_G4, 8, REST, 8, NOTE_F4, 4, NOTE_D4, 8, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_B3, 8, NOTE_B3, 8, NOTE_D5, 4, NOTE_A4, 4, REST, 8, NOTE_GS4, 8, REST, 8, NOTE_G4, 8, REST, 8, NOTE_F4, 4, NOTE_D4, 8, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_AS3, 8, NOTE_AS3, 8, NOTE_D5, 4, NOTE_A4, 4, REST, 8, NOTE_GS4, 8, REST, 8, NOTE_G4, 8, REST, 8, NOTE_F4, 4, NOTE_D4, 8, NOTE_F4, 8, NOTE_G4, 8
};

// 2. SUPER MARIO BROS THEME
const int marioMelody[] = {
  NOTE_E5, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, REST, 8, NOTE_C5, 8, NOTE_E5, 4,
  NOTE_G5, 4, REST, 4, NOTE_G4, 4, REST, 4,
  NOTE_C5, 4, REST, 8, NOTE_G4, 4, REST, 8, NOTE_E4, 4,
  NOTE_A4, 4, NOTE_B4, 4, NOTE_AS4, 8, NOTE_A4, 4,
  NOTE_G4, 6, NOTE_E5, 6, NOTE_G5, 6, NOTE_A5, 4, NOTE_F5, 8, NOTE_G5, 8
};

// 3. STAR WARS IMPERIAL MARCH
const int starWarsMelody[] = {
  NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 4, NOTE_F4, 6, NOTE_C5, 16,
  NOTE_A4, 4, NOTE_F4, 6, NOTE_C5, 16, NOTE_A4, 2,
  NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_F5, 6, NOTE_C5, 16,
  NOTE_GS4, 4, NOTE_F4, 6, NOTE_C5, 16, NOTE_A4, 2
};

// 4. TETRIS THEME
const int tetrisMelody[] = {
  NOTE_E5, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_C5, 8, NOTE_B4, 8,
  NOTE_A4, 4, NOTE_A4, 8, NOTE_C5, 8, NOTE_E5, 4, NOTE_D5, 8, NOTE_C5, 8,
  NOTE_B4, 4, NOTE_B4, 8, NOTE_C5, 8, NOTE_D5, 4, NOTE_E5, 4,
  NOTE_C5, 4, NOTE_A4, 4, NOTE_A4, 4, REST, 4
};

void playMelodyWithVisuals(const char* title, const int melody[], int notesCount, int tempoMs) {
  for (int i = 0; i < notesCount * 2; i += 2) {
    int note = melody[i];
    int divider = melody[i + 1];
    int noteDuration = tempoMs / divider;

    renderFrame(title, note);

    if (note != REST) {
      playTone(SPEAKER_PIN, note, (int)(noteDuration * 0.88));
    } else {
      delay(noteDuration);
    }
    delay((int)(noteDuration * 0.12));
  }

  renderFrame(title, REST);
  delay(300);
}

// Efectos retro
void soundPowerUp() {
  int powerNotes[] = {NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G5, NOTE_B5, NOTE_D6};
  for (int n : powerNotes) {
    renderFrame("SOUND: POWER-UP!", n);
    playTone(SPEAKER_PIN, n, 60);
  }
}

// =====================================================================
// SETUP & LOOP
// =====================================================================
void setup() {
  Serial.begin(115200);
  pinMode(SPEAKER_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);

  // Inicializar estrellas 3D
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = random(-64, 64);
    stars[i].y = random(-32, 32);
    stars[i].z = random(1, 100);
  }

  // Pantalla de Inicio Cyberpunk
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(16, 25, "CYBER AUDIO 32");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(24, 45, "INITIALIZING...");
  u8g2.sendBuffer();
  
  soundPowerUp();
  delay(1000);
}

void loop() {
  // 1. MEGALOVANIA
  currentAnimMode = 0; // Cubo 3D
  int count1 = sizeof(megalovaniaMelody) / sizeof(megalovaniaMelody[0]) / 2;
  playMelodyWithVisuals("MEGALOVANIA - 3D CUBE", megalovaniaMelody, count1, 1150);

  // 2. SUPER MARIO BROS
  currentAnimMode = 4; // Anime Girl
  int count2 = sizeof(marioMelody) / sizeof(marioMelody[0]) / 2;
  playMelodyWithVisuals("MARIO - ANIME GIRL", marioMelody, count2, 1350);

  // 3. STAR WARS
  currentAnimMode = 1; // Ojo Robot AI
  int count3 = sizeof(starWarsMelody) / sizeof(starWarsMelody[0]) / 2;
  playMelodyWithVisuals("STAR WARS - AI EYE", starWarsMelody, count3, 1750);

  // 4. TETRIS THEME
  currentAnimMode = 2; // Warp Starfield 3D
  int count4 = sizeof(tetrisMelody) / sizeof(tetrisMelody[0]) / 2;
  playMelodyWithVisuals("TETRIS - WARP SPEED", tetrisMelody, count4, 1250);

  // 5. EFECTOS CYBER SPECTRUM
  currentAnimMode = 3; // Waveform spectrum
  soundPowerUp();
  delay(1000);
}
