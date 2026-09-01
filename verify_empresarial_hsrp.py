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

print("=== VERIFICANDO ESTADO DE HSRP EN ROUTER_1 Y ROUTER_2 ===")

# 1. Enfocar Packet Tracer robustamente
if not focus_window_by_title("Cisco Packet Tracer"):
    print("[-] No se encontró la ventana de Packet Tracer!")
    exit(1)
time.sleep(1.0)

# 2. Maximizar la ventana de Packet Tracer
pyautogui.hotkey('win', 'up')
time.sleep(1.0)

# 1. Verificar Router_1
print("[+] Abriendo CLI de Router_1...")
pyautogui.click(780, 240)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)
pyautogui.typewrite("\nend\nshow standby brief\n", interval=0.03)
time.sleep(2)
pyautogui.press('esc')
time.sleep(0.5)

# 2. Verificar Router_2
print("[+] Abriendo CLI de Router_2...")
pyautogui.click(1140, 240)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)
pyautogui.typewrite("\nend\nshow standby brief\n", interval=0.03)
time.sleep(2)
pyautogui.press('esc')
time.sleep(0.5)

# Traer al frente la ventana principal
pyautogui.click(960, 540)

print("=== VERIFICACIÓN COMPLETADA CON ÉXITO ===")
