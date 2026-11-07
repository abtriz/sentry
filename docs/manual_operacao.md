# Manual de operação

Guia rápido para ligar, usar e resolver problemas do Sentry, pensado para o dia da apresentação.

## Antes de ligar

1. Esteira montada e **vazia**, com o sensor apontado para a lona.
2. Fonte na tomada e conectada à plaquinha (o LED da plaquinha acende).
3. Hotspot ou roteador ligado, na rede configurada no `secrets.h` (2,4 GHz).
4. ESP32 ligado na USB do computador ou num carregador.

## Ligando

1. Aperte **EN** no ESP32.
2. Pelo Monitor Serial (115200), acompanhe: contagem recuperada, calibração, conexão Wi-Fi, servidor na porta 80 e `Motor religado`.
3. A esteira começa a andar sozinha. O LED azul da placa fica aceso.
4. Abra no celular ou no computador o endereço mostrado em `Acesse: http://IP`.

## Usando o painel

| Perfil | Login | Senha | O que vê |
|---|---|---|---|
| Operador | `operador@sentry.demo` | `1234` | Painel Andon: status, contagem, parar e retomar |
| Gestor | `gestor@sentry.demo` | `1234` | Gestão: indicadores, gráficos e histórico de paradas |

Na tela de login, os botões **Preencher operador** e **Preencher gestor** completam os dados.

- **Parar a esteira:** botão **Parar esteira** no cartão da Esteira 1, escolhendo o motivo e o equipamento.
- **Depois de um travamento:** retire a peça e clique em **Retomar esteira**.
- **Zerar a contagem:** acesse `http://IP/zerar` no navegador.
- **Recalibrar:** com a esteira vazia, acesse `http://IP/calibrar`.

## Estados e sinais

| Estado | LED azul | Cartão no painel | Motor |
|---|---|---|---|
| Operando | aceso | verde, "Rodando" | gira |
| Parada de segurança | piscando | vermelho, "Parada", com o motivo do sensor | parado, relé aberto |
| Parada manual | apagado | vermelho, "Parada", com o motivo escolhido | parado, relé aberto |

## Ajustes sem regravar o código

| Ajuste | Como |
|---|---|
| Velocidade da esteira | `http://IP/velocidade?v=150` (0 a 255; com fonte de 12 V, não passe de ~200) |
| Nova referência do sensor | `http://IP/calibrar` com a esteira vazia |

Ajustes que exigem regravar ficam no `config.h`: limiar de detecção, tempo de travamento, sentido do motor e lógica do relé.

## Solução de problemas

| Sintoma | Causa provável | O que fazer |
|---|---|---|
| `VL53L0X nao encontrado` | Fio solto no D21/D22 ou sem 3V3/GND | Conferir as ligações do sensor e apertar EN |
| Esteira anda 3 s e para sozinha | Linha de base errada (algo sob o sensor ao ligar) | Esvaziar a esteira e apertar EN, ou acessar `/calibrar` |
| Relé clica, mas o motor não gira | Jumper do ENA no lugar, GND não comum ou fonte desligada | Tirar o jumper do ENA, conferir a fileira − e o LED da plaquinha |
| Motor zumbe e não gira | Velocidade baixa ou engrenagem patinando | Aumentar a velocidade e conferir a fixação da engrenagem |
| Esteira anda ao contrário | Fios do motor invertidos | Trocar OUT1 com OUT2 ou mudar `INVERTER_SENTIDO` no `config.h` |
| Motor gira quando deveria parar | Relé ativo em LOW | Mudar `RELE_ATIVO_EM_HIGH` para `false` no `config.h` |
| Site não abre, Serial mostra o IP | Rede de visitantes com isolamento entre aparelhos | Usar o hotspot do celular |
| Site mostra "Sem conexão com o ESP32" | Placa reiniciando ou fora da rede | Esperar a reconexão (até ~10 s) ou conferir o hotspot |
| Página "Site nao encontrado no LittleFS" | Site não foi enviado | Rodar `scripts/preparar_site.bat` e o upload do LittleFS |
| IDE trava em `Connecting...` | Placa não entrou em modo de gravação | Segurar BOOT até começar a gravação |
