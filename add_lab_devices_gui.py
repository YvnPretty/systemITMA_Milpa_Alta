import pyautogui
import time
import os
import sys
from Xlib import X, display

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

print("=== COLOCANDO DISPOSITIVOS EN EL LIENZO DE PACKET TRACER ===")

# 1. Focus Packet Tracer
offset_x, offset_y = get_window_offset("Cisco Packet Tracer")
print(f"[+] Detected Packet Tracer window offset: x={offset_x}, y={offset_y}")

# Focus click
pyautogui.click(offset_x + 960, offset_y + 540)
time.sleep(0.5)

# Coordenadas de categorías en la barra inferior (para resolución 1920x1080, relativas al offset del canvas)
CAT_ROUTERS = (offset_x + 35, offset_y + 965)
CAT_SWITCHES = (offset_x + 65, offset_y + 965)
CAT_END_DEVICES = (offset_x + 35, offset_y + 995)

MODEL_FIRST = (offset_x + 140, offset_y + 965) # Primer modelo de la categoría seleccionada

# 2. Agregar Routers (Router_1, Router_2, Router_Remote)
routers_pos = [(780, 240), (1140, 240), (960, 120)]
for pos in routers_pos:
    pyautogui.click(CAT_ROUTERS[0], CAT_ROUTERS[1])
    time.sleep(0.3)
    pyautogui.click(MODEL_FIRST[0], MODEL_FIRST[1])
    time.sleep(0.3)
    pyautogui.click(offset_x + pos[0], offset_y + pos[1])
    time.sleep(0.3)

# 3. Agregar Switches (Switch_Core, Switch_A, Switch_B)
switches_pos = [(960, 360), (600, 480), (1320, 480)]
for pos in switches_pos:
    pyautogui.click(CAT_SWITCHES[0], CAT_SWITCHES[1])
    time.sleep(0.3)
    pyautogui.click(MODEL_FIRST[0], MODEL_FIRST[1])
    time.sleep(0.3)
    pyautogui.click(offset_x + pos[0], offset_y + pos[1])
    time.sleep(0.3)

# 4. Agregar PCs y Servidores
pcs_pos = [
    (480, 600), (560, 600), (640, 600), (720, 600), # Edificio A (VLAN 10 y 20)
    (1200, 600), (1280, 600), (1360, 600), (1440, 600), # Edificio B (VLAN 30 y 40)
    (960, 50) # Servidor Central
]
for pos in pcs_pos:
    pyautogui.click(CAT_END_DEVICES[0], CAT_END_DEVICES[1])
    time.sleep(0.3)
    pyautogui.click(MODEL_FIRST[0], MODEL_FIRST[1])
    time.sleep(0.3)
    pyautogui.click(offset_x + pos[0], offset_y + pos[1])
    time.sleep(0.3)

print("=== DISPOSITIVOS AGREGADOS EXITOSAMENTE AL LIENZO ===")
