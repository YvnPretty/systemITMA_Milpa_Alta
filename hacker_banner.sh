#!/usr/bin/env bash
# Hacker OS Banner & Telemetry display
# Add 'bash ~/tsert/hacker_banner.sh' to your ~/.bashrc

CYAN='\033[1;36m'
GREEN='\033[1;32m'
GREEN_DIM='\033[0;32m'
RED='\033[1;31m'
YELLOW='\033[1;33m'
MAGENTA='\033[1;35m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

clear
echo -e "${GREEN}"
cat << "EOF"
 ██████╗██╗   ██╗██████╗ ███████╗██████╗ ██████╗ ██╗   ██╗███╗   ██╗██╗  ██╗
██╔════╝╚██╗ ██╔╝██╔══██╗██╔════╝██╔══██╗██╔══██╗██║   ██║████╗  ██║██║  ██║
██║      ╚████╔╝ ██████╔╝█████╗  ██████╔╝██████╔╝██║   ██║██╔██╗ ██║███████║
██║       ╚██╔╝  ██╔══██╗██╔══╝  ██╔══██╗██╔═══╝ ██║   ██║██║╚██╗██║██╔══██║
╚██████╗   ██║   ██████╔╝███████╗██║  ██║██║     ╚██████╔╝██║ ╚████║██║  ██║
 ╚═════╝   ╚═╝   ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝
EOF
echo -e "${CYAN}==========[ CYBERPUNK COMMAND CONSOLE // SYSTEM OK ]==========${NC}"

# Telemetry data
USER_HOST="$(whoami)@$(hostname)"
KERNEL="$(uname -r)"
UPTIME="$(uptime -p | sed 's/up //')"
CPU_LOAD="$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')%"
MEM_USED="$(free -m | awk '/Mem:/ { print $3 "MB / " $2 "MB (" int($3/$2*100) "%)" }')"
DISK_USED="$(df -h / | awk 'NR==2 {print $3 " / " $2 " (" $5 ")"}')"
LOCAL_IP="$(hostname -I | awk '{print $1}')"

echo -e " ${GREEN}▸ OPERATOR:${NC}    ${WHITE}${USER_HOST}${NC}"
echo -e " ${GREEN}▸ KERNEL:${NC}      ${WHITE}${KERNEL}${NC}"
echo -e " ${GREEN}▸ UPTIME:${NC}      ${WHITE}${UPTIME}${NC}"
echo -e " ${GREEN}▸ CPU LOAD:${NC}    ${YELLOW}${CPU_LOAD}${NC}"
echo -e " ${GREEN}▸ MEMORY:${NC}      ${CYAN}${MEM_USED}${NC}"
echo -e " ${GREEN}▸ DISK:${NC}        ${CYAN}${DISK_USED}${NC}"
echo -e " ${GREEN}▸ NETWORK IP:${NC}  ${MAGENTA}${LOCAL_IP:-127.0.0.1}${NC}"
echo -e "${CYAN}===============================================================${NC}"
echo -e " ${GREEN_DIM}Type ${WHITE}'matrix'${GREEN_DIM} for Matrix rain, ${WHITE}'sysdash'${GREEN_DIM} for Hacker Dashboard.${NC}\n"
