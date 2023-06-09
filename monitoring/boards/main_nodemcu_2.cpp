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
const char* ip = "192.168.1.24";
const char* gateway = "192.168.1.1";
const char* subnet = "255.255.255.0";

// configura la conexión MQTT //
const char* mqtt_server = "192.168.1.70";
const int mqtt_port = 1883;
const char* mqtt_topic = "nodemcu_2/dht11";

// configura los pines del sensor DHT11 //
const int DHTTYPE = DHT11;
const int DHTPIN = D4;
DHT dht(DHTPIN, DHTTYPE);

// configura los pines del sensor LDR //
const int pinLDR = A0;
const int photoResistence = 10000; //10k ohm

// configura los pines del LED RGB //
const int pinRojo = D1;
const int pinAzul = D2;
const int pinVerde = D3;

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
  float temperature = dht.readTemperature();

  // lee la humedad del sensor
  float humidity = dht.readHumidity();

  // lee el sensor LDR
  int analog_value = analogRead(pinLDR);
  float voltage = (analog_value * 3.3) / 1023.0;
  float photoresistor = ((3.3 / voltage) - 1) * photoResistence;

  digitalWrite(LED_BUILTIN, HIGH);
  commandLED(0, 20, 0, pinRojo, pinVerde, pinAzul);

  Serial.println("Temperatura: " + String(temperature) + " °C, Humedad: " + String(humidity) + " %, LDR: " + String(photoresistor));

  // usa el formato json para enviar datos
  StaticJsonDocument<64> doc;
  doc["temperatura"] = temperature;
  doc["humedad"] = humidity;
  doc["LDR"] = photoresistor;

  String jsonString;
  serializeJson(doc, jsonString);

  // publica los datos mediante protocolo MQTT
  client.publish(mqtt_topic, (char*)jsonString.c_str());

  // espera 30 segundos
  delay(30000);
}