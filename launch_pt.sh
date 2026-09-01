#!/bin/bash
export DISPLAY=:0
export XAUTHORITY=$(ls -t /run/user/1000/.mutter-Xwaylandauth* 2>/dev/null | head -n 1)

# Check if Packet Tracer is already running
if pgrep -f "PacketTracer" > /dev/null; then
    echo "Packet Tracer ya está en ejecución."
else
    echo "Iniciando Packet Tracer con Practica_Balanceo_Empresarial.pkt..."
    nohup /home/pretty/pt/packettracer /home/pretty/Desktop/Practica_Balanceo_Empresarial.pkt > /dev/null 2>&1 &
    sleep 3
fi

# Traer al frente la ventana
python3 -c "
import pyautogui, time
pyautogui.hotkey('alt', 'tab')
" 2>/dev/null || true
