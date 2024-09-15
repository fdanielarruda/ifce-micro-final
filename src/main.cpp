#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <WiFiClient.h>
#include "ArduinoJson.h"
#include <iostream>
#include <algorithm>
#include <WebServer.h>

// Defina o SSID e senha do WiFi
const char *ssid = "TESTE2_2G";
const char *password = "33831146";

// Defina a URL da API
#define API_URL "https://micro.apps.redtagmobile.com.br/api/credentials"

// Defina os pinos para o SPI
#define SS_PIN 5
#define RST_PIN 4
#define SCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19

#define GREEN_LED 26
#define RED_LED 25
#define BUZZER 15
#define CANAL_PWM 0
#define BRAID 13 // Tranca

unsigned long lastApiCallTime = 0; // Variável para armazenar o timestamp da última chamada à API
const unsigned long ONE_MINUTE = 60000; // 1 minuto em milissegundos

WebServer server(8080); // Inicializa o servidor web na porta 80
String lastTagRead = ""; // Variável para armazenar a última tag RFID lida
String lastAccessStatus = ""; // Variável para armazenar o último status de acesso

MFRC522 mfrc522(SS_PIN, RST_PIN);   // define os pinos de controle do módulo RFID
LiquidCrystal_I2C lcd(0x27, 16, 2); // define as informações do LCD

bool checkAccess(String idTag);
bool checkLocalFile(String idTag);
void saveLocalFile(String dados, String fileName);

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

    // Inicia o módulo RFID
    mfrc522.PCD_Init();

    Serial.println("RFID + ESP32");
    Serial.println("Aguardando tag RFID para verificar o id.");

    // Liga o LCD
    lcd.init();
    lcd.backlight();
    lcd.home();
    lcd.print("Aguardando");
    lcd.setCursor(0, 1);
    lcd.print("Leitura RFID");

    // Define pinos como saída
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BRAID, OUTPUT);
    ledcSetup(CANAL_PWM, 1000, 8);
    ledcAttachPin(BUZZER, CANAL_PWM);

    // Rota para enviar os dados em JSON
    server.on("/status", []() {
        String jsonResponse;
        jsonResponse += "{";
        jsonResponse += "\"lastTagRead\": \"" + lastTagRead + "\",";
        jsonResponse += "\"lastAccessStatus\": \"" + lastAccessStatus + "\"";
        jsonResponse += "}";
        
        server.send(200, "application/json", jsonResponse);
    });

    // Rota principal com AJAX
    server.on("/", []() {
        String page = "<!DOCTYPE html>";
        page += "<html lang='en'><head><meta charset='UTF-8'>";
        page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        page += "<style>";
        page += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; color: #333; margin: 0; padding: 0; text-align: center; }";
        page += ".container { width: 100%; max-width: 600px; margin: 50px auto; background-color: #fff; padding: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
        page += "h1 { font-size: 24px; color: #4CAF50; margin-bottom: 20px; }"; 
        page += "p { font-size: 18px; }";
        page += ".status { font-size: 20px; font-weight: bold; padding: 10px; border-radius: 5px; }";
        page += ".allowed { background-color: #4CAF50; color: white; }"; 
        page += ".denied { background-color: #f44336; color: white; }"; 
        page += "</style>";
        page += "<title>Status do ESP32</title></head><body>";
        
        // Corpo da página
        page += "<div class='container'>";
        page += "<h1>Status do ESP32</h1>";
        page += "<p><strong>ID da última tag lida:</strong> <span id='tagId'>Aguardando...</span></p>";
        page += "<p><strong>Status do acesso:</strong> <span id='status'>Aguardando...</span></p>";
        page += "</div>";

        // JavaScript para atualizar a página via AJAX
        page += "<script>";
        page += "function updateStatus() {";
        page += "  var xhr = new XMLHttpRequest();";
        page += "  xhr.onreadystatechange = function() {";
        page += "    if (xhr.readyState == 4 && xhr.status == 200) {";
        page += "      var data = JSON.parse(xhr.responseText);";
        page += "      document.getElementById('tagId').innerText = data.lastTagRead;";
        
        // Atualiza a cor do status com base no acesso
        page += "      var statusElem = document.getElementById('status');";
        page += "      statusElem.innerText = data.lastAccessStatus;";
        page += "      if (data.lastAccessStatus == 'Acesso Liberado') {";
        page += "        statusElem.className = 'status allowed';";
        page += "      } else if (data.lastAccessStatus == 'Acesso Negado') {";
        page += "        statusElem.className = 'status denied';";
        page += "      } else {";
        page += "        statusElem.className = 'status';"; // Estado neutro
        page += "      }";
        page += "    }";
        page += "  };";
        page += "  xhr.open('GET', '/status', true);";
        page += "  xhr.send();";
        page += "}";

        // Atualiza a cada 2 segundos
        page += "setInterval(updateStatus, 1000);"; // Atualiza a cada 2 segundos
        page += "</script>";

        page += "</body></html>";
        
        server.send(200, "text/html", page); // Envia o conteúdo da página ao cliente
    });


    server.begin(); // Inicia o servidor
    Serial.println("Servidor web iniciado. Acesse o IP do ESP32 no navegador.");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP()); // Exibe o IP do ESP32 no monitor serial
}

