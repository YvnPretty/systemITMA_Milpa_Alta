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

# Click Router1 / Router0 at coordinates (1020, 530)
pyautogui.click(1020, 530)
time.sleep(1)
pyautogui.hotkey('alt', 'c')
time.sleep(0.5)
pyautogui.typewrite("\n\n", interval=0.05)
time.sleep(0.3)

router_commands = """
enable
configure terminal
interface FastEthernet1/0
 ip address 192.168.3.1 255.255.255.0
 no shutdown
exit
router ospf 1
 network 192.168.3.0 0.0.0.255 area 0
 network 10.0.0.0 0.0.0.3 area 0
 network 10.0.0.4 0.0.0.3 area 0
exit
end
write memory
"""

for line in router_commands.strip().split('\n'):
    pyautogui.typewrite(line + "\n", interval=0.01)
    time.sleep(0.02)

time.sleep(0.5)
pyautogui.press('esc')
print("Router server interface & OSPF configured!")
