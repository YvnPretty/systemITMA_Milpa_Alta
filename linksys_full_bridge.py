import subprocess
import re
import serial
import time
import threading

SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200
IFACE = 'wlx001c1002e66d'

# Rotación automática de canales (Hopping 1 al 13) para capturar todos los routers vecinos
def channel_hopper():
    ch = 1
    while True:
        try:
            subprocess.run(["sudo", "iw", "dev", IFACE, "set", "channel", str(ch)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            ch += 1
            if ch > 13:
                ch = 1
            time.sleep(0.4)
        except Exception:
            pass

def start_clean_network_scanner():
    # Iniciar rotación de canales en segundo plano
    t = threading.Thread(target=channel_hopper, daemon=True)
    t.start()

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(1)
        print(f"--- Escáner Multicanal Linksys (Filtrado sin 'Monitor') ({IFACE}) -> OLED & Web ---")
        
        cmd = ["sudo", "tcpdump", "-l", "-i", IFACE, "-e", "-n", "type mgt subtype beacon"]
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)

        detected_aps = {}

        for line in proc.stdout:
            bssid_m = re.search(r"BSSID:([0-9a-fA-F:]+)", line)
            ssid_m = re.search(r"Beacon \((.*?)\)", line)
            signal_m = re.search(r"(-?\d+)dBm", line)
            ch_m = re.search(r"CH:\s*(\d+)", line)

            if bssid_m and ssid_m:
                bssid = bssid_m.group(1).upper()
                ssid = ssid_m.group(1).strip()
                
                # FILTRAR LA RED PROPIA DEL ESP8266 ("Monitor" / MAC 82:7D:3A...)
                if ssid == "Monitor" or "82:7D:3A" in bssid:
                    continue

                if len(ssid) == 0:
                    ssid = "[Red Oculta]"

                signal = int(signal_m.group(1)) if signal_m else -60
                chan = int(ch_m.group(1)) if ch_m else 1
                gateway = f"192.168.{chan}.1"

                now = time.time()
                if bssid not in detected_aps or (now - detected_aps[bssid]['time']) > 2.5:
                    detected_aps[bssid] = {
                        'ssid': ssid,
                        'signal': signal,
                        'chan': chan,
                        'gateway': gateway,
                        'time': now
                    }

                    payload = f"NET:{ssid}|{bssid}|{signal}|{chan}|{gateway}|WPA2\n"
                    print(f"📡 [ROUTER VECINO]: {ssid} | MAC: {bssid} | CH: {chan} | GW: {gateway} | Señal: {signal} dBm")

                    ser.write(payload.encode('utf-8'))
                    ser.flush()

    except KeyboardInterrupt:
        print("\nEscáner detenido.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    start_clean_network_scanner()
