import pyautogui
import time
import os
from Xlib import X, display
import Xlib.protocol.event

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

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

print("=== COLOCANDO Y CONFIGURANDO 10 PCs POR VLAN ===")

input("Presiona ENTER en esta terminal una vez que tengas Packet Tracer abierto y visible en pantalla...")

print("\n" + "!"*80)
print("¡IMPORTANTE! Tienes 5 segundos para traer al frente y hacer clic en la ventana")
print("de Cisco Packet Tracer. Esto es necesario debido a que el sistema de ventanas")
print("de Linux (Wayland) bloquea que los programas roben el foco automáticamente.")
print("¡HAZ CLIC EN LA VENTANA DE PACKET TRACER AHORA!")
print("!"*80 + "\n")

for i in range(5, 0, -1):
    print(f"Iniciando en {i} segundos...")
    time.sleep(1)

# 1. Enfocar Packet Tracer y maximizar
pyautogui.click(960, 540)
time.sleep(0.5)
pyautogui.hotkey('win', 'up')
time.sleep(1.0)

# Obtener offset real de la ventana
offset_x, offset_y = get_window_offset("Cisco Packet Tracer")
print(f"[+] Detected window offset: x={offset_x}, y={offset_y}")

# Clic inicial en el centro del lienzo
pyautogui.click(offset_x + 960, offset_y + 540)
time.sleep(0.5)

# Coordenadas de categorías en la barra inferior (relativas al offset)
CAT_END_DEVICES = (offset_x + 35, offset_y + 995)
MODEL_FIRST = (offset_x + 140, offset_y + 965) # Generic PC

new_pcs = [
    # VLAN 10 (Sistemas IT) - IPs .12 a .19
    {"name": "PC_IT_3", "ip": "192.168.10.12", "gw": "192.168.10.1", "pos": (480, 650)},
    {"name": "PC_IT_4", "ip": "192.168.10.13", "gw": "192.168.10.1", "pos": (560, 650)},
    {"name": "PC_IT_5", "ip": "192.168.10.14", "gw": "192.168.10.1", "pos": (440, 600)},
    {"name": "PC_IT_6", "ip": "192.168.10.15", "gw": "192.168.10.1", "pos": (440, 650)},
    {"name": "PC_IT_7", "ip": "192.168.10.16", "gw": "192.168.10.1", "pos": (400, 600)},
    {"name": "PC_IT_8", "ip": "192.168.10.17", "gw": "192.168.10.1", "pos": (400, 650)},
    {"name": "PC_IT_9", "ip": "192.168.10.18", "gw": "192.168.10.1", "pos": (360, 600)},
    {"name": "PC_IT_10", "ip": "192.168.10.19", "gw": "192.168.10.1", "pos": (360, 650)},

    # VLAN 20 (Contabilidad) - IPs .12 a .19
    {"name": "PC_Conta_3", "ip": "192.168.20.12", "gw": "192.168.20.1", "pos": (640, 650)},
    {"name": "PC_Conta_4", "ip": "192.168.20.13", "gw": "192.168.20.1", "pos": (720, 650)},
    {"name": "PC_Conta_5", "ip": "192.168.20.14", "gw": "192.168.20.1", "pos": (760, 600)},
    {"name": "PC_Conta_6", "ip": "192.168.20.15", "gw": "192.168.20.1", "pos": (760, 650)},
    {"name": "PC_Conta_7", "ip": "192.168.20.16", "gw": "192.168.20.1", "pos": (800, 600)},
    {"name": "PC_Conta_8", "ip": "192.168.20.17", "gw": "192.168.20.1", "pos": (800, 650)},
    {"name": "PC_Conta_9", "ip": "192.168.20.18", "gw": "192.168.20.1", "pos": (840, 600)},
    {"name": "PC_Conta_10", "ip": "192.168.20.19", "gw": "192.168.20.1", "pos": (840, 650)},

    # VLAN 30 (Ventas) - IPs .12 a .19
    {"name": "PC_Ventas_3", "ip": "192.168.30.12", "gw": "192.168.30.1", "pos": (1200, 650)},
    {"name": "PC_Ventas_4", "ip": "192.168.30.13", "gw": "192.168.30.1", "pos": (1280, 650)},
    {"name": "PC_Ventas_5", "ip": "192.168.30.14", "gw": "192.168.30.1", "pos": (1160, 600)},
    {"name": "PC_Ventas_6", "ip": "192.168.30.15", "gw": "192.168.30.1", "pos": (1160, 650)},
    {"name": "PC_Ventas_7", "ip": "192.168.30.16", "gw": "192.168.30.1", "pos": (1120, 600)},
    {"name": "PC_Ventas_8", "ip": "192.168.30.17", "gw": "192.168.30.1", "pos": (1120, 650)},
    {"name": "PC_Ventas_9", "ip": "192.168.30.18", "gw": "192.168.30.1", "pos": (1080, 600)},
    {"name": "PC_Ventas_10", "ip": "192.168.30.19", "gw": "192.168.30.1", "pos": (1080, 650)},

    # VLAN 40 (Gerencia/RH) - IPs .12 a .19
    {"name": "PC_Gerencia_3", "ip": "192.168.40.12", "gw": "192.168.40.1", "pos": (1360, 650)},
    {"name": "PC_Gerencia_4", "ip": "192.168.40.13", "gw": "192.168.40.1", "pos": (1440, 650)},
    {"name": "PC_Gerencia_5", "ip": "192.168.40.14", "gw": "192.168.40.1", "pos": (1480, 600)},
    {"name": "PC_Gerencia_6", "ip": "192.168.40.15", "gw": "192.168.40.1", "pos": (1480, 650)},
    {"name": "PC_Gerencia_7", "ip": "192.168.40.16", "gw": "192.168.40.1", "pos": (1520, 600)},
    {"name": "PC_Gerencia_8", "ip": "192.168.40.17", "gw": "192.168.40.1", "pos": (1520, 650)},
    {"name": "PC_Gerencia_9", "ip": "192.168.40.18", "gw": "192.168.40.1", "pos": (1560, 600)},
    {"name": "PC_Gerencia_10", "ip": "192.168.40.19", "gw": "192.168.40.1", "pos": (1560, 650)},
]

