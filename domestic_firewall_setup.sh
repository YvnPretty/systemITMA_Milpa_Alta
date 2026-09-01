#!/usr/bin/env bash
# ====================================================================
# SCRIPT DE CORTAFUEGOS Y SEGURIDAD PARA RED DOMÉSTICA / HOGAR
# Diseñado para proteger el host en la Wi-Fi local (192.168.1.x)
# ====================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  CONFIGURACIÓN DE CORTAFUEGOS PARA TU RED DOMÉSTICA ${NC}"
echo -e "${CYAN}======================================================${NC}"

# Autodetectar interfaz principal y subred local
IFACE=$(ip route | grep default | awk '{print $5}' | head -n 1)
SUBNET=$(ip route | grep -v default | grep "proto kernel" | awk '{print $1}' | head -n 1)
MY_IP=$(ip route | grep default | awk '{print $7}' | head -n 1)
GATEWAY=$(ip route | grep default | awk '{print $3}' | head -n 1)

if [ -z "$IFACE" ]; then
    IFACE="wlp0s20f3"
    SUBNET="192.168.1.0/24"
fi

echo -e "${YELLOW}[+] Interfaz detectada:${NC} $IFACE"
echo -e "${YELLOW}[+] Tu IP Local:${NC} $MY_IP"
echo -e "${YELLOW}[+] Subred Doméstica:${NC} $SUBNET"
echo -e "${YELLOW}[+] Router/Gateway:${NC} $GATEWAY"
echo -e "${CYAN}------------------------------------------------------${NC}"

# Verificar permisos de superusuario
if [ "$EUID" -ne 0 ]; then
    echo -e "${YELLOW}[!] Se requieren permisos sudo para aplicar el firewall.${NC}"
    echo -e "Ejecutando con sudo..."
    exec sudo bash "$0" "$@"
fi

if command -v ufw >/dev/null 2>&1; then
    echo -e "\n${YELLOW}[1/5] Restableciendo UFW a valores limpios...${NC}"
    ufw --force reset >/dev/null 2>&1

    echo -e "${YELLOW}[2/5] Configurando políticas base...${NC}"
    # Denegar todo acceso entrante no solicitado
    ufw default deny incoming
    # Permitir salidas (navegación web, descargas, etc.)
    ufw default allow outgoing
    # Activar protección IPv6
    sed -i 's/IPV6=no/IPV6=yes/' /etc/default/ufw 2>/dev/null

    echo -e "${YELLOW}[3/5] Aplicando reglas para la red doméstica...${NC}"
    # Permitir tráfico local de confianza (dispositivos IoT, ESP32, impresoras, PCs en tu Wi-Fi)
    ufw allow in on "$IFACE" from "$SUBNET" comment 'Permitir tráfico de red local doméstica'

    # Permitir interfaz loopback
    ufw allow in on lo comment 'Permitir Loopback local'

    # Limitar SSH para evitar fuerza bruta si estuviera activo
    ufw limit in on "$IFACE" proto tcp to any port 22 comment 'SSH con limitador de intentos'

    # Bloquear puertos vulnerables externos común en hogares (SMB, NetBIOS, RPC)
    ufw deny in proto tcp to any port 135,139,445 comment 'Bloquear SMB/RPC vulnerables'
    ufw deny in proto udp to any port 137,138,1900 comment 'Bloquear NetBIOS y SSDP público'

    # Habilitar logging medio
    ufw logging medium

    echo -e "${YELLOW}[4/5] Activando el Cortafuegos UFW...${NC}"
    ufw --force enable

    echo -e "\n${GREEN}[✔] CORTAFUEGOS DOMÉSTICO ACTIVADO CON ÉXITO.${NC}"
    echo -e "${CYAN}--- ESTADO ACTUAL DE UFW ---${NC}"
    ufw status verbose

else
    echo -e "\n${YELLOW}[1/4] Configurando Firewall con IPTABLES para red doméstica...${NC}"
    iptables -F
    iptables -X

    # Políticas por defecto
    iptables -P INPUT DROP
    iptables -P FORWARD DROP
    iptables -P OUTPUT ACCEPT

    # Loopback y conexones establecidas
    iptables -A INPUT -i lo -j ACCEPT
    iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

    # Permitir tráfico de tu subred local
    iptables -A INPUT -i "$IFACE" -s "$SUBNET" -j ACCEPT

    # Limitar Pings/ICMP
    iptables -A INPUT -p icmp --icmp-type echo-request -m limit --limit 1/s -j ACCEPT
    iptables -A INPUT -p icmp --icmp-type echo-request -j DROP

    # Protecciones contra escaneo de puertos
    iptables -A INPUT -m state --state INVALID -j DROP

    echo -e "\n${GREEN}[✔] REGLAS IPTABLES DOMÉSTICAS APLICADAS CON ÉXITO.${NC}"
fi

echo -e "\n${CYAN}======================================================${NC}"
echo -e "${GREEN} Tu equipo está protegido en la red Wi-Fi doméstica ($SUBNET).${NC}"
echo -e "${CYAN}======================================================${NC}"
