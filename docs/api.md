# API HTTP do Projeto Sentry

O ESP32 roda um servidor HTTP na **porta 80** (biblioteca `WebServer`, sobre TCP). O cliente precisa estar na mesma rede Wi-Fi da placa. O endereço IP aparece no Monitor Serial ao ligar (`Acesse: http://IP`) e também no campo `ip` do `/status`.

## Convenções

- Todas as respostas da API são **JSON** (`Content-Type: application/json`).
- Toda resposta traz `Access-Control-Allow-Origin: *`, para o painel funcionar mesmo aberto fora da placa, e `Cache-Control: no-store`, para o navegador nunca mostrar um estado antigo.
- Os comandos (`/ligar`, `/parar`, `/atuador`, `/velocidade`, `/zerar`, `/calibrar`) aceitam **POST**, que é o método usado pelo site e pelo script de testes, e também **GET**, para facilitar o teste direto no navegador.
- Depois de qualquer comando, a resposta é o **status completo** já atualizado. Assim o cliente confirma na mesma resposta que o comando foi executado.

## Códigos de resposta

| Código | Quando acontece |
|---|---|
| 200 | Requisição atendida |
| 400 | Parâmetro ausente ou inválido (`/atuador`, `/velocidade`) |
| 404 | Rota inexistente, ou site ainda não enviado ao LittleFS |

## Rotas de leitura

### `GET /status`

Estado completo do sistema.

```json
{
  "contagem": 12,
  "estado": "operando",
  "peca_presente": false,
  "distancia_mm": 61,
  "linha_base_mm": 62,
  "velocidade_pwm": 120,
  "rele_ligado": true,
  "paradas_seguranca": 1,
  "sensor_ok": true,
  "uptime_s": 384,
  "ip": "192.168.0.15",
  "mac": "24:6F:28:XX:XX:XX",
  "rssi": -54
}
```

| Campo | Tipo | Significado |
|---|---|---|
| `contagem` | número | Peças contadas (fica salva na memória flash) |
| `estado` | texto | `operando`, `parada_seguranca` (travamento) ou `parado` (parada manual) |
| `peca_presente` | booleano | Há uma peça sob o sensor neste momento |
| `distancia_mm` | número | Última leitura do VL53L0X, em milímetros |
| `linha_base_mm` | número | Distância até a esteira vazia, medida na calibração |
| `velocidade_pwm` | número | Velocidade configurada do motor (0–255) |
| `rele_ligado` | booleano | O relé está fechado (motor energizado) |
| `paradas_seguranca` | número | Quantas paradas por travamento já ocorreram |
| `sensor_ok` | booleano | O sensor foi encontrado ao iniciar |
| `uptime_s` | número | Segundos desde que a placa ligou |
| `ip`, `mac` | texto | Endereço IP e MAC do ESP32 na rede |
| `rssi` | número | Força do sinal Wi-Fi em dBm (mais perto de 0 = melhor) |

### `GET /sensor`

Só a leitura do sensor.

```json
{ "sensor_ok": true, "distancia_mm": 61, "linha_base_mm": 62, "peca_presente": false }
```

## Rotas de comando

| Rota | Parâmetro | O que faz |
|---|---|---|
| `POST /ligar` | — | Fecha o relé, define o sentido e liga o motor na velocidade configurada |
| `POST /parar` | — | Parada manual: PWM em zero e relé aberto |
| `POST /atuador` | `estado=on` ou `estado=off` | O mesmo que `/ligar` e `/parar`, no formato de rota de atuador |
| `POST /velocidade` | `v=0..255` | Muda a velocidade do motor (valores fora da faixa são limitados) |
| `POST /zerar` | — | Zera a contagem e as paradas, também na memória flash |
| `POST /calibrar` | — | Mede de novo a esteira vazia e atualiza a linha de base (leva cerca de 1 s) |

Erros possíveis:

```json
{ "erro": "use estado=on ou estado=off" }
{ "erro": "informe v=0..255" }
{ "erro": "rota nao encontrada" }
```

## Rotas do site

| Rota | Arquivo no LittleFS |
|---|---|
| `GET /` | `/index.html` |
| `GET /style.css` | `/style.css` |
| `GET /app.js` | `/app.js` |

Qualquer caminho que não seja da API é procurado no LittleFS. Se o site ainda não foi enviado, a rota `/` mostra uma mensagem explicando como fazer o upload, e a API continua funcionando normalmente.

## Exemplos

cURL (Git Bash, Linux ou macOS):

```bash
curl http://192.168.0.15/status
curl http://192.168.0.15/sensor
curl -X POST http://192.168.0.15/parar
curl -X POST http://192.168.0.15/ligar
curl -X POST "http://192.168.0.15/atuador?estado=off"
curl -X POST "http://192.168.0.15/velocidade?v=150"
```

PowerShell (Windows):

```powershell
Invoke-RestMethod http://192.168.0.15/status
Invoke-RestMethod -Method Post http://192.168.0.15/parar
Invoke-RestMethod -Method Post "http://192.168.0.15/velocidade?v=150"
```

No navegador basta abrir, por exemplo, `http://192.168.0.15/status` ou `http://192.168.0.15/parar`.
