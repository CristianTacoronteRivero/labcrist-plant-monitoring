/*
///////////////// IMPORTACION DE MODULOS \\\\\\\\\\\\\\\\\
*/
#include <ArduinoJson.h>
#include "main.h"
#include "wifi/wifi.h"
#include <OneWire.h>
#include <DallasTemperature.h>

/*
///////////////// ASIGNACION DE VALORES \\\\\\\\\\\\\\\\\
*/
// Variable para almacenar el tiempo anterior
unsigned long tiempoAnterior2 = 0;
// Intervalo de tiempo deseado para "Intensidad de señal"
// Cada 10s se monitoriza la intensidad de la señal
const unsigned long intervalo2 = 10000;

// Pin analógico conectado al sensor de humedad del suelo (ejemplo: GPIO32)
const int analogPin = 33;
const int HumedityMin = 998;
const int Humeditymax = 2870;

// Pin de datos de la sonda DS18B20
#define ONE_WIRE_BUS 32

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/*
///////////////// DECLARACION DE FUNCIONES \\\\\\\\\\\\\\\\\
*/
void setup() {
  // configura el serial monitor
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
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

  if (millis() - tiempoAnterior2 >= intervalo2) {
    tiempoAnterior2 = millis();

    int rssi = WiFi.RSSI();
    Serial.print("Intensidad de señal: ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }

  // Retardo al final del loop para reducir el consumo de energía
  delay(1000);
}