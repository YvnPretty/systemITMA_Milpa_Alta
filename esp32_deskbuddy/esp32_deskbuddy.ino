/*
 * =================================================================================
 * PROYECTO: ESP32 DESKBUDDY 1.0 - SMART DESKTOP PET
 * CREATOR: Rajesh K T (Edison Science Corner)
 * ADAPTACIÓN: Antigravity IDE (SDA=21, SCL=22, TOUCH=4, LED RGB=25/26/27)
 *             Soporte dinámico runtime para SSD1306 (0.96") y SH1106 (1.3")
 * =================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "time.h"
#include <math.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// ==================================================
// 1. CONFIGURACIÓN GENERAL Y PINES (CONSTANTES & MACROS)
// ==================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22
#define TOUCH_PIN 4

#define PIN_RED   25
#define PIN_GREEN 26
#define PIN_BLUE  27

#define CONFIG_AP_SSID   "DeskBuddy-Setup"
#define CONFIG_AP_PASS   "12345678"
#define CONFIG_HOLD_MS   3000

#define COLOR_WHITE 1
#define COLOR_BLACK 0

// ==================================================
// 2. ESTRUCTURAS DE DATOS
// ==================================================
struct Eye {
  float x, y;  
  float w, h;  
  float targetX, targetY, targetW, targetH;

  // Pupil Physics (Secondary Motion)
  float pupilX, pupilY;
  float targetPupilX, targetPupilY;

  // Physics constants
  float velX, velY, velW, velH;
  float pVelX, pVelY;
  float k = 0.12;   // Spring constant
  float d = 0.60;   // Damping constant
  float pk = 0.08;  // Pupil Spring
  float pd = 0.50;  // Pupil Damping

  bool blinking;
  unsigned long lastBlink;
  unsigned long nextBlinkTime;

  void init(float _x, float _y, float _w, float _h) {
    x = targetX = _x;
    y = targetY = _y;
    w = targetW = _w;
    h = targetH = _h;
    pupilX = targetPupilX = 0;
    pupilY = targetPupilY = 0;
    nextBlinkTime = millis() + random(1000, 4000);
  }

  void update() {
    // 1. Main Eye Physics
    float ax = (targetX - x) * k;
    float ay = (targetY - y) * k;
    float aw = (targetW - w) * k;
    float ah = (targetH - h) * k;

    velX = (velX + ax) * d;
    velY = (velY + ay) * d;
    velW = (velW + aw) * d;
    velH = (velH + ah) * d;

    x += velX;
    y += velY;
    w += velW;
    h += velH;

    // 2. Pupil Physics
    float pax = (targetPupilX - pupilX) * pk;
    float pay = (targetPupilY - pupilY) * pk;
    pVelX = (pVelX + pax) * pd;
    pVelY = (pVelY + pay) * pd;
    pupilX += pVelX;
    pupilY += pVelY;
  }
};

struct ForecastDay {
  String dayName;
  int temp;
  String iconType;
};

// ==================================================
// 3. VARIABLES GLOBALES E INSTANCIAS
// ==================================================
// Instanciación de displays
Adafruit_SSD1306 display_ssd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SH1106G display_sh(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_GFX *display_ptr = &display_ssd;
bool isSH1106 = false; // Toggle dinámico

// Redirigir llamadas de dibujo mediante macro (lcd para no chocar con .display())
#define lcd (*display_ptr)

// Config portal e internet
Preferences prefs;
WebServer configServer(80);
bool inConfigMode = false;

// Estado del Bot
int currentPage = 0;
bool highBrightness = true;
bool invertedColors = false;
int tapCounter = 0;
unsigned long lastTapTime = 0;
bool lastPinState = false;
unsigned long pressStartTime = 0;
bool isLongPressHandled = false;

const unsigned long LONG_PRESS_TIME = 800;
const unsigned long DOUBLE_TAP_DELAY = 300;
unsigned long lastPageSwitch = 0;
const unsigned long PAGE_INTERVAL = 8000;

// MOODS
#define MOOD_NORMAL 0
#define MOOD_HAPPY 1
#define MOOD_SURPRISED 2
#define MOOD_SLEEPY 3
#define MOOD_ANGRY 4
#define MOOD_SAD 5
#define MOOD_EXCITED 6
#define MOOD_LOVE 7
#define MOOD_SUSPICIOUS 8
int currentMood = MOOD_NORMAL;

// Clima y localización
String city;       
String countryCode; 
String apiKey;   
String wifiSsid; 
String wifiPass; 
unsigned long lastWeatherUpdate = 0;
float temperature = 0.0;
float feelsLike = 0.0;
int humidity = 0;
String weatherMain = "Loading";
String weatherDesc = "Wait...";

ForecastDay fcast[3];
const char* ntpServer = "pool.ntp.org";
String tzString;   

// --- TAMAGOTCHI CARE SYSTEM ---
int petHunger = 85;
int petHappiness = 90;
unsigned long lastCareDecay = 0;
bool showFoodAnim = false;
unsigned long foodAnimStart = 0;

// --- REAL STOPWATCH (CRONOMETRO REAL ASCENDENTE) ---
bool chronoRunning = false;
unsigned long chronoElapsed = 0;
unsigned long chronoStart = 0;

// --- WEATHER PARTICLES ---
struct RainDrop { int x, y, speed, len; };
RainDrop raindrops[10];

// --- QUOTES TICKER ---
const char* quotes[] = {
  "Animo Tecomitl!",
  "Toma agua 💧",
  "Pausa y estirate 🧘",
  "Codigo limpio 💻",
  "¡Excelente dia! ✨",
  "Sigue adelante 🚀"
};
int currentQuoteIdx = 0;
unsigned long lastQuoteSwitch = 0;

// --- MINI GAME: DODGE PET ---
float playerY = 32.0;
float playerVelY = 0.0;
int obstacleX = 128;
int obstacleGapY = 24;
int gameScore = 0;
int gameHighScore = 0;
bool gameOver = false;

// Ojos
Eye leftEye, rightEye;
unsigned long lastSaccade = 0;
unsigned long saccadeInterval = 3000;
float breathVal = 0;
bool wifiInitialized = false;

// ==================================================
// BITMAP IMAGES (Sin marcadores de posición)
// ==================================================
const unsigned char bmp_clear[] PROGMEM = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xc0, 0x80, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x00, 0x7f, 0xfe, 0x00, 0x00, 0xff, 0xff, 0x00, 0x06, 0xff, 0xff, 0x60, 0x06, 0xff, 0xff, 0x60, 
  0x06, 0xff, 0xff, 0x60, 0x00, 0xff, 0xff, 0x00, 0x3e, 0xff, 0xff, 0x7c, 0x3e, 0xff, 0xff, 0x7c, 
  0x3e, 0xff, 0xff, 0x7c, 0x00, 0xff, 0xff, 0x00, 0x06, 0xff, 0xff, 0x60, 0x06, 0xff, 0xff, 0x60, 
  0x06, 0xff, 0xff, 0x60, 0x00, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x01, 0x0f, 0xf0, 0x80, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 
  0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char bmp_clouds[] PROGMEM = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 
  0x00, 0x0f, 0xf8, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0x00, 
  0x00, 0x7f, 0xff, 0x80, 0x00, 0xff, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xf0, 
  0x03, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xfe, 
  0x0f, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 
  0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xfe, 
  0x07, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0xf8, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xff, 0x80, 
  0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char bmp_rain[] PROGMEM = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x3f, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0x80, 0x00, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xf0, 
  0x03, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 
  0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xfe, 
  0x07, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0xf8, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xff, 0x80, 
  0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x00, 0x60, 0x0c, 0x00, 
  0x00, 0xe0, 0x1c, 0x00, 0x00, 0xc0, 0x18, 0x00, 0x03, 0x80, 0x70, 0x00, 0x03, 0x80, 0x70, 0x00, 
  0x03, 0x00, 0x60, 0x00, 0x02, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char mini_sun[] PROGMEM = { 
  0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x10, 0x08, 0x04, 0x20, 0x03, 0xc0, 0x27, 0xe4, 0x07, 0xe0, 
  0x07, 0xe0, 0x27, 0xe4, 0x03, 0xc0, 0x04, 0x20, 0x10, 0x08, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00 
};

const unsigned char mini_cloud[] PROGMEM = { 
  0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 0x3f, 0xfc, 
  0x3f, 0xfc, 0x7f, 0xfe, 0x3f, 0xfe, 0x1f, 0xfc, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char mini_rain[] PROGMEM = { 
  0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 0x3f, 0xfc, 
  0x3f, 0xfc, 0x7f, 0xfe, 0x3f, 0xfe, 0x1f, 0xfc, 0x00, 0x00, 0x44, 0x44, 0x22, 0x22, 0x11, 0x11 
};

const unsigned char bmp_tiny_drop[] PROGMEM = { 
  0x10, 0x38, 0x7c, 0xfe, 0xfe, 0x7c, 0x38, 0x00 
};

const unsigned char bmp_heart[] PROGMEM = { 
  0x00, 0x00, 0x0c, 0x60, 0x1e, 0xf0, 0x3f, 0xf8, 0x7f, 0xfc, 0x7f, 0xfc, 0x7f, 0xfc, 0x3f, 0xf8, 
  0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char bmp_zzz[] PROGMEM = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x30, 0x00, 0x7e, 
  0x00, 0x00, 0x3c, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x30, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char bmp_anger[] PROGMEM = { 
  0x00, 0x00, 0x11, 0x10, 0x2a, 0x90, 0x44, 0x40, 0x80, 0x20, 0x80, 0x20, 0x44, 0x40, 0x2a, 0x90, 
  0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char bmp_apple[] PROGMEM = { 
  0x00, 0x30, 0x00, 0x20, 0x01, 0x40, 0x0e, 0x00, 0x1f, 0xc0, 0x3f, 0xe0, 0x7f, 0xf0, 0x7f, 0xf0, 
  0x7f, 0xf0, 0x7f, 0xf0, 0x3f, 0xe0, 0x1f, 0xc0, 0x1f, 0xc0, 0x0c, 0x80, 0x00, 0x00, 0x00, 0x00 
};

// ==================================================
// 4. FORWARD DECLARATIONS (DECLARACIONES DE FUNCIONES)
// ==================================================
void setRGBColor(int r, int g, int b);
void updateRGBStatus();
void initDisplay();
void clearDisplay();
void showDisplay();
void setDisplayContrast(uint8_t contrast);
void setInvertedColors(bool invert);
void initRainParticles();
void drawRainParticles();
void updateTamagotchi();
void feedPet();
void patPet();
void drawQuotesTicker();
void drawPomodoroPage();
void togglePomodoro();
void resetStopwatch();
void startMiniGame();
void updateMiniGame();
void drawMiniGame();
void drawEmoPage();
void handleMobileDashboard();
void handleStatusJson();
void setupWebDashboardRoutes();
void getWeatherAndForecast();
void updateMoodBasedOnWeather();
void handleTouch();
void checkSerialCommands();
void playBootAnimation();
void startConfigPortal();
void loadConfig();
void saveConfig(const String& s, const String& p, const String& ak, const String& cty, const String& ctry, const String& tz);
void handleConfigRoot();
void handleConfigSave();
void drawEyelidMask(float x, float y, float w, float h, int mood, bool isLeft);
void drawEyebrows(Eye& e, bool isLeft);
void drawFilledHeart(int cx, int cy, int size, uint16_t color);
void drawLoveEye(Eye& e, bool isLeft);
void drawHappyEye(Eye& e, bool isLeft);
void drawUltraKawaiiEye(Eye& e, bool isLeft);
void updatePhysicsAndMood();
void drawCuteMouth();
void drawEmoPage();
void drawForecastPage();
void drawClock();
void drawWeatherCard();
void drawWorldClock();
void drawTecNMLogoPage();
const unsigned char* getBigIcon(String w);
const unsigned char* getMiniIcon(String w);

// ==================================================
// 5. IMPLEMENTACIÓN DE FUNCIONES
// ==================================================

void setRGBColor(int r, int g, int b) {
  analogWrite(PIN_RED,   constrain(r, 0, 255));
  analogWrite(PIN_GREEN, constrain(g, 0, 255));
  analogWrite(PIN_BLUE,  constrain(b, 0, 255));
}

void initDisplay() {
  Serial.println("-> Iniciando bus I2C (SDA=21, SCL=22)...");
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setTimeOut(100); // Previene cuelgues si la pantalla no responde o falta pull-up
  
  if (isSH1106) {
    Serial.println("-> Inicializando driver SH1106...");
    display_ptr = &display_sh;
    display_sh.begin(0x3C, true);
    display_sh.setTextColor(COLOR_WHITE);
  } else {
    Serial.println("-> Inicializando driver SSD1306...");
    display_ptr = &display_ssd;
    display_ssd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display_ssd.setTextColor(COLOR_WHITE);
  }
  Serial.println("-> Pantalla configurada.");
}

void clearDisplay() {
  if (isSH1106) display_sh.clearDisplay();
  else display_ssd.clearDisplay();
}

void showDisplay() {
  if (isSH1106) display_sh.display();
  else display_ssd.display();
}

void setDisplayContrast(uint8_t contrast) {
  if (!isSH1106) {
    display_ssd.ssd1306_command(SSD1306_SETCONTRAST);
    display_ssd.ssd1306_command(contrast);
  } else {
    display_sh.setContrast(contrast);
  }
}

void setInvertedColors(bool invert) {
  invertedColors = invert;
  if (!isSH1106) {
    display_ssd.invertDisplay(invert);
  } else {
    display_sh.invertDisplay(invert);
  }
  showDisplay();
}

void loadConfig() {
  prefs.begin("deskbuddy", true);
  wifiSsid    = prefs.getString("ssid", "");
  wifiPass    = prefs.getString("pass", "");
  apiKey      = prefs.getString("apikey", "");
  city        = prefs.getString("city", "");
  countryCode = prefs.getString("country", "");
  tzString    = prefs.getString("tz", "");
  prefs.end();
  
  if (wifiSsid.isEmpty() || city == "Chicago" || city == "Mexico City" || tzString.indexOf("CDT") != -1 || city.isEmpty()) {
    wifiSsid    = "INFINITUM8788_2.4";
    wifiPass    = "chkQwQ9Y9f";
    apiKey      = "45fcf5807a5920e2006c2b8a077d423f";
    city        = "Tecomitl";
    countryCode = "MX";
    tzString    = "CST6"; // San Antonio Tecómitl, CDMX UTC-6

    saveConfig(wifiSsid, wifiPass, apiKey, city, countryCode, tzString);
  }
}

void saveConfig(const String& s, const String& p, const String& ak,
                const String& cty, const String& ctry, const String& tz) {
  prefs.begin("deskbuddy", false);
  prefs.putString("ssid", s);
  prefs.putString("pass", p);
  prefs.putString("apikey", ak);
  prefs.putString("city", cty);
  prefs.putString("country", ctry);
  prefs.putString("tz", tz);
  prefs.end();
}

void handleConfigRoot() {
  prefs.begin("deskbuddy", true);
  String sSsid = prefs.getString("ssid", "");
  String sApik = prefs.getString("apikey", "");
  String sCity = prefs.getString("city", "Chicago");
  String sCtry = prefs.getString("country", "US");
  String sTz   = prefs.getString("tz", "CST6CDT,M3.2.0,M11.1.0");
  prefs.end();

  String html = R"rawliteral(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>DeskBuddy Config</title>
<style>
body{font-family:sans-serif;max-width:420px;margin:30px auto;padding:24px;background:#0c1929;color:#e8f4fc;}
h1{color:#5ba3f5;margin-bottom:8px;}
input{width:100%;padding:10px;margin:6px 0;border:1px solid #2d4a6f;border-radius:6px;box-sizing:border-box;background:#1a2d47;color:#e8f4fc;}
input:focus{outline:none;border-color:#5ba3f5;}
button{width:100%;padding:12px;background:#3498db;color:#fff;border:none;border-radius:6px;font-size:16px;cursor:pointer;margin-top:16px;}
button:hover{background:#2980b9;}
label{display:block;margin-top:14px;color:#8ab4e8;font-size:14px;}
.section{margin-top:20px;padding-top:16px;border-top:1px solid #1e3a5f;}
.section-title{color:#5ba3f5;font-size:13px;margin-bottom:8px;}
</style></head><body>
<h1>DeskBuddy Setup</h1>
<form action="/save" method="POST">
<label>WiFi SSID</label><input name="ssid" placeholder="Your WiFi name" value=")rawliteral";
  html += sSsid;
  html += R"rawliteral(">
<label>WiFi Password</label><input name="pass" type="password" placeholder="WiFi password">
<div class="section"><div class="section-title">Weather (OpenWeatherMap)</div>
<label>API Key</label><input name="apikey" placeholder="API key" value=")rawliteral";
  html += sApik;
  html += R"rawliteral(">
<label>City</label><input name="city" placeholder="e.g. London" value=")rawliteral";
  html += sCity;
  html += R"rawliteral(">
<label>Country Code</label><input name="country" placeholder="e.g. US, MX, ES" value=")rawliteral";
  html += sCtry;
  html += R"rawliteral(">
</div>
<div class="section"><div class="section-title">Time</div>
<label>Timezone String</label><input name="tz" placeholder="e.g. CST6CDT, IST-5:30" value=")rawliteral";
  html += sTz;
  html += R"rawliteral(">
</div>
<button type="submit">Save &amp; Reboot</button>
</form></body></html>)rawliteral";
  configServer.send(200, "text/html", html);
}

void handleConfigSave() {
  if (!configServer.hasArg("ssid") || configServer.arg("ssid").length() == 0) {
    configServer.send(400, "text/plain", "SSID required");
    return;
  }
  String s   = configServer.arg("ssid");
  String p   = configServer.arg("pass");
  String ak  = configServer.arg("apikey");
  String cty = configServer.arg("city");
  String ctr = configServer.arg("country");
  String tz  = configServer.arg("tz");
  
  prefs.begin("deskbuddy", true);
  if (ak.isEmpty())  ak  = prefs.getString("apikey", "45fcf5807a5920e2006c2b8a077d423f");
  if (cty.isEmpty()) cty = prefs.getString("city", "Chicago");
  if (ctr.isEmpty()) ctr = prefs.getString("country", "US");
  if (tz.isEmpty())  tz  = prefs.getString("tz", "CST6CDT,M3.2.0,M11.1.0");
  prefs.end();
  
  saveConfig(s, p, ak, cty, ctr, tz);
  configServer.send(200, "text/html",
    "<html><body style='font-family:sans-serif;background:#0c1929;color:#e8f4fc;padding:40px;'>"
    "<h2 style='color:#5ba3f5'>Saved!</h2><p>Rebooting in 2 seconds...</p></body></html>");
  delay(2000);
  ESP.restart();
}

void startConfigPortal() {
  inConfigMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PASS);
  configServer.on("/", handleConfigRoot);
  configServer.on("/save", HTTP_POST, handleConfigSave);
  configServer.begin();
  
  clearDisplay();
  lcd.setFont(NULL);
  lcd.setCursor(0, 0);
  lcd.print("Portal de Setup\n\nConecta WiFi a:\n");
  lcd.print(CONFIG_AP_SSID);
  lcd.print("\n\nEntra en tu web:\n192.168.4.1");
  showDisplay();
}

void updateRGBStatus() {
  if (!wifiInitialized) {
    setRGBColor(0, 0, 180); // Azul fijo durante inicio
    return;
  }

  if (inConfigMode) {
    setRGBColor(255, 80, 0); // Amarillo/Naranja estático
    return;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    // Parpadeo azul rápido al no estar conectado
    if (millis() % 600 < 300) {
      setRGBColor(0, 0, 180);
    } else {
      setRGBColor(0, 0, 0);
    }
    return;
  }

  // Latido/respiración de brillo suave sinusoidal (0.75 a 1.0)
  float pulse = 0.8 + 0.2 * sin(millis() / 450.0);

  int r = 0, g = 0, b = 0;
  
  if (currentPage == 0) { // Ojos / Emociones
    switch (currentMood) {
      case MOOD_NORMAL:   r = 0;   g = 180; b = 255; break; // Cyan
      case MOOD_HAPPY:    r = 0;   g = 255; b = 0;   break; // Verde
      case MOOD_LOVE:     r = 255; g = 100; b = 180; break; // Rosa
      case MOOD_SLEEPY:   
        r = 30;  g = 0;   b = 70;  
        pulse = 0.5 + 0.3 * sin(millis() / 900.0); // Respiración más lenta
        break;
      case MOOD_ANGRY:    r = 255; g = 0;   b = 0;   break; // Rojo
      case MOOD_SAD:      r = 0;   g = 0;   b = 255; break; // Azul
      case MOOD_EXCITED:  r = 150; g = 255; b = 0;   break; // Lima
      case MOOD_SURPRISED:r = 255; g = 150; b = 0;   break; // Amarillo
      case MOOD_SUSPICIOUS:r = 255; g = 0;  b = 255; break; // Violeta
      default:            r = 0;   g = 180; b = 255; break;
    }
  } else if (currentPage == 1) { // Reloj
    r = 90; g = 120; b = 180; // Blanco-Azulino suave
  } else if (currentPage == 2) { // Clima actual
    r = 0; g = 180; b = 180; // Turquesa
  } else if (currentPage == 3) { // Reloj mundial
    r = 80; g = 40; b = 120; // Morado
  } else if (currentPage == 4) { // Pronóstico de 3 días
    r = 20; g = 140; b = 50; // Verde azulado
  } else if (currentPage == 5) { // Cronómetro
    if (chronoRunning) { r = 0; g = 255; b = 100; } // Verde en ejecución
    else { r = 255; g = 100; b = 0; } // Naranja en pausa/idle
  } else if (currentPage == 6) { // Mini juego
    r = 255; g = 200; b = 0; // Amarillo Arcade
  } else if (currentPage == 7) { // Logo ITMA II
    r = 255; g = 180; b = 0; // Dorado ITMA II
  }

  setRGBColor((int)(r * pulse), (int)(g * pulse), (int)(b * pulse));
}

void handleTouch() {
  bool currentPinState = digitalRead(TOUCH_PIN);
  unsigned long now = millis();
  
  if (currentPinState && !lastPinState) {
    pressStartTime = now;
    isLongPressHandled = false;
  } 
  else if (currentPinState && lastPinState) {
    if ((now - pressStartTime > LONG_PRESS_TIME) && !isLongPressHandled) {
      lastPageSwitch = now;
      if (currentPage == 0) {
        currentMood++;
        if (currentMood > MOOD_SUSPICIOUS) currentMood = 0;
        lastSaccade = 0;
      } else if (currentPage == 1) {
        isSH1106 = !isSH1106;
        initDisplay();
        Serial.print("🔄 Driver de pantalla cambiado via TOUCH HOLD: ");
        Serial.println(isSH1106 ? "SH1106 (1.3\")" : "SSD1306 (0.96\")");
      }
      else if (currentPage == 2) currentPage = 4;
      else if (currentPage == 5) resetStopwatch();
      else if (currentPage == 6) startMiniGame();
      isLongPressHandled = true;
    }
  } 
  else if (!currentPinState && lastPinState) {
    if ((now - pressStartTime < LONG_PRESS_TIME) && !isLongPressHandled) {
      if (currentPage == 6) {
        if (gameOver) startMiniGame();
        else playerVelY = -4.2f;
      } else {
        tapCounter++;
        lastTapTime = now;
      }
    }
  }
  
  lastPinState = currentPinState;
  
  if (tapCounter > 0) {
    if (now - lastTapTime > DOUBLE_TAP_DELAY) {
      lastPageSwitch = now;
      if (tapCounter == 2) {
        if (currentPage == 5) {
          currentPage = 6;
          startMiniGame();
        } else {
          highBrightness = !highBrightness;
          setDisplayContrast(highBrightness ? 255 : 1);
          showDisplay();
        }
      } else if (tapCounter == 1) {
        if (currentPage == 5) {
          togglePomodoro();
        } else if (currentPage == 3) currentPage = 1;
        else if (currentPage == 4) currentPage = 2;
        else {
          if (currentPage == 0) currentPage = 7;
          else if (currentPage == 7) currentPage = 1;
          else if (currentPage == 1) currentPage = 2;
          else if (currentPage == 2) currentPage = 5;
          else if (currentPage == 5) currentPage = 6;
          else if (currentPage == 6) currentPage = 0;
          else currentPage = 0;

          if (currentPage == 0) {
            leftEye.init(16, 12, 36, 38);
            rightEye.init(76, 12, 36, 38);
          }
          if (currentPage == 6) startMiniGame();
        }
      }
      tapCounter = 0;
    }
  }
}

void getWeatherAndForecast() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  
  // Consultar por coordenadas GPS precisas de San Antonio Tecómitl (19.2214, -99.0003)
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=19.2214&lon=-99.0003&appid=" + apiKey + "&units=metric";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    JSONVar myObject = JSON.parse(payload);
    if (JSON.typeof(myObject) != "undefined") {
      temperature = double(myObject["main"]["temp"]);
      feelsLike = double(myObject["main"]["feels_like"]);
      humidity = int(myObject["main"]["humidity"]);
      weatherMain = (const char*)myObject["weather"][0]["main"];
      weatherDesc = (const char*)myObject["weather"][0]["description"];
      if (weatherDesc.length() > 0) weatherDesc[0] = toupper(weatherDesc[0]);
      updateMoodBasedOnWeather();
    }
  } else {
    Serial.printf("Error HTTP Clima: %d\n", httpCode);
  }
  http.end();
  
  url = "http://api.openweathermap.org/data/2.5/forecast?lat=19.2214&lon=-99.0003&appid=" + apiKey + "&units=metric";
  http.begin(url);
  httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    JSONVar fo = JSON.parse(payload);
    if (JSON.typeof(fo) != "undefined") {
      struct tm t;
      getLocalTime(&t);
      int today = t.tm_wday;
      const char* days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
      int indices[3] = { 7, 15, 23 };
      for (int i = 0; i < 3; i++) {
        int idx = indices[i];
        fcast[i].temp = (int)double(fo["list"][idx]["main"]["temp"]);
        fcast[i].iconType = (const char*)fo["list"][idx]["weather"][0]["main"];
        int nextDayIndex = (today + i + 1) % 7;
        fcast[i].dayName = days[nextDayIndex];
      }
    }
  } else {
    Serial.printf("Error HTTP Pronostico: %d\n", httpCode);
  }
  http.end();
}

void updateMoodBasedOnWeather() {
  int m = MOOD_NORMAL;
  if (weatherMain == "Clear") m = MOOD_HAPPY;
  else if (weatherMain == "Rain" || weatherMain == "Drizzle") m = MOOD_SAD;
  else if (weatherMain == "Thunderstorm") m = MOOD_SURPRISED;
  else if (weatherMain == "Clouds") m = MOOD_NORMAL;
  else if (temperature > 26) m = MOOD_EXCITED;
  else if (temperature < 8) m = MOOD_SLEEPY;
  currentMood = m;
}

void drawEyelidMask(float x, float y, float w, float h, int mood, bool isLeft) {
  int ix = (int)x;
  int iy = (int)y;
  int iw = (int)w;
  int ih = (int)h;

  // ANGRY: Recorte afilado interior
  if (mood == MOOD_ANGRY) {
    if (isLeft) {
      for (int i = 0; i < 16; i++) lcd.drawLine(ix, iy + i, ix + iw, iy - 6 + i, COLOR_BLACK);
    } else {
      for (int i = 0; i < 16; i++) lcd.drawLine(ix, iy - 6 + i, ix + iw, iy + i, COLOR_BLACK);
    }
  }
  // SAD: Recorte exterior inclinado
  else if (mood == MOOD_SAD) {
    if (isLeft) {
      for (int i = 0; i < 16; i++) lcd.drawLine(ix, iy - 6 + i, ix + iw, iy + i, COLOR_BLACK);
    } else {
      for (int i = 0; i < 16; i++) lcd.drawLine(ix, iy + i, ix + iw, iy - 6 + i, COLOR_BLACK);
    }
  }
  // HAPPY/LOVE/EXCITED: Recorte curvo desde abajo (crescent shape)
  else if (mood == MOOD_HAPPY || mood == MOOD_LOVE || mood == MOOD_EXCITED) {
    lcd.fillRect(ix, iy + ih - 12, iw, 14, COLOR_BLACK);
    lcd.fillCircle(ix + iw / 2, iy + ih + 6, iw / 1.3, COLOR_BLACK);
  }
  // SLEEPY: Párpados pesados cubriendo la mitad superior
  else if (mood == MOOD_SLEEPY) {
    lcd.fillRect(ix, iy, iw, ih / 2 + 2, COLOR_BLACK);
  }
  // SUSPICIOUS: Guiño izquierdo y ojo derecho abierto
  else if (mood == MOOD_SUSPICIOUS) {
    if (isLeft) lcd.fillRect(ix, iy, iw, ih / 2 - 2, COLOR_BLACK);
    else lcd.fillRect(ix, iy + ih - 8, iw, 8, COLOR_BLACK);
  }
}

void drawEyebrows(Eye& e, bool isLeft) {
  int ix = (int)e.x;
  int iy = (int)e.y;
  int iw = (int)e.w;

  if (currentMood == MOOD_ANGRY) {
    if (isLeft) {
      lcd.fillTriangle(ix, iy - 10, ix + iw + 4, iy - 2, ix, iy - 5, COLOR_WHITE);
    } else {
      lcd.fillTriangle(ix - 4, iy - 2, ix + iw, iy - 10, ix + iw, iy - 5, COLOR_WHITE);
    }
  } else if (currentMood == MOOD_SAD) {
    if (isLeft) {
      lcd.fillTriangle(ix, iy - 2, ix + iw, iy - 10, ix, iy - 6, COLOR_WHITE);
    } else {
      lcd.fillTriangle(ix, iy - 10, ix + iw, iy - 2, ix + iw, iy - 6, COLOR_WHITE);
    }
  } else if (currentMood == MOOD_SURPRISED) {
    lcd.drawCircle(ix + iw / 2, iy - 6, iw / 3, COLOR_WHITE);
    lcd.drawCircle(ix + iw / 2, iy - 7, iw / 3, COLOR_WHITE);
  } else if (currentMood == MOOD_HAPPY || currentMood == MOOD_LOVE || currentMood == MOOD_EXCITED) {
    int bounce = (int)(sin(millis() / 150.0) * 1.5);
    lcd.fillRoundRect(ix + 4, iy - 8 + bounce, iw - 8, 3, 1, COLOR_WHITE);
  } else if (currentMood == MOOD_SUSPICIOUS) {
    if (isLeft) lcd.fillRoundRect(ix + 2, iy - 4, iw - 4, 3, 1, COLOR_WHITE);
    else lcd.drawCircle(ix + iw / 2, iy - 8, iw / 3, COLOR_WHITE);
  } else {
    // Normal subtle eyebrow
    lcd.fillRoundRect(ix + 6, iy - 6, iw - 12, 2, 1, COLOR_WHITE);
  }
}

void drawFilledHeart(int cx, int cy, int size, uint16_t color) {
  int r = size / 2;
  if (r < 2) r = 2;
  lcd.fillCircle(cx - r / 2, cy - r / 3, r / 2 + 1, color);
  lcd.fillCircle(cx + r / 2, cy - r / 3, r / 2 + 1, color);
  lcd.fillTriangle(cx - r - 1, cy - r / 4, cx + r + 1, cy - r / 4, cx, cy + r + 1, color);
}

void drawLoveEye(Eye& e, bool isLeft) {
  int cx = (int)e.x + (int)e.w / 2;
  int cy = (int)e.y + (int)e.h / 2;

  // Latido de corazón suave y tierno (rítmico)
  float beat = 1.0f + 0.14f * sin(millis() / 140.0f);
  int outerSize = (int)(e.w * 0.48f * beat);
  int innerSize = (int)(outerSize * 0.52f);

  if (outerSize < 6) outerSize = 6;

  // 1. Esclerótica exterior en forma de corazón blanco brillante
  drawFilledHeart(cx, cy, outerSize, COLOR_WHITE);

  // 2. Pupila interior en forma de corazón negro expresivo
  if (innerSize >= 3) {
    drawFilledHeart(cx, cy, innerSize, COLOR_BLACK);
    // Destello blanco especular de vida en el corazón
    lcd.fillCircle(cx + innerSize / 3, cy - innerSize / 3, 1, COLOR_WHITE);
  }

  // 3. Destellos / Chispas enamoradas flotando alrededor de los ojos
  int sparkOffset = (int)(sin(millis() / 180.0f + (isLeft ? 0.0f : 3.14f)) * 2.0f);
  if (isLeft) {
    lcd.drawPixel(cx - outerSize - 4, cy - 4 + sparkOffset, COLOR_WHITE);
    lcd.drawPixel(cx - outerSize - 3, cy - 3 + sparkOffset, COLOR_WHITE);
    lcd.drawPixel(cx + outerSize + 3, cy - 6 - sparkOffset, COLOR_WHITE);
  } else {
    lcd.drawPixel(cx + outerSize + 4, cy - 4 + sparkOffset, COLOR_WHITE);
    lcd.drawPixel(cx + outerSize + 3, cy - 3 + sparkOffset, COLOR_WHITE);
    lcd.drawPixel(cx - outerSize - 3, cy - 6 - sparkOffset, COLOR_WHITE);
  }

  // 4. Cejas de felicidad y amor
  drawEyebrows(e, isLeft);
}

void drawHappyEye(Eye& e, bool isLeft) {
  int cx = (int)e.x + (int)e.w / 2;
  int cy = (int)e.y + (int)e.h / 2;

  // Rebote alegre y dinámico estilo kawaii
  int bounce = (int)(sin(millis() / 130.0f) * 2.0f);
  cy += bounce;

  int radius = (int)(e.w / 2.2f);
  if (radius < 4) radius = 4;

  // 1. Arco sonriente principal (Arco blanco suave recortado)
  lcd.fillCircle(cx, cy, radius, COLOR_WHITE);
  lcd.fillCircle(cx, cy + 3, radius - 3, COLOR_BLACK);
  // Limpieza limpia de la mitad inferior
  lcd.fillRect(cx - radius - 3, cy + 1, (radius + 3) * 2, radius + 6, COLOR_BLACK);

  // 2. Extremos curvados hacia arriba estilo kawaii (^ ^)
  lcd.drawPixel(cx - radius, cy, COLOR_WHITE);
  lcd.drawPixel(cx - radius + 1, cy - 1, COLOR_WHITE);
  lcd.drawPixel(cx + radius, cy, COLOR_WHITE);
  lcd.drawPixel(cx + radius - 1, cy - 1, COLOR_WHITE);

  // 3. Sonrojo sonriente en las mejillas bajo el ojo
  if (isLeft) {
    lcd.drawFastHLine(cx - radius + 1, cy + 6, 4, COLOR_WHITE);
    lcd.drawFastHLine(cx - radius + 2, cy + 8, 4, COLOR_WHITE);
  } else {
    lcd.drawFastHLine(cx + radius - 5, cy + 6, 4, COLOR_WHITE);
    lcd.drawFastHLine(cx + radius - 6, cy + 8, 4, COLOR_WHITE);
  }

  // 4. Cejas alegres
  drawEyebrows(e, isLeft);
}

void drawUltraKawaiiEye(Eye& e, bool isLeft) {
  int ix = (int)e.x;
  int iy = (int)e.y;
  int iw = (int)e.w;
  int ih = (int)e.h;

  if (ih <= 4) {
    // Parpadeo tierno: una hermosa curva sonriente (^ ^)
    lcd.drawFastHLine(ix + 2, iy + 2, iw - 4, COLOR_WHITE);
    lcd.drawPixel(ix + 1, iy + 3, COLOR_WHITE);
    lcd.drawPixel(ix + iw - 2, iy + 3, COLOR_WHITE);
    return;
  }

  // Renderizado dedicado de alta calidad para AMOR y FELIZ
  if (currentMood == MOOD_LOVE) {
    drawLoveEye(e, isLeft);
    return;
  }
  if (currentMood == MOOD_HAPPY) {
    drawHappyEye(e, isLeft);
    return;
  }

  // 1. Esclerótica (Fondo blanco del ojo súper redondeado y suave)
  int r = 12;
  if (iw < 24) r = 4;
  lcd.fillRoundRect(ix, iy, iw, ih, r, COLOR_WHITE);

  // 2. Pupila (Gran pupila negra interior expresiva)
  int cx = ix + iw / 2;
  int cy = iy + ih / 2;
  int pw = iw * 0.58;
  int ph = ih * 0.58;

  int px = cx + (int)e.pupilX - (pw / 2);
  int py = cy + (int)e.pupilY - (ph / 2);

  // Limitar rango de pupila dentro del ojo
  if (px < ix + 2) px = ix + 2;
  if (px + pw > ix + iw - 2) px = ix + iw - pw - 2;
  if (py < iy + 2) py = iy + 2;
  if (py + ph > iy + ih - 2) py = iy + ih - ph - 2;

  lcd.fillRoundRect(px, py, pw, ph, r / 2, COLOR_BLACK);

  // 3. Destellos Kawaii Especulares (3 destellos de ternura!)
  if (iw > 16 && ih > 16) {
    // Destello 1: Círculo brillante grande superior derecho
    lcd.fillCircle(px + pw - 4, py + 4, 2, COLOR_WHITE);
    // Destello 2: Punto brillante secundario inferior izquierdo
    lcd.fillCircle(px + 4, py + ph - 4, 1, COLOR_WHITE);
    // Destello 3: Micro destello superior izquierdo
    lcd.drawPixel(px + 4, py + 4, COLOR_WHITE);
  }

  // 4. Aplicar máscaras de emociones y cejas expresivas
  drawEyelidMask(e.x, e.y, e.w, e.h, currentMood, isLeft);
  drawEyebrows(e, isLeft);
}

void updatePhysicsAndMood() {
  unsigned long now = millis();
  breathVal = sin(now / 800.0) * 1.5;

  // --- LOGICA DE PARPADEO ---
  if (now > leftEye.nextBlinkTime) {
    leftEye.blinking = true;
    leftEye.lastBlink = now;
    rightEye.blinking = true;
    leftEye.nextBlinkTime = now + random(2000, 6000);
  }
  if (leftEye.blinking) {
    leftEye.targetH = 2;
    rightEye.targetH = 2;
    if (now - leftEye.lastBlink > 120) {
      leftEye.blinking = false;
      rightEye.blinking = false;
    }
  }

  // --- LOGICA DE SACADAS (MIRADA DE OJOS) ---
  if (!leftEye.blinking && now - lastSaccade > saccadeInterval) {
    lastSaccade = now;
    saccadeInterval = random(500, 3000);

    int dir = random(0, 10);
    float lx = 0, ly = 0;

    if (dir < 4) { lx = 0; ly = 0; }  // Centro
    else if (dir == 4) { lx = -6; ly = -4; } // Arriba-Izquierda
    else if (dir == 5) { lx = 6;  ly = -4; } // Arriba-Derecha
    else if (dir == 6) { lx = -6; ly = 4;  } // Abajo-Izquierda
    else if (dir == 7) { lx = 6;  ly = 4;  } // Abajo-Derecha
    else if (dir == 8) { lx = 8;  ly = 0;  } // Derecha
    else if (dir == 9) { lx = -8; ly = 0;  } // Izquierda

    leftEye.targetPupilX = lx;
    leftEye.targetPupilY = ly;
    rightEye.targetPupilX = lx;
    rightEye.targetPupilY = ly;

    // Arrastre sutil de la cabeza/contorno de los ojos
    leftEye.targetX = 18 + (lx * 0.3);
    leftEye.targetY = 14 + (ly * 0.3);
    rightEye.targetX = 74 + (lx * 0.3);
    rightEye.targetY = 14 + (ly * 0.3);
  }

  // --- SOBREESCRIBIR EXPRESIONES ---
  if (!leftEye.blinking) {
    float baseW = 36;
    float baseH = 36;

    // Sumar respiración a la altura
    baseH += breathVal;

    switch (currentMood) {
      case MOOD_NORMAL:
        leftEye.targetW = baseW; leftEye.targetH = baseH;
        rightEye.targetW = baseW; rightEye.targetH = baseH;
        break;
      case MOOD_HAPPY:
      case MOOD_LOVE:
        leftEye.targetW = 40; leftEye.targetH = 32;
        rightEye.targetW = 40; rightEye.targetH = 32;
        break;
      case MOOD_SURPRISED:
        leftEye.targetW = 30; leftEye.targetH = 45;
        rightEye.targetW = 30; rightEye.targetH = 45;
        leftEye.targetPupilX += random(-1, 2); // Jitter temblor
        break;
      case MOOD_SLEEPY:
        leftEye.targetW = 38; leftEye.targetH = 30;
        rightEye.targetW = 38; rightEye.targetH = 30;
        break;
      case MOOD_ANGRY:
        leftEye.targetW = 34; leftEye.targetH = 32;
        rightEye.targetW = 34; rightEye.targetH = 32;
        break;
      case MOOD_SAD:
        leftEye.targetW = 34; leftEye.targetH = 40;
        rightEye.targetW = 34; rightEye.targetH = 40;
        break;
      case MOOD_SUSPICIOUS:
        leftEye.targetW = 36; leftEye.targetH = 20;  // Izquierdo entornado
        rightEye.targetW = 36; rightEye.targetH = 42;  // Derecho sorprendido
        break;
    }

    // --- ESTRUCTURA Y SQUASH & STRETCH DINÁMICO ---
    float stretchW = constrain(abs(leftEye.velX) * 0.8f, 0.0f, 4.0f);
    float stretchH = constrain(abs(leftEye.velY) * 0.8f, 0.0f, 4.0f);
    leftEye.targetW += stretchW - stretchH;
    leftEye.targetH += stretchH - stretchW;
    rightEye.targetW += stretchW - stretchH;
    rightEye.targetH += stretchH - stretchW;
  }

  leftEye.update();
  rightEye.update();
}

void drawCuteMouth() {
  int cx = 64;
  int cy = 45;

  switch (currentMood) {
    case MOOD_HAPPY:
    case MOOD_EXCITED:
      // Boca sonriente abierta
      lcd.drawCircle(cx, cy, 3, COLOR_WHITE);
      lcd.fillRect(cx - 4, cy - 4, 9, 4, COLOR_BLACK);
      break;
    case MOOD_LOVE:
      // Boca cat-smile (w)
      lcd.drawPixel(cx - 3, cy, COLOR_WHITE);
      lcd.drawPixel(cx - 2, cy + 1, COLOR_WHITE);
      lcd.drawPixel(cx - 1, cy, COLOR_WHITE);
      lcd.drawPixel(cx, cy + 1, COLOR_WHITE);
      lcd.drawPixel(cx + 1, cy, COLOR_WHITE);
      lcd.drawPixel(cx + 2, cy + 1, COLOR_WHITE);
      lcd.drawPixel(cx + 3, cy, COLOR_WHITE);
      break;
    case MOOD_ANGRY:
      // Boca fruncida zic-zac
      lcd.drawLine(cx - 3, cy + 1, cx, cy - 1, COLOR_WHITE);
      lcd.drawLine(cx, cy - 1, cx + 3, cy + 1, COLOR_WHITE);
      break;
    case MOOD_SAD:
      // Boca triste invertida
      lcd.drawCircle(cx, cy + 4, 3, COLOR_WHITE);
      lcd.fillRect(cx - 4, cy + 4, 9, 4, COLOR_BLACK);
      break;
    case MOOD_SURPRISED:
      // Boca abierta 'O'
      lcd.drawCircle(cx, cy, 3, COLOR_WHITE);
      break;
    case MOOD_SLEEPY:
      // Burbujita de sueño
      lcd.fillCircle(cx + 4, cy, 2, COLOR_WHITE);
      break;
    case MOOD_SUSPICIOUS:
      // Boca smirking horizontal
      lcd.drawLine(cx - 2, cy, cx + 4, cy - 1, COLOR_WHITE);
      break;
    default:
      // Boca tierna 'u'
      lcd.drawPixel(cx - 2, cy, COLOR_WHITE);
      lcd.drawPixel(cx - 1, cy + 1, COLOR_WHITE);
      lcd.drawPixel(cx, cy + 1, COLOR_WHITE);
      lcd.drawPixel(cx + 1, cy, COLOR_WHITE);
      break;
  }
}

void drawEmoPage() {
  updatePhysicsAndMood();
  updateTamagotchi();

  // Dibujar partículas flotantes de estado
  if (currentMood == MOOD_LOVE) {
    lcd.drawBitmap(56, 0, bmp_heart, 16, 16, COLOR_WHITE);
  } else if (currentMood == MOOD_SLEEPY) {
    lcd.drawBitmap(110, 0, bmp_zzz, 16, 16, COLOR_WHITE);
  } else if (currentMood == MOOD_ANGRY) {
    lcd.drawBitmap(56, 0, bmp_anger, 16, 16, COLOR_WHITE);
  }

  drawRainParticles();
  drawUltraKawaiiEye(leftEye, true);
  drawUltraKawaiiEye(rightEye, false);
  drawCuteMouth();
  drawQuotesTicker();

  if (showFoodAnim) {
    if (millis() - foodAnimStart < 2000) {
      lcd.drawBitmap(56, 40, bmp_apple, 16, 16, COLOR_WHITE);
    } else {
      showFoodAnim = false;
    }
  }
}

void initRainParticles() {
  for (int i = 0; i < 10; i++) {
    raindrops[i].x = random(0, 128);
    raindrops[i].y = random(-20, 64);
    raindrops[i].speed = random(2, 5);
    raindrops[i].len = random(3, 7);
  }
}

void drawRainParticles() {
  if (weatherMain == "Rain" || weatherMain == "Drizzle" || weatherMain == "Thunderstorm") {
    for (int i = 0; i < 10; i++) {
      lcd.drawLine(raindrops[i].x, raindrops[i].y, raindrops[i].x - 1, raindrops[i].y + raindrops[i].len, COLOR_WHITE);
      raindrops[i].y += raindrops[i].speed;
      if (raindrops[i].y > 64) {
        raindrops[i].y = random(-10, 0);
        raindrops[i].x = random(0, 128);
      }
    }
  }
}

void updateTamagotchi() {
  if (millis() - lastCareDecay > 45000) {
    lastCareDecay = millis();
    petHunger = max(0, petHunger - 2);
    petHappiness = max(0, petHappiness - 1);
  }
}

void feedPet() {
  petHunger = min(100, petHunger + 25);
  petHappiness = min(100, petHappiness + 15);
  showFoodAnim = true;
  foodAnimStart = millis();
  currentMood = MOOD_HAPPY;
  Serial.printf("🍎 Pet Alimentado! Hambre: %d%%, Felicidad: %d%%\n", petHunger, petHappiness);
}

void patPet() {
  petHappiness = min(100, petHappiness + 20);
  currentMood = MOOD_LOVE;
  Serial.printf("🖐️ Pet Acariciado! Felicidad: %d%%\n", petHappiness);
}

void drawQuotesTicker() {
  if (millis() - lastQuoteSwitch > 10000) {
    currentQuoteIdx = (currentQuoteIdx + 1) % 6;
    lastQuoteSwitch = millis();
  }
  lcd.setFont(NULL);
  lcd.setTextColor(COLOR_WHITE);
  const char* q = quotes[currentQuoteIdx];
  int len = strlen(q);
  int x = (128 - (len * 6)) / 2;
  if (x < 0) x = 0;
  lcd.setCursor(x, 56);
  lcd.print(q);
}

void togglePomodoro() {
  if (!chronoRunning) {
    chronoRunning = true;
    chronoStart = millis();
    Serial.println("⏱️ Cronómetro Iniciado (Conteo Ascendente 00:00)");
  } else {
    chronoElapsed += millis() - chronoStart;
    chronoRunning = false;
    Serial.println("⏱️ Cronómetro Pausado");
  }
}

void resetStopwatch() {
  chronoRunning = false;
  chronoElapsed = 0;
  chronoStart = 0;
  Serial.println("⏱️ Cronómetro Reiniciado a 00:00");
}

void drawPomodoroPage() {
  lcd.fillRect(0, 0, 128, 14, COLOR_WHITE);
  lcd.setFont(NULL);
  lcd.setTextColor(COLOR_BLACK);
  lcd.setCursor(18, 3);
  if (chronoRunning) lcd.print("CRONOMETRO: RUN");
  else if (chronoElapsed > 0) lcd.print("CRONOMETRO: PAUSA");
  else lcd.print("CRONOMETRO REAL");
  lcd.setTextColor(COLOR_WHITE);

  unsigned long totalMs = chronoElapsed;
  if (chronoRunning) {
    totalMs += (millis() - chronoStart);
  }

  int totalSec = totalMs / 1000;
  int m = (totalSec / 60) % 60;
  int s = totalSec % 60;
  int ds = (totalMs / 100) % 10;

  lcd.setFont(&FreeSansBold18pt7b);
  char buf[8];
  sprintf(buf, "%02d:%02d", m, s);
  int16_t x1, y1; uint16_t w, h;
  lcd.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
  lcd.setCursor((128 - w) / 2 - 8, 46);
  lcd.print(buf);

  // Décimas de segundo en pequeño al lado del segundero
  lcd.setFont(NULL);
  lcd.setCursor((128 - w) / 2 - 8 + w + 2, 40);
  lcd.printf(".%d", ds);

  lcd.setCursor(4, 55);
  if (!chronoRunning && totalMs == 0) lcd.print("Tap: Iniciar Cronometro");
  else if (chronoRunning) lcd.print("Tap: Pausa | Hold: 00:00");
  else lcd.print("Tap: Seguir | Hold: Reset");
}

void startMiniGame() {
  playerY = 32.0;
  playerVelY = 0.0;
  obstacleX = 128;
  obstacleGapY = random(16, 40);
  gameScore = 0;
  gameOver = false;
  Serial.println("🎮 Mini juego DODGE PET iniciado!");
}

void updateMiniGame() {
  if (gameOver) return;

  playerVelY += 0.35f;
  playerY += playerVelY;
  if (playerY < 4) { playerY = 4; playerVelY = 0; }
  if (playerY > 56) { playerY = 56; gameOver = true; }

  obstacleX -= 3;
  if (obstacleX < -10) {
    obstacleX = 128;
    obstacleGapY = random(16, 40);
    gameScore++;
    if (gameScore > gameHighScore) gameHighScore = gameScore;
  }

  if (obstacleX > 10 && obstacleX < 24) {
    if (playerY < obstacleGapY - 6 || playerY > obstacleGapY + 16) {
      gameOver = true;
      currentMood = MOOD_SAD;
    }
  }
}

void drawMiniGame() {
  updateMiniGame();

  if (gameOver) {
    lcd.setFont(NULL);
    lcd.setCursor(32, 16);
    lcd.print("GAME OVER!");
    lcd.setCursor(20, 34);
    lcd.printf("Score: %d  Max: %d", gameScore, gameHighScore);
    lcd.setCursor(10, 52);
    lcd.print("Tap Touch para reiniciar");
    return;
  }

  // Jugador (Ojo robot saltarín)
  lcd.fillCircle(16, (int)playerY, 5, COLOR_WHITE);
  lcd.fillCircle(17, (int)playerY - 1, 2, COLOR_BLACK);

  // Obstáculo (Tubos arriba y abajo)
  lcd.fillRect(obstacleX, 0, 8, obstacleGapY - 8, COLOR_WHITE);
  lcd.fillRect(obstacleX, obstacleGapY + 16, 8, 64 - (obstacleGapY + 16), COLOR_WHITE);

  // Puntaje
  lcd.setFont(NULL);
  lcd.setCursor(2, 2);
  lcd.printf("%d", gameScore);
}

void drawTecNMLogoPage() {
  float t = millis() * 0.0015f;
  float angleGear = t * 1.2f;       // La tuerca gira en sentido HORARIO (+)
  float angleText = -t * 0.9f;      // Las letras giran en sentido ANTIHORARIO (-)

  // CENTRO EXACTO DE LA PANTALLA OLED 128x64
  int cx = 64;
  int cy = 32;
  int r_outer = 18;
  int r_inner = 13;
  int num_teeth = 8;

  // =========================================================
  // 1. TUERCA ROTATORIA CENTRAL (SENTIDO HORARIO)
  // =========================================================
  for (int i = 0; i < num_teeth; i++) {
    float a1 = angleGear + (i * 2.0f * 3.14159265f / num_teeth) - 0.22f;
    float a2 = angleGear + (i * 2.0f * 3.14159265f / num_teeth) + 0.22f;

    int x1 = cx + (int)(cos(a1) * r_inner);
    int y1 = cy + (int)(sin(a1) * r_inner);
    int x2 = cx + (int)(cos(a1) * r_outer);
    int y2 = cy + (int)(sin(a1) * r_outer);
    int x3 = cx + (int)(cos(a2) * r_outer);
    int y3 = cy + (int)(sin(a2) * r_outer);
    int x4 = cx + (int)(cos(a2) * r_inner);
    int y4 = cy + (int)(sin(a2) * r_inner);

    lcd.fillTriangle(x1, y1, x2, y2, x3, y3, COLOR_WHITE);
    lcd.fillTriangle(x1, y1, x3, y3, x4, y4, COLOR_WHITE);
  }

  // Anillo exterior e interior
  lcd.fillCircle(cx, cy, r_inner, COLOR_WHITE);
  lcd.fillCircle(cx, cy, r_inner - 3, COLOR_BLACK);

  // =========================================================
  // 2. EMBLEMA INTERIOR (VOLCÁN TEUHTLI + NOPALERA + ANTENA)
  // =========================================================
  // Sol / Rayo radiante en la cumbre
  int sunY = cy - 5 + (int)(sin(t * 2.5f) * 1.5f);
  lcd.fillTriangle(cx - 3, cy - 2, cx, sunY - 3, cx + 3, cy - 2, COLOR_WHITE);

  // Volcanes Teuhtli / Popocatépetl (Picos)
  lcd.fillTriangle(cx - 9, cy + 6, cx - 2, cy - 2, cx + 2, cy + 6, COLOR_WHITE);
  lcd.fillTriangle(cx - 2, cy + 6, cx + 3, cy - 4, cx + 9, cy + 6, COLOR_WHITE);

  // Antena de Telecomunicaciones en la montaña
  int antX = cx + 5;
  lcd.drawFastVLine(antX, cy - 6, 10, COLOR_WHITE);
  lcd.drawFastHLine(antX - 2, cy - 4, 5, COLOR_WHITE);

  // Nopaleras y Colinas Verdes
  lcd.fillCircle(cx, cy + 5, 6, COLOR_BLACK);
  lcd.drawCircle(cx, cy + 4, 6, COLOR_WHITE);

  // Pencas de Nopal
  lcd.fillCircle(cx - 3, cy + 5, 2, COLOR_WHITE);

  // =========================================================
  // 3. TEXTO CIRCULAR GIRATORIO ULTRA CRISTALINO Y LEGIBLE (SENTIDO ANTIHORARIO)
  // =========================================================
  lcd.setFont(NULL);
  lcd.setTextSize(1);

  const char* circleText = "ITMA II  *  TecNM  *  ";
  int len = strlen(circleText);
  float r_text = 25.0f; // Espaciado perfecto y separado

  for (int i = 0; i < len; i++) {
    char ch = circleText[i];
    if (ch == ' ') continue;

    float a = angleText + (i * 2.0f * 3.14159265f / len);
    int tx = cx + (int)(cos(a) * r_text);
    int ty = cy + (int)(sin(a) * r_text);

    // Mantenemos dentro de los bordes OLED 128x64
    if (tx >= 2 && tx <= 122 && ty >= 3 && ty <= 57) {
      lcd.setCursor(tx - 2, ty - 3);
      lcd.print(ch);
    }
  }
}

void handleStatusJson() {
  String json = "{";
  json += "\"hunger\":" + String(petHunger) + ",";
  json += "\"happiness\":" + String(petHappiness) + ",";
  json += "\"mood\":" + String(currentMood) + ",";
  json += "\"score\":" + String(gameScore) + ",";
  json += "\"maxScore\":" + String(gameHighScore) + ",";
  json += "\"page\":" + String(currentPage);
  json += "}";
  configServer.send(200, "application/json", json);
}

void handleMobileDashboard() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>DeskBuddy 1.0 Mobile Controller</title><style>";
  html += "body { font-family: system-ui, sans-serif; background: #0d1117; color: #f0f6fc; text-align: center; margin: 0; padding: 15px; }";
  html += "h1 { color: #58a6ff; font-size: 1.4rem; margin-bottom: 5px; }";
  html += ".card { background: #161b22; border-radius: 12px; padding: 15px; margin-bottom: 15px; border: 1px solid #30363d; }";
  html += ".btn { background: #238636; color: white; border: none; padding: 12px 20px; font-size: 1.1rem; font-weight: bold; border-radius: 8px; margin: 5px; cursor: pointer; width: 85%; }";
  html += ".btn-jump { background: #e36049; padding: 25px; font-size: 1.6rem; margin-top: 10px; }";
  html += ".btn-secondary { background: #21262d; border: 1px solid #8b949e; width: auto; font-size: 0.9rem; padding: 8px 12px; }";
  html += ".bar-container { background: #21262d; border-radius: 6px; height: 16px; width: 100%; overflow: hidden; margin-top: 4px; }";
  html += ".bar-fill { background: #3fb950; height: 100%; width: 80%; transition: width 0.3s; }";
  html += ".bar-fill-f { background: #d29922; }";
  html += "</style><script>";
  html += "function sendCmd(p) { fetch(p).then(() => updateStatus()); }";
  html += "function updateStatus() { fetch('/status').then(r => r.json()).then(d => {";
  html += "document.getElementById('hunger-val').innerText = d.hunger + '%';";
  html += "document.getElementById('hunger-bar').style.width = d.hunger + '%';";
  html += "document.getElementById('hap-val').innerText = d.happiness + '%';";
  html += "document.getElementById('hap-bar').style.width = d.happiness + '%';";
  html += "document.getElementById('score-val').innerText = d.score + ' (Max: ' + d.maxScore + ')';";
  html += "}); } setInterval(updateStatus, 3000);";
  html += "</script></head><body>";
  html += "<h1>🤖 DeskBuddy 1.0 Mobile</h1>";
  html += "<div class=\"card\"><h3>🎮 Dodge Pet Game</h3><p>Puntaje: <strong id=\"score-val\">0</strong></p>";
  html += "<button class=\"btn btn-jump\" onclick=\"sendCmd('/jump')\">🚀 SALTAR / JUMP</button></div>";
  html += "<div class=\"card\"><h3>🍎 Tamagotchi Care</h3><div>Hambre: <span id=\"hunger-val\">80%</span><div class=\"bar-container\"><div id=\"hunger-bar\" class=\"bar-fill\"></div></div></div><br>";
  html += "<div>Felicidad: <span id=\"hap-val\">90%</span><div class=\"bar-container\"><div id=\"hap-bar\" class=\"bar-fill bar-fill-f\"></div></div></div><br>";
  html += "<button class=\"btn\" onclick=\"sendCmd('/feed')\">🍎 ALIMENTAR</button>";
  html += "<button class=\"btn\" onclick=\"sendCmd('/pat')\">🖐️ ACARICIAR</button></div>";
  html += "<div class=\"card\"><h3>🎭 Expresiones & Pantallas</h3>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/mood?m=1')\">😊 Feliz</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/mood?m=4')\">😡 Enojado</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/mood?m=7')\">💖 Amor</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/mood?m=3')\">😴 Dormir</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/mood?m=0')\">👀 Normal</button><br><br>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/page?p=0')\">👁️ Ojos</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/page?p=1')\">🕒 Reloj</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/page?p=2')\">🌤️ Clima</button>";
  html += "<button class=\"btn btn-secondary\" onclick=\"sendCmd('/page?p=5')\">⏱️ Pomodoro</button></div></body></html>";
  configServer.send(200, "text/html", html);
}

void setupWebDashboardRoutes() {
  configServer.on("/", handleMobileDashboard);
  configServer.on("/jump", []() {
    if (currentPage == 6) {
      if (gameOver) startMiniGame();
      else playerVelY = -4.5f;
    } else {
      currentPage = 6;
      startMiniGame();
    }
    configServer.send(200, "text/plain", "OK");
  });
  configServer.on("/feed", []() {
    feedPet();
    configServer.send(200, "text/plain", "OK");
  });
  configServer.on("/pat", []() {
    patPet();
    configServer.send(200, "text/plain", "OK");
  });
  configServer.on("/mood", []() {
    if (configServer.hasArg("m")) {
      currentMood = configServer.arg("m").toInt();
      currentPage = 0;
    }
    configServer.send(200, "text/plain", "OK");
  });
  configServer.on("/page", []() {
    if (configServer.hasArg("p")) {
      currentPage = configServer.arg("p").toInt();
      if (currentPage == 6) startMiniGame();
      if (currentPage == 0) {
        leftEye.init(16, 12, 36, 38);
        rightEye.init(76, 12, 36, 38);
      }
    }
    configServer.send(200, "text/plain", "OK");
  });
  configServer.on("/status", handleStatusJson);
}

// --- STANDARD PAGES ---
void drawForecastPage() {
  lcd.fillRect(0, 0, 128, 16, COLOR_WHITE);
  lcd.setFont(NULL);
  lcd.setTextColor(COLOR_BLACK);
  lcd.setCursor(20, 4);
  lcd.print("3-DAY FORECAST");
  lcd.setTextColor(COLOR_WHITE);
  lcd.drawLine(42, 16, 42, 64, COLOR_WHITE);
  lcd.drawLine(85, 16, 85, 64, COLOR_WHITE);
  
  for (int i = 0; i < 3; i++) {
    int xStart = i * 43;
    int centerX = xStart + 21;
    lcd.setFont(NULL);
    String d = fcast[i].dayName;
    if (d == "") d = "Wait";
    lcd.setCursor(centerX - (d.length() * 3), 20);
    lcd.print(d);
    lcd.drawBitmap(centerX - 8, 28, getMiniIcon(fcast[i].iconType), 16, 16, COLOR_WHITE);
    lcd.setFont(&FreeSansBold9pt7b);
    int16_t x1, y1;
    uint16_t w, h;
    lcd.getTextBounds(String(fcast[i].temp).c_str(), 0, 0, &x1, &y1, &w, &h);
    lcd.setCursor(centerX - (w / 2) - 2, 60);
    lcd.print(fcast[i].temp);
    lcd.fillCircle(centerX + (w / 2) + 1, 52, 2, COLOR_WHITE);
  }
}

void drawClock() {
  struct tm t;
  if (!getLocalTime(&t)) {
    lcd.setFont(NULL);
    lcd.setCursor(30, 30);
    lcd.print("Syncing...");
    return;
  }
  String ampm = (t.tm_hour >= 12) ? "PM" : "AM";
  int h12 = t.tm_hour % 12;
  if (h12 == 0) h12 = 12;
  
  lcd.setTextColor(COLOR_WHITE);
  lcd.setFont(NULL);
  lcd.setTextSize(1);
  lcd.setCursor(114, 0);
  lcd.print(ampm);
  
  lcd.setFont(&FreeSansBold18pt7b);
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", h12, t.tm_min);
  int16_t x1, y1;
  uint16_t w, h;
  lcd.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  lcd.setCursor((SCREEN_WIDTH - w) / 2, 42);
  lcd.print(timeStr);
  
  lcd.setFont(&FreeSans9pt7b);
  char dateStr[20];
  strftime(dateStr, 20, "%a, %b %d", &t);
  lcd.getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
  lcd.setCursor((SCREEN_WIDTH - w) / 2, 62);
  lcd.print(dateStr);
}

void drawWeatherCard() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.setFont(NULL);
    lcd.setCursor(0, 0);
    lcd.print("No WiFi");
    return;
  }
  lcd.drawBitmap(96, 0, getBigIcon(weatherMain), 32, 32, COLOR_WHITE);
  lcd.setFont(&FreeSansBold9pt7b);
  String c = city;
  c.toUpperCase();
  lcd.setCursor(0, 14);
  if (c.length() > 9) c = c.substring(0, 8) + ".";
  lcd.print(c);
  lcd.setFont(&FreeSansBold18pt7b);
  int tempInt = (int)temperature;
  lcd.setCursor(0, 48);
  lcd.print(tempInt);
  
  int16_t x1, y1;
  uint16_t w, h;
  lcd.getTextBounds(String(tempInt).c_str(), 0, 48, &x1, &y1, &w, &h);
  lcd.fillCircle(x1 + w + 5, 26, 4, COLOR_WHITE);
  
  lcd.setFont(NULL);
  lcd.drawBitmap(88, 32, bmp_tiny_drop, 8, 8, COLOR_WHITE);
  lcd.setCursor(100, 32);
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(88, 45);
  lcd.print("~");
  lcd.print((int)feelsLike);
  lcd.drawLine(0, 52, 128, 52, COLOR_WHITE);
  lcd.setCursor(0, 55);
  lcd.print(weatherDesc);
}

void drawWorldClock() {
  time_t nowTime;
  time(&nowTime);
  // India y Sídney como zonas horarias secundarias
  time_t indiaEpoch = nowTime + (5 * 3600) + (30 * 60);
  time_t sydneyEpoch = nowTime + (11 * 3600);
  struct tm* indiatm = gmtime(&indiaEpoch);
  int i_h = indiatm->tm_hour;
  int i_m = indiatm->tm_min;
  struct tm* sydneytm = gmtime(&sydneyEpoch);
  int s_h = sydneytm->tm_hour;
  int s_m = sydneytm->tm_min;
  
  lcd.fillRect(0, 0, 128, 16, COLOR_WHITE);
  lcd.setFont(NULL);
  lcd.setTextColor(COLOR_BLACK);
  lcd.setCursor(32, 4);
  lcd.print("WORLD CLOCK");
  lcd.setTextColor(COLOR_WHITE);
  lcd.drawLine(64, 18, 64, 54, COLOR_WHITE);
  
  lcd.setFont(NULL);
  lcd.setCursor(16, 22);
  lcd.print("INDIA");
  lcd.setFont(&FreeSansBold9pt7b);
  char iStr[10];
  sprintf(iStr, "%02d:%02d", i_h, i_m);
  lcd.setCursor(5, 46);
  lcd.print(iStr);
  
  lcd.setFont(NULL);
  lcd.setCursor(78, 22);
  lcd.print("SYDNEY");
  lcd.setFont(&FreeSansBold9pt7b);
  char sStr[10];
  sprintf(sStr, "%02d:%02d", s_h, s_m);
  lcd.setCursor(69, 46);
  lcd.print(sStr);
  
  lcd.setFont(NULL);
  lcd.setCursor(35, 56);
  lcd.print("Tap to Exit");
}

void playBootAnimation() {
  lcd.setTextColor(COLOR_WHITE);
  int cx = 64;
  int cy = 32;
  // Expansión circular
  for (int r = 0; r < 80; r += 4) {
    clearDisplay();
    lcd.fillCircle(cx, cy, r, COLOR_WHITE);
    showDisplay();
    delay(10);
  }
  // Barrido inverso
  for (int r = 0; r < 80; r += 4) {
    clearDisplay();
    lcd.fillCircle(cx, cy, 80, COLOR_WHITE);
    lcd.fillCircle(cx, cy, r, COLOR_BLACK);
    showDisplay();
    delay(10);
  }

  // Texto del Bot
  lcd.setFont(&FreeSansBold9pt7b);
  String bootText = "DeskBuddy";

  int16_t x1, y1;
  uint16_t w, h;
  lcd.getTextBounds(bootText, 0, 0, &x1, &y1, &w, &h);

  clearDisplay();
  lcd.setCursor((SCREEN_WIDTH - w) / 2, 36);
  lcd.print(bootText);
  showDisplay();
  delay(1500);
}

void checkSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Descartar saltos de línea sobrantes
    while(Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }
    
    switch (cmd) {
      case 'x': case 'X':
        isSH1106 = !isSH1106;
        initDisplay();
        Serial.print("🔄 Driver de pantalla cambiado via SERIAL: ");
        Serial.println(isSH1106 ? "SH1106 (OLED 1.3\")" : "SSD1306 (OLED 0.96\")");
        break;
      case 'i': case 'I':
        setInvertedColors(!invertedColors);
        Serial.print("🎨 Modo de color de pantalla (Fondo Neón Glow): ");
        Serial.println(invertedColors ? "ACTIVADO (Invertido Cyberpunk)" : "DESACTIVADO (Normal)");
        break;
      case 'f': case 'F':
        feedPet();
        break;
      case 't': case 'T':
        patPet();
        break;
      case 'm': case 'M':
        currentPage = 5;
        togglePomodoro();
        break;
      case 'g': case 'G':
        currentPage = 6;
        startMiniGame();
        break;
      case 'j': case 'J':
        if (currentPage == 6) {
          if (gameOver) startMiniGame();
          else playerVelY = -4.2f;
        }
        break;
      case '5': currentPage = 5; Serial.println("📄 Página 5: Pomodoro Timer"); break;
      case '6': currentPage = 6; startMiniGame(); break;
      case 'h': case 'H': 
        currentPage = 0; currentMood = MOOD_HAPPY; 
        Serial.println("🟢 Expresión: FELIZ"); 
        break;
      case 's': case 'S': 
        currentPage = 0; currentMood = MOOD_SAD; 
        Serial.println("🟢 Expresión: TRISTE"); 
        break;
      case 'a': case 'A': 
        currentPage = 0; currentMood = MOOD_ANGRY; 
        Serial.println("🟢 Expresión: ENOJADO"); 
        break;
      case 'p': case 'P': 
        currentPage = 0; currentMood = MOOD_SLEEPY; 
        Serial.println("🟢 Expresión: DURMIENDO"); 
        break;
      case 'c': case 'C': 
        currentPage = 0; currentMood = MOOD_SURPRISED; 
        Serial.println("🟢 Expresión: SORPRENDIDO"); 
        break;
      case 'u': case 'U':
        currentPage = 0; currentMood = MOOD_SUSPICIOUS;
        Serial.println("🟢 Expresión: SUSPICAZ");
        break;
      case 'l': case 'L':
        currentPage = 0; currentMood = MOOD_LOVE;
        Serial.println("🟢 Expresión: AMOR");
        break;
      case 'e': case 'E':
        currentPage = 0; currentMood = MOOD_EXCITED;
        Serial.println("🟢 Expresión: EMOCIONADO");
        break;
      case 'b': case 'B':
        leftEye.blinking = true;
        leftEye.lastBlink = millis();
        rightEye.blinking = true;
        Serial.println("⚡ Forzando pestañeo...");
        break;
      case '0': currentPage = 0; Serial.println("🔄 Pagina: EXPRESIONES"); break;
      case '1': currentPage = 1; Serial.println("🔄 Pagina: RELOJ"); break;
      case '2': currentPage = 2; Serial.println("🔄 Pagina: CLIMA"); break;
      case '3': currentPage = 3; Serial.println("🔄 Pagina: RELOJ MUNDIAL"); break;
      case '4': currentPage = 4; Serial.println("🔄 Pagina: PRONOSTICO"); break;
      case '7': currentPage = 7; Serial.println("📄 Pagina: LOGO TECNM"); break;
    }
  }
}

const unsigned char* getBigIcon(String w) {
  if (w == "Clear") return bmp_clear;
  if (w == "Clouds") return bmp_clouds;
  if (w == "Rain" || w == "Drizzle") return bmp_rain;
  return bmp_clouds;
}

const unsigned char* getMiniIcon(String w) {
  if (w == "Clear") return mini_sun;
  if (w == "Rain" || w == "Drizzle" || w == "Thunderstorm") return mini_rain;
  return mini_cloud;
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Desactivar detector de Brownout
  
  Serial.begin(115200);
  delay(500); // Esperar a la inicializacion del puerto serie
  Serial.println("\n========================================");
  Serial.println("  ESP32 DESKBUDDY 1.0 - BOOT SEQUENCE   ");
  Serial.println("========================================");

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  
  pinMode(TOUCH_PIN, INPUT_PULLDOWN);

  Serial.println("Configurando LED de estado (Azul)...");
  setRGBColor(0, 0, 255);
  delay(200);

  Serial.println("Inicializando pantalla...");
  initDisplay();

  Serial.println("Cargando preferencias guardadas...");
  loadConfig();

  // Comprobar si se mantiene presionado el pin de touch para forzar portal de configuración
  Serial.println("Comprobando boton/touch de setup...");
  bool forceConfig = false;
  unsigned long bootStart = millis();
  while (millis() - bootStart < CONFIG_HOLD_MS) {
    updateRGBStatus(); // Actualiza color LED
    if (digitalRead(TOUCH_PIN)) { 
      forceConfig = true; 
      break; 
    }
    delay(50);
  }

  // Configurar modo AP si se fuerza portal
  if (forceConfig) {
    Serial.println("!!! PORTAL DE SETUP FORZADO POR TOUCH PIN !!!");
    wifiInitialized = true; // Permite que updateRGBStatus use WiFi sin crashear
    startConfigPortal();
    return;
  }

  Serial.println("Inicializando animacion de ojos...");
  leftEye.init(18, 14, 36, 36);
  rightEye.init(74, 14, 36, 36);

  Serial.println("Ejecutando animacion de inicio...");
  playBootAnimation();

  clearDisplay();
  lcd.setFont(NULL);
  lcd.setCursor(20, 28);
  lcd.print("Conectando WiFi...");
  showDisplay();
  
  Serial.printf("Intentando conectar a WiFi SSID: %s...\n", wifiSsid.c_str());
  wifiInitialized = true;
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - wifiStart < 6000)) {
    updateRGBStatus();
    delay(100);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se pudo conectar a WiFi en 6s. Activando AP + Modo Offline...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PASS);
    setupWebDashboardRoutes();
    configServer.begin();

    clearDisplay();
    lcd.setCursor(25, 20);
    lcd.print("Modo Offline");
    lcd.setCursor(5, 40);
    lcd.print("AP: DeskBuddy-Setup");
    showDisplay();
    delay(1800);
  } else {
    Serial.println("Conectado con exito a WiFi!");
    Serial.print("📶 IP Local para Dashboard: http://");
    Serial.println(WiFi.localIP());

    setupWebDashboardRoutes();
    configServer.begin();
    configTime(0, 0, ntpServer);
    setenv("TZ", tzString.c_str(), 1);
    tzset();
    
    Serial.println("Solicitando clima actual...");
    getWeatherAndForecast();
  }

  lastWeatherUpdate = millis();
  lastPageSwitch = millis();
}

void loop() {
  checkSerialCommands();
  updateRGBStatus();
  configServer.handleClient();
  
  unsigned long now = millis();
  handleTouch();
  
  if (now - lastWeatherUpdate > 600000) {
    getWeatherAndForecast();
    lastWeatherUpdate = now;
  }

  // Cambio de pagina automatico cada PAGE_INTERVAL (Ciclo: Carita (0) -> Logo ITMA II (7) -> Reloj (1) -> Clima (2) -> Cronometro (5))
  // Si el cronometro esta contando, NO se cambia de pagina para no interrumpir el conteo!
  if (!chronoRunning && (currentPage == 0 || currentPage == 7 || currentPage == 1 || currentPage == 2 || currentPage == 5) && now - lastPageSwitch > PAGE_INTERVAL) {
    if (currentPage == 0) currentPage = 7;
    else if (currentPage == 7) currentPage = 1;
    else if (currentPage == 1) currentPage = 2;
    else if (currentPage == 2) currentPage = 5;
    else if (currentPage == 5) currentPage = 0;

    lastPageSwitch = now;
    lastSaccade = 0;
    if (currentPage == 0) {
      leftEye.init(16, 12, 36, 38);
      rightEye.init(76, 12, 36, 38);
      initRainParticles();
    }
  }

  clearDisplay();
  switch (currentPage) {
    case 0: drawEmoPage(); break;
    case 1: drawClock(); break;
    case 2: drawWeatherCard(); break;
    case 3: drawWorldClock(); break;
    case 4: drawForecastPage(); break;
    case 5: drawPomodoroPage(); break;
    case 6: drawMiniGame(); break;
    case 7: drawTecNMLogoPage(); break;
  }
  showDisplay();
  delay(12); // High FPS (60+ FPS fluidos)
}
