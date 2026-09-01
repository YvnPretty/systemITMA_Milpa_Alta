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

def focus_pt():
    d = display.Display()
    root = d.screen().root
    client_list_atom = d.intern_atom('_NET_CLIENT_LIST_STACKING')
    window_ids_prop = root.get_full_property(client_list_atom, X.AnyPropertyType)
    if not window_ids_prop:
        client_list_atom = d.intern_atom('_NET_CLIENT_LIST')
        window_ids_prop = root.get_full_property(client_list_atom, X.AnyPropertyType)
    if not window_ids_prop:
        return False
    for window_id in window_ids_prop.value:
        win = d.create_resource_object('window', window_id)
        win_name = win.get_wm_name()
        if win_name and 'Packet Tracer' in win_name:
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

focus_pt()
time.sleep(1)

# Device click coordinates relative to screen resolution 1920x1080
devices = {
    'MultilayerSwitch0': (680, 230),
    'MultilayerSwitch1': (530, 650),
    'Router0': (300, 490),
}

configs = {
    'MultilayerSwitch0': """
enable
configure terminal
hostname MultilayerSwitch0
ip routing
vlan 10
 name LAN_Sistemas
exit
interface FastEthernet0/1
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/2
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/3
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/4
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/5
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/6
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface GigabitEthernet0/1
 no switchport
 ip address 10.0.0.1 255.255.255.252
 no shutdown
exit
interface GigabitEthernet0/2
 switchport trunk encapsulation dot1q
 switchport mode trunk
 switchport trunk native vlan 1
 switchport trunk allowed vlan 10
 no shutdown
exit
interface Vlan10
 ip address 192.168.1.2 255.255.255.0
 standby 10 ip 192.168.1.1
 standby 10 priority 150
 standby 10 preempt
 standby 10 track GigabitEthernet0/1 decrement 60
 no shutdown
exit
router ospf 1
 network 10.0.0.0 0.0.0.3 area 0
 network 192.168.1.0 0.0.0.255 area 0
exit
spanning-tree mode pvst
spanning-tree vlan 10 priority 4096
end
write memory
""",
    'MultilayerSwitch1': """
enable
configure terminal
hostname MultilayerSwitch1
ip routing
vlan 10
 name LAN_Sistemas
exit
interface FastEthernet0/1
 switchport mode access
 switchport access vlan 10
 spanning-tree portfast
exit
interface FastEthernet0/2
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/3
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/4
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface FastEthernet0/5
 switchport access vlan 10
 switchport mode access
 spanning-tree portfast
exit
interface GigabitEthernet0/1
 no switchport
 ip address 10.0.0.5 255.255.255.252
 no shutdown
exit
interface GigabitEthernet0/2
 switchport trunk encapsulation dot1q
 switchport mode trunk
 switchport trunk native vlan 1
 switchport trunk allowed vlan 10
 no shutdown
exit
interface Vlan10
 ip address 192.168.1.3 255.255.255.0
 standby 10 ip 192.168.1.1
 standby 10 priority 100
 standby 10 preempt
 no shutdown
exit
router ospf 1
 network 10.0.0.4 0.0.0.3 area 0
 network 192.168.1.0 0.0.0.255 area 0
exit
spanning-tree mode pvst
spanning-tree vlan 10 priority 8192
end
write memory
""",
    'Router0': """
enable
configure terminal
hostname Router0
interface GigabitEthernet0/0
 ip address 10.0.0.2 255.255.255.252
 duplex auto
 speed auto
 no shutdown
exit
interface GigabitEthernet0/1
 ip address 10.0.0.6 255.255.255.252
 duplex auto
 speed auto
 no shutdown
exit
interface GigabitEthernet0/2
 ip address 192.168.3.1 255.255.255.0
 duplex auto
 speed auto
 no shutdown
exit
router ospf 1
 network 10.0.0.0 0.0.0.3 area 0
 network 10.0.0.4 0.0.0.3 area 0
 network 192.168.3.0 0.0.0.255 area 0
exit
ip dhcp excluded-address 192.168.3.1 192.168.3.10
ip dhcp pool servidores
 network 192.168.3.0 255.255.255.0
 default-router 192.168.3.1
exit
end
write memory
"""
}

def apply_config(name, pos):
    print(f"[+] Applying config to {name} at {pos}...")
    pyautogui.click(pos[0], pos[1])
    time.sleep(1.2)
    pyautogui.hotkey('alt', 'c')
    time.sleep(0.5)
    pyautogui.typewrite("\n\n", interval=0.05)
    time.sleep(0.3)
    lines = configs[name].strip().split('\n')
    for l in lines:
        pyautogui.typewrite(l + "\n", interval=0.01)
        time.sleep(0.02)
    time.sleep(0.5)
    pyautogui.press('esc')
    time.sleep(0.8)

for dev, pos in devices.items():
    apply_config(dev, pos)

print("[✔] All core devices configured successfully!")
