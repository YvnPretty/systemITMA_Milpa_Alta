#!/usr/bin/env python3
"""
ESP32 Live MP3 Karaoke Synchronizer - "La Bebé (Remix)"
Sincronización Maestra Calibrada directamente desde el Canto del Usuario.
"""

import os
import sys
import time
import serial
import select
import subprocess
import argparse

MP3_PATH = "/home/pretty/Downloads/Yng Lvcas & Peso Pluma - La Bebe (Remix) [Video Oficial].mp3"

# Sincronización Maestra basada en la interpretación cantada del usuario
SONG_LYRICS = [
    (0.0,   "♪ LA BEBE REMIX - INTRO ♪"),
    (12.0,  "Desafanate, loca, luego empapate"),
    (14.8,  "De mi cuerpo mojate, usted sabe, en el Benz, montate"),
    (17.6,  "Klk, mi bebe en el Mercedes-Benz"),
    (20.4,  "Las estrellas en el techo, ya estelar se fue"),
    (23.2,  "Pero sabes tu que esta noche estas pa' mi"),
    (26.0,  "Ven, trepate encima 'e mi, manda la ubi, paso por ti"),
    (28.8,  "Quiere que le ponga musica pa' que baile hasta abajo la bebe"),
    (33.6,  "Bebimos par de botellas y aun asi recuerda que lo hicimo' ayer"),
    (38.4,  "Quiere que le ponga musica pa' que baile hasta abajo la bebe"),
    (43.2,  "Bebimos par de botellas y aun asi recuerda que lo hicimo' ayer"),
    (50.5,  "No se le ha olvidado como la pasamos"),
    (53.8,  "Fuimos a la disco y los dos bailamos pegados"),
    (57.1,  "Como perros pegados, besos y un par de tragos"),
    (60.4,  "Se quedo a mi lado y dijo: Creo me he enamorado"),
    (63.7,  "Ella fuma, ella toma"),
    (65.5,  "Es diablita, chiquita, pero picosa"),
    (68.8,  "Me encanta cuando el pantalon me lo roza"),
    (72.1,  "A ella le encanta, se ve en su cara lo goza"),
    (75.4,  "Tiene novio y no se comporta"),
    (78.2,  "Me dice: Tranqui, que la relacion 'ta rota"),
    (81.0,  "Nuestros cuerpo' chocan y chocan"),
    (83.8,  "Se juntan las boca', los leggings le ahorcan"),
    (86.6,  "Quiere que le ponga musica pa' que baile hasta abajo la bebe"),
    (91.4,  "Bebimos par de botellas y aun asi recuerda que lo hicimo' ayer"),
    (96.2,  "Quiere que le ponga musica pa' que baile hasta abajo la bebe"),
    (101.0, "Bebimo' par de botella' y aun asi recuerda que lo hicimo' ayer"),
    (105.8, "Ayer, ayer te movias muy bien, muy bien"),
    (109.0, "Se impregno el olor de Chanel"),
    (112.0, "'Tamos mil grados Fahrenheit"),
    (115.0, "Esta buena y de cara bonita"),
    (118.0, "Le pones musica y solita se excita"),
    (121.0, "Pa' la fiesta nunca se limita"),
    (124.0, "Par de amigas, completa la cuadrilla"),
    (127.0, "Pero tu, mama, sabes, me mamas"),
    (130.0, "Loca, ven pa'ca, pegate hasta atras"),
    (133.0, "Nos vamos tu y yo en un viaje fugaz"),
    (136.0, "Interestellar, comerte toda"),
    (139.0, "Fumaremos los dos marihuana"),
    (142.0, "En el cuarto, en la azotea y en mi cama"),
    (145.0, "Nena, creo te llaman, contesta mañana"),
    (148.0, "Que no se apague la llama"),
    (151.0, "En el AMG escuchando 'Ruedas y Cristal'"),
    (154.0, "Quemando veneno que me trae volando, ma"),
    (157.0, "Besito a la Barbie, pa' que baile pega'o"),
    (160.0, "Ojitos chinitos como puki de Taiwan"),
    (163.0, "Como puki de Taiwan"),
    (165.5, "Esa pussy me la como cuando yo quiera, mami, ma"),
    (168.5, "Desafanate, loca, luego empapate"),
    (171.3, "De mi cuerpo mojate, usted sabe, en el Benz, montate"),
    (174.1, "Klk, mi bebe en el Mercedes-Benz"),
    (176.9, "Las estrellas en el techo, ya estelar se fue"),
    (179.7, "Uy, quiere que le ponga musica pa' que baile hasta abajo la bebe"),
    (184.5, "Bebimos par de botellas y aun asi recuerda que lo hicimo' ayer"),
    (189.3, "Quiere que le ponga musica pa' que baile hasta abajo la bebe"),
    (194.1, "Bebimos par de botellas y aun asi recuerda que lo hicimo' ayer"),
    (198.9, "Hoy es noche de estar soltera"),
    (201.7, "Le gusta el perreo y bailarte cerca"),
    (204.5, "La Young Religion - La Doble P - Lil Yng (ES EL REMIX)")
]

