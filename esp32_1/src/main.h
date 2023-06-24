#ifndef MAIN_H
#define MAIN_H

/*
///////////////// DEFINICION DE VARIABLES \\\\\\\\\\\\\\\\\
*/
// configura la conexión WiFi //
extern const char* ssid;
extern const char* password;
extern const char* ip;
extern const char* gateway;
extern const char* subnet;

// configura la conexión MQTT //
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_topic;

// configura los pines del sensor DHT11 //
extern const int DHTTYPE;
extern const int DHTPIN;

// configura los pines del sensor LDR //
extern const int pinLDR;
extern const int photoResistence;

// configura los pines del LED RGB //
extern const int pinRojo;
extern const int pinAzul;
extern const int pinVerde;

// configura el tiempo para medir la calidad de la señal en dB
extern unsigned long tiempoAnterior2;
extern const unsigned long intervalo2;

// configura el sensor de humedad del suelo
extern const int pinHumedadSuelo;

#endif /* MAIN_H */
