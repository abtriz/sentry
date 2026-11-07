#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "config.h"
#include "esteira.h"

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #error "Crie o arquivo secrets.h: copie secrets_example.h e preencha o Wi-Fi."
#endif

WebServer servidor(PORTA_HTTP);
bool siteOk = false;
unsigned long ultimoCheckWifi = 0;

void iniciarArquivos() {
  if (!LittleFS.begin(true)) {
    Serial.println("[ERRO] Falha ao montar o LittleFS.");
    return;
  }
  File raiz = LittleFS.open("/");
  File f = raiz.openNextFile();
  while (f) {
    Serial.printf("[LittleFS] %s - %d bytes\n", f.name(), (int)f.size());
    f = raiz.openNextFile();
  }
  siteOk = LittleFS.exists("/index.html");
  if (!siteOk) Serial.println("[AVISO] index.html nao encontrado. Faca o upload da pasta data.");
}

void iniciarWifi() {
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
  Serial.print("[INFO] Conectando ao Wi-Fi");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED)
    Serial.println("[AVISO] Sem Wi-Fi por enquanto. A seguranca local continua funcionando.");
}

void verificarWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - ultimoCheckWifi > INTERVALO_RECONEXAO_MS) {
    ultimoCheckWifi = millis();
    Serial.println("[AVISO] Wi-Fi desconectado, tentando reconectar...");
    WiFi.reconnect();
  }
}

void cabecalhos() {
  servidor.sendHeader("Access-Control-Allow-Origin", "*");
  servidor.sendHeader("Cache-Control", "no-store");
}

void enviarJson(int codigo, const char* json) {
  cabecalhos();
  servidor.send(codigo, "application/json", json);
}

void rotaStatus() {
  char json[512];
  snprintf(json, sizeof(json),
    "{\"contagem\":%lu,\"estado\":\"%s\",\"peca_presente\":%s,"
    "\"distancia_mm\":%d,\"linha_base_mm\":%d,\"velocidade_pwm\":%d,"
    "\"rele_ligado\":%s,\"paradas_seguranca\":%lu,\"sensor_ok\":%s,"
    "\"uptime_s\":%lu,\"ip\":\"%s\",\"mac\":\"%s\",\"rssi\":%d}",
    contagem, nomeEstado(), pecaPresente ? "true" : "false",
    ultimaDistancia, linhaBase, velocidadePWM,
    estado == OPERANDO ? "true" : "false", paradasSeguranca,
    sensorOk ? "true" : "false", millis() / 1000,
    WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), WiFi.RSSI());
  enviarJson(200, json);
}

void rotaSensor() {
  char json[160];
  snprintf(json, sizeof(json),
    "{\"sensor_ok\":%s,\"distancia_mm\":%d,\"linha_base_mm\":%d,\"peca_presente\":%s}",
    sensorOk ? "true" : "false", ultimaDistancia, linhaBase, pecaPresente ? "true" : "false");
  enviarJson(200, json);
}

void rotaAtuador() {
  String e = servidor.arg("estado");
  if (e == "on") religar();
  else if (e == "off") pararManual();
  else { enviarJson(400, "{\"erro\":\"use estado=on ou estado=off\"}"); return; }
  rotaStatus();
}

void rotaVelocidade() {
  if (!servidor.hasArg("v")) { enviarJson(400, "{\"erro\":\"informe v=0..255\"}"); return; }
  definirVelocidade(servidor.arg("v").toInt());
  rotaStatus();
}

bool servirArquivo(String caminho) {
  if (caminho.endsWith("/")) caminho += "index.html";
  if (!LittleFS.exists(caminho)) return false;

  String tipo = "text/plain";
  if      (caminho.endsWith(".html")) tipo = "text/html; charset=utf-8";
  else if (caminho.endsWith(".css"))  tipo = "text/css";
  else if (caminho.endsWith(".js"))   tipo = "application/javascript";
  else if (caminho.endsWith(".png"))  tipo = "image/png";
  else if (caminho.endsWith(".svg"))  tipo = "image/svg+xml";
  else if (caminho.endsWith(".ico"))  tipo = "image/x-icon";

  File f = LittleFS.open(caminho, "r");
  servidor.streamFile(f, tipo);
  f.close();
  return true;
}

void rotaNaoEncontrada() {
  if (servidor.method() == HTTP_GET && servirArquivo(servidor.uri())) return;

  if (servidor.uri() == "/" && !siteOk) {
    cabecalhos();
    servidor.send(404, "text/html; charset=utf-8",
      "<h1>Site nao encontrado no LittleFS</h1><p>Faca o upload da pasta data. A API continua em /status.</p>");
    return;
  }
  enviarJson(404, "{\"erro\":\"rota nao encontrada\"}");
}

void iniciarServidor() {
  servidor.on("/status",     HTTP_GET, rotaStatus);
  servidor.on("/sensor",     HTTP_GET, rotaSensor);
  servidor.on("/ligar",      []() { religar();       rotaStatus(); });
  servidor.on("/parar",      []() { pararManual();   rotaStatus(); });
  servidor.on("/zerar",      []() { zerarContagem(); rotaStatus(); });
  servidor.on("/calibrar",   []() { calibrar();      rotaStatus(); });
  servidor.on("/atuador",    rotaAtuador);
  servidor.on("/velocidade", rotaVelocidade);

  servidor.onNotFound(rotaNaoEncontrada);

  servidor.begin();
  Serial.printf("[INFO] Servidor HTTP na porta %u\n", PORTA_HTTP);
}
