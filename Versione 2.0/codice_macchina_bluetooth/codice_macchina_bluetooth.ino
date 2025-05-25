#include <Wire.h>                 // Libreria per comunicazione I2C (necessaria per MPU6050)
#include <LiquidCrystal.h>       // Libreria per controllare il display LCD

// Inizializzazione LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 52, 50, 48, 46);

// Pin del pulsante per avvio manuale
const int buttonPin = 34;


// Indirizzo I2C del sensore MPU6050
const int MPU_I2C_ADDR = 0x68;
int16_t AcX, AcY, AcZ;                   // Variabili per accelerazioni grezze
int16_t ax_offset = 0;                  // Offset asse X
int16_t ay_offset = 0;                  // Offset asse Y
int16_t az_offset = 0;                  // Offset asse Z

// Timing lettura accelerometro
unsigned long lastReadTime = 0;
const unsigned long interval = 2;       // Intervallo di 2ms = 500 Hz

// Parametri per media mobile sull'asse Z
#define WINDOW_SIZE 10
float AcZ_window[WINDOW_SIZE] = {0};    // Finestra dati asse Z
int window_index = 0;                   // Indice per aggiornare finestra


// Pin motori (due motori, con controllo direzione e velocità PWM)
const int enA = 10, in1 = 9, in2 = 8;    // Motore A
const int in3 = 7, in4 = 6, enB = 5;     // Motore B

// Stato dei motori
bool isMoving = false;

// Timer per avviare i motori
unsigned long motorStartTime = 0;
bool buttonPressed = false;

// Parametri decelerazione
const unsigned long decelStartDelay = 5000;   // Aspetta 5s prima di decelerare
const unsigned long decelDuration = 5000;     // Decelerazione dura 5s
int pwmValue = 255;                           // PWM massimo (velocità massima)
bool isDecelerating = false;
unsigned long decelStartTime = 0;


// Comunicazione Bluetooth (Serial1)
#define BTSerial Serial1

void setup() {
  // Imposta i pin dei motori come output
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);  // Pulsante con pull-up interno

  // Inizializza comunicazione seriale e Bluetooth
  Serial.begin(9600);
  BTSerial.begin(9600);

  // Inizializza LCD
  lcd.begin(16, 2);
  lcd.print("Connessione...");

  // Inizializza MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x6B);       // Registro di "power management"
  Wire.write(0);          // Wake-up MPU6050
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x1C);       // Registro di configurazione accelerometro
  Wire.write(0x08);       // ±4g (sensibilità intermedia)
  Wire.endTransmission(true);

  // Display pronto
  lcd.setCursor(0, 1);
  lcd.print("Pronto         ");
}

void loop() {
  unsigned long currentTime = millis();

  // Legge accelerometro ogni 2ms
  if (currentTime - lastReadTime >= interval) {
    lastReadTime = currentTime;
    readAccel();                     // Acquisisce nuovo valore da MPU
    float filteredZ = filteredAcZ(); // Calcola media mobile

    // Invia valore filtrato via Bluetooth e mostra su LCD
    String out = String(filteredZ, 3);   // 3 cifre decimali
    BTSerial.println(out);              // Invia via Bluetooth
    lcd.setCursor(0, 0);
    lcd.print("Z: ");
    lcd.print(out);
    lcd.print(" g   ");                 // Mostra su display
  }

  // Gestione del pulsante per avvio singolo
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && !buttonPressed) {
    buttonPressed = true;             // Evita rimbalzi

    if (!isMoving) {
      isMoving = true;
      motorStartTime = millis();      // Salva tempo di avvio
      pwmValue = 255;                 // Imposta PWM iniziale massimo
      isDecelerating = false;
      startMotors();                  // Avvia i motori
      lcd.setCursor(0, 1);
      lcd.print("Motori ON      ");
    }
  }
  if (currentButtonState == HIGH) {
    buttonPressed = false;           // Permette nuovi clic
  }

  // Gestione logica di decelerazione automatica
  if (isMoving) {
    unsigned long elapsed = millis() - motorStartTime;

    // Inizia decelerazione dopo delay
    if (!isDecelerating && elapsed >= decelStartDelay) {
      isDecelerating = true;
      decelStartTime = millis();
    }

    // Fase di decelerazione
    if (isDecelerating) {
      unsigned long decelElapsed = millis() - decelStartTime;
      if (decelElapsed <= decelDuration) {
        // Diminuzione lineare del PWM
        pwmValue = 255 - (255 * decelElapsed) / decelDuration;
        pwmValue = constrain(pwmValue, 0, 255);
        analogWrite(enA, pwmValue);
        analogWrite(enB, pwmValue);
      } else {
        // Fine decelerazione: spegne i motori
        stopMotors();
        isMoving = false;
        isDecelerating = false;
        lcd.setCursor(0, 1);
        lcd.print("Motori OFF     ");
      }
    }
  }
}

// ===== FUNZIONI =====

// Legge dati grezzi dall'accelerometro MPU6050
void readAccel() {
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(0x3B); // Indirizzo ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_I2C_ADDR, 6, true); // Legge X, Y, Z (2 byte ciascuno)

  AcX = (Wire.read() << 8 | Wire.read()) + ax_offset;
  AcY = (Wire.read() << 8 | Wire.read()) + ay_offset;
  AcZ = (Wire.read() << 8 | Wire.read()) + az_offset;

  float AcZ_g = (float)AcZ / 8192.0; // Conversione in g (±4g → 8192 LSB/g)

  // Inserisce il nuovo valore nella finestra di media mobile
  AcZ_window[window_index] = AcZ_g;
  window_index = (window_index + 1) % WINDOW_SIZE;
}

// Calcola la media mobile dei valori Z per ridurre il rumore
float filteredAcZ() {
  float sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += AcZ_window[i];
  }
  return sum / WINDOW_SIZE;
}

// Attiva i motori in avanti con PWM corrente
void startMotors() {
  analogWrite(enA, pwmValue);
  analogWrite(enB, pwmValue);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); // direzione avanti A
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW); // direzione avanti B
}

// Ferma completamente i motori
void stopMotors() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}
