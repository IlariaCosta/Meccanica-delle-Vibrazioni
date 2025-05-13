#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 52, 50, 48, 46);

// Motori
const int enA = 10, in1 = 9, in2 = 8;
const int in3 = 7, in4 = 6, enB = 5;

// Pulsante fisico
const int buttonPin = 34;
bool lastButtonState = HIGH;

// Accelerometro
const int MPU_I2C_ADDR = 0x68;
int16_t AcX, AcY, AcZ;

// Stato
bool isMoving = false;
bool lastButtonPressed = false;

// Comunicazione Bluetooth (Serial1)
#define BTSerial Serial1

void setup() {
  // Setup pin motori
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);  // Pulsante con pull-up

  // Comunicazioni seriali
  Serial.begin(9600);
  BTSerial.begin(9600);

  // LCD
  lcd.begin(16, 2);
  lcd.print("Connessione...");

  // Inizializza MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x6B);  // Wake-up
  Wire.write(0);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x08);  // Range ±2g
  Wire.write(0x00);
  Wire.endTransmission(true);

  lcd.setCursor(0, 1);
  lcd.print("Pronto         ");
}

void loop() {
  // Legge dati accelerometro e invia via Bluetooth
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x3B);  // Registro ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_I2C_ADDR, 6, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  String out = String(AcZ);
  //BTSerial.println(out);
  //Serial.println(out);  // Commenta questa riga se non serve il monitor seriale

  lcd.setCursor(0, 0);
  lcd.print(out);
  lcd.print("     ");  // Spazi per pulire residui

  // Legge lo stato del pulsante
  if (digitalRead(buttonPin) == LOW && !isMoving) {
    
    isMoving = true;
    lcd.setCursor(0, 1);
    lcd.print("Avvio         ");

    // Accelerazione graduale (0 → 255)
    
    for (int speed = 0; speed <= 255; speed += 1) {
      leggi();
      analogWrite(enA, speed);
      analogWrite(enB, speed);
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
      delay(0);
    }

    // Movimento a velocità costante
    for (int i = 0; i < 300; i++) {
      leggi();
      delay(0);
    }

    // Decelerazione graduale (255 → 0)
    for (int speed = 255; speed >= 0; speed -= 1) {
      leggi();
      analogWrite(enA, speed);
      analogWrite(enB, speed);
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
      delay(0);
    }

    // Ferma motori
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    digitalWrite(in3, LOW); digitalWrite(in4, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Stop          ");
    isMoving = false;
  }

  // Legge input Bluetooth (se presente)
  if (BTSerial.available()) {
    char received = BTSerial.read(); // Legge solo un carattere
    if (!isMoving) { 
      isMoving = true;
      lcd.setCursor(0, 1);
      lcd.print("Avvio          ");
    } else if (isMoving) { 
      isMoving = false;
      lcd.setCursor(0, 1);
      lcd.print("Decelera       ");
    }
  }

  // Gestisce il movimento dei motori
  static int speed = 0;
  const int step = 15;
  const int maxSpeed = 255;

  if (isMoving && speed < maxSpeed) {
    speed += step;  // Accelerazione
    if (speed > maxSpeed) speed = maxSpeed;
  } else if (!isMoving && speed > 0) {
    speed -= step;  // Decelerazione
    if (speed < 0) speed = 0;
  }

  analogWrite(enA, speed);
  analogWrite(enB, speed);

  if (speed > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  }

  
}

void leggi() {
  // Legge dati accelerometro e invia via Bluetooth
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x3B);  // Registro ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_I2C_ADDR, 6, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  String out = String(AcZ);
  BTSerial.println(out);
  //Serial.println(out);  // Commenta questa riga se non serve il monitor seriale

}
