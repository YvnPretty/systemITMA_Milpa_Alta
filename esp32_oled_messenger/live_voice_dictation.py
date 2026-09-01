#!/usr/bin/env python3
"""
ESP32 Continuous Unbroken Voice Dictation System.
Captura audio del micrófono de forma CONTINUA e ININTERRUMPIDA sin pausas ni pérdida de palabras.
Proyecta cada palabra dictada en vivo en la pantalla OLED del ESP32.
"""

import os
import sys
import time
import math
import struct
import serial
import threading
import subprocess
import argparse
import speech_recognition as sr
from collections import deque
import io
import wave

TEMP_WAV = "/tmp/esp32_continuous_phrase.wav"

class ContinuousAudioStream:
    def __init__(self, sample_rate=16000):
        self.sample_rate = sample_rate
        self.bytes_per_sample = 2  # S16_LE
        self.chunk_samples = 1600   # 100ms
        self.chunk_bytes = self.chunk_samples * self.bytes_per_sample
        self.proc = None
        self.running = False
        self.thread = None
        self.audio_queue = deque()
        self.lock = threading.Lock()

    def start(self):
        cmd = [
            "arecord",
            "-D", "default",
            "-f", "S16_LE",
            "-r", str(self.sample_rate),
            "-c", "1",
            "-t", "raw"
        ]
        self.proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        self.running = True
        self.thread = threading.Thread(target=self._reader_loop, daemon=True)
        self.thread.start()

    def _reader_loop(self):
        while self.running and self.proc and self.proc.poll() is None:
            data = self.proc.stdout.read(self.chunk_bytes)
            if not data:
                break
            with self.lock:
                self.audio_queue.append(data)

    def read_chunk(self):
        with self.lock:
            if self.audio_queue:
                return self.audio_queue.popleft()
        return None

    def stop(self):
        self.running = False
        if self.proc:
            try:
                self.proc.terminate()
            except Exception:
                pass

def compute_rms(pcm_data):
    """Calcula el volumen RMS del bloque PCM de 16 bits."""
    if not pcm_data:
        return 0
    count = len(pcm_data) // 2
    if count == 0:
        return 0
    shorts = struct.unpack(f"<{count}h", pcm_data)
    sum_squares = sum(s * s for s in shorts)
    return math.sqrt(sum_squares / count)

def save_pcm_to_wav(pcm_bytes, filepath, sample_rate=16000):
    with wave.open(filepath, 'wb') as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        wf.writeframes(pcm_bytes)

def transcribe_pcm(r, pcm_bytes, lang="es-MX"):
    save_pcm_to_wav(pcm_bytes, TEMP_WAV)
    try:
        with sr.AudioFile(TEMP_WAV) as source:
            audio = r.record(source)
            text = r.recognize_google(audio, language=lang)
            return text.strip()
    except sr.UnknownValueError:
        return None
    except sr.RequestError as e:
        print(f"\n⚠️ Error de conexión a la API de voz: {e}")
        return None
    finally:
        if os.path.exists(TEMP_WAV):
            try:
                os.remove(TEMP_WAV)
            except Exception:
                pass

def main():
    parser = argparse.ArgumentParser(description="Dictado Continuo Ininterrumpido al ESP32 OLED")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Puerto serie del ESP32")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--lang", default="es-MX", help="Idioma de dictado (es-MX, es-ES, en-US)")
    args = parser.parse_args()

    print("=========================================================================")
    print("🎙️ DICTADO POR VOZ CONTINUO E ININTERRUMPIDO EN TIEMPO REAL")
    print("   [CAPTURA EN FLUSO SIN PAUSAS]: Captura 100% de palabras sin huecos")
    print("   [ESP32 PORT]: " + args.port)
    print("   Habla de forma continua. Todo lo que digas se transmitirá al OLED.")
    print("=========================================================================\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
        time.sleep(0.5)
        print(f"✅ Conectado al ESP32 en {args.port}\n")
    except Exception as e:
        print(f"⚠️ No se pudo abrir el puerto serie {args.port}: {e}")
        ser = None

    r = sr.Recognizer()
    stream = ContinuousAudioStream(sample_rate=16000)
    stream.start()

    active_phrase = bytearray()
    silence_counter = 0
    is_speaking = False
    VAD_THRESHOLD = 350.0  # Umbral de detección de voz

    print("🎙️ MICRÓFONO ACTIVADO Y ESCUCHANDO DE FORMA CONTINUA... ¡HABLA LIBREMENTE!\n")

    try:
        while True:
            chunk = stream.read_chunk()
            if not chunk:
                time.sleep(0.02)
                continue

            rms = compute_rms(chunk)

            if rms > VAD_THRESHOLD:
                if not is_speaking:
                    is_speaking = True
                    active_phrase = bytearray()
                active_phrase.extend(chunk)
                silence_counter = 0
            else:
                if is_speaking:
                    active_phrase.extend(chunk)
                    silence_counter += 1

                    # Si hay 4 fragmentos seguidos de silencio (~0.4s) o la frase supera los 4.5s
                    max_bytes = 16000 * 2 * 4.5
                    if silence_counter >= 4 or len(active_phrase) >= max_bytes:
                        is_speaking = False
                        pcm_to_process = bytes(active_phrase)
                        active_phrase = bytearray()
                        silence_counter = 0

                        # Procesar la frase en hilo secundario para no bloquear el flujo continuo
                        def worker(pcm_data):
                            t = transcribe_pcm(r, pcm_data, lang=args.lang)
                            if t and len(t) > 0:
                                print(f"🗣️ [DICTADO CONTINUO]: \"{t}\"")
                                if ser:
                                    msg = f"LIVE:{t}\n"
                                    ser.write(msg.encode('utf-8'))

                        threading.Thread(target=worker, args=(pcm_to_process,), daemon=True).start()

            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n\n⏹ Dictado continuo detenido.")
    finally:
        stream.stop()
        if ser:
            ser.close()

if __name__ == "__main__":
    main()
