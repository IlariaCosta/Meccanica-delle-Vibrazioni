#include <MPU6050.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <MPU6050_6Axis_MotionApps612.h>
#include <MPU6050_9Axis_MotionApps41.h>
#include <helper_3dmath.h>

//#include <AFMotor.h> //permette di controllare la scia dei motori
#include <SoftwareSerial.h> //controllo tramite bluetooth

//motore a
int enA = 10;
int in1 = 9;
int in2 = 8;
//motore b
int in3 = 7;
int in4 = 6;
int enB = 5;
char val;

int txPin = 3;
int rxPin = 2;
SoftwareSerial bluetooth(rxPin, txPin); //primo rx secondo tx

void setup() {
  //imposta i motori come output
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  analogWrite(enA, 255);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  Serial.begin(9600); //imposta la velocita dei dati in bit al secondo per la trasmissione dati seriale
  bluetooth.begin(9600);
}

void loop() {
  if (bluetooth.available()>0){ //i dati vengono inviati all'arduino
    val = bluetooth.read(); //val e' uguale al valore letto
    Serial.println(val); //stampa valori sul terminale di arduino (in alto a destra)
  }

  analogWrite(enA, 255); //imposto velocita motoreA a 255(max)
  analogWrite(enB, 255); //imposto velocita motoreB a 255(max)
  

  if( val == 'F') // Forward
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW); 
  }

  else if(val == 'B') // Back
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }

  else if(val == 'L') //Left
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }

  else if(val == 'R') //Right
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW); 
  }

  else if(val == 'S') //Stop
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW); 
  }

  else if(val == 'I') //Forward Right
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }
  else if(val == 'J') //Backward Right
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }
  else if(val == 'G') //Forward Left
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);     
    digitalWrite(in4, LOW);
  }
  else if(val == 'H') //Backward Left
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH); 
  }

}
