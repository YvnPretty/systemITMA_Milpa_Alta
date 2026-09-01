import serial
import time
import math
import sys

SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200

def run_serial_stream():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(1.5)
        print(f"--- Envitando Animaciones por Serie a OLED ({SERIAL_PORT}) ---")

        frames = [
            "NET:Cubo3D|00:11:22:33:44:55|-45|6|192.168.6.1|ANIM",
            "NET:CyberEye|AA:BB:CC:DD:EE:FF|-50|1|192.168.1.1|ANIM",
            "NET:Starfield|11:22:33:44:55:66|-65|11|192.168.11.1|ANIM",
            "NET:Spectrum|99:88:77:66:55:44|-55|3|192.168.3.1|ANIM"
        ]

        idx = 0
        while True:
            msg = frames[idx]
            ser.write((msg + "\n").encode('utf-8'))
            ser.flush()
            print(f"[OLED Streaming]: {msg}")
            time.sleep(2)
            idx = (idx + 1) % len(frames)

    except KeyboardInterrupt:
        print("\nStreaming finalizado.")
    except Exception as e:
        print(f"Error en puerto serie: {e}")

if __name__ == '__main__':
    run_serial_stream()
