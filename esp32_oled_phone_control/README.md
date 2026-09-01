# ESP32 OLED Phone Web Control

Este proyecto convierte tu **ESP32** en un servidor web interactivo con Punto de Acceso Wi-Fi (SoftAP), permitiéndote controlar una pantalla **OLED 128x64** (SH1106 o SSD1306) en tiempo real directamente desde la pantalla táctil de tu teléfono celular.

---

## 🚀 Características

1. **Punto de Acceso Wi-Fi Integrado**:
   - Genera su propia red Wi-Fi: `ESP32-OLED-Control` (sin contraseña).
   - **Captive Portal**: Redirección automática al conectar el teléfono (IP: `192.168.4.1` o `http://oled.local`).
2. **Interfaz Web Móvil Futurista (Dark Mode)**:
   - **💬 Texto**: Envía mensajes a la OLED en vivo con fuente grande, centrada o en marquesina (scroll flotante).
   - **🎨 Lienzo Táctil (Canvas 128x64)**: Dibuja a mano alzada con el dedo en tu celular y mira cómo se dibuja en la pantalla OLED en tiempo real.
   - **🎬 Animaciones 3D & Cyberpunk**:
     - Cubo 3D giratorio en renderizado dinámico.
     - Ojo Cibernético interactivo (Robot Eye AI).
     - Lluvia de código estilo Matrix Rain.
     - Viaje espacial (Starfield Warp Speed).
     - Tacómetro / Cyber Ring Telemetry.
   - **⚙️ Ajustes de Pantalla**: Control de contraste/brillo, inversión de colores (blanco/negro) y telemetría de memoria RAM/uptime.

---

## 🛠️ Diagrama de Conexiones (Pinout)

| ESP32 Pin | Pantalla OLED (I2C) | Descripción |
|-----------|--------------------|-------------|
| **3V3 / 5V** | **VCC** | Alimentación (3.3V o 5V) |
| **GND** | **GND** | Tierra común |
| **GPIO 21** | **SDA** | Datos I2C |
| **GPIO 22** | **SCL** | Reloj I2C |

---

## 📦 Librerías Requeridas en Arduino IDE

Instala desde el **Gestor de Librerías de Arduino IDE** (`Ctrl + Shift + I`):
- `U8g2` por Oliver Kraus (para soporte completo SH1106 / SSD1306).

*(Las librerías `WiFi.h`, `WebServer.h`, `DNSServer.h` y `ESPmDNS.h` ya vienen incluidas en la instalación base del ESP32 en Arduino IDE)*.

---

## 📲 Cómo Usar

1. **Compilar y Cargar**: Abre [esp32_oled_phone_control.ino](file:///home/pretty/tsert/esp32_oled_phone_control/esp32_oled_phone_control.ino) en Arduino IDE, selecciona tu placa **ESP32 Dev Module** y súbelo.
2. **Conectar Celular**:
   - En tu celular, ve a **Ajustes Wi-Fi**.
   - Conéctate a la red: `ESP32-OLED-Control`.
3. **Abrir Interfaz Web**:
   - En tu navegador (Chrome, Safari, etc.), entra a `http://192.168.4.1` o `http://oled.local`.
4. **¡A Disfrutar!**: Escribe mensajes, dibuja con el dedo o cambia de animaciones.
