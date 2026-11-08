#pragma once

const int PIN_SDA = 21;
const int PIN_SCL = 22;

const int  PIN_RELE           = 26;
const bool RELE_ATIVO_EM_HIGH = true;

const int  PIN_ENA           = 25;
const int  PIN_IN1           = 33;
const int  PIN_IN2           = 32;
const bool INVERTER_SENTIDO  = false;
const int  VELOCIDADE_PADRAO = 120;

const int PIN_LED = 2;

const int LIMIAR_PRESENCA_MM = 15;
const int HISTERESE_MM       = 6;
const int LEITURAS_CONFIRMA  = 2;
const int ALCANCE_MAX_MM     = 1200;
const unsigned long TEMPO_TRAVAMENTO_MS = 3000;
const unsigned long INTERVALO_SALVAMENTO_MS = 5000;

const uint16_t PORTA_HTTP = 80;
const unsigned long INTERVALO_RECONEXAO_MS = 10000;
