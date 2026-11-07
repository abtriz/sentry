const int pinoRele = 4;

void setup() {
  pinMode(pinoRele, OUTPUT);
}

void loop() {
  digitalWrite(pinoRele, HIGH);
  delay(2000);

  digitalWrite(pinoRele, LOW);
  delay(2000);
}
