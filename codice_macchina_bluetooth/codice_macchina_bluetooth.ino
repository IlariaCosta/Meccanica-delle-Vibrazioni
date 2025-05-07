// Pin motori
const int enA = 10, in1 = 9, in2 = 8;
const int in3 = 7, in4 = 6, enB = 5;

void setup() {
  // Imposta i pin
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  // Imposta direzione avanti
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);

  // Imposta velocità (0-255)
  analogWrite(enA, 200);
  analogWrite(enB, 200);
}

void loop() {
  // Non serve fare nulla qui: i motori girano sempre
}
