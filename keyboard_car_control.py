#!/usr/bin/env python3
import sys
import os
import time
import curses
import serial

def find_serial_port():
    ports = ['/dev/ttyACM0', '/dev/ttyACM1', '/dev/ttyUSB0', '/dev/ttyUSB1']
    for p in ports:
        if os.path.exists(p):
            return p
    return '/dev/ttyACM0'

PORT = find_serial_port()
BAUD = 9600

def safe_addstr(stdscr, y, x, text, attr=0):
    """Función ultra segura que jamás se rompe por tamaño de ventana de terminal"""
    try:
        max_y, max_x = stdscr.getmaxyx()
        if y < max_y and x < max_x:
            available = max_x - x
            if available > 0:
                stdscr.addstr(y, x, text[:available], attr)
    except Exception:
        pass

def run_curses_control(stdscr):
    curses.noecho()
    curses.cbreak()
    curses.curs_set(0)
    stdscr.keypad(True)
    stdscr.nodelay(True)

    if curses.has_colors():
        curses.start_color()
        curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)   # Forward
        curses.init_pair(2, curses.COLOR_YELLOW, curses.COLOR_BLACK)  # Backward
        curses.init_pair(3, curses.COLOR_MAGENTA, curses.COLOR_BLACK) # Left
        curses.init_pair(4, curses.COLOR_CYAN, curses.COLOR_BLACK)    # Right
        curses.init_pair(5, curses.COLOR_RED, curses.COLOR_BLACK)     # Stop
        curses.init_pair(6, curses.COLOR_WHITE, curses.COLOR_BLACK)   # Normal

    ser = None
    serial_error = None
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
        time.sleep(1.5)
    except Exception as e:
        serial_error = str(e)

    traction_state = "PARADO"
    steering_state = "CENTRO"
    speed = 230
    last_action_desc = "Ninguna"
    running = True

    while running:
        try:
            key = stdscr.getch()
        except Exception:
            key = -1

        new_cmd = None

        if key != -1:
            # W / Up -> Avanzar (M4 FORWARD)
            if key in (curses.KEY_UP, ord('w'), ord('W')):
                new_cmd = 'w'
                traction_state = "ADELANTE (M4)"
                last_action_desc = "W -> M4 FORWARD"

            # S / Down -> Retroceder (M4 BACKWARD)
            elif key in (curses.KEY_DOWN, ord('s'), ord('S')):
                new_cmd = 's'
                traction_state = "ATRÁS (M4)"
                last_action_desc = "S -> M4 BACKWARD"

            # A / Left -> Dirección Izquierda (M2 FORWARD)
            elif key in (curses.KEY_LEFT, ord('a'), ord('A')):
                new_cmd = 'a'
                steering_state = "IZQUIERDA (M2)"
                last_action_desc = "A -> M2 FORWARD"

            # D / Right -> Dirección Derecha (M2 BACKWARD)
            elif key in (curses.KEY_RIGHT, ord('d'), ord('D')):
                new_cmd = 'd'
                steering_state = "DERECHA (M2)"
                last_action_desc = "D -> M2 BACKWARD"

            # Q -> Diagonal Adelante + Izquierda
            elif key in (ord('q'), ord('Q')):
                new_cmd = 'q'
                traction_state = "ADELANTE (M4)"
                steering_state = "IZQUIERDA (M2)"
                last_action_desc = "Q -> Adelante + Izq"

            # E -> Diagonal Adelante + Derecha
            elif key in (ord('e'), ord('E')):
                new_cmd = 'e'
                traction_state = "ADELANTE (M4)"
                steering_state = "DERECHA (M2)"
                last_action_desc = "E -> Adelante + Der"

            # Z -> Diagonal Atrás + Izquierda
            elif key in (ord('z'), ord('Z')):
                new_cmd = 'z'
                traction_state = "ATRÁS (M4)"
                steering_state = "IZQUIERDA (M2)"
                last_action_desc = "Z -> Atrás + Izq"

            # V -> Diagonal Atrás + Derecha
            elif key in (ord('v'), ord('V')):
                new_cmd = 'v'
                traction_state = "ATRÁS (M4)"
                steering_state = "DERECHA (M2)"
                last_action_desc = "V -> Atrás + Der"

            # R -> Centrar M2
            elif key in (ord('r'), ord('R')):
                new_cmd = 'r'
                steering_state = "CENTRO (M2)"
                last_action_desc = "R -> Centrar M2"

            # Espacio / X -> Detener Todo
            elif key in (ord(' '), ord('x'), ord('X')):
                new_cmd = ' '
                traction_state = "PARADO"
                steering_state = "CENTRO"
                last_action_desc = "ESPACIO -> DETENER"

            # + -> Subir Velocidad
            elif key in (ord('+'), ord('=')):
                if speed <= 240: speed += 15
                new_cmd = '+'
                last_action_desc = "+ -> Subir Velocidad"

            # - -> Bajar Velocidad
            elif key in (ord('-'), ord('_')):
                if speed >= 115: speed -= 15
                new_cmd = '-'
                last_action_desc = "- -> Bajar Velocidad"

            # ESC / Q -> Salir
            elif key in (27, ord('q'), ord('Q')) and last_action_desc != "Q -> Adelante + Izq":
                running = False
                if ser:
                    try:
                        ser.write(b' ')
                        ser.flush()
                    except Exception:
                        pass
                break

            if new_cmd and ser:
                try:
                    ser.write(new_cmd.encode('utf-8'))
                    ser.flush()
                except Exception:
                    pass

        # Dibujar Interfaz Segura
        stdscr.erase()

        safe_addstr(stdscr, 0, 0, "==========================================================", curses.color_pair(4))
        safe_addstr(stdscr, 1, 0, " 🚗 CONTROL ARDUINO L293D (M2 DIRECCIÓN | M4 TRACCIÓN)", curses.A_BOLD | curses.color_pair(4))
        safe_addstr(stdscr, 2, 0, "==========================================================", curses.color_pair(4))

        if serial_error:
            safe_addstr(stdscr, 4, 0, f" ❌ ERROR PUERTO SERIE ({PORT}): {serial_error}", curses.color_pair(5))
        else:
            safe_addstr(stdscr, 4, 0, f" Puerto: {PORT} @ {BAUD} | Estado: CONECTADO", curses.color_pair(1))

        safe_addstr(stdscr, 6, 0, "----------------------------------------------------------", curses.color_pair(4))
        
        # Indicador Tracción
        safe_addstr(stdscr, 7, 0, " 🏎️ TRACCIÓN (M4 ATRÁS): ")
        if "ADELANTE" in traction_state:
            safe_addstr(stdscr, 7, 24, "▲ ADELANTE", curses.A_BOLD | curses.color_pair(1))
        elif "ATRÁS" in traction_state:
            safe_addstr(stdscr, 7, 24, "▼ ATRÁS", curses.A_BOLD | curses.color_pair(2))
        else:
            safe_addstr(stdscr, 7, 24, "■ PARADO", curses.A_BOLD | curses.color_pair(5))

        # Indicador Dirección
        safe_addstr(stdscr, 8, 0, " 🎯 DIRECCIÓN (M2 ENFRENTE): ")
        if "IZQUIERDA" in steering_state:
            safe_addstr(stdscr, 8, 28, "◀ IZQUIERDA", curses.A_BOLD | curses.color_pair(3))
        elif "DERECHA" in steering_state:
            safe_addstr(stdscr, 8, 28, "▶ DERECHA", curses.A_BOLD | curses.color_pair(4))
        else:
            safe_addstr(stdscr, 8, 28, "◯ CENTRO", curses.A_BOLD | curses.color_pair(6))

        safe_addstr(stdscr, 9, 0, "----------------------------------------------------------", curses.color_pair(4))
        safe_addstr(stdscr, 10, 0, f" VELOCIDAD: {speed}/255 [Ajustar: + / -]")
        safe_addstr(stdscr, 11, 0, f" ÚLTIMA TECLA: {last_action_desc}")

        safe_addstr(stdscr, 13, 0, " CONTROLES:")
        safe_addstr(stdscr, 14, 0, "   • W / Flecha Arriba   : M4 Adelante")
        safe_addstr(stdscr, 15, 0, "   • S / Flecha Abajo    : M4 Atrás")
        safe_addstr(stdscr, 16, 0, "   • A / Flecha Izq      : M2 Izquierda")
        safe_addstr(stdscr, 17, 0, "   • D / Flecha Der      : M2 Derecha")
        safe_addstr(stdscr, 18, 0, "   • Espacio             : Detener")
        safe_addstr(stdscr, 19, 0, "   • ESC                 : Salir")
        safe_addstr(20, 0, "==========================================================", curses.color_pair(4))

        try:
            stdscr.refresh()
        except Exception:
            pass

        time.sleep(0.04)

    if ser:
        ser.close()

def main():
    try:
        curses.wrapper(run_curses_control)
    except Exception as e:
        print(f"Error en script: {e}")

if __name__ == '__main__':
    main()
