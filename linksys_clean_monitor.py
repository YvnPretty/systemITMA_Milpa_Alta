import socket
import serial
import time
import sys

# Configuración del puerto serie del ESP8266
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200
IFACE = 'wlx001c1002e66d'

def start_clean_monitor():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(1)
        print(f"--- Monitor Limpio Linksys en {IFACE} -> OLED ---")
        
        # Enviar estado limpio inicial
        ser.write(b"Linksys Monitor OK\n")
        ser.flush()
        
        # Socket crudo para contar paquetes de radiofrecuencia en la interfaz
        try:
            sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.ntohs(0x0003))
            sock.bind((IFACE, 0))
        except Exception as e:
            print(f"Aviso de socket: {e}")
            sock = None

        pkt_count = 0
        last_sec = time.time()
        pkts_this_sec = 0

        while True:
            if sock:
                try:
                    data = sock.recv(2048)
                    if data:
                        pkt_count += 1
                        pkts_this_sec += 1
                except Exception:
                    pass

            now = time.time()
            if now - last_sec >= 1.0:
                pps = pkts_this_sec
                pkts_this_sec = 0
                last_sec = now

                # Formatear salida LIMPIA de 1 sola línea para no atiborrar la pantalla
                summary = f"Ch: 6 | {pps} p/s | Total: {pkt_count}"
                print(f"[OLED LIMPIO]: {summary}")
                
                ser.write((summary + "\n").encode('utf-8'))
                ser.flush()

    except KeyboardInterrupt:
        print("\nMonitor detenido.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    start_clean_monitor()
