import pyautogui
import time
import os

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== REVISANDO CONFIGURACIÓN DE ROUTER0 Y ROUTER1 ===")

# Enfocar Packet Tracer
pyautogui.click(960, 540)
time.sleep(0.5)

# 1. Revisar Router0
print("[+] Revisando Router0...")
pyautogui.click(350, 300)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)

pyautogui.typewrite("\nenable\nshow run int fa0/0\nshow standby brief\n", interval=0.05)
time.sleep(1.5)

pyautogui.press('esc')
time.sleep(0.5)

# 2. Revisar Router1
print("[+] Revisando Router1...")
pyautogui.click(350, 450)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)

pyautogui.typewrite("\nenable\nshow run int fa0/0\nshow standby brief\n", interval=0.05)
time.sleep(1.5)

pyautogui.press('esc')
time.sleep(0.5)

print("=== REVISIÓN FINALIZADA ===")
