#!/usr/bin/env python3
"""
ESP8266 / ESP32 Live Data Streamer (Robust Serial Reconnect Version).
Transmite datos continuos en tiempo real con recuperación automática de reconexión serie.
"""

import sys
import time
import serial
import psutil
from datetime import datetime

PORT = "/dev/ttyUSB0"
BAUD = 115200

MESSAGES = [
    "*** SYSTEM ONLINE ***",
    "MATRIX DATA STREAMING...",
    "HACKER TELEPROMPTER",
    "WIFI: 192.168.4.1",
    "CYBERPUNK MODE: ON",
    "ESP8266 LCD ACTIVE"
]

def open_serial():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        time.sleep(0.5)
        return ser
    except Exception as e:
        return None

def main():
    print("=========================================================================")
    print("📡 TRANSMISOR DE DATOS EN TIEMPO REAL AL DISPOSITIVO (ROBUST RECONNECT)")
    print(f"   Puerto Serie: {PORT} ({BAUD} baudios)")
    print("=========================================================================\n")

    ser = open_serial()
    if ser:
        print(f"✅ Conexión establecida en {PORT}\n")
    else:
        print(f"⚠️ Intentando conectar a {PORT}...\n")

    msg_idx = 0
    try:
        while True:
            now_str = datetime.now().strftime("%H:%M:%S")
            cpu = psutil.cpu_percent()
            ram = psutil.virtual_memory().percent

            current_banner = MESSAGES[msg_idx % len(MESSAGES)]
            status_line = f"CPU:{cpu:.0f}% RAM:{ram:.0f}%"
            payload = f"LIVE:{current_banner}\n"

            if ser is None or not ser.is_open:
                ser = open_serial()

            if ser:
                try:
                    ser.write(payload.encode('utf-8'))
                    print(f"📡 [TRANSMITIENDO]: [{now_str}] {current_banner} | {status_line}")
                except Exception as e:
                    print(f"⚠️ Reintentando conexión serie ({e})...")
                    try:
                        ser.close()
                    except Exception:
                        pass
                    ser = None

            msg_idx += 1
            time.sleep(2.5)

    except KeyboardInterrupt:
        print("\n\n⏹ Transmisión de datos detenida por el usuario.")
    finally:
        if ser:
            try:
                ser.close()
            except Exception:
                pass

if __name__ == "__main__":
    main()
