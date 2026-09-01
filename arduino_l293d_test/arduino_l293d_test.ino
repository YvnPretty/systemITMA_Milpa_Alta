/*
 * Arduino Uno + L293D Motor Shield (Adafruit v1)
 * Configuración Exacta del Usuario:
 *  - Motor Enfrente (Dirección): Bornera M2
 *  - Motor Atrás (Tracción): Bornera M4
 */

#include <AFMotor.h>

AF_DCMotor motorDireccion(2); // M2: Motor de Enfrente (Dirección)
AF_DCMotor motorTraccion(4);  // M4: Motor de Atrás (Tracción)

int velocidadTraccion = 230;  // 0-255
int velocidadDireccion = 255; // 0-255

void detenerTodo() {
  motorDireccion.run(RELEASE);
  motorTraccion.run(RELEASE);
}

void avanzar() {
  motorTraccion.setSpeed(velocidadTraccion);
  motorTraccion.run(FORWARD);
}

void retroceder() {
  motorTraccion.setSpeed(velocidadTraccion);
  motorTraccion.run(BACKWARD);
}

void pararTraccion() {
  motorTraccion.run(RELEASE);
}

void girarIzquierda() {
  motorDireccion.setSpeed(velocidadDireccion);
  motorDireccion.run(FORWARD);
}

void girarDerecha() {
  motorDireccion.setSpeed(velocidadDireccion);
  motorDireccion.run(BACKWARD);
}

void centrarDireccion() {
  motorDireccion.run(RELEASE);
}

void setup() {
  Serial.begin(9600);
  detenerTodo();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      // --- TRACCIÓN (BORNERA M4) ---
      case 'w': case 'W': avanzar(); break;
      case 's': case 'S': retroceder(); break;
      case 'k': case 'K': pararTraccion(); break;

      // --- DIRECCIÓN (BORNERA M2) ---
      case 'a': case 'A': girarIzquierda(); break;
      case 'd': case 'D': girarDerecha(); break;
      case 'r': case 'R': centrarDireccion(); break;

      // --- MOVIMIENTOS COMBINADOS DIAGONALES ---
      case 'q': case 'Q': avanzar(); girarIzquierda(); break;
      case 'e': case 'E': avanzar(); girarDerecha(); break;
      case 'z': case 'Z': retroceder(); girarIzquierda(); break;
      case 'v': case 'V': retroceder(); girarDerecha(); break;

      // --- FRENADO TOTAL ---
      case ' ': case 'x': case 'X': detenerTodo(); break;

      // --- VELOCIDAD ---
      case '+':
        if (velocidadTraccion <= 240) velocidadTraccion += 15;
        motorTraccion.setSpeed(velocidadTraccion);
        break;
      case '-':
        if (velocidadTraccion >= 115) velocidadTraccion -= 15;
        motorTraccion.setSpeed(velocidadTraccion);
        break;
    }
  }
}
