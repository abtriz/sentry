#pragma once

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Preferences.h>
#include "config.h"

enum Estado { OPERANDO, PARADA_SEGURANCA, PARADO_MANUAL };

Adafruit_VL53L0X lox;
Preferences memoria;

Estado estado = PARADO_MANUAL;
unsigned long contagem = 0;
unsigned long paradasSeguranca = 0;
int  velocidadePWM = VELOCIDADE_PADRAO;
int  linhaBase = ALCANCE_MAX_MM;
int  ultimaDistancia = 0;
bool pecaPresente = false;
int  cntPerto = 0, cntLonge = 0;
unsigned long inicioPresenca = 0;
bool sensorOk = false;
bool contagemAlterada = false;
unsigned long ultimoSalvamento = 0;

void carregarContagem() {
  memoria.begin("sentry", false);
  contagem = memoria.getULong("contagem", 0);
  paradasSeguranca = memoria.getULong("paradas", 0);
  Serial.printf("[INFO] Contagem recuperada: %lu pecas, %lu paradas\n", contagem, paradasSeguranca);
}

void salvarContagem(bool agora = false) {
  if (!contagemAlterada) return;
  if (!agora && millis() - ultimoSalvamento < INTERVALO_SALVAMENTO_MS) return;
  memoria.putULong("contagem", contagem);
  memoria.putULong("paradas", paradasSeguranca);
  contagemAlterada = false;
  ultimoSalvamento = millis();
}

void pwmIniciar() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PIN_ENA, 1000, 8);
#else
  ledcSetup(0, 1000, 8);
  ledcAttachPin(PIN_ENA, 0);
#endif
}

void pwmEscrever(int valor) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PIN_ENA, valor);
#else
  ledcWrite(0, valor);
#endif
}

void releLigado(bool ligado) {
  digitalWrite(PIN_RELE, (ligado == RELE_ATIVO_EM_HIGH) ? HIGH : LOW);
}

void motorLigar() {
  digitalWrite(PIN_IN1, INVERTER_SENTIDO ? LOW : HIGH);
  digitalWrite(PIN_IN2, INVERTER_SENTIDO ? HIGH : LOW);
  releLigado(true);
  delay(50);
  pwmEscrever(255);
  delay(150);
  pwmEscrever(velocidadePWM);
}

void motorDesligar() {
  pwmEscrever(0);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  releLigado(false);
}

void definirVelocidade(int valor) {
  velocidadePWM = constrain(valor, 0, 255);
  if (estado == OPERANDO) pwmEscrever(velocidadePWM);
}

const char* nomeEstado() {
  switch (estado) {
    case OPERANDO:         return "operando";
    case PARADA_SEGURANCA: return "parada_seguranca";
    default:               return "parado";
  }
}

void religar() {
  estado = OPERANDO;
  inicioPresenca = millis();
  motorLigar();
  Serial.println("[INFO] Motor religado.");
}

void pararManual() {
  estado = PARADO_MANUAL;
  motorDesligar();
  Serial.println("[INFO] Parada manual.");
}

void paradaDeSeguranca() {
  estado = PARADA_SEGURANCA;
  paradasSeguranca++;
  contagemAlterada = true;
  motorDesligar();
  salvarContagem(true);
  Serial.println("[ERRO] TRAVAMENTO detectado! Motor desligado pelo rele.");
}

void zerarContagem() {
  contagem = 0;
  paradasSeguranca = 0;
  contagemAlterada = true;
  salvarContagem(true);
  Serial.println("[INFO] Contagem zerada.");
}

int normalizar(int d) {
  if (d <= 0 || d > ALCANCE_MAX_MM) return ALCANCE_MAX_MM;
  return d;
}

void calibrar() {
  lox.stopRangeContinuous();
  Serial.println("[INFO] Calibrando... deixe a esteira VAZIA.");
  long soma = 0;
  int n = 0;
  for (int i = 0; i < 30; i++) {
    VL53L0X_RangingMeasurementData_t m;
    lox.rangingTest(&m, false);
    if (m.RangeStatus != 4) { soma += normalizar(m.RangeMilliMeter); n++; }
    delay(30);
  }
  linhaBase = (n > 0) ? soma / n : ALCANCE_MAX_MM;
  Serial.printf("[INFO] Linha de base: %d mm (%d leituras validas)\n", linhaBase, n);
  lox.startRangeContinuous(30);
}

void lerSensor() {
  salvarContagem();
  if (!sensorOk || !lox.isRangeComplete()) return;
  int d = normalizar(lox.readRange());
  ultimaDistancia = d;

  bool perto = d < linhaBase - LIMIAR_PRESENCA_MM;
  bool longe = d > linhaBase - (LIMIAR_PRESENCA_MM - HISTERESE_MM);

  if (!pecaPresente) {
    cntPerto = perto ? cntPerto + 1 : 0;
    if (cntPerto >= LEITURAS_CONFIRMA) {
      pecaPresente = true;
      cntPerto = 0;
      inicioPresenca = millis();
      if (estado == OPERANDO) {
        contagem++;
        contagemAlterada = true;
        Serial.printf("[INFO] Peca #%lu (%d mm)\n", contagem, d);
      }
    }
  } else {
    cntLonge = longe ? cntLonge + 1 : 0;
    if (cntLonge >= LEITURAS_CONFIRMA) {
      pecaPresente = false;
      cntLonge = 0;
    } else if (estado == OPERANDO && millis() - inicioPresenca > TEMPO_TRAVAMENTO_MS) {
      paradaDeSeguranca();
    }
  }
}

void iniciarEsteira() {
  digitalWrite(PIN_RELE, RELE_ATIVO_EM_HIGH ? LOW : HIGH);
  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pwmIniciar();
  motorDesligar();
  carregarContagem();

  Wire.begin(PIN_SDA, PIN_SCL);
  sensorOk = lox.begin();
  if (sensorOk) {
    calibrar();
  } else {
    Serial.println("[ERRO] VL53L0X nao encontrado! Confira VIN/GND/SDA(D21)/SCL(D22).");
  }
}

void lerSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == 'r') religar();
  else if (c == 's') pararManual();
  else if (c == 'z') zerarContagem();
}

void atualizarLed() {
  if (estado == OPERANDO) digitalWrite(PIN_LED, HIGH);
  else if (estado == PARADA_SEGURANCA) digitalWrite(PIN_LED, (millis() / 150) % 2);
  else digitalWrite(PIN_LED, LOW);
}

void imprimirDebug() {
  static unsigned long t = 0;
  if (millis() - t < 500) return;
  t = millis();
  Serial.printf("dist=%4d mm | base=%4d | peca=%d | contagem=%lu | %s\n",
                ultimaDistancia, linhaBase, pecaPresente, contagem, nomeEstado());
}
