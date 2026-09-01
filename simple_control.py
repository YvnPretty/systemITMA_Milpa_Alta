#!/usr/bin/env python3
import sys, os, time, tty, termios, select, serial

def find_port():
    for p in ['/dev/ttyACM0', '/dev/ttyACM1', '/dev/ttyUSB0', '/dev/ttyUSB1']:
        if os.path.exists(p): return p
    return '/dev/ttyACM0'

PORT = find_port()
print(f"=== CONTROL DIRECTO SIMPLE ARDUINO L293D ({PORT}) ===")

try:
    ser = serial.Serial(PORT, 9600, timeout=0.1)
    time.sleep(1.5)
except Exception as e:
    print(f"Error abriendo puerto: {e}")
    sys.exit(1)

print("\nControles:")
print(" W = Avanzar | S = Atrás | A = Izquierda | D = Derecha | ESPACIO = Parar | Q = Salir\n")

fd = sys.stdin.fileno()
old = termios.tcgetattr(fd)

try:
    tty.setcbreak(fd)
    while True:
        r, _, _ = select.select([sys.stdin], [], [], 0.05)
        if r:
            ch = sys.stdin.read(1).lower()
            if ch == 'q':
                ser.write(b' ')
                break
            elif ch in ('w', 's', 'a', 'd', ' ', 'x', '+', '-', 'q', 'e', 'z', 'v', 'r'):
                ser.write(ch.encode('utf-8'))
                ser.flush()
                print(f"\rComando enviado: {repr(ch)}   ", end='', flush=True)
finally:
    termios.tcsetattr(fd, termios.TCSADRAIN, old)
    ser.close()
    print("\nControl finalizado.")
