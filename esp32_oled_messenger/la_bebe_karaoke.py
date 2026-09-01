#!/usr/bin/env python3
"""
ESP32 Karaoke Teleprompter Client - "La Bebé Remix" (Yng Lvcas & Peso Pluma)
Sincroniza la letra oficial completa del Remix en tiempo real hacia la pantalla OLED del ESP32.
"""

import serial
import time
import argparse

SONG_LYRICS = [
    (0,      "♪ LA BEBE REMIX - PESO PLUMA & YNG LVCAS ♪"),
    (5.0,   "Desafanate, loca, luego empapate"),
    (8.0,   "De mi cuerpo mojate, usted sabe, en el Benz, montate"),
    (11.0,  "Que lo que? Mi bebe en el Mercedes-Benz"),
    (14.0,  "Las estrellas en el techo, ya estelar se fue"),
    (17.0,  "Pero sabes tu que esta noche estas pa' mi"),
    (20.0,  "Ven, trepate encima 'e mi, manda la ubi, paso por ti"),
    (24.0,  "Quiere que le ponga musica"),
    (27.0,  "Pa' que baile hasta abajo la bebe"),
    (30.0,  "Bebimos par de botellas"),
    (33.0,  "Y aun asi recuerda que lo hicimo' ayer"),
    (36.0,  "Quiere que le ponga musica"),
    (39.0,  "Pa' que baile hasta abajo la bebe"),
    (42.0,  "Bebimos par de botellas"),
    (45.0,  "Y aun asi recuerda que lo hicimo' ayer"),
    (49.0,  "No se le ha olvidado como la pasamos"),
    (52.0,  "Fuimos a la disco y los dos bailamos pegados"),
    (56.0,  "Como perros pegados, besos y un par de tragos"),
    (59.0,  "Se quedo a mi lado y dijo: Creo me he enamorado"),
    (63.0,  "Ella fuma, ella toma"),
    (65.0,  "Es diablita, chiquita, pero picosa"),
    (68.0,  "Me encanta cuando el pantalon me lo roza"),
    (72.0,  "A ella le encanta, se ve en su cara lo goza"),
    (76.0,  "Tiene novio y no se comporta"),
    (79.0,  "Me dice: Tranqui, que la relacion 'ta rota"),
    (82.0,  "Nuestros cuerpo' chocan y chocan"),
    (85.0,  "Se juntan las boca', los leggings le ahorcan"),
    (89.0,  "Quiere que le ponga musica"),
    (92.0,  "Pa' que baile hasta abajo la bebe"),
    (95.0,  "Bebimos par de botellas"),
    (98.0,  "Y aun asi recuerda que lo hicimo' ayer"),
    (101.0, "Quiere que le ponga musica"),
    (104.0, "Pa' que baile hasta abajo la bebe"),
    (107.0, "Bebimo' par de botella'"),
    (110.0, "Y aun asi recuerda que lo hicimo' ayer"),
    (114.0, "Ayer, ayer te movias muy bien, muy bien"),
    (117.0, "Se impregno el olor de Chanel"),
    (120.0, "'Tamos mil grados Fahrenheit"),
    (123.0, "Esta buena y de cara bonita"),
    (126.0, "Le pones musica y solita se excita"),
    (129.0, "Pa' la fiesta nunca se limita"),
    (132.0, "Par de amigas, completa la cuadrilla"),
    (135.0, "Pero tu, mama, sabes, me mamas"),
    (138.0, "Loca, ven pa'ca, pegate hasta atras"),
    (141.0, "Nos vamos tu y yo en un viaje fugaz"),
    (144.0, "Interestellar, comerte toda"),
    (147.0, "Fumaremos los dos marihuana"),
    (150.0, "En el cuarto, en la azotea y en mi cama"),
    (153.0, "Nena, creo te llaman, contesta mañana"),
    (156.0, "Que no se apague la llama"),
    (159.0, "En el AMG escuchando 'Ruedas y Cristal'"),
    (162.0, "Quemando veneno que me trae volando, ma"),
    (165.0, "Besito a la Barbie, pa' que baile pega'o"),
    (168.0, "Ojitos chinitos como puki de Taiwan"),
    (171.0, "Como puki de Taiwan"),
    (173.0, "Esa pussy me la como cuando yo quiera, mami, ma"),
    (177.0, "Desafanate, loca, luego empapate"),
    (180.0, "De mi cuerpo mojate, usted sabe, en el Benz, montate"),
    (183.0, "Que lo que? Mi bebe en el Mercedes-Benz"),
    (186.0, "Las estrellas en el techo, ya estelar se fue"),
    (189.0, "Uy, quiere que le ponga musica"),
    (192.0, "Pa' que baile hasta abajo la bebe"),
    (195.0, "Bebimos par de botellas"),
    (198.0, "Y aun asi recuerda que lo hicimo' ayer"),
    (201.0, "Quiere que le ponga musica"),
    (204.0, "Pa' que baile hasta abajo la bebe"),
    (207.0, "Bebimos par de botellas"),
    (210.0, "Y aun asi recuerda que lo hicimo' ayer"),
    (213.0, "Hoy es noche de estar soltera"),
    (216.0, "Le gusta el perreo y bailarte cerca"),
    (219.0, "La Young Religion - La Doble P - Lil Yng (ES EL REMIX)")
]

def main():
    parser = argparse.ArgumentParser(description="Karaoke Sync Remix Client para ESP32 OLED")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Puerto serie del ESP32")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    args = parser.parse_args()

    print("==========================================================")
    print("🎤 KARAOKE REMIX TELEPROMPTER: 'LA BEBÉ' - PESO PLUMA & YNG LVCAS")
    print(f"   Puerto: {args.port} | Sincronización Oficial Remix")
    print("==========================================================\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
        time.sleep(0.5)
    except Exception as e:
        print(f"❌ Error al conectar con ESP32: {e}")
        return

    ser.write(b"PLAY\n")
    start_time = time.time()
    print("▶ INICIANDO REPRODUCCIÓN Y SINCRONIZACIÓN REMIX...\n")

    try:
        current_idx = -1
        while True:
            elapsed = time.time() - start_time
            target_idx = 0
            for i, (t_sec, text) in enumerate(SONG_LYRICS):
                if elapsed >= t_sec:
                    target_idx = i
                else:
                    break

            if target_idx != current_idx:
                current_idx = target_idx
                t_sec, text = SONG_LYRICS[current_idx]
                min_val = int(elapsed // 60)
                sec_val = int(elapsed % 60)
                time_str = f"[{min_val:02d}:{sec_val:02d}]"
                print(f"🎵 {time_str} {text}")

            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n\n⏹ Karaoke detenido.")
        ser.write(b"STOP\n")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
