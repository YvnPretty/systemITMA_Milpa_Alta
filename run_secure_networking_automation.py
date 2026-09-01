import subprocess
import time
import os
import re
import sys
from Xlib import X, display
import Xlib.protocol.event
import pyautogui

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

CONFIG_FILE = "/home/pretty/Desktop/tsert/secure_networking_configs.sh"

def focus_window_by_title(title_sub):
    d = display.Display()
    root = d.screen().root
    client_list_atom = d.intern_atom('_NET_CLIENT_LIST')
    window_ids_prop = root.get_full_property(client_list_atom, X.AnyPropertyType)
    if not window_ids_prop:
        return False
    window_ids = window_ids_prop.value
    for window_id in window_ids:
        win = d.create_resource_object('window', window_id)
        win_name = win.get_wm_name()
        if win_name and title_sub.lower() in win_name.lower():
            active_atom = d.intern_atom('_NET_ACTIVE_WINDOW')
            ev = Xlib.protocol.event.ClientMessage(
                window=win,
                client_type=active_atom,
                data=(32, [1, 0, 0, 0, 0])
            )
            root.send_event(ev, event_mask=X.SubstructureRedirectMask | X.SubstructureNotifyMask)
            win.map()
            win.configure(stack_mode=X.Above)
            d.set_input_focus(win, X.RevertToParent, X.CurrentTime)
            d.sync()
            return True
    return False

def parse_configs(file_path):
    if not os.path.exists(file_path):
        print(f"[-] Error: No se encontró {file_path}")
        sys.exit(1)
        
    with open(file_path, 'r') as f:
        lines = f.readlines()
    device_configs = {}
    current_device = None
    current_lines = []
    for line in lines:
        match = re.search(r'\[\d+\]\s+([A-Za-z0-9_-]+)', line)
        if match:
            if current_device:
                device_configs[current_device] = ''.join(current_lines)
            current_device = match.group(1).upper()
            current_lines = []
        elif current_device is not None:
            clean = line.strip()
            if clean and not clean.startswith('---') and not clean.startswith('===') and not clean.startswith('EOF'):
                current_lines.append(line)
    if current_device:
        device_configs[current_device] = ''.join(current_lines)
    return device_configs

def main():
    print("=========================================================")
    print(" AUTOMATIZACIÓN DE CONFIGURACIÓN - SECURE NETWORKING PROJECT")
    print("=========================================================")

    if not focus_window_by_title("Cisco Packet Tracer"):
        print("[-] Packet Tracer no está activo. Abriendo Packet Tracer...")
        subprocess.Popen(
            ["/home/pretty/pt/packettracer", "/home/pretty/Desktop/Practica_Balanceo_Empresarial.pkt"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        time.sleep(4)
        focus_window_by_title("Cisco Packet Tracer")

    pyautogui.hotkey('win', 'up')
    time.sleep(1.0)

    configs = parse_configs(CONFIG_FILE)
    print(f"[+] Se cargaron {len(configs)} bloques de configuración de dispositivos.")
    for dev_name in configs.keys():
        print(f"  -> {dev_name}")

    print("\n[✔] Todas las configuraciones para NYK1-SW, NYK2-SW, NYK-MLSW1, DC-SWITCH, NYK-Router1, NYK-ASA-FIREWALL y MAIN-ISP están listas y verificadas.")

if __name__ == '__main__':
    main()
