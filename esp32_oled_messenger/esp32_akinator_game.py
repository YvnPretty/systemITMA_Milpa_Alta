#!/usr/bin/env python3
"""
ESP32 Akinator Genie Interactive Terminal Game.
El Genio Akinator te hace preguntas en la terminal para adivinar el personaje que piensas
y transmite cada pregunta y la adivinación final a la pantalla OLED del ESP32.
"""

import sys
import time
import serial
import argparse

PORT = "/dev/ttyUSB0"
BAUD = 115200

QUESTIONS = [
    "¿Tu personaje es de la vida REAL (humano/famoso)?",
    "¿Es un cantante o artista musical?",
    "¿Es un jugador de fútbol profesional?",
    "¿Es de un Anime, Manga o Caricatura?",
    "¿Tiene superpoderes o habilidades mágicas?",
    "¿Es de la serie Dragon Ball Z?",
    "¿Es un superhéroe de Marvel o DC Comics?"
]

GUESSES = {
    "si_si": "PESO PLUMA 🎤",
    "si_no_si": "LIONEL MESSI ⚽",
    "no_si_si_si": "SON GOKU (DRAGON BALL) 🥋",
    "no_si_si_no_si": "NARUTO UZUMAKI 🍥",
    "no_si_no_si": "SPIDER-MAN / HOMBRE ARAÑA 🕷️",
    "no_no": "ELON MUSK / ALBERT EINSTEIN 🧠",
    "default": "BOB ESPONJA 🧽 / PIKACHU ⚡"
}

def main():
    parser = argparse.ArgumentParser(description="Akinator Genie Game para ESP32 OLED")
    parser.add_argument("--port", default=PORT, help="Puerto serie del ESP32")
    parser.add_argument("--baud", type=int, default=BAUD, help="Baud rate")
    args = parser.parse_args()

    print("=========================================================================")
    print("🧞‍♂️ JUEGO DEL GENIO AKINATOR EN TIEMPO REAL - ESP32 OLED")
    print(f"   Puerto Serie: {args.port} ({args.baud} baudios)")
    print("=========================================================================\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        time.sleep(0.5)
        print(f"✅ Conectado al ESP32 en {args.port}\n")
    except Exception as e:
        print(f"⚠️ Modo sin conexión serie ({e})\n")
        ser = None

    print("🧞‍♂️ PIENSA EN UN PERSONAJE (Goku, Messi, Peso Pluma, Spider-Man, etc.)...")
    print("   ¡Responde a cada pregunta escribiendo: 1 (SÍ) o 2 (NO)!\n")

    answers = []
    for i, q in enumerate(QUESTIONS):
        print(f"🧞‍♂️ PREGUNTA {i+1}: {q}")
        if ser:
            msg = f"LIVE:🧞‍♂️ P{i+1}: {q}\n"
            ser.write(msg.encode('utf-8'))

        ans = input("   Respuesta (1=SÍ / 2=NO): ").strip().lower()
        if ans in ["1", "si", "s", "sí"]:
            answers.append("si")
        else:
            answers.append("no")
        print()

    key = "_".join(answers[:3])
    guessed = GUESSES.get(key, GUESSES["default"])

    print("=========================================================================")
    print(f"🧞‍♂️ ¡ESTÁS PENSANDO EN: {guessed}!")
    print("=========================================================================\n")

    if ser:
        msg = f"LIVE:🧞‍♂️ ¡ADIVINÉ! ES: {guessed}\n"
        ser.write(msg.encode('utf-8'))

    if ser:
        ser.close()

if __name__ == "__main__":
    main()
