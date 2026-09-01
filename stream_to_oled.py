import serial
import sys
import time

port = '/dev/ttyUSB0'
baud = 115200

try:
    ser = serial.Serial(port, baud, timeout=1)
    time.sleep(1)

    print("--- Puente Terminal -> OLED Activo (Ctrl+C para salir) ---")
    
    # Si se pasa un argumento directo
    if len(sys.argv) > 1:
        msg = " ".join(sys.argv[1:])
        ser.write((msg + "\n").encode('utf-8'))
        ser.flush()
        print(f"[Enviado]: {msg}")
    else:
        # Modo Stream: lee cualquier salida que le pases por tubería (|)
        for line in sys.stdin:
            clean = line.strip()
            if clean:
                print(f"[OLED]: {clean}")
                ser.write((clean + "\n").encode('utf-8'))
                ser.flush()
                time.sleep(0.3)
                
    ser.close()
except KeyboardInterrupt:
    print("\nPuente finalizado.")
except Exception as e:
    print(f"Error en puerto serie: {e}")
