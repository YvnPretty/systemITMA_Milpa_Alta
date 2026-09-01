# 🛡️ Guía de Seguridad y Cortafuegos para Tu Red Doméstica (Wi-Fi de Casa)

Esta guía te explica paso a paso cómo asegurar completamente tu **red doméstica** (`192.168.1.0/24`), tu **módem/router de casa** (ubicado en `http://192.168.1.254`) y tu **computadora Linux**.

---

## 💻 1. Cortafuegos en Tu Computadora (Linux)

Hemos creado el script [`domestic_firewall_setup.sh`](file:///home/pretty/tsert/domestic_firewall_setup.sh) que autodetecta tu interfaz Wi-Fi (`wlp0s20f3`) y configura el firewall local.

### ¿Qué hace este firewall?
* **Deniega por defecto** todo intento de conexión entrante no solicitada desde el exterior.
* **Permite el tráfico de confianza** entre tus dispositivos locales (celulares, ESP32, Smart TVs en `192.168.1.x`).
* **Bloquea puertos vulnerables** (NetBIOS, SMB, RPC) que suelen ser blanco de malware en redes públicas/domésticas.
* **Protección Anti-Fuerza Bruta SSH** y descarte de pings masivos (ICMP floods).

### Cómo ejecutarlo en tu terminal:
```bash
sudo bash /home/pretty/tsert/domestic_firewall_setup.sh
```

---

## 📶 2. Configurar el Cortafuegos y Seguridad en Tu Router / Módem de Casa

Para que toda tu casa esté protegida (no solo tu PC, sino también celulares y dispositivos IoT), ingresa a la interfaz web de tu módem/router:

1. Abre tu navegador e ingresa a: **`http://192.168.1.254`** (o `http://192.168.1.1`).
2. Introduce el usuario y contraseña de administración (suelen estar en la etiqueta posterior del router).

### Configuración Recomendada en el Router:

#### A. Habilitar el Firewall Integrado (SPI Firewall)
* Ve a la sección **Security / Firewall / Seguridad**.
* Marca la casilla **Enable SPI Firewall** (Stateful Packet Inspection) o selecciona nivel de seguridad **Medio / Alto**.
* Activa la opción **Block Anonymous WAN Requests / Filter Anonymous Internet Requests** (esto evita que atacantes en Internet escaneen la IP de tu casa).

#### B. Desactivar Funciones Vulnerables
* **Desactivar UPnP (Universal Plug and Play):** UPnP permite que malware o dispositivos infectados abran puertos en tu router automáticamente hacia Internet.
  * Ve a *Advanced > Network > UPnP* o *Forwarding > UPnP* y selecciona **Disable**.
* **Desactivar WPS (Wi-Fi Protected Setup):** El PIN de WPS es vulnerable a ataques de fuerza bruta.
  * Ve a *Wireless > WPS* y selecciona **Disable**.

#### C. Securizar la Red Wi-Fi
* **Encriptación:** Cambia la seguridad Wi-Fi a **WPA2-PSK (AES)** o **WPA3-SAE**. (Evita WPA/TKIP o WEP).
* **Cambiar Contraseña por Defecto del Router:** Cambia la clave de acceso al panel web del router para que nadie conectado a la Wi-Fi pueda modificar la configuración.

#### D. Crear una Red de Invitados / IoT (Opcional pero recomendado)
* Si tienes muchos dispositivos inteligentes o invitaciones frecuentes, activa la **Guest Network / Red de Invitados** e aísla ese tráfico de tus computadoras principales.

---

## 🔍 3. Comprobar que el Firewall Funciona

Para verificar que tu cortafuegos local está activo y filtrando paquetes:

```bash
sudo ufw status verbose
```

O si deseas monitorear las conexiones activas en tu red doméstica:
```bash
ss -tulpn
```
