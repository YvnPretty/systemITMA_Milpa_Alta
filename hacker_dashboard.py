#!/usr/bin/env python3
"""
Interactive Cyberpunk / Hacker Terminal Dashboard
Displays system stats, simulated packet sniffer logs, and CPU graphs in real time.
"""

import sys
import time
import os
import random
import shutil
import subprocess

GREEN = "\033[92;1m"
GREEN_DIM = "\033[32m"
CYAN = "\033[96;1m"
WHITE = "\033[97;1m"
RED = "\033[91;1m"
YELLOW = "\033[93;1m"
RESET = "\033[0m"
CLEAR = "\033[2J\033[H"
HIDE_CURSOR = "\033[?25l"
SHOW_CURSOR = "\033[?25h"

PROTOCOL_LIST = ["TCP", "UDP", "HTTPS", "SSH", "MQTT", "WS", "DNS"]
TARGET_IPS = ["192.168.1.1", "10.0.4.15", "172.16.0.44", "8.8.8.8", "45.33.32.156"]

def get_cpu_ram():
    try:
        with open("/proc/loadavg", "r") as f:
            load = f.read().split()[0]
    except:
        load = "0.15"

    try:
        with open("/proc/meminfo", "r") as f:
            lines = f.readlines()
            mem_total = int(lines[0].split()[1])
            mem_avail = int(lines[2].split()[1])
            mem_used_pct = int(((mem_total - mem_avail) / mem_total) * 100)
    except:
        mem_used_pct = 35

    return load, mem_used_pct

def make_bar(pct, width=20):
    filled = int((pct / 100.0) * width)
    bar = "█" * filled + "░" * (width - filled)
    if pct > 80:
        return f"{RED}[{bar}] {pct}%{RESET}"
    elif pct > 50:
        return f"{YELLOW}[{bar}] {pct}%{RESET}"
    return f"{GREEN}[{bar}] {pct}%{RESET}"

def generate_packet():
    proto = random.choice(PROTOCOL_LIST)
    src = random.choice(TARGET_IPS)
    dst = f"192.168.1.{random.randint(2, 254)}"
    port = random.choice([80, 443, 22, 1883, 8080, 53])
    bytes_count = random.randint(64, 4096)
    status = "ALLOW" if random.random() > 0.1 else "ALERT"
    status_color = GREEN if status == "ALLOW" else RED
    return f"[{time.strftime('%H:%M:%S')}] {proto:<5} {src:<15} -> {dst:<15} PORT:{port:<5} {bytes_count}B {status_color}{status}{RESET}"

def main():
    sys.stdout.write(CLEAR + HIDE_CURSOR)
    sys.stdout.flush()
    packet_logs = []

    try:
        while True:
            cols, rows = shutil.get_terminal_size((80, 24))
            load, mem_pct = get_cpu_ram()

            # Add new random log
            packet_logs.append(generate_packet())
            if len(packet_logs) > 8:
                packet_logs.pop(0)

            # Build UI Header
            ui = [f"\033[1;1H{CYAN}┌───[ CYBERPUNK TELEMETRY & NETWORK DASHBOARD ]" + "─" * max(0, cols - 48) + "┐" + RESET]
            ui.append(f"{CYAN}│{RESET} {GREEN}NODE STATUS:${RESET} {WHITE}ONLINE{RESET}  |  {GREEN}CPU LOAD:${RESET} {WHITE}{load}{RESET}  |  {GREEN}MEMORY:${RESET} {make_bar(mem_pct, 15)}")
            ui.append(f"{CYAN}├" + "─" * (cols - 2) + "┤" + RESET)
            ui.append(f"{CYAN}│{RESET} {YELLOW}PACKET STREAM SNIFFER (SIMULATED):{RESET}")

            for log in packet_logs:
                ui.append(f"{CYAN}│{RESET}  {log}")

            ui.append(f"{CYAN}├" + "─" * (cols - 2) + "┤" + RESET)
            ui.append(f"{CYAN}│{RESET} {GREEN}ACTIVE HARDWARE MONITORS:${RESET} [ESP32 STREAM: READY] [SERIAL: PORT OK] [BLE: SCANNING]")
            ui.append(f"{CYAN}└" + "─" * (cols - 2) + "┘" + RESET)
            ui.append(f"\n{GREEN_DIM}Press Ctrl+C to exit dashboard...{RESET}")

            sys.stdout.write(CLEAR + "\n".join(ui))
            sys.stdout.flush()
            time.sleep(0.5)

    except KeyboardInterrupt:
        pass
    finally:
        sys.stdout.write(SHOW_CURSOR + RESET + "\n")
        sys.stdout.flush()

if __name__ == "__main__":
    main()
