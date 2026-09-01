#!/bin/bash

cat << 'EOF'
====================================================================
  CONFIGURACIÓN DE CORTAFUEGOS Y SEGURIDAD AVANZADA CISCO (FIREWALL)
====================================================================

--------------------------------------------------------------------
[1] ROUTER_1 - FIREWALL DE INSPECCIÓN DE ESTADO (CBAC/ZFW) Y SEGURIDAD VLAN
--------------------------------------------------------------------
enable
configure terminal
hostname Router_1

! --- 1. Definición de Inspección de Estado (Stateful Firewall) ---
ip inspect name FW_INSPECT tcp
ip inspect name FW_INSPECT udp
ip inspect name FW_INSPECT icmp

! --- 2. Filtro de Seguridad Avanzado e Aislamiento de VLANs ---
ip access-list extended FW_RULES_VLAN
 ! Permiso total para Sistemas IT (VLAN 10)
 permit ip 192.168.10.0 0.0.0.255 any
 ! Permiso para Gerencia RH (VLAN 40) a la red corporativa y servidores
 permit ip 192.168.40.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit ip 192.168.40.0 0.0.0.255 192.168.10.0 0.0.0.255
 ! Contabilidad (VLAN 20) solo accede al Servidor Web/Servicios en 10.0.0.1
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.20.0 0.0.0.255 192.168.30.0 0.0.0.255
 deny ip 192.168.20.0 0.0.0.255 10.0.0.0 0.0.0.255
 ! Ventas (VLAN 30) solo accede al Servidor Web/Servicios en 10.0.0.1
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.30.0 0.0.0.255 192.168.20.0 0.0.0.255
 deny ip 192.168.30.0 0.0.0.255 10.0.0.0 0.0.0.255
 ! Permitir el resto de tráfico verificado hacia el exterior
 permit ip any any
 exit

! --- 3. Aplicación de Firewall en Interfaces ---
interface GigabitEthernet 0/1
 ip access-group FW_RULES_VLAN in
 ip inspect FW_INSPECT out
 exit

! --- 4. Protecciones Anti-Spoofing e Ingress Filtering ---
ip access-list extended ANTI_SPOOFING
 deny ip 192.168.0.0 0.0.255.255 any
 deny ip 10.0.0.0 0.0.255.255 any
 permit ip any any
 exit

! --- 5. Asegurar Acceso SSH a la Consola de Administración (Solo VLAN 10 IT) ---
ip access-list standard ACCESO_ADMIN_SSH
 permit 192.168.10.0 0.0.0.255
 exit

line vty 0 4
 access-class ACCESO_ADMIN_SSH in
 transport input ssh
 exit

end
write memory


--------------------------------------------------------------------
[2] ROUTER_2 - FIREWALL REDUNDANTE HSRP Y SEGURIDAD INTER-VLAN
--------------------------------------------------------------------
enable
configure terminal
hostname Router_2

! --- 1. Definición de Inspección de Estado (Stateful Firewall) ---
ip inspect name FW_INSPECT tcp
ip inspect name FW_INSPECT udp
ip inspect name FW_INSPECT icmp

! --- 2. Filtro de Seguridad Avanzado e Aislamiento de VLANs ---
ip access-list extended FW_RULES_VLAN
 permit ip 192.168.10.0 0.0.0.255 any
 permit ip 192.168.40.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit ip 192.168.40.0 0.0.0.255 192.168.10.0 0.0.0.255
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.20.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.20.0 0.0.0.255 192.168.30.0 0.0.0.255
 deny ip 192.168.20.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 80
 permit tcp 192.168.30.0 0.0.0.255 host 10.0.0.1 eq 443
 deny ip 192.168.30.0 0.0.0.255 192.168.20.0 0.0.0.255
 deny ip 192.168.30.0 0.0.0.255 10.0.0.0 0.0.0.255
 permit ip any any
 exit

! --- 3. Aplicación en Interfaz WAN ---
interface GigabitEthernet 0/1
 ip access-group FW_RULES_VLAN in
 ip inspect FW_INSPECT out
 exit

! --- 4. Asegurar Acceso de Administración (VLAN 10 IT) ---
ip access-list standard ACCESO_ADMIN_SSH
 permit 192.168.10.0 0.0.0.255
 exit

line vty 0 4
 access-class ACCESO_ADMIN_SSH in
 transport input ssh
 exit

end
write memory


--------------------------------------------------------------------
[3] ROUTER_REMOTE - FIREWALL PERIMETRAL Y PROTECCIÓN CONTRA ATAQUES
--------------------------------------------------------------------
enable
configure terminal
hostname Router_Remote

! --- 1. Inspección de Estado en Frontera WAN ---
ip inspect name PERIMETER_FW tcp
ip inspect name PERIMETER_FW udp
ip inspect name PERIMETER_FW icmp

! --- 2. Filtro Perimetral Anti-DDoS y Bloqueo de Tráfico Malicioso ---
ip access-list extended PERIMETER_FILTER
 ! Bloquear paquetes con IP origen falsificada (Bogons / Anti-Spoofing)
 deny ip 127.0.0.0 0.255.255.255 any
 deny ip 0.0.0.0 0.255.255.255 any
 ! Permitir solo tráfico legítimo hacia el Servidor de Aplicaciones Web (10.0.0.1)
 permit tcp any host 10.0.0.1 eq 80
 permit tcp any host 10.0.0.1 eq 443
 permit icmp any any echo-reply
 permit ip 192.168.10.0 0.0.0.255 any
 permit ip 192.168.40.0 0.0.0.255 any
 ! Bloquear todo tráfico entrante no solicitado a los servidores internos
 deny ip any 10.0.0.0 0.0.0.255
 permit ip any any
 exit

interface GigabitEthernet 0/2
 ip access-group PERIMETER_FILTER out
 ip inspect PERIMETER_FW in
 exit

end
write memory
EOF
