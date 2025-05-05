#include <Wire.h>

// Usando la UART1 hardware di Arduino Mega (Pin 18 - TX1, Pin 19 - RX1)
#define BTSerial Serial1  // HC-05 connesso su Serial1 (pin 18 - TX1, pin 19 - RX1)

const int MPU_I2C_ADDR = 0x68;
int16_t AcX, AcY, AcZ;

void setup() {
  Serial.begin(9600);       // Monitor USB per debug
  BTSerial.begin(9600);     // Comunicazione con HC-05 tramite Serial1 (pin 18 - TX1, pin 19 - RX1)
  Wire.begin();             // Avvia la comunicazione I2C per MPU-6050
  
  // Inizializza MPU-6050 (wake-up)
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x6B);  // Registro power management
  Wire.write(0);     // Wake-up
  Wire.endTransmission(true);

  // Impostazione accelerometro su ±4g
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x1C);  // Registro ACCEL_CONFIG
  Wire.write(0x08);  // ±4g
  Wire.endTransmission(true);
  
  Serial.println("Pronto: invio dati via BTSerial");
}

void loop() {
  // Leggi i dati dell'accelerometro
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x3B);  // Inizia lettura da ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_I2C_ADDR, 6, true);  // Richiedi 6 byte di dati

  // Calcola i valori dei tre assi
  AcX = Wire.read() << 8 | Wire.read(); // Accelerazione su asse X
  AcY = Wire.read() << 8 | Wire.read(); // Accelerazione su asse Y
  AcZ = Wire.read() << 8 | Wire.read(); // Accelerazione su asse Z

  // Crea la stringa di output con i dati letti
  String out = /*"X=" + String(AcX) + "  Y=" + String(AcY) + "  Z=" + */String(AcZ);

  // Invia i dati sia al Monitor Serial (USB) che via Bluetooth (HC-05)
  Serial.println(out);       // Visualizza sul Monitor USB
  BTSerial.println(out);     // Invia a Tera Term tramite Bluetooth

  delay(100); // Delay di 100ms (circa 10 Hz di aggiornamento)
}
