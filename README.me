# Sistema de Controle de Acesso com ESP32, RFID e API

## Descrição

Este projeto implementa um sistema de controle de acesso utilizando o ESP32, um leitor de RFID, e uma API externa para validação de credenciais. O sistema também conta com armazenamento local de credenciais, utilizando o sistema de arquivos SPIFFS, para otimizar futuras consultas em caso de perda de conexão com a internet.

### Funcionalidades:

- Verificar tags RFID usando uma API externa;
- Armazenar e verificar credenciais localmente;
- Controlar LEDs e um buzzer para indicar se o acesso foi liberado ou negado;
- Exibir o status do sistema via uma página web com AJAX;
- Utilizar um display LCD I2C para informar o status de leitura das tags.

## Componentes Utilizados

- **ESP32**: Microcontrolador que gerencia o sistema de controle de acesso.
- **MFRC522**: Módulo de leitura de RFID para capturar as credenciais.
- **SPIFFS**: Sistema de arquivos embutido no ESP32 para armazenamento local de credenciais.
- **LCD I2C**: Tela de 16x2 para exibir mensagens de status.
- **LEDs**: Indicação visual de acesso liberado (LED verde) ou negado (LED vermelho).
- **Buzzer**: Indicação sonora para eventos de acesso.
- **API**: Utilizada para consultar a base de dados remota de credenciais.
- **WiFi**: Conexão com a internet para comunicação com a API.
- **Serviço Web**: Fornece uma interface web para monitoramento do status de leitura e resultado de acesso.

## Funcionamento

1. **Leitura da Tag RFID**: Quando uma tag é aproximada do leitor, o ESP32 captura seu ID e tenta validar o acesso.
2. **Validação via API**: Se houver conexão WiFi disponível, o sistema tenta validar a tag através de uma API externa.
3. **Armazenamento Local**: As credenciais retornadas da API são salvas localmente para futuras consultas. Caso a conexão com a internet seja interrompida, o sistema verifica as credenciais armazenadas no arquivo SPIFFS.
4. **Indicação de Acesso**:
   - Se o acesso for permitido, o sistema acende o LED verde, destranca o mecanismo de tranca e emite um sinal sonoro.
   - Se o acesso for negado, o LED vermelho é aceso e o buzzer é ativado.
5. **Interface Web**: O sistema hospeda uma página web acessível pelo IP do ESP32, onde é possível verificar o ID da última tag lida e o status do acesso em tempo real.

## Pré-requisitos

- **Placa ESP32**
- **Leitor RFID MFRC522**
- **LCD I2C**
- **LEDs e Buzzer**
- **Bibliotecas Arduino**: 
  - `MFRC522`
  - `SPIFFS`
  - `WiFi`
  - `HTTPClient`
  - `ArduinoJson`
  - `LiquidCrystal_I2C`
  - `WebServer`

## Conexões

### RFID (MFRC522)

- **SCK**: Pino 18
- **MOSI**: Pino 23
- **MISO**: Pino 19
- **SDA/SS**: Pino 5
- **RST**: Pino 4

### LEDs e Buzzer

- **LED Verde**: Pino 26
- **LED Vermelho**: Pino 25
- **Buzzer**: Pino 15
- **Tranca**: Pino 13

### LCD I2C

- Conectado aos pinos I2C (SDA, SCL) do ESP32.

## Instalação e Uso

1. **Configuração do Ambiente**: Certifique-se de que as bibliotecas necessárias estão instaladas no Arduino IDE.
2. **Configuração WiFi**: No arquivo `main.ino`, altere as variáveis `ssid` e `password` para as credenciais da sua rede WiFi.
3. **Compilação e Upload**: Compile e envie o código para o ESP32 usando a Arduino IDE.
4. **Execução**: Ao iniciar o ESP32, ele se conectará ao WiFi e ficará aguardando a leitura de tags RFID.
5. **Interface Web**: Acesse o IP exibido no console serial para visualizar a interface de monitoramento.

## Estrutura de Arquivos

- **main.ino**: Código principal do sistema.
- **/data/credenciais.txt**: Arquivo de credenciais locais armazenado no SPIFFS.

## Funcionamento das Rotas Web

- **/**: Página principal que exibe o ID da última tag lida e o status do acesso em tempo real.
- **/status**: Retorna o último ID de tag lido e o status de acesso em formato JSON.

## Contribuições e Melhorias Futuras

- Implementar um sistema de logs para manter histórico de acessos.
- Melhorar o mecanismo de cache local, atualizando credenciais apenas quando houver modificações na API.
- Adicionar suporte para múltiplos usuários simultâneos com permissões diferenciadas.

## Licença

Este projeto está licenciado sob a licença MIT - veja o arquivo LICENSE para mais detalhes.
