#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SPIFFS.h>

// Defina o SSID e senha do WiFi
const char *ssid = "SEU_SSID";
const char *password = "SUA_SENHA";

// Defina a URL da API
#define API_URL "https://eb17-138-204-187-144.ngrok-free.app/api/credentials"

// Define the pins for the SPI interface
#define SS_PIN 5
#define RST_PIN 4
#define SCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19

#define GREEN_LED 26
#define RED_LED 25
#define BUZZER 15
#define CANAL_PWM 0
#define BRAID 13 // tranca

MFRC522 mfrc522(SS_PIN, RST_PIN);   // define os pinos de controle do modulo de leitura de cartoes RFID
LiquidCrystal_I2C lcd(0x27, 16, 2); // define informacoes do lcd como o endereço I2C (0x27) e tamanho do mesmo

void setup()
{
    // Inicializa a conexão WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Conectando ao WiFi...");
    }
    Serial.println("WiFi conectado!");

    // Inicializa o SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("Erro ao montar o sistema de arquivos SPIFFS");
        return;
    }

    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    Serial.begin(115200);
    Wire.begin();

    // inicia o modulo RFID
    mfrc522.PCD_Init();

    Serial.println("RFID + ESP32");
    Serial.println("Aguardando tag RFID para verificar o id da mesma.");

    lcd.home();                // bota o cursor do lcd na posicao inicial
    lcd.print("Aguardando");   // imprime na primeira linha a string "Aguardando"
    lcd.setCursor(0, 1);       // seta o cursor para a segunda linha
    lcd.print("Leitura RFID"); // mostra na tela a string "Leitura RFID"

    // define alguns pinos como saida
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BRAID, OUTPUT);
    ledcSetup(CANAL_PWM, 1000, 8);
    ledcAttachPin(BUZZER, CANAL_PWM);
}

void loop()
{
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return; // se nao tiver um cartao para ser lido recomeça o void loop
    }

    if (!mfrc522.PICC_ReadCardSerial())
    {
        return; // se nao conseguir ler o cartao recomeça o void loop tambem
    }

    String conteudo = ""; // cria uma string

    Serial.print("id da tag :"); // imprime na serial o id do cartao

    // faz uma verificacao dos bits da memoria do cartao
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        // ambos comandos abaixo vão concatenar as informacoes do cartao...
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    Serial.println();
    conteudo.toUpperCase(); // deixa as letras da string todas maiusculas

    if (checkAccess(conteudo))
    {
        // Código para liberar o acesso
        digitalWrite(GREEN_LED, HIGH); // ligamos o led verde
        lcd.clear();                   // limpamos oque havia sido escrito no lcd
        lcd.print("Acesso Liberado");  // informamos pelo lcd que a tranca foi aberta

        digitalWrite(BRAID, HIGH); // abrimos a tranca por 5 segundos

        // vai informando ao usuario quantos segundos faltam para a tranca ser fechada
        for (byte s = 5; s > 0; s--)
        {
            lcd.setCursor(8, 1);
            lcd.print(s);
            delay(1000);
        }

        digitalWrite(BRAID, LOW);     // fecha a tranca
        digitalWrite(GREEN_LED, LOW); // e desliga o led
        lcd.clear();                  // limpa os caracteres q estao escritos no lcd
    }
    else
    {
        // Código para negar o acesso
        digitalWrite(RED_LED, HIGH); // vamos ligar o led vermelho

        for (byte s = 5; s > 0; s--)
        { // uma contagem / espera para poder fazer uma nova leitura

            lcd.clear();                // limpa as informacoes que estao na tela
            lcd.home();                 // nota na posicao inicial
            lcd.print("Acesso negado"); // infoma ao usuario que ele nao tem acesso
            lcd.setCursor(8, 1);        // coloca o cursor na coluna 8 da linha 2
            lcd.print(s);               // informa quantos segundos faltam para pode fazer uma nova leitura

            // faz o BUZZER emitir um bip por segundo
            ledcWrite(CANAL_PWM, 500);
            delay(800);
            ledcWrite(CANAL_PWM, 0);
            delay(200);
        }

        digitalWrite(RED_LED, LOW); // desliga o led vermelho
        lcd.clear();                // limpa as informacoes do lcd
    }
}

// Função para verificar o acesso através da API
bool checkAccess(String idTag)
{
    Serial.println("Acessando a API");

    // Verifica o arquivo local primeiro
    if (checkLocalFile(idTag))
    {
        return true;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(API_URL);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(payload);

            saveLocalCredentials(payload);

            if (payload.indexOf(idTag) > 0)
            {
                Serial.println("Acesso permitido");
                http.end();
                return true;
            }
            else
            {
                Serial.println("Acesso negado");
                http.end();
                return false;
            }
        }
        else
        {
            Serial.println("Erro ao acessar a API");
            return false;
        }
    }
    else
    {
        Serial.println("Erro de conexão WiFi");
        return false;
    }
}

bool checkLocalFile(String idTag)
{
    Serial.println("Acessando arquivos locais");

    File arquivo = SPIFFS.open("/credenciais.txt", "r");
    if (!arquivo)
    {
        Serial.println("Não foi possível abrir o arquivo");
        return false;
    }

    while (arquivo.available())
    {
        String linha = arquivo.readStringUntil('\n');
        if (linha.indexOf(idTag) >= 0)
        {
            Serial.println("Acesso permitido - credencial local");
            arquivo.close();
            return true;
        }
    }

    arquivo.close();
    return false;
}

void saveLocalCredentials(String dados)
{
    Serial.println("Salvando as credenciais locais");

    File arquivo = SPIFFS.open("/credenciais.txt", "w");
    if (!arquivo)
    {
        Serial.println("Não foi possível abrir o arquivo para escrita");
        return;
    }

    arquivo.println(dados);
    arquivo.close();
    Serial.println("Credenciais salvas localmente");
}