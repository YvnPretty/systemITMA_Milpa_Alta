import pyautogui
import time
import os

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== VERIFICANDO ESTADO DE HSRP Y REDUNDANCIA ===")

# Enfocar Packet Tracer
pyautogui.click(960, 540)
time.sleep(0.5)

# 1. Verificar Router0 (debe ser Active)
pyautogui.click(350, 300)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)

pyautogui.typewrite("\nend\nshow standby brief\n", interval=0.05)
time.sleep(1)

pyautogui.press('esc')
time.sleep(0.5)

# 2. Verificar Router1 (debe ser Standby)
pyautogui.click(350, 450)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)

pyautogui.typewrite("\nend\nshow standby brief\n", interval=0.05)
time.sleep(1)

pyautogui.press('esc')
time.sleep(0.5)

# Traer al frente la ventana principal
pyautogui.click(960, 540)

print("=== VERIFICACIÓN COMPLETADA ===")
