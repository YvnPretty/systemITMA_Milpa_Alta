import pyautogui
import time
import os

os.environ['DISPLAY'] = ':0'
xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
xauth = os.popen(xauth_cmd).read().strip()
if xauth:
    os.environ['XAUTHORITY'] = xauth

pyautogui.FAILSAFE = False

print("=== INICIANDO CONFIGURACIÓN AUTOMÁTICA DE TODOS LOS DEMÁS EQUIPOS ===")

configs = {
    'Router_2': ((1140, 240), '''enable
configure terminal
hostname Router_2
interface GigabitEthernet 0/0
 no shutdown
 exit
interface GigabitEthernet 0/0.10
 encapsulation dot1Q 10
 ip address 192.168.10.3 255.255.255.0
 standby 10 ip 192.168.10.1
 standby 10 priority 100
 exit
interface GigabitEthernet 0/0.20
 encapsulation dot1Q 20
 ip address 192.168.20.3 255.255.255.0
 standby 20 ip 192.168.20.1
 standby 20 priority 100
 exit
interface GigabitEthernet 0/0.30
 encapsulation dot1Q 30
 ip address 192.168.30.3 255.255.255.0
 standby 30 ip 192.168.30.1
 standby 30 priority 120
 standby 30 preempt
 exit
interface GigabitEthernet 0/0.40
 encapsulation dot1Q 40
 ip address 192.168.40.3 255.255.255.0
 standby 40 ip 192.168.40.1
 standby 40 priority 120
 standby 40 preempt
 exit
interface GigabitEthernet 0/1
 ip address 10.0.2.1 255.255.255.0
 ip access-group FILTRO_SEGURIDAD in
 no shutdown
 exit
ip access-list extended FILTRO_SEGURIDAD
 permit ip 192.168.10.0 0.0.0.255 any
 permit ip 192.168.40.0 0.0.0.255 any
 deny ip 192.168.20.0 0.0.0.255 10.0.0.0 0.0.0.255
 deny ip 192.168.30.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit ip any any
 exit
router ospf 1
 router-id 2.2.2.2
 network 192.168.0.0 0.0.255.255 area 0
 network 10.0.2.0 0.0.0.255 area 0
 exit
end
write memory
'''),

    'Switch_Core': ((960, 360), '''enable
configure terminal
hostname Switch_Core
vlan 10
 name Sistemas_IT
vlan 20
 name Contabilidad
vlan 30
 name Ventas
vlan 40
 name Gerencia_RH
exit
interface range fastEthernet 0/1 - 2
 switchport trunk encapsulation dot1q
 switchport mode trunk
 channel-group 1 mode active
 exit
interface range fastEthernet 0/3 - 4
 switchport trunk encapsulation dot1q
 switchport mode trunk
 channel-group 2 mode active
 exit
interface GigabitEthernet 0/1
 switchport trunk encapsulation dot1q
 switchport mode trunk
 exit
interface GigabitEthernet 0/2
 switchport trunk encapsulation dot1q
 switchport mode trunk
 exit
end
write memory
'''),

    'Switch_A': ((600, 480), '''enable
configure terminal
hostname Switch_A
vlan 10
 name Sistemas_IT
vlan 20
 name Contabilidad
vlan 30
 name Ventas
vlan 40
 name Gerencia_RH
exit
interface range fastEthernet 0/1 - 2
 switchport mode trunk
 channel-group 1 mode active
 exit
interface range fastEthernet 0/3 - 4
 switchport mode access
 switchport access vlan 10
 exit
interface range fastEthernet 0/5 - 6
 switchport mode access
 switchport access vlan 20
 exit
end
write memory
'''),

    'Switch_B': ((1320, 480), '''enable
configure terminal
hostname Switch_B
vlan 10
 name Sistemas_IT
vlan 20
 name Contabilidad
vlan 30
 name Ventas
vlan 40
 name Gerencia_RH
exit
interface range fastEthernet 0/1 - 2
 switchport mode trunk
 channel-group 2 mode active
 exit
interface range fastEthernet 0/3 - 4
 switchport mode access
 switchport access vlan 30
 exit
interface range fastEthernet 0/5 - 6
 switchport mode access
 switchport access vlan 40
 exit
end
write memory
'''),

    'Router_Remote': ((960, 120), '''enable
configure terminal
hostname Router_Remote
interface GigabitEthernet 0/0
 ip address 10.0.1.2 255.255.255.0
 no shutdown
 exit
interface GigabitEthernet 0/1
 ip address 10.0.2.2 255.255.255.0
 no shutdown
 exit
interface GigabitEthernet 0/2
 ip address 10.0.0.1 255.255.255.0
 no shutdown
 exit
router ospf 1
 router-id 3.3.3.3
 network 10.0.1.0 0.0.0.255 area 0
 network 10.0.2.0 0.0.0.255 area 0
 network 10.0.0.0 0.0.0.255 area 0
 exit
end
write memory
''')
}

# Focus Packet Tracer
pyautogui.click(960, 540)
time.sleep(0.5)

for dev_name, (pos, cmd_text) in configs.items():
    print(f"[+] Configurando {dev_name} en {pos}...")
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
        
    print(f"[✔] {dev_name} configurado exitosamente.")
    time.sleep(0.5)
    
    # Cerrar ventana CLI del dispositivo con Esc
    pyautogui.press('esc')
    time.sleep(0.5)

print("=== CONFIGURACIÓN DE TODOS LOS EQUIPOS COMPLETADA CON ÉXITO ===")