def find_audio_player():
    for cmd in ["ffplay", "mpv", "vlc", "mpg123"]:
        try:
            subprocess.run([cmd, "--help"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return cmd
        except Exception:
            continue
    return None

def main():
    parser = argparse.ArgumentParser(description="Reproductor MP3 Karaoke Master Audio Sync")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Puerto serie del ESP32")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--offset", type=float, default=0.0, help="Desfase manual inicial en segundos")
    args = parser.parse_args()

    if not os.path.exists(MP3_PATH):
        print(f"❌ No se encontró el archivo MP3 en: {MP3_PATH}")
        return

    player_cmd = find_audio_player()
    if not player_cmd:
        print("❌ No se encontró un reproductor de audio.")
        return

    print("=========================================================================")
    print("🎵 KARAOKE MP3 (MASTER SYNC CALIBRADO DIRECTAMENTE DEL CANTO DEL USUARIO)")
    print(f"   Archivo MP3: {os.path.basename(MP3_PATH)}")
    print("   [TECLAS DE CALIBRACIÓN EN VIVO]:")
    print("      Presiona [+] para ADELANTAR la letra 0.2s")
    print("      Presiona [-] para RETRASAR la letra 0.2s")
    print("=========================================================================\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
        time.sleep(0.5)
    except Exception as e:
        print(f"⚠️ Advertencia: No se pudo conectar a {args.port}: {e}")
        ser = None

    if player_cmd == "ffplay":
        audio_proc = subprocess.Popen(["ffplay", "-nodisp", "-autoexit", MP3_PATH], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elif player_cmd == "mpv":
        audio_proc = subprocess.Popen(["mpv", "--no-video", MP3_PATH], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elif player_cmd == "vlc":
        audio_proc = subprocess.Popen(["vlc", "--intf", "dummy", "--play-and-exit", MP3_PATH], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    else:
        audio_proc = subprocess.Popen([player_cmd, MP3_PATH], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    start_time = time.time()
    user_offset = args.offset

    if ser:
        ser.write(b"PLAY\n")

    print("▶ REPRODUCIENDO CON LA SINCRONIZACIÓN MAESTRA DE TU AUDIO...")

    try:
        current_idx = -1
        while audio_proc.poll() is None:
            if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
                line = sys.stdin.readline().strip()
                if "+" in line:
                    user_offset += 0.2
                    print(f"⏩ [CALIBRACIÓN] Adelantado +0.2s (Offset: {user_offset:+.1f}s)")
                elif "-" in line:
                    user_offset -= 0.2
                    print(f"⏪ [CALIBRACIÓN] Retrasado -0.2s (Offset: {user_offset:+.1f}s)")

            elapsed = (time.time() - start_time) + user_offset
            target_idx = 0
            for i, (t_sec, text) in enumerate(SONG_LYRICS):
                if elapsed >= t_sec:
                    target_idx = i
                else:
                    break

            if target_idx != current_idx:
                current_idx = target_idx
                t_sec, text = SONG_LYRICS[current_idx]
                display_sec = max(0, elapsed)
                min_val = int(display_sec // 60)
                sec_val = int(display_sec % 60)
                time_str = f"[{min_val:02d}:{sec_val:02d}]"
                print(f"🎤 {time_str} {text}")

                if ser:
                    msg = f"LIVE:{text}\n"
                    ser.write(msg.encode('utf-8'))

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n\n⏹ Karaoke detenido por el usuario.")
        audio_proc.terminate()
        if ser:
            ser.write(b"STOP\n")
    finally:
        if audio_proc.poll() is None:
            audio_proc.terminate()
        if ser:
            ser.close()

if __name__ == "__main__":
    main()
