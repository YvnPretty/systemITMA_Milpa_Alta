from Xlib import X, display
import Xlib.protocol.event
import os
import time

def focus_window_by_title(title_sub):
    os.environ['DISPLAY'] = ':0'
    xauth_cmd = "ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1"
    xauth = os.popen(xauth_cmd).read().strip()
    if xauth:
        os.environ['XAUTHORITY'] = xauth

    d = display.Display()
    root = d.screen().root
    
    # Get all windows
    client_list_atom = d.intern_atom('_NET_CLIENT_LIST')
    window_ids_prop = root.get_full_property(client_list_atom, X.AnyPropertyType)
    if not window_ids_prop:
        print("No windows found on desktop")
        return False
        
    window_ids = window_ids_prop.value
    
    for window_id in window_ids:
        win = d.create_resource_object('window', window_id)
        win_name = win.get_wm_name()
        if win_name and title_sub.lower() in win_name.lower():
            print(f"Found window: {win_name}")
            
            # Send message to window manager to activate the window (_NET_ACTIVE_WINDOW)
            active_atom = d.intern_atom('_NET_ACTIVE_WINDOW')
            
            # Client message format: window, client_type, data
            # Data format: source (1 = application), timestamp (0 = current), requestor's active window, etc.
            ev = Xlib.protocol.event.ClientMessage(
                window=win,
                client_type=active_atom,
                data=(32, [1, 0, 0, 0, 0])
            )
            
            root.send_event(ev, event_mask=X.SubstructureRedirectMask | X.SubstructureNotifyMask)
            
            # Additional maps as fallback
            win.map()
            win.configure(stack_mode=X.Above)
            d.set_input_focus(win, X.RevertToParent, X.CurrentTime)
            d.sync()
            print(f"Successfully focused and raised window: {win_name}")
            return True
            
    print(f"Window containing '{title_sub}' not found")
    return False

if __name__ == '__main__':
    focus_window_by_title("Cisco Packet Tracer")
