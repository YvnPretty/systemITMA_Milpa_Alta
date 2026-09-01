#!/usr/bin/env python3
"""
Script de automatización para aplicar reglas de Firewall / Cortafuegos
en los routers Cisco de Packet Tracer (Router_1, Router_2 y Router_Remote).
"""

import pyautogui
import time
import os

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== INICIANDO APLICACIÓN DE FIREWALL/CORTAFUEGOS EN CISCO PACKET TRACER ===")

firewall_configs = {
    'Router_1': ((350, 300), '''enable
configure terminal
hostname Router_1
ip inspect name FW_INSPECT tcp
ip inspect name FW_INSPECT udp
ip inspect name FW_INSPECT icmp
ip access-list extended FW_RULES_VLAN
 permit ip 192.168.10.0 0.0.0.255 any
 permit ip 192.168.40.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.20.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.30.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit ip any any
 exit
interface GigabitEthernet 0/1
 ip access-group FW_RULES_VLAN in
 ip inspect FW_INSPECT out
 exit
ip access-list standard ACCESO_ADMIN_SSH
 permit 192.168.10.0 0.0.0.255
 exit
line vty 0 4
 access-class ACCESO_ADMIN_SSH in
 transport input ssh
 exit
end
write memory
'''),

    'Router_2': ((1140, 240), '''enable
configure terminal
hostname Router_2
ip inspect name FW_INSPECT tcp
ip inspect name FW_INSPECT udp
ip inspect name FW_INSPECT icmp
ip access-list extended FW_RULES_VLAN
 permit ip 192.168.10.0 0.0.0.255 any
 permit ip 192.168.40.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.20.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.30.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit ip any any
 exit
interface GigabitEthernet 0/1
 ip access-group FW_RULES_VLAN in
 ip inspect FW_INSPECT out
 exit
end
write memory
'''),

    'Router_Remote': ((960, 120), '''enable
configure terminal
hostname Router_Remote
ip inspect name PERIMETER_FW tcp
ip inspect name PERIMETER_FW udp
ip inspect name PERIMETER_FW icmp
ip access-list extended PERIMETER_FILTER
 deny ip 127.0.0.0 0.255.255.255 any
 deny ip 0.0.0.0 0.255.255.255 any
 permit tcp any host 10.0.0.1 eq 80
 permit tcp any host 10.0.0.1 eq 443
 permit icmp any any echo-reply
 permit ip 192.168.10.0 0.0.0.255 any
 permit ip 192.168.40.0 0.0.0.255 any
 deny ip any 10.0.0.0 0.0.0.255
 permit ip any any
 exit
interface GigabitEthernet 0/2
 ip access-group PERIMETER_FILTER out
 ip inspect PERIMETER_FW in
 exit
end
write memory
''')
}

# Focus Packet Tracer window
pyautogui.click(960, 540)
time.sleep(0.5)

for dev_name, (pos, cmd_text) in firewall_configs.items():
    print(f"[+] Aplicando Cortafuegos/Firewall en {dev_name} ({pos})...")
    pyautogui.click(pos[0], pos[1])
    time.sleep(1)
    
    # Abrir CLI (Alt+C)
    pyautogui.hotkey('alt', 'c')
    time.sleep(0.5)
    
    pyautogui.typewrite('\n', interval=0.03)
    time.sleep(0.2)
    
    for line in cmd_text.strip().split('\n'):
        pyautogui.typewrite(line + '\n', interval=0.02)
        time.sleep(0.03)
        
    print(f"[✔] Firewall de {dev_name} configurado exitosamente.")
    time.sleep(0.5)
    
    # Cerrar CLI
    pyautogui.press('esc')
    time.sleep(0.5)

print("=== REGLAS DE FIREWALL APLICADAS EN TODOS LOS ROUTERS ===")
