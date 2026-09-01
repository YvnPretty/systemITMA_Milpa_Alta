# ESP8266 OLED Messenger 📟

Sistema para módulo ESP8266 (NodeMCU / Wemos D1 Mini) que recibe mensajes desde múltiples fuentes y los despliega de forma dinámica en una pantalla OLED de 128x64 píxeles.

---

## 🚀 Características

- 📶 **Red Wi-Fi Access Point Propia**: Crea la red `ESP8266-Messenger` con portal cautivo automático.
- 🎨 **Dashboard Web Moderno**: Interfaz responsive integrada en el ESP8266 (acceso en `http://192.168.4.1`) con simulador OLED en vivo, botones de mensajes rápidos y modo oscuro cyberpunk.
- ⚡ **API REST HTTP**: Endpoints para enviar mensajes programáticamente (`/send`, `/api/send`, `/api/status`, `/api/clear`).
- 🐍 **Cliente CLI Python**: Incluye script `send_message.py` para interactuar desde la consola de tu computadora.
- 🔌 **Consola Serie USB**: Permite enviar mensajes mediante la conexión USB enviando cadenas como `MSG:Hola Mundo`.
- 📜 **Formateo Dinámico de Texto (Word-Wrap)**: Ajuste automático de saltos de línea para mensajes largos sin cortar palabras en la pantalla OLED.

---

## 🛠️ Esquema de Conexiones (Pinout)

Conecta la pantalla OLED I2C al módulo ESP8266 de la siguiente manera:

| Pin OLED I2C | NodeMCU (ESP8266) | Wemos D1 Mini | Descripción |
| :--- | :--- | :--- | :--- |
| **VCC** | `3.3V` / `3V3` | `3.3V` | Alimentación positiva (3.3V a 5V) |
| **GND** | `GND` | `GND` | Tierra / Masa |
| **SCL / SCK** | `D1` (GPIO 5) | `D1` (GPIO 5) | Reloj I2C |
| **SDA** | `D2` (GPIO 4) | `D2` (GPIO 4) | Datos I2C |

> **Nota para Pantallas**:
> - Si usas una pantalla de **1.3 pulgadas (SH1106)**, la librería está lista por defecto (`U8G2_SH1106_128X64_NONAME_F_SW_I2C`).
> - Si usas una pantalla de **0.96 pulgadas (SSD1306)**, cambia la línea 24 en `esp8266_oled_messenger.ino` a:
>   `U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 5, 4, U8X8_PIN_NONE);`

---

## 📚 Librerías Necesarias en Arduino IDE

Instala las siguientes librerías desde el **Gestor de Librerías de Arduino IDE** (*Tools -> Manage Libraries...*):
1. **`U8g2`** por Oliver Kraus (para manejo gráfico eficiente de la pantalla OLED).

*(Las librerías `ESP8266WiFi`, `ESP8266WebServer` y `DNSServer` vienen incluidas automáticamente al instalar el soporte de placas ESP8266 en Arduino IDE).*

---

## 📱 Forma de Uso

### 1. Desde el Teléfono o PC (Web UI)
1. Enciende el ESP8266.
2. Conéctate a la red Wi-Fi `ESP8266-Messenger` desde tu celular o computadora.
3. Abre tu navegador e ingresa a `http://192.168.4.1`.
4. Escribe tu nombre, el mensaje y presiona **🚀 Enviar a OLED**.

### 2. Desde la Terminal con Python (CLI)
Conéctate a la red `ESP8266-Messenger` y ejecuta en tu terminal:

```bash
# Modo interactivo
./send_message.py

# Enviar mensaje directo
./send_message.py --sender "Alex" --msg "Llego en 10 minutos"

# Consultar telemetría y estado
./send_message.py --status

# Limpiar pantalla
./send_message.py --clear
```

### 3. Mediante cURL o Peticiones HTTP REST
```bash
curl -X POST "http://192.168.4.1/send" \
     -d "sender=Sistema&msg=Alerta+de+sensor+activada"
```

### 4. Desde el Monitor Serie de Arduino IDE (USB)
Abre el Monitor Serie a **115200 baudios** e ingresa:
- `MSG:Tu mensaje aquí`
- `SENDER:Alex|MSG:Hola a todos`
- `CLEAR`
