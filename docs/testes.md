# Plano e relatório de testes

Este documento descreve como validamos o Sentry por partes, como pede o regulamento: primeiro cada componente, depois a rede e, por fim, o sistema completo. Preencha a coluna **Resultado** durante os testes e guarde prints ou vídeos como evidência.

## 0. Provas de conceito (Etapa 1)

Antes de integrar o sistema, cada parte foi testada sozinha com um programa simples. Esses programas de teste são separados do código final e não fazem parte do firmware da esteira.

| Programa | O que validou | Resultado obtido |
|---|---|---|
| `teste_sensor_laser` | Comunicação I2C com o VL53L0X (SDA no D21, SCL no D22) e leitura da distância em milímetros a cada 0,5 s | [preencher] |
| `teste_rele` | Acionamento do módulo relé 5 V pelo ESP32, ligando e desligando a cada 2 s | [preencher] |
| `teste_webserver` | Conexão Wi-Fi e servidor HTTP na porta 80 respondendo a um navegador, com as rotas `/H` e `/L` ligando e desligando o pino 26 | [preencher] |

Neste teste do relé, o sinal estava no GPIO 4. Na versão final ele passou para o D26. O teste do servidor foi a base do servidor HTTP do firmware final.

## 1. Testes de componente (sem rede)

Use o Monitor Serial a 115200. Ele mostra uma linha a cada 0,5 s com a distância, a linha de base, a peça e a contagem.

| # | Teste | Como fazer | Resultado esperado | Resultado |
|---|---|---|---|---|
| C1 | Sensor encontrado | Ligar a placa | Aparece `Linha de base: X mm (30 leituras validas)` | |
| C2 | Leitura estável | Esteira vazia por 10 s | `dist` varia poucos milímetros em torno da `base` | |
| C3 | Detecção de peça | Passar uma peça sob o sensor | `dist` cai pelo menos 15 mm e aparece `Peca #1` | |
| C4 | Sem contagem dupla | Passar uma peça devagar | Conta só uma vez | |
| C5 | Relé | Digitar `s` e depois `r` no Serial | Clique do relé nas duas vezes | |
| C6 | Motor e sentido | Com a esteira ligada | Lona anda para o lado certo, sem zumbir parada | |
| C7 | Travamento | Segurar a peça sob o sensor | Após 3 s: `TRAVAMENTO detectado!`, motor para, LED azul pisca | |
| C8 | Memória da contagem | Contar peças, esperar 5 s e apertar EN | Aparece `Contagem recuperada` com o mesmo número | |

## 2. Testes de rede

| # | Teste | Como fazer | Resultado esperado | Resultado |
|---|---|---|---|---|
| R1 | Conexão Wi-Fi | Ligar a placa | Aparece `Wi-Fi conectado. Acesse: http://IP` | |
| R2 | Ping | `ping IP_DO_ESP32` no terminal | Respostas sem perda | |
| R3 | Porta 80 aberta | PowerShell: `Test-NetConnection IP_DO_ESP32 -Port 80` | `TcpTestSucceeded : True` | |
| R4 | Leitura pela rede | Navegador: `http://IP/status` | JSON com os dados da esteira | |
| R5 | Comando pela rede | Navegador: `http://IP/parar` e `http://IP/ligar` | Relé clica e o motor para e volta | |
| R6 | Erros tratados | `http://IP/atuador?estado=x` e `http://IP/abc` | HTTP 400 e HTTP 404 em JSON | |

## 3. Script automático e teste de estresse

Com a esteira vazia e livre, rode no Git Bash:

```bash
./tests/teste_api.sh IP_DO_ESP32
```

O script testa as rotas de leitura e comando, os erros 400 e 404 e faz **100 requisições seguidas** a `/status`.

| Medida | Resultado |
|---|---|
| Requisições com sucesso | /100 |
| Falhas | |
| Tempo médio por requisição | ms |

## 4. Testes de falha

| # | Teste | Como fazer | Resultado esperado | Resultado |
|---|---|---|---|---|
| F1 | Queda do Wi-Fi | Desligar o hotspot com a esteira rodando | Contagem e parada de segurança continuam; painel mostra "Sem conexão" | |
| F2 | Reconexão | Ligar o hotspot de novo | Em até ~10 s o ESP32 reconecta e o painel volta a atualizar | |
| F3 | Reinício da placa | Apertar EN com a esteira rodando | Motor para na hora (relé no NO) e volta após calibrar | |
| F4 | Sensor desconectado | Ligar a placa sem o fio do SDA | `VL53L0X nao encontrado`, motor não liga, `sensor_ok: false` | |
| F5 | Site ausente | Apagar o LittleFS ou não enviar o site | Rota `/` mostra a mensagem de upload; `/status` continua funcionando | |

## 5. Teste ponta a ponta (demonstração)

| # | Ação no painel ou na esteira | Resultado esperado | Resultado |
|---|---|---|---|
| P1 | Entrar como operador | Esteira 1 aparece com a contagem real e "ESP32 conectado" | |
| P2 | Passar peças | Contagem do cartão sobe em até 1 s | |
| P3 | Segurar uma peça | Cartão fica vermelho e o evento aparece em "Eventos recentes" | |
| P4 | Clicar em "Retomar esteira" | Motor volta a girar | |
| P5 | Clicar em "Parar esteira" e escolher o motivo | Motor para e a parada entra no histórico como "Manual" | |
| P6 | Entrar como gestor | Paradas do teste aparecem no histórico, com origem Sistema e Manual | |
