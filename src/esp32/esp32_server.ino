#include "esteira.h"
#include "servidor.h"

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== PROJETO SENTRY ===");

  iniciarEsteira();
  iniciarArquivos();
  iniciarWifi();
  iniciarServidor();

  if (sensorOk) religar();
  else Serial.println("[AVISO] Motor mantido desligado (sem sensor).");
}

void loop() {
  lerSensor();
  servidor.handleClient();
  verificarWifi();
  lerSerial();
  atualizarLed();
  imprimirDebug();
}
