#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[ESP8266 I2C Scanner]");

  // Pines I2C estándar para NodeMCU / WeMos D1 Mini: SDA = D2 (GPIO 4), SCL = D1 (GPIO 5)
  Wire.begin(4, 5);

  Serial.println("Escaneando bus I2C (SDA=GPIO4, SCL=GPIO5)...");
  int nDevices = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Dispositivo I2C encontrado en 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("No se encontraron dispositivos I2C en SDA=GPIO4, SCL=GPIO5.");
  } else {
    Serial.println("Escaneo completado.");
  }
}

void loop() {
  delay(5000);
}