# Paso 1: Colocar las PCs usando Ctrl+Click (modo colocación múltiple)
print("[+] Seleccionando categoría de dispositivos finales...")
pyautogui.click(CAT_END_DEVICES[0], CAT_END_DEVICES[1])
time.sleep(0.5)

print("[+] Activando colocación múltiple de PCs...")
pyautogui.keyDown('ctrl')
pyautogui.click(MODEL_FIRST[0], MODEL_FIRST[1])
pyautogui.keyUp('ctrl')
time.sleep(0.5)

print("[+] Colocando las 32 PCs en el lienzo...")
for pc in new_pcs:
    pyautogui.click(offset_x + pc["pos"][0], offset_y + pc["pos"][1])
    time.sleep(0.3)

# Salir de modo colocación múltiple
pyautogui.press('esc')
time.sleep(0.5)

# Paso 2: Configurar nombre e IP de cada PC
print("[+] Configurando nombre y dirección IP para cada PC...")
for pc in new_pcs:
    print(f" -> Configurando {pc['name']} ({pc['ip']})...")
    # Abrir configuración
    pyautogui.doubleClick(offset_x + pc["pos"][0], offset_y + pc["pos"][1])
    time.sleep(1.0)
    
    # Cambiar a pestaña Config para renombrar
    pyautogui.hotkey('alt', 'c')
    time.sleep(0.4)
    # Escribir nombre (el campo Display Name suele estar enfocado)
    pyautogui.hotkey('ctrl', 'a')
    pyautogui.typewrite(pc["name"], interval=0.01)
    time.sleep(0.2)
    
    # Cambiar a pestaña Desktop
    pyautogui.hotkey('alt', 'd')
    time.sleep(0.4)
    # Abrir IP Configuration (es la primera aplicación, por lo que presionar Enter la abre)
    pyautogui.press('enter')
    time.sleep(0.5)
    
    # Escribir dirección IP
    pyautogui.typewrite(pc["ip"], interval=0.01)
    pyautogui.press('tab')
    time.sleep(0.1) # esperar autocompletado de máscara de red
    pyautogui.press('tab')
    # Escribir Gateway
    pyautogui.typewrite(pc["gw"], interval=0.01)
    time.sleep(0.2)
    
    # Cerrar sub-ventana de IP y ventana principal de PC
    pyautogui.press('esc')
    time.sleep(0.2)
    pyautogui.press('esc')
    time.sleep(0.3)

print("=== AUTOMATIZACIÓN DE PCs FINALIZADA ===")
