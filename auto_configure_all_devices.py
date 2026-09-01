import pyautogui
import time
import os
import re
from Xlib import X, display
import Xlib.protocol.event

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== INICIANDO CONFIGURACIÓN AUTOMÁTICA DE TODOS LOS DISPOSITIVOS ===")

# Función para parsear configuraciones de cisco_lab_configs.sh
def parse_configs(file_path):
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

config_file = '/home/pretty/Desktop/tsert/cisco_lab_configs.sh'
device_configs = parse_configs(config_file)

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
            # Traer al frente
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
            
            # Obtener posición
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

# Coordenadas aproximadas de los equipos en el lienzo (relativas a la ventana de Packet Tracer)
devices = {
    'Switch_Core': (960, 360),
    'Switch_A': (600, 480),
    'Switch_B': (1320, 480),
    'Router_1': (780, 240),
    'Router_2': (1140, 240),
    'Router_Remote': (960, 120)
}

# 1. Enfocar Packet Tracer robustamente y maximizar
pyautogui.click(960, 540) # Foco preliminar
time.sleep(0.5)
pyautogui.hotkey('win', 'up')
time.sleep(1.0)

# Obtener offset real de la ventana
offset_x, offset_y = get_window_offset("Cisco Packet Tracer")
print(f"[+] Detected window offset: x={offset_x}, y={offset_y}")

# Función para enviar bloques de comandos
def send_cli_commands(device_name, coords):
    config_key = device_name.upper()
    if config_key not in device_configs:
        print(f"[-] No se encontró configuración para {device_name}")
        return

    # Ajustar coordenadas usando el offset dinámico
    abs_x = offset_x + coords[0]
    abs_y = offset_y + coords[1]
    
    print(f"[+] Configurando {device_name} en coordenadas absolutas ({abs_x}, {abs_y})...")
    pyautogui.click(abs_x, abs_y)
    time.sleep(1)
    
    # Cambiar a la pestaña CLI con Alt+C o Clic
    pyautogui.hotkey('alt', 'c')
    time.sleep(0.5)
    
    pyautogui.typewrite("\n", interval=0.05)
    time.sleep(0.2)
    
    # Enviar comandos línea por línea
    cmds = device_configs[config_key].strip().split('\n')
    for cmd in cmds:
        pyautogui.typewrite(cmd + "\n", interval=0.02)
        time.sleep(0.03)

    print(f"[✔] {device_name} configurado exitosamente en Packet Tracer.")
    time.sleep(0.5)
    
    # Cerrar la ventana del dispositivo con Esc
    pyautogui.press('esc')
    time.sleep(0.5)

# Clic inicial
pyautogui.click(offset_x + 960, offset_y + 540)
time.sleep(1)

for dev, pos in devices.items():
    send_cli_commands(dev, pos)

print("=== CONFIGURACIÓN AUTOMÁTICA FINALIZADA ===")
