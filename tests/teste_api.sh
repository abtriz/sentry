#arquivo teste
#!/usr/bin/env bash

IP="${1:?Informe o IP do ESP32. Ex.: ./tests/teste_api.sh 192.168.0.15}"
BASE="http://$IP"

passo() { echo; echo "=== $1"; }
req()   { curl -s -m 5 -w "  [HTTP %{http_code} em %{time_total}s]\n" "$@"; }

passo "1. Conexao: GET /status"
req "$BASE/status" || { echo "Sem resposta do ESP32 em $IP"; exit 1; }

passo "2. Leitura do sensor: GET /sensor"
req "$BASE/sensor"

passo "3. Atuador: POST /parar (esteira deve parar e o rele dar um clique)"
req -X POST "$BASE/parar"
sleep 2

passo "4. Atuador: POST /ligar (esteira deve voltar a girar)"
req -X POST "$BASE/ligar"
sleep 2

passo "5. Atuador: POST /atuador?estado=off e estado=on"
req -X POST "$BASE/atuador?estado=off"
sleep 1
req -X POST "$BASE/atuador?estado=on"

passo "6. Velocidade: POST /velocidade?v=150 e volta para 120"
req -X POST "$BASE/velocidade?v=150"
sleep 2
req -X POST "$BASE/velocidade?v=120"

passo "7. Erros tratados (esperado HTTP 400 e 404)"
req -X POST "$BASE/atuador?estado=talvez"
req "$BASE/rota-que-nao-existe"

passo "8. Teste de estresse: 100 requisicoes GET /status seguidas"
ok=0; falha=0; inicio=$(date +%s%N)
for i in $(seq 1 100); do
  codigo=$(curl -s -m 2 -o /dev/null -w "%{http_code}" "$BASE/status")
  if [ "$codigo" = "200" ]; then ok=$((ok+1)); else falha=$((falha+1)); fi
done
fim=$(date +%s%N)
media=$(( (fim - inicio) / 100 / 1000000 ))
echo "Sucesso: $ok | Falhas: $falha | Tempo medio por requisicao: ${media} ms"

passo "9. Estado final: GET /status"
req "$BASE/status"
echo
echo "Testes concluidos."
