#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <SPIFFS.h>

const char* WIFI_SSID = "Flavia";
const char* WIFI_PASS = "thzz0399";

const int PIN_RELE = 26;
const int PIN_ENA  = 25;
const int PIN_IN1  = 33;
const int PIN_IN2  = 32;
const int PIN_LED  = 2;

const bool RELE_ATIVO_EM_HIGH = true;

int velocidadePWM = 120;
const bool INVERTER_SENTIDO = false;

const int LIMIAR_PRESENCA_MM = 15;
const int HISTERESE_MM       = 6;
const int LEITURAS_CONFIRMA  = 2;
const int ALCANCE_MAX_MM     = 1200;
const unsigned long TEMPO_TRAVAMENTO_MS = 3000;

Adafruit_VL53L0X lox;
WebServer servidor(80);

enum Estado { OPERANDO, PARADA_SEGURANCA, PARADO_MANUAL };
Estado estado = PARADO_MANUAL;

unsigned long contagem = 0;
unsigned long paradasSeguranca = 0;
int  linhaBase = ALCANCE_MAX_MM;
int  ultimaDistancia = 0;
bool pecaPresente = false;
int  cntPerto = 0, cntLonge = 0;
unsigned long inicioPresenca = 0;
bool sensorOk = false;
unsigned long ultimoCheckWifi = 0;

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
  motorDesligar();
  Serial.println("[ERRO] TRAVAMENTO detectado! Motor desligado pelo rele.");
}

int normalizar(int d) {
  if (d <= 0 || d > ALCANCE_MAX_MM) return ALCANCE_MAX_MM;
  return d;
}

void calibrar() {
  Serial.println("[INFO] Calibrando... deixe a esteira VAZIA.");
  long soma = 0; int n = 0;
  for (int i = 0; i < 30; i++) {
    VL53L0X_RangingMeasurementData_t m;
    lox.rangingTest(&m, false);
    if (m.RangeStatus != 4) { soma += normalizar(m.RangeMilliMeter); n++; }
    delay(30);
  }
  linhaBase = (n > 0) ? soma / n : ALCANCE_MAX_MM;
  Serial.printf("[INFO] Linha de base: %d mm (%d leituras validas)\n", linhaBase, n);
}

void lerSensor() {
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

// O IP e o motivo da queda agora sao impressos pelos eventos do Wi-Fi (no setup).
// Aqui so tentamos reconectar, no maximo a cada 10 s.
void verificarWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - ultimoCheckWifi > 10000) {
    ultimoCheckWifi = millis();
    Serial.println("[AVISO] Wi-Fi desconectado, tentando reconectar...");
    WiFi.reconnect();
  }
}

void cabecalhos() { servidor.sendHeader("Access-Control-Allow-Origin", "*"); }

void rotaStatus() {
  char json[400];
  snprintf(json, sizeof(json),
    "{\"contagem\":%lu,\"estado\":\"%s\",\"peca_presente\":%s,"
    "\"distancia_mm\":%d,\"linha_base_mm\":%d,\"velocidade_pwm\":%d,"
    "\"paradas_seguranca\":%lu,\"sensor_ok\":%s,\"uptime_s\":%lu,"
    "\"ip\":\"%s\",\"mac\":\"%s\",\"rssi\":%d}",
    contagem, nomeEstado(), pecaPresente ? "true" : "false",
    ultimaDistancia, linhaBase, velocidadePWM, paradasSeguranca,
    sensorOk ? "true" : "false", millis() / 1000,
    WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), WiFi.RSSI());
  cabecalhos();
  servidor.send(200, "application/json", json);
}

void responderOk() { rotaStatus(); }

// ---------- Pagina web ----------
void rotaPagina() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    servidor.send(404, "text/html", "<h1>Erro: index.html nao encontrado!</h1>");
    return;
  }
  servidor.streamFile(file, "text/html");
  file.close();
}

// ---------- Rotas de controle ----------
void rotaLigar()    { religar();     responderOk(); }
void rotaParar()    { pararManual(); responderOk(); }
void rotaZerar()    { contagem = 0; paradasSeguranca = 0; responderOk(); }
void rotaCalibrar() { calibrar();    responderOk(); }
void rotaVelocidade() {
  if (servidor.hasArg("v")) {
    velocidadePWM = constrain(servidor.arg("v").toInt(), 0, 255);
    if (estado == OPERANDO) pwmEscrever(velocidadePWM);
  }
  responderOk();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BOOT: versao com eventos Wi-Fi ===");

  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pwmIniciar();
  motorDesligar();

  if (!SPIFFS.begin(true)) {
    Serial.println("[ERRO] SPIFFS falhou!");
  }
  File raiz = SPIFFS.open("/");
  File f = raiz.openNextFile();
  while (f) {
    Serial.printf("[SPIFFS] %s - %d bytes\n", f.name(), (int)f.size());
    f = raiz.openNextFile();
  }
  Wire.begin();
  sensorOk = lox.begin();
  if (sensorOk) {
    lox.startRangeContinuous();
    calibrar();
  } else {
    Serial.println("[ERRO] VL53L0X nao encontrado!");
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.printf("[WIFI] Desconectou, motivo: %d\n", info.wifi_sta_disconnected.reason);
  }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print("[INFO] Wi-Fi conectado. Acesse: http://");
    Serial.println(WiFi.localIP());
  }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  servidor.on("/", rotaPagina);
  servidor.on("/status", rotaStatus);
  servidor.on("/ligar", rotaLigar);
  servidor.on("/parar", rotaParar);
  servidor.on("/zerar", rotaZerar);
  servidor.on("/calibrar", rotaCalibrar);
  servidor.on("/velocidade", rotaVelocidade);
  servidor.begin();
}

void loop() {
  servidor.handleClient();
  lerSensor();
  verificarWifi();
}
