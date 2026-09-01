import pyautogui
import time
import os

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== INICIANDO AUTOMATIZACIÓN EN ENTORNO GRÁFICO ===")

# 1. Enfocar ventana principal de Cisco Packet Tracer
print("[1] Enfocando ventana de Cisco Packet Tracer...")
pyautogui.click(960, 540)
time.sleep(1)

# 2. Abrir consola / CLI o dispositivo central (Switch_Core)
print("[2] Abriendo CLI de dispositivo central (Switch_Core)...")
pyautogui.click(447, 252) # Posición de Switch_Core en la pantalla
time.sleep(1)

# 3. Entrar a la pestaña CLI si se abre ventana de dispositivo
pyautogui.hotkey('ctrl', 'tab')
time.sleep(0.5)

# 4. Enviar comandos CLI directamente
print("[3] Escribiendo comandos de verificación en CLI...")
pyautogui.typewrite("\n", interval=0.1)
pyautogui.typewrite("enable\n", interval=0.1)
pyautogui.typewrite("show ip interface brief\n", interval=0.1)
pyautogui.typewrite("show vlan brief\n", interval=0.1)

print("=== AUTOMATIZACIÓN COMPLETADA CON ÉXITO ===")
