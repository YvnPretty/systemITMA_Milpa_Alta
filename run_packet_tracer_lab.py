import subprocess
import time
import os
import re
import sys
from Xlib import X, display
import Xlib.protocol.event
import pyautogui

# Set display environment variables for X11 compatibility
os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

LAB_FILE = "/home/pretty/Desktop/Practica_Balanceo_Empresarial.pkt"
CONFIG_FILE = "/home/pretty/Desktop/tsert/cisco_lab_configs.sh"

def launch_packet_tracer(file_path):
    print("=== INICIANDO CISCO PACKET TRACER ===")
    # Check if Packet Tracer is already running
    pgrep = subprocess.run(["pgrep", "-f", "PacketTracer"], stdout=subprocess.PIPE)
    if pgrep.returncode == 0:
        print("[+] Packet Tracer ya está en ejecución.")
    else:
        print(f"[+] Iniciando Packet Tracer con la práctica: {os.path.basename(file_path)}...")
        # Start Packet Tracer in the background
        subprocess.Popen(
            ["/home/pretty/pt/packettracer", file_path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            preexec_fn=os.setpgrp # Decouple from parent process group
        )
        print("[+] Proceso iniciado. Esperando a que el software cargue...")
        time.sleep(3)

def get_window_offset(title_sub):
    d = display.Display()
    root = d.screen().root
    client_list_atom = d.intern_atom('_NET_CLIENT_LIST')
    window_ids_prop = root.get_full_property(client_list_atom, X.AnyPropertyType)
    if not window_ids_prop:
        return (0, 0)
    window_ids = window_ids_prop.value
    for window_id in window_ids:
        win = d.create_resource_object('window', window_id)
        win_name = win.get_wm_name()
        if win_name and title_sub.lower() in win_name.lower():
            # Bring window to front
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
            
            # Obtain window position geometry recursively
            curr = win
            abs_x, abs_y = 0, 0
            while curr.id != root.id:
                geom = curr.get_geometry()
                abs_x += geom.x
                abs_y += geom.y
                parent = curr.query_tree().parent
                if not parent or parent.id == root.id:
                    break
                curr = parent
            return (abs_x, abs_y)
    return (0, 0)

# Parsing cisco_lab_configs.sh
def parse_configs(file_path):
    if not os.path.exists(file_path):
        print(f"[-] Error: No se encontró el archivo de configuraciones en {file_path}")
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

# Devices coordinates on screen (approximate coordinates relative to PT window)
devices = {
    'Switch_Core': (960, 360),
    'Switch_A': (600, 480),
    'Switch_B': (1320, 480),
    'Router_1': (780, 240),
    'Router_2': (1140, 240),
    'Router_Remote': (960, 120)
}

def main():
    launch_packet_tracer(LAB_FILE)
    
    # Wait for the user to make sure the app has loaded and log in if necessary
    print("\n" + "="*80)
    print("ATENCIÓN: Packet Tracer puede requerir que inicies sesión (Cisco NetAcad / SkillsForAll).")
    print("1. Por favor, inicia sesión si es necesario.")
    print("2. Asegúrate de que puedes ver el lienzo con los equipos de la práctica.")
    print("3. Presiona ENTER aquí en la terminal una vez que la topología esté completamente visible para continuar...")
    print("="*80 + "\n")
    input()
    
    print("\n" + "!"*80)
    print("¡IMPORTANTE! Tienes 5 segundos para traer al frente y hacer clic en la ventana")
    print("de Cisco Packet Tracer. Esto es necesario debido a que el sistema de ventanas")
    print("de Linux (Wayland) bloquea que los programas roben el foco automáticamente.")
    print("¡HAZ CLIC EN LA VENTANA DE PACKET TRACER AHORA!")
    print("!"*80 + "\n")
    
    for i in range(5, 0, -1):
        print(f"Iniciando en {i} segundos...")
        time.sleep(1)
    
    # Focus and maximize window
    print("[+] Detectando ventana de Cisco Packet Tracer...")
    offset_x, offset_y = get_window_offset("Cisco Packet Tracer")
    if offset_x == 0 and offset_y == 0:
        print("[-] Advertencia: No se pudo detectar dinámicamente la ventana 'Cisco Packet Tracer'. Usando coordenadas por defecto (0,0).")
    else:
        print(f"[+] Ventana detectada con offset: x={offset_x}, y={offset_y}")
        
    pyautogui.click(offset_x + 960, offset_y + 540) # Click in center to focus
    time.sleep(0.5)
    pyautogui.hotkey('win', 'up') # Maximize window
    time.sleep(1.0)
    
    # Get offset again after maximize
    offset_x, offset_y = get_window_offset("Cisco Packet Tracer")
    print(f"[+] Offset final (maximizado): x={offset_x}, y={offset_y}")
    
    # Parse configurations
    print("[+] Cargando configuraciones de red de cisco_lab_configs.sh...")
    device_configs = parse_configs(CONFIG_FILE)
    
    # Configure each network device
    for dev, pos in devices.items():
        config_key = dev.upper()
        if config_key not in device_configs:
            print(f"[-] No se encontró configuración definida para {dev}. Saltando...")
            continue
            
        abs_x = offset_x + pos[0]
        abs_y = offset_y + pos[1]
        
        print(f"\n[+] Iniciando configuración de {dev} (Coords: {abs_x}, {abs_y})...")
        pyautogui.click(abs_x, abs_y)
        time.sleep(1.2)
        
        # Change to CLI tab via Alt+C
        pyautogui.hotkey('alt', 'c')
        time.sleep(0.6)
        
        # Press enter to activate the CLI prompt
        pyautogui.typewrite("\n", interval=0.05)
        time.sleep(0.3)
        
        # Send configurations line by line
        cmds = device_configs[config_key].strip().split('\n')
        for cmd in cmds:
            pyautogui.typewrite(cmd + "\n", interval=0.02)
            time.sleep(0.03)
            
        print(f"[✔] Configuración inyectada en {dev}.")
        time.sleep(0.5)
        
        # Close the device window via Esc
        pyautogui.press('esc')
        time.sleep(0.5)

    print("\n=== CONFIGURACIÓN AUTOMÁTICA FINALIZADA ===")

if __name__ == '__main__':
    main()
