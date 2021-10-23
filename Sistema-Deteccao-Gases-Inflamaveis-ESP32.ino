#include <WiFi.h>
#include <FirebaseESP32.h>
#include "time.h"

#define WIFI_SSID "SSID-WiFi"
#define WIFI_PASSWORD "Senha-WiFi"

#define FIREBASE_HOST "https://monitorco2-c08d0-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "DNUgNTwc0QUZ6Orwp5p9VgrsFAsnk7zVNjsOPPVK"

#define LED_VERDE 16
#define LED_VERMELHO 17
#define SENSOR_GAS_MQ2 34

#define CAMINHO_FIREBASE_RTDB "/sensores/gas_inflamavel/"
FirebaseData firebaseData;
FirebaseJson json;

int valorSensorGas = -1;
bool gasDetectado = false;
bool ultimoGasDetectado = false;
bool primeiraInteracao = true;

/* Configurações do servidor NTP */
const char* ntpServer = "south-america.pool.ntp.org";
const long  gmtOffset_sec = 3 * -(3600);
const int   daylightOffset_sec = 0;
char dataHora[64];

void conectaWiFi(){
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Conectando-se ao Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Conectado com o IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void atualizaDataHora()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Falha ao obter informações do tempo");
    return;
  }
  
  strftime(dataHora, 64,"%d-%m-%Y %H:%M:%S", &timeinfo);
}

void alteraStatusRTDB(){
  atualizaDataHora();
  String identificador = dataHora;
  String caminho = CAMINHO_FIREBASE_RTDB + identificador;
  json.set("detectado", gasDetectado);
  Firebase.set(firebaseData, caminho, json);
  
  if (gasDetectado) Serial.println("Gás inflamável detectado"); else Serial.println("Não há presença de gás inflamável");
  
  Serial.print("Registro Firebase RTDB: ");
  Serial.println(caminho);
}

void acendeLEDVermelho (){
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
}

void acendeLEDVerde(){
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);
}

void detectaGasInflamavel(){
  valorSensorGas = analogRead(SENSOR_GAS_MQ2);
  
  if (valorSensorGas > 1400){
    gasDetectado = true;
    acendeLEDVermelho();
  } else {
    gasDetectado = false;
    acendeLEDVerde();
  }

  /* Verificação para reduzir o volume de dados enviado ao Firebase RTDB */
  if (primeiraInteracao || gasDetectado != ultimoGasDetectado){
    ultimoGasDetectado = gasDetectado;
    primeiraInteracao = false;
    alteraStatusRTDB();
  }
}

void setup()
{
  Serial.begin(9600);
  
  conectaWiFi();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  pinMode(SENSOR_GAS_MQ2, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
 
void loop()
{
  delay (1000);
  detectaGasInflamavel();
}
