#!/usr/bin/env bash
# Desktop Hacker Setup Script: Conky HUD Widgets & Animated Backgrounds

CYAN='\033[1;36m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
WHITE='\033[1;37m'
NC='\033[0m'

echo -e "${CYAN}=== Configurando Widgets de Escritorio Hacker (Conky HUD) ===${NC}\n"

# 1. Create Conky config folder
CONKY_DIR="$HOME/.config/conky"
mkdir -p "$CONKY_DIR"

CONKY_CONF="$CONKY_DIR/cyberpunk.conkyrc"

cat << 'EOF' > "$CONKY_CONF"
conky.config = {
    alignment = 'top_right',
    background = true,
    border_width = 1,
    cpu_avg_samples = 2,
    default_color = '00ff66',
    default_outline_color = '00ff66',
    default_shade_color = '00ff66',
    draw_borders = false,
    draw_graph_borders = true,
    draw_outline = false,
    draw_shades = false,
    use_xft = true,
    font = 'Monospace:size=10',
    gap_x = 35,
    gap_y = 60,
    minimum_height = 5,
    minimum_width = 280,
    net_avg_samples = 2,
    no_buffers = true,
    out_to_console = false,
    out_to_nmain = false,
    out_to_stderr = false,
    extra_newline = false,
    own_window = true,
    own_window_class = 'Conky',
    own_window_type = 'desktop',
    own_window_transparent = true,
    own_window_hints = 'undecorated,below,sticky,skip_taskbar,skip_pager',
    stippled_borders = 0,
    update_interval = 1.0,
    uppercase = false,
    use_spacer = 'none',
    show_graph_scale = false,
    show_graph_range = false
}

conky.text = [[
${color 00f3ff}${font Monospace:size=12:bold}SYSTEM TELEMETRY // CYBERPUNK${font}
${color 00ff66}${hr 2}
${color e0ffe8}OPERATOR:${color 00f3ff} ${exec whoami}@${nodename}
${color e0ffe8}UPTIME:${color 00f3ff} $uptime
${color e0ffe8}KERNEL:${color 00f3ff} $kernel
${color 00ff66}${hr 1}
${color 00f3ff}${font Monospace:bold}CPU LOAD:${color e0ffe8} $cpu% ${cpubar 8,120 00ff66 00f3ff}
${color 00ff66}${cpugraph cpu0 30,280 00ff66 00f3ff}
${color 00f3ff}${font Monospace:bold}RAM USAGE:${color e0ffe8} $mem/$memmax ($memperc%)
${color 00ff66}${membar 8,280 00ff66 00f3ff}
${color 00ff66}${hr 1}
${color 00f3ff}${font Monospace:bold}PROCESSES:${color e0ffe8} $processes ($running_processes active)
${color e0ffe8}TOP CPU:${color 00ff66} ${top name 1} ${top cpu 1}%
${color e0ffe8}TOP RAM:${color 00f3ff} ${top_mem name 1} ${top_mem mem_res 1}
${color 00ff66}${hr 2}
${color 5c9c6f}TS_ERT SECURE NODE // ACTIVE
]]
EOF

echo -e "${GREEN}✓ Configuración creada en: $CONKY_CONF${NC}"

echo -e "\n${YELLOW}Instalando paquetes Conky y Guake para el escritorio...${NC}"
echo -e "${WHITE}Ejecuta el siguiente comando en tu terminal para activar los widgets de escritorio:${NC}"
echo -e "${CYAN}sudo apt install -y conky-all guake mpv xwinwrap${NC}"
echo -e "${WHITE}Y luego inicia los widgets transparentes con:${NC}"
echo -e "${CYAN}conky -c ~/.config/conky/cyberpunk.conkyrc &${NC}"
