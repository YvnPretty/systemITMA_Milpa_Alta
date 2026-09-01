import serial
import time
import sys

port = '/dev/ttyUSB0'
baud = 115200

try:
    ser = serial.Serial(port, baud, timeout=1)
    time.sleep(2)
    
    msg = sys.argv[1] if len(sys.argv) > 1 else "Linksys Ralink RT2870"
    print(f"Enviando mensaje a la pantalla OLED: {msg}")
    ser.write((msg + "\n").encode('utf-8'))
    ser.flush()
    print("¡Mensaje enviado con éxito!")
    ser.close()
except Exception as e:
    print(f"Error: {e}")
