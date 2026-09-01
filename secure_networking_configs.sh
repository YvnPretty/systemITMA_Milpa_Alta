#!/bin/bash

cat << 'EOF'
====================================================================
  CONFIGURACIONES CISCO - SECURE NETWORKING PROJECT (USA GROUP)
  Topología: Site A, Site B (VLAN 20), HQ DMZ ZONE, ASA Firewall,
            Core MLSW1 (PAgP + HSRP), NYK-Router1 (OSPF + ACL), MAIN-ISP
====================================================================

--------------------------------------------------------------------
[1] NYK1-SW (Switch Site A - 2960-24TT)
--------------------------------------------------------------------
enable
configure terminal
hostname NYK1-SW
vlan 10
 name SITE_A_VLAN
vlan 20
 name SITE_B_VLAN
exit
interface range fastEthernet 0/1 - 2
 switchport mode trunk
 channel-group 1 mode desirable
 exit
interface range fastEthernet 0/3 - 15
 switchport mode access
 switchport access vlan 10
 exit
end
write memory


--------------------------------------------------------------------
[2] NYK2-SW (Switch Site B - 2960-24TT - NET 172.16.2.0/24 DHCP)
--------------------------------------------------------------------
enable
configure terminal
hostname NYK2-SW
vlan 10
 name SITE_A_VLAN
vlan 20
 name SITE_B_VLAN
exit
interface range fastEthernet 0/1 - 2
 switchport mode trunk
 channel-group 2 mode desirable
 exit
interface range fastEthernet 0/3 - 20
 switchport mode access
 switchport access vlan 20
 exit
end
write memory


--------------------------------------------------------------------
[3] NYK-MLSW1 (Multilayer Core Switch 3650 - PAgP + HSRP)
--------------------------------------------------------------------
enable
configure terminal
hostname NYK-MLSW1
ip routing

vlan 10
 name SITE_A_VLAN
vlan 20
 name SITE_B_VLAN
exit

! --- EtherChannel PAgP a NYK1-SW ---
interface range GigabitEthernet1/0/1 - 2
 switchport trunk encapsulation dot1q
 switchport mode trunk
 channel-group 1 mode desirable
 exit

! --- EtherChannel PAgP a NYK2-SW ---
interface range GigabitEthernet1/0/3 - 4
 switchport trunk encapsulation dot1q
 switchport mode trunk
 channel-group 2 mode desirable
 exit

! --- Interfaz L3 a NYK-Router1 ---
interface GigabitEthernet1/0/24
 no switchport
 ip address 10.10.4.6 255.255.255.252
 no shutdown
 exit

! --- Configuracion HSRP Gateways Redundantes ---
interface Vlan10
 ip address 172.16.1.2 255.255.255.0
 standby 10 ip 172.16.1.1
 standby 10 priority 120
 standby 10 preempt
 exit

interface Vlan20
 ip address 172.16.2.2 255.255.255.0
 standby 20 ip 172.16.2.1
 standby 20 priority 120
 standby 20 preempt
 exit

! --- OSPF ---
router ospf 1
 router-id 1.1.1.1
 network 172.16.1.0 0.0.0.255 area 0
 network 172.16.2.0 0.0.0.255 area 0
 network 10.10.4.4 0.0.0.3 area 0
 exit

end
write memory


--------------------------------------------------------------------
[4] DC-SWITCH (DMZ Zone Switch - 2960-24TT)
--------------------------------------------------------------------
enable
configure terminal
hostname DC-SWITCH
vlan 50
 name DMZ_ZONE
exit
interface range fastEthernet 0/1 - 10
 switchport mode access
 switchport access vlan 50
 exit
interface GigabitEthernet 0/1
 switchport mode trunk
 exit
end
write memory


--------------------------------------------------------------------
[5] NYK-ROUTER1 (Router Core 2811 - OSPF + ACL)
--------------------------------------------------------------------
enable
configure terminal
hostname NYK-Router1

interface FastEthernet0/0
 ip address 10.10.4.5 255.255.255.252
 ip access-group ACL_SECURITY_NYK in
 no shutdown
 exit

interface FastEthernet0/1
 ip address 10.10.4.2 255.255.255.252
 no shutdown
 exit

ip access-list extended ACL_SECURITY_NYK
 permit ip 172.16.1.0 0.0.0.255 any
 permit ip 172.16.2.0 0.0.0.255 any
 deny ip 172.16.2.0 0.0.0.255 10.10.1.224 0.0.0.31
 permit ip any any
 exit

router ospf 1
 router-id 2.2.2.2
 network 10.10.4.4 0.0.0.3 area 0
 network 10.10.4.0 0.0.0.3 area 0
 exit

end
write memory


--------------------------------------------------------------------
[6] NYK-ASA-FIREWALL (Cisco ASA 5506-X - Inside/Outside/DMZ)
--------------------------------------------------------------------
enable
configure terminal
hostname NYK-ASA-FIREWALL

interface GigabitEthernet1/1
 nameif inside
 security-level 100
 ip address 10.10.4.1 255.255.255.252
 no shutdown
 exit

interface GigabitEthernet1/2
 nameif dmz
 security-level 50
 ip address 10.10.1.225 255.255.255.224
 no shutdown
 exit

interface GigabitEthernet1/3
 nameif outside
 security-level 0
 ip address 200.100.50.2 255.255.255.252
 no shutdown
 exit

object network NET_INSIDE
 subnet 172.16.0.0 255.255.0.0
 nat (inside,outside) dynamic interface
 exit

object network NET_DMZ
 subnet 10.10.1.224 255.255.255.224
 nat (dmz,outside) static interface
 exit

route outside 0.0.0.0 0.0.0.0 200.100.50.1
route inside 172.16.0.0 255.255.0.0 10.10.4.2
route dmz 10.10.1.224 255.255.255.224 10.10.1.225

write memory


--------------------------------------------------------------------
[7] MAIN-ISP (Router 2811 - Internet Provider)
--------------------------------------------------------------------
enable
configure terminal
hostname MAIN-ISP

interface FastEthernet0/0
 ip address 200.100.50.1 255.255.255.252
 no shutdown
 exit

interface FastEthernet0/1
 ip address 200.100.50.5 255.255.255.252
 no shutdown
 exit

interface FastEthernet1/0
 ip address 200.100.50.33 255.255.255.252
 no shutdown
 exit

ip route 0.0.0.0 0.0.0.0 FastEthernet0/0 200.100.50.2

end
write memory

====================================================================
EOF
