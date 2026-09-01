#!/usr/bin/env bash
# ====================================================================
# SCRIPT DE CONFIGURACIÓN DE FIREWALL / CORTAFUEGOS PARA SISTEMA LINUX
# Protege el host contra escaneos de puertos, ataques DDoS e intrusiones.
# ====================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}====================================================${NC}"
echo -e "${CYAN}   CONFIGURANDO CORTAFUEGOS (FIREWALL UFW / IPTABLES)${NC}"
echo -e "${CYAN}====================================================${NC}"

# Verificar si se ejecuta como root / sudo
if [ "$EUID" -ne 0 ]; then
    echo -e "${YELLOW}[!] Este script requiere permisos de superusuario (sudo).${NC}"
    echo -e "Ejecutando con sudo..."
    exec sudo bash "$0" "$@"
fi

# Check if UFW is available
if command -v ufw >/dev/null 2>&1; then
    echo -e "\n${YELLOW}[1/4] Restableciendo reglas por defecto de UFW...${NC}"
    ufw --force reset >/dev/null 2>&1

    echo -e "${YELLOW}[2/4] Definiendo políticas base (Denegar entrada, Permitir salida)...${NC}"
    ufw default deny incoming
    ufw default allow outgoing

    echo -e "${YELLOW}[3/4] Aplicando reglas de seguridad avanzadas...${NC}"
    # Permitir tráfico en interfaz de loopback local
    ufw allow in on lo

    # Limitar SSH (previene ataques de fuerza bruta)
    ufw limit 22/tcp comment 'SSH con limitación de tasa contra fuerza bruta'

    # Permitir servicios web comunes si se requiere
    ufw allow 80/tcp comment 'HTTP Web Server'
    ufw allow 443/tcp comment 'HTTPS Web Server'

    # Habilitar registro de eventos (logging)
    ufw logging medium

    echo -e "${YELLOW}[4/4] Activando Firewall UFW...${NC}"
    ufw --force enable

    echo -e "\n${GREEN}[✔] CORTAFUEGOS UFW ACTIVADO Y CONFIGURADO CON ÉXITO.${NC}"
    ufw status verbose

else
    echo -e "\n${YELLOW}[1/3] UFW no encontrado. Aplicando reglas directas de IPTABLES...${NC}"
    
    # Limpiar reglas anteriores
    iptables -F
    iptables -X
    iptables -Z
    
    # Políticas por defecto
    iptables -P INPUT DROP
    iptables -P FORWARD DROP
    iptables -P OUTPUT ACCEPT
    
    # Permitir tráfico local
    iptables -A INPUT -i lo -j ACCEPT
    
    # Permitir conexiones ya establecidas o relacionadas (Stateful Inspection)
    iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
    
    # Protección contra SYN Floods
    iptables -A INPUT -p tcp --syn -m limit --limit 1/s --limit-burst 3 -j ACCEPT
    
    # Bloquear paquetes inválidos
    iptables -A INPUT -m state --state INVALID -j DROP
    
    # Permitir SSH con tasa limitada
    iptables -A INPUT -p tcp --dport 22 -m state --state NEW -m recent --set
    iptables -A INPUT -p tcp --dport 22 -m state --state NEW -m recent --update --seconds 60 --hitcount 4 -j DROP
    iptables -A INPUT -p tcp --dport 22 -j ACCEPT
    
    echo -e "\n${GREEN}[✔] CORTAFUEGOS IPTABLES CONFIGURADO EXITOSAMENTE.${NC}"
fi

echo -e "\n${CYAN}====================================================${NC}"
echo -e "${GREEN} Tu red/sistema local ahora cuenta con protección de firewall activo.${NC}"
echo -e "${CYAN}====================================================${NC}"
