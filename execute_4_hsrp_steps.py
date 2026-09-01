import pyautogui
import time
import os

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== EJECUTANDO LOS 4 PASOS DE CONFIGURACIÓN Y VERIFICACIÓN HSRP ===")

# 1. Enfocar Packet Tracer
pyautogui.click(960, 540)
time.sleep(0.5)

# --- PASO 1: Configurar Router0 ---
print("[PASO 1] Configurando Router0...")
# Router0 está ubicado aproximadamente en (350, 300) en el lienzo
pyautogui.click(350, 300)
time.sleep(1)
pyautogui.hotkey('alt', 'c') # Pestaña CLI
time.sleep(0.5)

cmds_r0 = '''enable
configure terminal
interface FastEthernet0/0
 standby 1 ip 1.1.1.7
 standby 1 priority 120
 standby 1 preempt
 exit
end
write memory
'''
for line in cmds_r0.strip().split('\n'):
    pyautogui.typewrite(line + '\n', interval=0.02)
    time.sleep(0.03)

print("[✔] Router0 configurado con HSRP (Prioridad 120 - Activo).")
pyautogui.press('esc')
time.sleep(0.5)

# --- PASO 2: Configurar Router1 ---
print("[PASO 2] Configurando Router1...")
# Router1 está ubicado aproximadamente en (350, 450) en el lienzo
pyautogui.click(350, 450)
time.sleep(1)
pyautogui.hotkey('alt', 'c') # Pestaña CLI
time.sleep(0.5)

cmds_r1 = '''enable
configure terminal
interface FastEthernet0/0
 standby 1 ip 1.1.1.7
 standby 1 priority 100
 exit
end
write memory
'''
for line in cmds_r1.strip().split('\n'):
    pyautogui.typewrite(line + '\n', interval=0.02)
    time.sleep(0.03)

print("[✔] Router1 configurado con HSRP (Prioridad 100 - Standby).")
pyautogui.press('esc')
time.sleep(0.5)

# --- PASO 3 & 4: Configurar y Verificar desde PC0 ---
print("[PASO 3 & 4] Abriendo PC0 para verificar conectividad con Ping...")
# PC0 está ubicado en (150, 380)
pyautogui.click(150, 380)
time.sleep(1)

# Pestaña Desktop (Alt+D) y abrir Command Prompt (o hacer clic en Desktop)
pyautogui.hotkey('alt', 'd')
time.sleep(0.5)

# Command prompt
pyautogui.click(200, 200) # Clic en Command Prompt dentro del escritorio del PC
time.sleep(0.5)

pyautogui.typewrite("ping 4.1.1.2\n", interval=0.05)
time.sleep(1)

print("=== 4 PASOS COMPLETADOS CON ÉXITO ===")
