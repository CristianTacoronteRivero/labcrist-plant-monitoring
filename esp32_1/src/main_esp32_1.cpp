/*
///////////////// IMPORTACION DE MODULOS \\\\\\\\\\\\\\\\\
*/
#include <ArduinoJson.h>
// Ofrecerinterfaz MQTT
#include <PubSubClient.h>
// Leer sensores a traves de distintas plataformas y librerias
#include <Adafruit_Sensor.h>
// Leer datos del sensor DHT11
#include <DHT.h>
// Leer sonda temperatura DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

#include "main.h"
#include "wifi/wifi.h"
#include "wifi/wifi.h"
#include "mqtt/mqtt.h"

/*
///////////////// ASIGNACION DE VALORES \\\\\\\\\\\\\\\\\
*/
// Configurar conexión WiFi //
const char* ssid = "LabCristjz";
const char* password = "CasaArribaCrist";
const char* ip = "192.168.1.26";
const char* gateway = "192.168.1.1";
const char* subnet = "255.255.255.0";

// Configurar la conexión MQTT //
const char* mqtt_server = "192.168.1.70";
const int mqtt_port = 1883;
const char* mqtt_topic_params = "esp32_1/params";
const char* mqtt_topic_coverage = "esp32_1/coverage";

// Variable para almacenar el tiempo anterior
unsigned long tiempoAnterior2 = 0;
// Intervalo de tiempo deseado para "Intensidad de señal"
// Cada 10s se monitoriza la intensidad de la señal
const unsigned long intervalo2 = 10000;

// Pin analógico conectado al sensor de humedad del suelo (ejemplo: GPIO32)
const int analogPin = 33;
// Calibracion del sensor capacitivo: Aire-Agua
const int HumedityMin = 998;
const int Humeditymax = 2870;

// Pin de datos de la sonda DS18B20
#define ONE_WIRE_BUS 32
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// configura los pines del LED RGB //
const int pinRojo = 23;
const int pinAzul = 22;
const int pinVerde = 21;

/*
///////////////// DECLARACION DE FUNCIONES \\\\\\\\\\\\\\\\\
*/
void setup() {
  // Configurar serial monitor
  Serial.begin(9600);

  // Iniciar servicio sonda de temperatura
  sensors.begin();

  // Conectar a la red wifi local
  setup_wifi(ssid, password, ip, gateway, subnet);

  // Condifurar servidor mqtt para enviar datos
  mqtt_init(mqtt_server, mqtt_port);
}

void loop() {

  // comprueba si esta conectado o no
  mqtt_is_connected();

  // Lectura de la humedad del suelo
  int sensorValue = analogRead(analogPin);
  int levelHumedity = 100 - map(sensorValue, HumedityMin, Humeditymax, 0, 100);
  // Conversión a voltaje (rango de 0 a 3.3V)
  float voltage = sensorValue * (3.3 / 4095.0);

  // Lectura de la temperatura del suelo
  sensors.requestTemperatures();
  float temperatureCelsius = sensors.getTempCByIndex(0);
  // Convierte el valor analógico en tensión (para ESP32, el rango es de 0 a 4095)

  Serial.print("Temperatura: ");
  Serial.print(temperatureCelsius);
  Serial.println(" °C");

  Serial.print("Sensor value: ");
  Serial.print(levelHumedity);
  Serial.print("%");
  Serial.print(", Voltage: ");
  Serial.print(voltage, 2);
  Serial.println("V");

  // usa el formato json para enviar datos
  StaticJsonDocument<64> params;
  params["temperatura"] = temperatureCelsius;
  params["humedad"] = levelHumedity;

  String jsonStringParams;
  serializeJson(params, jsonStringParams);

   // publica los datos mediante protocolo MQTT
  client.publish(mqtt_topic_params, (char*)jsonStringParams.c_str());


  if (millis() - tiempoAnterior2 >= intervalo2) {
    tiempoAnterior2 = millis();

    int rssi = WiFi.RSSI();
    Serial.print("Intensidad de señal: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    StaticJsonDocument<64> coverage;
    coverage["dBm"] = rssi;

    String jsonStringCoverage;
    serializeJson(coverage, jsonStringCoverage);

    // publica los datos mediante protocolo MQTT
    client.publish(mqtt_topic_coverage, (char*)jsonStringCoverage.c_str());
  }

  // Retardo al final del loop para reducir el consumo de energía
  delay(1000);
}