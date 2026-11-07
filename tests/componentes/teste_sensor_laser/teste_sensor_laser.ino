#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);

  while (! Serial) {
    delay(1);
  }

  Serial.println("Inicializando o sensor VL53L0X...");

  if (!lox.begin()) {
    Serial.println(F("Falha ao inicializar o VL53L0X. Verifique as conexões (SDA no pino 21, SCL no pino 22)"));
    while(1);
  }

  Serial.println(F("VL53L0X Iniciado com sucesso!\n\n"));
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;

  Serial.print("Lendo distância... ");
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    Serial.print("Distância (mm): ");
    Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println("Fora de alcance");
  }

  delay(500);
}
