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
const int pinHumedadSuelo = A0;

/*
///////////////// DECLARACION DE FUNCIONES \\\\\\\\\\\\\\\\\
*/
void setup() {
  // configura el serial monitor
  Serial.begin(9600);

}

void loop() {
  // Lectura de la humedad del suelo
  int humedad = analogRead(pinHumedadSuelo);

  // Mapea el valor leído a un rango de humedad
  float humedadPorcentaje = 100 - map(humedad, 0, 1023, 0, 100);

  // Imprime la lectura de la humedad del suelo en el monitor serial
  Serial.print("Nivel de humedad del suelo: ");
  Serial.print(humedadPorcentaje);
  Serial.println("%");

  // espera 1 segundos
  delay(1000);
}