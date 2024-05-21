#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/*
 * Definição das credenciais da rede Wi-Fi
 */
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

/*
 * Definição do endpoint da API para alertas de movimento
 */
const char* apiEndpoint = "http://yourapi.com/alert";

/*
 * Pino do sensor PIR
 *
 * Obs: Digital pin D7
 */
const int pirPin = 13;

/*
 * Estado inicial do sensor PIR
 */
int pirState = LOW;
int val = 0;

/*
 * Objeto WiFiUDP para o cliente NTP
 */
WiFiUDP ntpUDP;

/*
 * Configuração do cliente NTP
 *
 * pool.ntp.org é o servidor NTP
 * -3*3600 define o fuso horário (UTC-3 para o Brasil)
 * 60000 define o intervalo de atualização (1 minuto)
 */
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000);

void setup() {
  /*
   * Inicializa a comunicação serial
   */
  Serial.begin(115200);

  /*
   * Configura o pino do sensor PIR como entrada
   */
  pinMode(pirPin, INPUT);

  /*
   * Conecta ao Wi-Fi
   */
  connectWifi();

  /*
   * Inicializa o cliente NTP
   */
  timeClient.begin();
}

void loop() {
  /*
   * Atualiza o cliente NTP
   */
  timeClient.update();

  /*
   * Lê o estado do sensor PIR
   */
  val = digitalRead(pirPin);

  /*
   * Verifica se o sensor detectou movimento
   */
  if (val == HIGH && pirState == LOW) {
    Serial.println("Movimento detectado!");
    sendMovementAlert();
    pirState = HIGH;
  } else if (val == LOW && pirState == HIGH) {
    pirState = LOW;
  }
}

/*
 * Conecta ao Wi-Fi
 */
void connectWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

/*
 * Função para enviar alerta de movimento para a API
 */
void sendMovementAlert() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    /*
     * Inicia a conexão com o endpoint da API
     */
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");

    /*
     * Obtém a hora e a data atuais
     */
    time_t rawTime = timeClient.getEpochTime();
    struct tm* timeInfo = localtime(&rawTime);
    char dateTimeBuffer[30];
    strftime(dateTimeBuffer, sizeof(dateTimeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);

    /*
     * Define o payload da requisição (JSON)
     */
    String jsonPayload = "{\"movement\": \"true\", \"datetime\": \"" + String(dateTimeBuffer) + "\"}";

    /*
     * Envia a requisição POST e obtém o código de resposta
     */
    int httpResponseCode = http.POST(jsonPayload);

    /*
     * Verifica o código de resposta da requisição
     */
    if (httpResponseCode > 0) {
      Serial.print("Resposta da API: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Erro ao enviar o POST: ");
      Serial.println(httpResponseCode);
    }

    /*
     * Finaliza a conexão com a API
     */
    http.end();
  } else {
    Serial.println("Erro: Não conectado ao Wi-Fi");
  }
}
