#!/usr/bin/env python3
"""
ESP8266 OLED Messenger CLI Client
Script para enviar mensajes desde la terminal PC al ESP8266 vía HTTP REST API.
"""

import sys
import argparse
import urllib.request
import urllib.parse
import json

ESP_IP_DEFAULT = "192.168.4.1"

def send_message(esp_ip, sender, message):
    url = f"http://{esp_ip}/send"
    data = urllib.parse.urlencode({
        'sender': sender,
        'msg': message
    }).encode('utf-8')
    
    req = urllib.request.Request(url, data=data, method='POST')
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            res_data = response.read().decode('utf-8')
            res_json = json.loads(res_data)
            print(f"✅ Mensaje enviado exitosamente!")
            print(f"   Remitente: {res_json.get('sender')}")
            print(f"   Mensaje:   {res_json.get('message')}")
            print(f"   Conteo #:  {res_json.get('count')}")
    except Exception as e:
        print(f"❌ Error al conectar con el ESP8266 ({esp_ip}): {e}")

def clear_display(esp_ip):
    url = f"http://{esp_ip}/api/clear"
    req = urllib.request.Request(url, data=b"", method='POST')
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            print(f"🧹 Pantalla limpiada exitosamente.")
    except Exception as e:
        print(f"❌ Error: {e}")

def get_status(esp_ip):
    url = f"http://{esp_ip}/api/status"
    try:
        with urllib.request.urlopen(url, timeout=5) as response:
            res_json = json.loads(response.read().decode('utf-8'))
            print("📊 ESTADO DEL ESP8266 OLED MESSENGER:")
            print(f"   Estado:      {res_json.get('status')}")
            print(f"   IP:          {res_json.get('ip')}")
            print(f"   Total Msgs:  {res_json.get('count')}")
            print(f"   Último De:   {res_json.get('sender')}")
            print(f"   Último Msg:  {res_json.get('message')}")
            print(f"   Heap Libre:  {res_json.get('free_heap')} bytes")
    except Exception as e:
        print(f"❌ Error al consultar estado: {e}")

def main():
    parser = argparse.ArgumentParser(description="Envía mensajes a pantalla ESP8266 OLED")
    parser.add_argument("--ip", default=ESP_IP_DEFAULT, help=f"IP del ESP8266 (por defecto: {ESP_IP_DEFAULT})")
    parser.add_argument("--sender", "-s", default="PC-Terminal", help="Nombre del remitente")
    parser.add_argument("--msg", "-m", help="Texto del mensaje a enviar")
    parser.add_argument("--clear", action="store_true", help="Limpiar la pantalla OLED")
    parser.add_argument("--status", action="store_true", help="Obtener telemetría del ESP8266")

    args = parser.parse_args()

    if args.clear:
        clear_display(args.ip)
    elif args.status:
        get_status(args.ip)
    elif args.msg:
        send_message(args.ip, args.sender, args.msg)
    else:
        print("⌨️  Modo Interactivo ESP8266 Messenger Client")
        sender = input(f"Ingresa tu nombre [PC-Terminal]: ").strip() or "PC-Terminal"
        while True:
            try:
                msg = input("Mensaje (o 'exit' para salir): ").strip()
                if not msg or msg.lower() == 'exit':
                    break
                if msg.lower() == 'clear':
                    clear_display(args.ip)
                else:
                    send_message(args.ip, sender, msg)
            except KeyboardInterrupt:
                print("\nSaliendo...")
                break

if __name__ == "__main__":
    main()
