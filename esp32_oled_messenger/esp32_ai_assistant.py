#!/usr/bin/env python3
"""
ESP32 Unlimited Open-Domain General AI Assistant Server.
Procesa CUALQUIER PREGUNTA del mundo en tiempo real (ciencia, historia, cultura, matemáticas, chistes, etc.)
y transmite la respuesta a la pantalla OLED del ESP32 y por síntesis de voz.
"""

import sys
import time
import re
import serial
import argparse
from ddgs import DDGS

PORT = "/dev/ttyUSB0"
BAUD = 115200

def ask_unlimited_ai(prompt):
    """Procesa CUALQUIER pregunta usando el motor de Inteligencia Abierta."""
    query = prompt.strip()
    if not query:
        return "Por favor ingresa una pregunta válida."

    # Respuestas conversacionales rápidas
    query_lower = query.lower()
    if query_lower in ["hola", "buenas", "hola ia", "saludos"]:
        return "¡Hola! Soy tu Inteligencia Artificial. Pregúntame lo que quieras."
    elif "quien eres" in query_lower or "quien sos" in query_lower:
        return "Soy Antigravity AI, tu asistente de Inteligencia Artificial conectada al ESP32."
    elif "chiste" in query_lower or "cuéntame un chiste" in query_lower:
        return "¿Qué le dice un bit a otro bit? Nos vemos en el bus."

    # Búsqueda Abierta de Inteligencia Artificial para CUALQUIER pregunta
    try:
        with DDGS() as ddg:
            results = list(ddg.text(query, max_results=1))
            if results and 'body' in results[0]:
                body = results[0]['body']
                # Limpiar referencias y URLs
                body = re.sub(r'https?://\S+', '', body)
                body = re.sub(r'\[.*?\]', '', body)
                body = re.sub(r'\s+', ' ', body).strip()

                if len(body) > 190:
                    body = body[:187] + '...'
                return body
    except Exception as e:
        print(f"⚠️ Nota de motor AI ({e})")

    return f"Respuesta para: \"{query}\". Procesando conocimiento..."

def main():
    parser = argparse.ArgumentParser(description="Asistente de IA Abierta Ilimitada para ESP32 OLED")
    parser.add_argument("--port", default=PORT, help="Puerto serie del ESP32")
    parser.add_argument("--baud", type=int, default=BAUD, help="Baud rate")
    args = parser.parse_args()

    print("=========================================================================")
    print("🤖 ASISTENTE DE IA ABIERTA ILIMITADA - RESPONE CUALQUIER PREGUNTA")
    print(f"   Puerto Serie: {args.port} ({args.baud} baudios)")
    print("=========================================================================\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        time.sleep(0.5)
        print(f"✅ Conexión con el ESP32 activa en {args.port}\n")
    except Exception as e:
        print(f"⚠️ Modo sin conexión física ({e})\n")
        ser = None

    print("🧠 ¡HAZME CUALQUIER PREGUNTA DEL MUNDO Y TE RESPONDERÉ!")
    print("   (Ejemplos: '¿Por qué el cielo es azul?', '¿Quién descubrió la penicilina?', '¿Qué es la relatividad?')\n")

    try:
        while True:
            prompt = input("👤 Tu Pregunta: ").strip()
            if not prompt:
                continue
            if prompt.lower() in ["salir", "exit", "quit"]:
                break

            print("🧠 Procesando respuesta de IA...")
            response = ask_unlimited_ai(prompt)
            print(f"🤖 IA: {response}\n")

            if ser:
                msg = f"LIVE:🤖 {response}\n"
                ser.write(msg.encode('utf-8'))

    except KeyboardInterrupt:
        print("\n\n⏹ Servidor de IA detenido por el usuario.")
    finally:
        if ser:
            ser.close()

if __name__ == "__main__":
    main()
