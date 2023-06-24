/*
///////////////// IMPORTACION DE MODULOS \\\\\\\\\\\\\\\\\
*/
#include <ArduinoJson.h>
#include "main.h"
#include "wifi/wifi.h"

/*
///////////////// ASIGNACION DE VALORES \\\\\\\\\\\\\\\\\
*/
// configura la conexión WiFi //
const char* ssid = "LabCristjz";
const char* password = "CasaArribaCrist";
const char* ip = "192.168.1.24";
const char* gateway = "192.168.1.1";
const char* subnet = "255.255.255.0";

// Variable para almacenar el tiempo anterior
unsigned long tiempoAnterior2 = 0;
// Intervalo de tiempo deseado para "Intensidad de señal"
const unsigned long intervalo2 = 10000;

// Define el pin analógico al que está conectado el sensor de humedad
// const int pinHumedadSuelo = A0;

/*
///////////////// DECLARACION DE FUNCIONES \\\\\\\\\\\\\\\\\
*/
void setup() {
  // configura el serial monitor
  Serial.begin(9600);

  // configura el LED del nodemcu para que se pueda escribir
  // pinMode(LED_BUILTIN, OUTPUT);

  // conecta a la red wifi local
  setup_wifi(ssid, password, ip, gateway, subnet);
}

void loop() {
  // Lectura de la humedad del suelo
  // int humedad = analogRead(pinHumedadSuelo);

  // Mapea el valor leído a un rango de humedad
  // float humedadPorcentaje = map(humedad, 0, 1023, 0, 100);

  // Imprime la lectura de la humedad del suelo en el monitor serial
  Serial.print("Nivel de humedad del suelo: ");
  // Serial.print(humedadPorcentaje);
  Serial.print("NaN");
  Serial.println("%");

  if (millis() - tiempoAnterior2 >= intervalo2) {
    tiempoAnterior2 = millis();

    int rssi = WiFi.RSSI();
    Serial.print("Intensidad de señal: ");
    Serial.print(rssi);
    Serial.println(" dBm");
  }

  delay(1000); // Retardo al final del loop para reducir el consumo de energía
}