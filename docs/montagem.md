# Montagem física

Passo a passo de como a esteira e o circuito do Sentry foram montados. As ligações pino a pino estão em [hardware.md](hardware.md).

## 1. Esteira (kit em MDF EA100)

1. Encaixe as porcas nas aberturas das peças 6 e 7 (pode ser preciso forçar com um alicate sobre uma base plana).
2. Sobre uma das peças laterais (peça 1), encaixe as peças 4, 5, 6, 7, 9 e os rolos. O pino maior vai na peça 5 e o menor na peça 4.
3. Coloque a lona elástica em volta dos rolos, de preferência com ajuda de outra pessoa.
4. Feche com a outra peça lateral e aperte os parafusos M3x10 onde ficaram as porcas.
5. Instale as travas laterais (peças 10). Elas regulam a tensão da lona: lona frouxa escorrega, lona esticada demais pesa para o motor.

## 2. Motor e engrenagens

1. Prenda o motor DC na parte de fora da lateral, do mesmo lado da engrenagem maior, usando a engrenagem 1 (para motor DC) e os parafusos maiores.
2. **Fixe a engrenagem pequena no eixo do motor.** O furo do MDF fica folgado no eixo do motor TT e a engrenagem patina. Use um parafuso pequeno com arruela na ponta do eixo ou uma gota de cola.
3. Afrouxe os parafusos do motor e deslize-o nos furos oblongos até os dentes das duas engrenagens se encaixarem por inteiro, com uma folga mínima. Aperte de novo.
4. Gire os rolos à mão: devem girar leves, sem raspar nas laterais.

## 3. Sensor

1. Retire a película de proteção do VL53L0X, se houver.
2. Prenda o sensor **acima da lona, apontando para baixo**, entre 4 e 8 cm de altura.
3. Evite luz forte direta (janela ou luminária) sobre o ponto de medição.

## 4. Circuito (sem protoboard)

A plaquinha da fonte faz o papel da protoboard: a fileira **−** é o GND comum e a fileira **+** leva a energia da fonte ao relé.

1. **Sensor:** VIN no 3V3, GND no GND do lado do 3V3, SDA no D21, SCL no D22.
2. **Relé:** S no D26, + no VIN, − na fileira −, COM na fileira +, NO no +12V do L298N.
3. **L298N:** retire o jumper do ENA; ENA no D25, IN1 no D33, IN2 no D32, GND na fileira −, motor em OUT1/OUT2.
4. **GND comum:** GND do lado do VIN do ESP32 na fileira −.
5. **Fonte:** plugue P4 no CN1 da plaquinha, **por último**.

Use jumpers fêmea nos pinos do ESP32, do sensor e do relé, e ponta macho na plaquinha e nos bornes de parafuso. Não apoie o ESP32 sobre espuma antiestática preta, que conduz eletricidade.

## 5. Conferência antes de energizar

- [ ] Fileira **+** da plaquinha liga **só** no COM do relé.
- [ ] Fileira **−** recebe o − do relé, o GND do L298N e o GND do ESP32.
- [ ] Jumper do ENA retirado; jumper 5V-EN do L298N no lugar.
- [ ] Nada ligado no +5V do L298N.
- [ ] Nenhuma ponta de fio desencapada encostando em outra.
- [ ] Código gravado no ESP32 antes de ligar a fonte.