void loop()
{
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        server.handleClient(); // Processa as requisições do servidor
        return;
    }

    String conteudo = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    conteudo.toUpperCase();
    conteudo.trim();
    Serial.println("ID da tag: " + conteudo);
    lastTagRead = conteudo; // Atualiza a última tag lida

    if (checkAccess(conteudo))
    {
        lastAccessStatus = "Acesso Liberado";
        digitalWrite(GREEN_LED, HIGH);
        lcd.clear();
        lcd.print("Acesso Liberado");
        digitalWrite(BRAID, HIGH);

        ledcWrite(CANAL_PWM, 500);
        delay(800);
        ledcWrite(CANAL_PWM, 0);
        delay(200);

        for (byte s = 5; s > 0; s--)
        {
            lcd.setCursor(8, 1);
            lcd.print(s);
            delay(1000);
        }

        digitalWrite(BRAID, LOW);
        digitalWrite(GREEN_LED, LOW);
        lcd.clear();
    }
    else
    {
        lastAccessStatus = "Acesso Negado";
        ledcWrite(CANAL_PWM, 500);
        delay(800);
        ledcWrite(CANAL_PWM, 0);
        delay(200);

        digitalWrite(RED_LED, HIGH);
        lcd.clear();
        lcd.home();
        lcd.print("Acesso negado");
        delay(1000);
        digitalWrite(RED_LED, LOW);
        lcd.clear();
    }

    server.handleClient(); // Processa as requisições do servidor
}

// Função para verificar o acesso através da API
bool checkAccess(String idTag)
{
    Serial.println("Acessando a API");

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

            saveLocalFile(payload, "/credenciais.txt");

            lastApiCallTime = millis(); // Atualiza o tempo da última chamada à API
            Serial.println(lastApiCallTime);

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

bool checkLocalFile(String idTag) {
    Serial.println("Acessando arquivos locais");

    File arquivo = SPIFFS.open("/credenciais.txt", "r");
    if (!arquivo) {
        Serial.println("Não foi possível abrir o arquivo");
        return false;
    }

    while (arquivo.available()) {
        String linha = arquivo.readStringUntil('\n');
        if (linha.indexOf(idTag) >= 0) {
            Serial.println("Acesso permitido - credencial local");
            arquivo.close();
            return true;
        }
    }

    arquivo.close();
    return false;
}

void saveLocalFile(String dados, String fileName)
{
    Serial.println("Salvando as dados locais");

    File arquivo = SPIFFS.open(fileName, FILE_WRITE);
    if (!arquivo)
    {
        Serial.println("Não foi possível abrir o arquivo para escrita");
        return;
    }

    if (arquivo.println(dados)) {
        Serial.println("dados salvas localmente" + fileName);
    } else {
        Serial.println("Erro ao salvar dados localmente" + fileName);
    }
    arquivo.close();
    Serial.println("Dados salvas localmente");
}