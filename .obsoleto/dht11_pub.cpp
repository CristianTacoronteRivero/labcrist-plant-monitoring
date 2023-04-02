#include <ArduinoJson.h>
#include <ESP8266WiFi.h> // permite la conexion via WiFi
#include <PubSubClient.h> // ofrece una interfaz MQTT
#include <Adafruit_Sensor.h> //permite leer sensores a traves de distintas plataformas y librerias
#include <DHT.h> // permite leer datos del sensor DHT11
//#include <DHT_U.h> // y esta es la del DHT22
#include <ESP8266Ping.h>

// configura los detalles de la conexión WiFi
const char* ssid = "LabCristjz"; // lo ponemos de esta forma y no char X porque es una variable fija
const char* password = "CasaArribaCrist";

// configura los detalles de la conexión MQTT
const char* mqtt_server = "192.168.1.70";
const int mqtt_port = 1883;
const char* mqtt_topic = "nodemcu_1/dht11";

#define DHTPIN D4     // pin de datos del sensor DHT11 conectado al pin D3/GPIO0
#define DHTTYPE DHT11   // tipo de sensor DHT11

// defino los pines del LED RGB
#define pinRojo D1
#define pinAzul D2
#define pinVerde D3

// configura el cliente MQTT, de esta forma se puede enviar datos via MQTT  a traves de la red
WiFiClient espClient; // iniccializa un obejto llamado espClient que se conecta al WifFi
PubSubClient client(espClient); // y esto permite la publicacion de datos MQTT

DHT dht(DHTPIN, DHTTYPE);

void encenderRojo(int intensidad) {
  analogWrite(pinRojo, intensidad);
}

void encenderAzul(int intensidad) {
  analogWrite(pinAzul, intensidad);
}

void encenderVerde(int intensidad) {
  analogWrite(pinVerde, intensidad);
}

void apagar() {
  analogWrite(pinRojo, 0);
  analogWrite(pinAzul, 0);
  analogWrite(pinVerde, 0);
}

void setup_wifi() {
  delay(10); // pequena demora para que el node se inicie

  // configuro IP fija para este nodecmu
  IPAddress ip(192, 168, 1, 23);  // Dirección IP deseada
  IPAddress gateway(192, 168, 1, 1);  // Dirección IP del router
  IPAddress subnet(255, 255, 255, 0);  // Máscara de subred
  WiFi.config(ip, gateway, subnet);

  WiFi.begin(ssid, password); // conecta a la red WiFi

  // WL_CONNECTED variable de la libreria ESP8266WiFi.h que devuelve el estado de la red
  while (WiFi.status() != WL_CONNECTED) { // si no se ha conectado...
    Serial.print("Esperando conexion...");
    delay(500);
  }
}

void reconnect() { // funcion que se encarga de reconectarse de nuevo...
  // loop hasta que nos podamos conectar al servidor MQTT
  while (!client.connected()) { // si no se encuentra conectado ...PubSubClient client(espClient);
    Serial.print("Intentando reconectar...");
    // intenta conectar al servidor MQTT
    if (client.connect("nodemcu-client")) {
      Serial.print("Reconexion exitosa...");
      // nos hemos conectado al servidor MQTT
    } else {
      // error al conectar, esperamos y lo intentamos de nuevo
      delay(5000);
    }
  }
}

int checkConnection(String ip) {
  int result = 0; // variable donde se guardará el resultado del ping

  IPAddress address;
  if (!address.fromString(ip)) {
    Serial.println("Invalid IP address.");
    result = -1;
  }
  if (Ping.ping(address)) {
    Serial.println("Connection successful.");
    result = 1;
  } else {
    Serial.println("Connection failed.");
    result = 0;
  }
  return result;
}

void setup() {
  Serial.begin(9600); // baudios en 9600 para que funcione correctamente el Serial Monitor

  pinMode(LED_BUILTIN, OUTPUT); // configuracion del LED del nodemcu

  pinMode(pinRojo, OUTPUT); // configuracion del LED RGB
  pinMode(pinAzul, OUTPUT);
  pinMode(pinVerde, OUTPUT);

  apagar();

  dht.begin(); // inicia el sensor DHT11

  setup_wifi(); // conecta a la red WiFi
  // aqui se configura la direccion IP del servidor MQTT y su puerto
  // y de esta forma se sabe a donde se tienen que enviar los datos
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {

  // comprueba si esta conectado o no
  if (!client.connected()) {
    // si no lo esta, reconecta
    apagar();
    encenderRojo(1023);
    reconnect();
  }
  // una vez comprobada la conexion, se ejecuta esta funcion que se encarga de matentener
  // la conexion establecida y procesar los mensajes
  encenderRojo(0);
  encenderVerde(10);
  client.loop();

  encenderAzul(1023);
  digitalWrite(LED_BUILTIN, LOW);
  float temperature = dht.readTemperature(); // lee la temperatura del sensor
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(LED_BUILTIN, LOW);
  float humidity = dht.readHumidity(); // lee la humedad del sensor
  digitalWrite(LED_BUILTIN, HIGH);
  encenderAzul(0);

  // uso el formato json para enviar datos
  StaticJsonDocument<64> doc;
  doc["temperatura"] = temperature;
  doc["humedad"] = humidity;
  // doc["conexion_broker"] = checkConnection(mqtt_server);

  String jsonString;
  serializeJson(doc, jsonString);

  // se espera que sea un puntero y no un valor. Un puntero es una direccion
  // de un valor de otra variable almancenada en la RAM. la funcion .publish necesita un puntero, por eso
  // se usa .c_str()
  client.publish(mqtt_topic, (char*)jsonString.c_str());

  delay(3000); // espera 30 segundos
}