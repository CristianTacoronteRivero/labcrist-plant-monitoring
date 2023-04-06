/*
///////////////// IMPORTACION DE MODULOS \\\\\\\\\\\\\\\\\
*/
#include <Arduino.h>
#include <ArduinoJson.h>
// #include <ESP8266WiFi.h> // permite la conexion via WiFi
#include <PubSubClient.h> // ofrece una interfaz MQTT
#include <Adafruit_Sensor.h> //permite leer sensores a traves de distintas plataformas y librerias
#include <DHT.h> // permite leer datos del sensor DHT11

#include "led/rgb.h"
#include "wifi/wifi.h"

/*
///////////////// DECLARACION DE VARIABLES \\\\\\\\\\\\\\\\\
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
const char* mqtt_topic = "nodemcu_1/dht11";
WiFiClient espClient;
PubSubClient client(espClient);

// configura los pines del sensor DHT11 //
#define DHTTYPE DHT11
#define DHTPIN D4
DHT dht(DHTPIN, DHTTYPE);

// configura los pines del LED RGB //
#define pinRojo D1
#define pinAzul D2
#define pinVerde D3

/*
///////////////// DECLARACION DE FUNCIONES \\\\\\\\\\\\\\\\\
*/
void reconnect() { // funcion que se encarga de reconectarse de nuevo...
  // loop hasta que nos podamos conectar al servidor MQTT
  while (!client.connected()) { // si no se encuentra conectado ...PubSubClient client(espClient);
    Serial.print("Intentando reconectar...");
    // intenta conectar al servidor MQTT
    if (client.connect("nodemcu-client")) {
      // nos hemos conectado al servidor MQTT
    } else {
      // error al conectar, esperamos y lo intentamos de nuevo
      delay(5000);
    }
  }
}

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
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {

  // comprueba si esta conectado o no
  if (!client.connected()) {
    // si no lo esta, reconecta
    turnOffLED(pinRojo, pinVerde, pinAzul);
    commandLED(1023, 0, 0, pinRojo, pinVerde, pinAzul);
    reconnect();
  }
  // una vez comprobada la conexion, se ejecuta esta funcion que se encarga de matentener
  // la conexion establecida y procesar los mensajes
  turnOffLED(pinRojo, pinVerde, pinAzul);
  commandLED(0, 20, 0, pinRojo, pinVerde, pinAzul);
  client.loop();

  turnOffLED(pinRojo, pinVerde, pinAzul);
  commandLED(1023, 0, 1023, pinRojo, pinVerde, pinAzul);
  digitalWrite(LED_BUILTIN, LOW);
  float temperature = dht.readTemperature(); // lee la temperatura del sensor
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(LED_BUILTIN, LOW);
  float humidity = dht.readHumidity(); // lee la humedad del sensor
  digitalWrite(LED_BUILTIN, HIGH);
  commandLED(0, 20, 0, pinRojo, pinVerde, pinAzul);

  // uso el formato json para enviar datos
  StaticJsonDocument<64> doc;
  doc["temperatura"] = temperature;
  doc["humedad"] = humidity;

  String jsonString;
  serializeJson(doc, jsonString);

  // se espera que sea un puntero y no un valor. Un puntero es una direccion
  // de un valor de otra variable almancenada en la RAM. la funcion .publish necesita un puntero, por eso
  // se usa .c_str()
  client.publish(mqtt_topic, (char*)jsonString.c_str());

  delay(3000); // espera 30 segundos
}