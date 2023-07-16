/*
///////////////// IMPORTACION DE MODULOS \\\\\\\\\\\\\\\\\
*/
#include <ArduinoJson.h>
#include <PubSubClient.h> // ofrece una interfaz MQTT
#include <Adafruit_Sensor.h> //permite leer sensores a traves de distintas plataformas y librerias
#include <DHT.h> // permite leer datos del sensor DHT11

#include "main.h"
#include "led/rgb.h"
#include "wifi/wifi.h"
#include "mqtt/mqtt.h"

/*
///////////////// ASIGNACION DE VALORES \\\\\\\\\\\\\\\\\
*/
// configura la conexión WiFi //
const char* ssid = "LabCristjz";
const char* password = "CasaArribaCrist";
const char* ip = "192.168.1.23";
const char* gateway = "192.168.1.1";
const char* subnet = "255.255.255.0";

// configura la conexión MQTT //
const char* mqtt_server = "192.168.1.70";
const int mqtt_port = 1883;
const char* mqtt_topic_params = "nodemcu_1/params";
const char* mqtt_topic_coverage = "nodemcu_1/coverage";

// Variable para almacenar el tiempo anterior
unsigned long tiempoAnterior = 0;
// Intervalo de tiempo deseado para "Intensidad de señal"
// Cada 10s se monitoriza la intensidad de la señal
const unsigned long intervalo = 60000;

// configura los pines del sensor DHT11 //
const int DHTTYPE = DHT11;
const int DHTPIN = D4;
DHT dht(DHTPIN, DHTTYPE);

// Pin analógico conectado al sensor de humedad del suelo (ejemplo: GPIO32)
const int analogPin = A0;
// Calibracion del sensor capacitivo: Aire-Agua
const int HumedityMin = 257;
const int Humeditymax = 646;

// configura los pines del LED RGB //
const int pinRojo = D7;
const int pinVerde = D6;
const int pinAzul = D5;

/*
///////////////// DECLARACION DE FUNCIONES \\\\\\\\\\\\\\\\\
*/
void setup() {
  // configura el serial monitor
  Serial.begin(9600);

  // configura el LED RGB para que se pueda escribir
  pinMode(pinRojo, OUTPUT);
  pinMode(pinAzul, OUTPUT);
  pinMode(pinVerde, OUTPUT);

  // apaga desde un inicio el LED RGB
  turnOffLED(pinRojo, pinVerde, pinAzul);

  // configura el LED del nodemcu para que se pueda escribir
  pinMode(LED_BUILTIN, OUTPUT);

  // inicia el sensor DHT11
  dht.begin();

  // conecta a la red wifi local
  setup_wifi(ssid, password, ip, gateway, subnet);

  // configura el servidor mqtt para enviar datos
  mqtt_init(mqtt_server, mqtt_port);

}

void loop() {

  // comprueba si esta conectado o no
  mqtt_is_connected();

  turnOffLED(pinRojo, pinVerde, pinAzul);
  commandLED(1023, 0, 1023, pinRojo, pinVerde, pinAzul);
  digitalWrite(LED_BUILTIN, LOW);
  // lee la temperatura del sensor
  float temperatureDHT = dht.readTemperature();

  // lee la humedad del sensor
  float humidityDHT = dht.readHumidity();
  digitalWrite(LED_BUILTIN, HIGH);

  // Lectura de la humedad del suelo
  int sensorValue = analogRead(analogPin);
  int humidityCapacitor = 100 - map(sensorValue, HumedityMin, Humeditymax, 0, 100);
  // Conversión a voltaje (rango de 0 a 3.3V)
  float voltage = sensorValue * (3.3 / 1023.0);
  commandLED(0, 20, 0, pinRojo, pinVerde, pinAzul);

  // Serial.print("Temperatura: ");
  // Serial.print(temperatureDHT);
  // Serial.print(" °C, Humedad: ");
  // Serial.print(humidityDHT);
  // Serial.println(" %");

  // Serial.print("% humedad: ");
  // Serial.print(humidityCapacitor);
  // Serial.print("%");
  // Serial.print(", Voltaje: ");
  // Serial.print(voltage, 2);
  // Serial.println("V");

  // usa el formato json para enviar datos
  StaticJsonDocument<64> params;
  params["temperatura_dht"] = temperatureDHT;
  params["humedad_dht"] = humidityDHT;
  params["humedad_capacitor"] = humidityCapacitor;

  String jsonStringParams;
  serializeJson(params, jsonStringParams);

  // publica los datos mediante protocolo MQTT
  client.publish(mqtt_topic_params, (char*)jsonStringParams.c_str());

  if (millis() - tiempoAnterior >= intervalo) {
    tiempoAnterior = millis();

    int rssi = WiFi.RSSI();
    // Serial.print("Intensidad de señal: ");
    // Serial.print(rssi);
    // Serial.println(" dBm");

    StaticJsonDocument<64> coverage;
    coverage["dBm"] = rssi;

    String jsonStringCoverage;
    serializeJson(coverage, jsonStringCoverage);

    // publica los datos mediante protocolo MQTT
    client.publish(mqtt_topic_coverage, (char*)jsonStringCoverage.c_str());
  }

  // espera 30 segundos
  delay(30000);
}