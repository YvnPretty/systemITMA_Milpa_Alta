#!/usr/bin/env python3
"""
ESP32 Live Keyboard Streamer CLI
Te permite escribir cualquier texto en tu teclado PC y enviarlo EN TIEMPO REAL
carácter por carácter a la pantalla OLED del ESP32.
"""

import sys
import tty
import termios
import serial
import time
import argparse

def get_key():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def main():
    parser = argparse.ArgumentParser(description="Live Keyboard Streamer para ESP32 OLED")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Puerto serie del ESP32")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    args = parser.parse_args()

    print("=================================================================")
    print("⌨️  MODO TECLADO EN TIEMPO REAL (LIVE KEYBOARD STREAM)")
    print(f"   Conectado al puerto: {args.port} a {args.baud} baudios")
    print("   Instrucciones:")
    print("    - Escribe directamente en tu teclado PC y observa la pantalla OLED.")
    print("    - Usa BACKSPACE (Retroceso) para borrar letras.")
    print("    - Presiona Ctrl+C o ESC para salir.")
    print("=================================================================\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
    except Exception as e:
        print(f"❌ Error al abrir el puerto {args.port}: {e}")
        return

    buffer = ""
    print("Escribe tu mensaje en vivo: ", end="", flush=True)

    try:
        while True:
            ch = get_key()
            if ord(ch) == 3 or ord(ch) == 27: # Ctrl+C o ESC
                print("\n\nSaliendo del modo teclado en vivo.")
                break
            elif ord(ch) == 127 or ord(ch) == 8: # Backspace
                if len(buffer) > 0:
                    buffer = buffer[:-1]
                    sys.stdout.write("\b \b")
                    sys.stdout.flush()
            elif ord(ch) == 13 or ord(ch) == 10: # Enter
                print(f"\n✅ Mensaje enviado: '{buffer}'")
                buffer = ""
                print("Escribe nuevo mensaje: ", end="", flush=True)
            else:
                buffer += ch
                sys.stdout.write(ch)
                sys.stdout.flush()

            # Transmitir en tiempo real al ESP32
            msg = f"LIVE:{buffer}\n"
            ser.write(msg.encode('utf-8'))
    except KeyboardInterrupt:
        print("\nSaliendo...")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
